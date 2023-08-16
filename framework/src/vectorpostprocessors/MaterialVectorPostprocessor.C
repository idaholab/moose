//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialVectorPostprocessor.h"
#include "Material.h"
#include "IndirectSort.h"
#include "MooseMesh.h"

#include "libmesh/quadrature.h"

#include <numeric>

registerMooseObject("MooseApp", MaterialVectorPostprocessor);

InputParameters
MaterialVectorPostprocessor::validParams()
{
  InputParameters params = ElementVectorPostprocessor::validParams();
  params.addClassDescription("Records all scalar material properties of a material object on "
                             "elements at the indicated execution points.");
  params.addRequiredParam<MaterialName>("material",
                                        "Material for which all properties will be recorded.");
  params.addParam<std::vector<dof_id_type>>(
      "elem_ids",
      "Subset of element IDs to print data for. If omitted, all elements will be printed.");
  return params;
}

MaterialVectorPostprocessor::MaterialVectorPostprocessor(const InputParameters & parameters)
  : ElementVectorPostprocessor(parameters),
    _elem_ids(declareVector("elem_id")),
    _qp_ids(declareVector("qp_id")),
    _x_coords(declareVector("x")),
    _y_coords(declareVector("y")),
    _z_coords(declareVector("z"))
{
  auto & mat = getMaterialByName(getParam<MaterialName>("material"), true);
  auto & prop_names = mat.getSuppliedItems();
  if (mat.isBoundaryMaterial())
    mooseError(name(), ": boundary materials (i.e. ", mat.name(), ") cannot be used");

  // Get list of elements from user
  if (parameters.isParamValid("elem_ids"))
  {
    const auto & ids = getParam<std::vector<dof_id_type>>("elem_ids");
    _elem_filter = std::set<dof_id_type>(ids.begin(), ids.end());

    // check requested materials are available
    for (const auto & id : ids)
    {
      auto el = _mesh.getMesh().query_elem_ptr(id);

      // We'd better have found the requested element on *some*
      // processor.
      bool found_elem = (el != nullptr);
      this->comm().max(found_elem);

      // We might not have el on this processor in a distributed mesh,
      // but it should be somewhere and it ought to have a material
      // defined for its subdomain
      if (!found_elem || (el && !mat.hasBlocks(el->subdomain_id())))
        mooseError(name(), ": material ", mat.name(), " is not defined on element ", id);
    }
  }

  for (auto & prop : prop_names)
  {
    if (hasMaterialProperty<Real>(prop))
      _prop_refs.push_back(&getMaterialProperty<Real>(prop));
    else if (hasMaterialProperty<unsigned int>(prop))
      _prop_refs.push_back(&getMaterialProperty<unsigned int>(prop));
    else if (hasMaterialProperty<int>(prop))
      _prop_refs.push_back(&getMaterialProperty<int>(prop));
    else
    {
      mooseWarning("property " + prop +
                   " is of unsupported type and skipped by MaterialVectorPostprocessor");
      continue;
    }
    _prop_vecs.push_back(&declareVector(prop));
    _prop_names.push_back(prop);
  }
}

void
MaterialVectorPostprocessor::initialize()
{
  if (!containsCompleteHistory())
  {
    _elem_ids.clear();
    _qp_ids.clear();
    _x_coords.clear();
    _y_coords.clear();
    _z_coords.clear();
    for (auto vec : _prop_vecs)
      vec->clear();
  }
}

void
MaterialVectorPostprocessor::execute()
{
  // skip execution if element not in filter, assuming filter was used
  const auto elem_id = _current_elem->id();
  if (_elem_filter && !_elem_filter->count(elem_id))
    return;

  unsigned int nqp = _qrule->n_points();
  for (unsigned int qp = 0; qp < nqp; qp++)
  {
    _elem_ids.push_back(elem_id);
    _qp_ids.push_back(qp);
    _x_coords.push_back(_q_point[qp](0));
    _y_coords.push_back(_q_point[qp](1));
    _z_coords.push_back(_q_point[qp](2));
  }

  for (unsigned int i = 0; i < _prop_names.size(); i++)
  {
    auto prop_name = _prop_names[i];
    auto prop = _prop_vecs[i];
    std::vector<Real> vals;
    if (hasMaterialProperty<Real>(prop_name))
    {
      auto vals = dynamic_cast<const MaterialProperty<Real> *>(_prop_refs[i]);
      for (unsigned int qp = 0; qp < nqp; qp++)
        prop->push_back((*vals)[qp]);
    }
    else if (hasMaterialProperty<unsigned int>(prop_name))
    {
      auto vals = dynamic_cast<const MaterialProperty<unsigned int> *>(_prop_refs[i]);
      for (unsigned int qp = 0; qp < nqp; qp++)
        prop->push_back((*vals)[qp]);
    }
    else if (hasMaterialProperty<int>(prop_name))
    {
      auto vals = dynamic_cast<const MaterialProperty<int> *>(_prop_refs[i]);
      for (unsigned int qp = 0; qp < nqp; qp++)
        prop->push_back((*vals)[qp]);
    }
  }
}

void
MaterialVectorPostprocessor::finalize()
{
  // collect all processor data
  comm().gather(0, _elem_ids);
  comm().gather(0, _qp_ids);
  comm().gather(0, _x_coords);
  comm().gather(0, _y_coords);
  comm().gather(0, _z_coords);
  for (auto vec : _prop_vecs)
    comm().gather(0, *vec);
  sortVecs();
}

void
MaterialVectorPostprocessor::threadJoin(const UserObject & y)
{
  auto & vpp = static_cast<const MaterialVectorPostprocessor &>(y);
  _elem_ids.insert(_elem_ids.end(), vpp._elem_ids.begin(), vpp._elem_ids.end());
  _qp_ids.insert(_qp_ids.end(), vpp._qp_ids.begin(), vpp._qp_ids.end());
  _x_coords.insert(_x_coords.end(), vpp._x_coords.begin(), vpp._x_coords.end());
  _y_coords.insert(_y_coords.end(), vpp._y_coords.begin(), vpp._y_coords.end());
  _z_coords.insert(_z_coords.end(), vpp._z_coords.begin(), vpp._z_coords.end());

  for (unsigned int i = 0; i < _prop_vecs.size(); i++)
  {
    auto & vec = *_prop_vecs[i];
    auto & othervec = *vpp._prop_vecs[i];
    vec.insert(vec.end(), othervec.begin(), othervec.end());
  }
  sortVecs();
}

void
MaterialVectorPostprocessor::sortVecs()
{
  std::vector<size_t> ind;
  ind.resize(_elem_ids.size());
  std::iota(ind.begin(), ind.end(), 0);
  std::sort(ind.begin(),
            ind.end(),
            [&](size_t a, size_t b) -> bool
            {
              if (_elem_ids[a] == _elem_ids[b])
              {
                return _qp_ids[a] < _qp_ids[b];
              }
              return _elem_ids[a] < _elem_ids[b];
            });

  Moose::applyIndices(_elem_ids, ind);
  Moose::applyIndices(_qp_ids, ind);
  Moose::applyIndices(_x_coords, ind);
  Moose::applyIndices(_y_coords, ind);
  Moose::applyIndices(_z_coords, ind);
  for (auto vec : _prop_vecs)
    Moose::applyIndices(*vec, ind);
}
