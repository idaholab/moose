//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SidesetInfoVectorPostprocessor.h"
#include <limits>

#include "libmesh/quadrature.h"

registerMooseObject("MooseApp", SidesetInfoVectorPostprocessor);

InputParameters
SidesetInfoVectorPostprocessor::validParams()
{
  InputParameters params = SideVectorPostprocessor::validParams();
  MultiMooseEnum meta_data_types("centroid=0 min=1 max=2 area=3");
  params.addParam<MultiMooseEnum>(
      "meta_data_types", meta_data_types, "Data types that are obtained and written to file.");
  params.addClassDescription("This VectorPostprocessor collects meta data for provided sidesets.");
  return params;
}

SidesetInfoVectorPostprocessor::SidesetInfoVectorPostprocessor(const InputParameters & parameters)
  : SideVectorPostprocessor(parameters),
    _meta_data_types(getParam<MultiMooseEnum>("meta_data_types")),
    _sideset_ids(declareVector("Boundary IDs"))
{
  // this sets up the _vpp_entry_names vector
  for (unsigned int j = 0; j < _meta_data_types.size(); ++j)
  {
    if (_meta_data_types[j] == "centroid" || _meta_data_types[j] == "min" ||
        _meta_data_types[j] == "max")
    {
      for (unsigned int d = 0; d < _mesh.dimension(); ++d)
      {
        std::stringstream ss;
        ss << _meta_data_types[j] << "_";
        if (d == 0)
          ss << "x";
        else if (d == 1)
          ss << "y";
        else if (d == 2)
          ss << "z";
        _vpp_entry_names.push_back(ss.str());
      }
    }
    else
      _vpp_entry_names.push_back(_meta_data_types[j]);
  }

  // now we can initialize the _meta_data vector
  _meta_data.resize(_vpp_entry_names.size());
  for (unsigned int j = 0; j < _vpp_entry_names.size(); ++j)
    _meta_data[j] = &declareVector(_vpp_entry_names[j]);
}

void
SidesetInfoVectorPostprocessor::initialize()
{
  // Clear existing data
  _sideset_ids.clear();
  for (unsigned int j = 0; j < _vpp_entry_names.size(); ++j)
    _meta_data[j]->clear();

  for (auto & e : boundaryIDs())
    _boundary_data[e] = BoundaryData();

  // resize containers for possibly new number of boundaries
  _sideset_ids.resize(numBoundaryIDs());
  for (unsigned int j = 0; j < _vpp_entry_names.size(); ++j)
    _meta_data[j]->resize(numBoundaryIDs());
}

void
SidesetInfoVectorPostprocessor::execute()
{
  mooseAssert(_boundary_data.find(_current_boundary_id) != _boundary_data.end(),
              "_current_boundary_id not found in _boundary_data.");

  auto & bd = _boundary_data.find(_current_boundary_id)->second;
  bd.area += _current_side_volume;
  bd.centroid += _current_side_elem->vertex_average() * _current_side_volume;

  BoundingBox box = _current_side_elem->loose_bounding_box();
  Point lmin = box.min();
  Point lmax = box.max();

  for (unsigned int j = 0; j < 3; ++j)
  {
    if (lmin(j) < bd.min(j))
      bd.min(j) = lmin(j);

    if (lmax(j) > bd.max(j))
      bd.max(j) = lmax(j);
  }
}

void
SidesetInfoVectorPostprocessor::finalize()
{
  for (auto & e : _boundary_data)
  {
    auto & bd = e.second;
    gatherSum(bd.area);
    for (unsigned int j = 0; j < 3; ++j)
    {
      gatherMin(bd.min(j));
      gatherMax(bd.max(j));
      gatherSum(bd.centroid(j));
    }
  }

  // centroid needs to be divided by area
  for (auto & e : _boundary_data)
    e.second.centroid /= e.second.area;

  // fill vectors
  unsigned int j = 0;
  for (auto & e : _boundary_data)
  {
    // store away the sideset id first
    _sideset_ids[j] = e.first;

    // now work through the _vpp_entry_names vector
    for (unsigned int i = 0; i < _vpp_entry_names.size(); ++i)
      (*_meta_data[i])[j] = dataHelper(e.first, _vpp_entry_names[i]);

    // increment counter
    ++j;
  }
}

void
SidesetInfoVectorPostprocessor::threadJoin(const UserObject & y)
{
  const SidesetInfoVectorPostprocessor & vpp =
      static_cast<const SidesetInfoVectorPostprocessor &>(y);

  for (auto & e : _boundary_data)
  {
    mooseAssert(vpp._boundary_data.find(e.first) != vpp._boundary_data.end(),
                "Boundary not found in threadJoin");
    auto & vpp_bd = vpp._boundary_data.find(e.first)->second;
    auto & bd = e.second;

    bd.area += vpp_bd.area;
    bd.centroid += vpp_bd.centroid;

    for (unsigned int j = 0; j < 3; ++j)
    {
      if (vpp_bd.min(j) < bd.min(j))
        bd.min(j) = vpp_bd.min(j);

      if (vpp_bd.max(j) > bd.max(j))
        bd.max(j) = vpp_bd.max(j);
    }
  }
}

Real
SidesetInfoVectorPostprocessor::dataHelper(BoundaryID bid, std::string mdat_tpe) const
{
  mooseAssert(_boundary_data.find(bid) != _boundary_data.end(),
              "boundary id not found in _boundary_data.");

  auto & bd = _boundary_data.find(bid)->second;

  if (mdat_tpe == "centroid_x")
    return bd.centroid(0);
  else if (mdat_tpe == "centroid_y")
    return bd.centroid(1);
  else if (mdat_tpe == "centroid_z")
    return bd.centroid(2);
  else if (mdat_tpe == "min_x")
    return bd.min(0);
  else if (mdat_tpe == "min_y")
    return bd.min(1);
  else if (mdat_tpe == "min_z")
    return bd.min(2);
  else if (mdat_tpe == "max_x")
    return bd.max(0);
  else if (mdat_tpe == "max_y")
    return bd.max(1);
  else if (mdat_tpe == "max_z")
    return bd.max(2);
  else if (mdat_tpe == "area")
    return bd.area;
  else
    mooseError("meta_data_type not recognized. This should never happen.");
}
