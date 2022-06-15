//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PropertyReadFile.h"
#include "MooseRandom.h"
#include "MooseMesh.h"

#include <fstream>

registerMooseObject("MooseApp", PropertyReadFile);
registerMooseObjectRenamed("MooseApp",
                           ElementPropertyReadFile,
                           "06/30/2021 24:00",
                           PropertyReadFile);

InputParameters
PropertyReadFile::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription("User Object to read property data from an external file and assign "
                             "to elements.");
  params.addRequiredParam<FileName>("prop_file_name", "Name of the property file name");
  params.addRequiredParam<unsigned int>("nprop", "Number of tabulated property values");
  params.addParam<unsigned int>(
      "nvoronoi", 0, "Number of voronoi tesselations/grains/nearest neighbor regions");
  params.addDeprecatedParam<unsigned int>(
      "ngrain", "Number of grains.", "ngrain is deprecated, use nvoronoi instead");
  params.addParam<unsigned int>("nblock", 0, "Number of blocks");
  params.addRequiredParam<MooseEnum>("read_type",
                                     MooseEnum("element voronoi block node grain"),
                                     "Type of property distribution: "
                                     "element:by element "
                                     "node: by node "
                                     "voronoi:nearest neighbor / voronoi grain structure "
                                     "block:by mesh block "
                                     "grain: deprecated, use voronoi");
  params.addParam<bool>("use_random_voronoi",
                        false,
                        "Wether to generate random positions for the Voronoi tesselation");
  params.addParam<unsigned int>("rand_seed", 2000, "random seed");
  params.addParam<MooseEnum>(
      "rve_type",
      MooseEnum("periodic none", "none"),
      "Periodic or non-periodic grain distribution: Default is non-periodic");
  params.addParam<bool>(
      "use_zero_based_block_indexing", true, "Are the blocks numbered starting at zero?");
  return params;
}

PropertyReadFile::PropertyReadFile(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _prop_file_name(getParam<FileName>("prop_file_name")),
    _reader(_prop_file_name),

    _read_type(getParam<MooseEnum>("read_type").getEnum<ReadTypeEnum>()),
    _use_random_tesselation(getParam<bool>("use_random_voronoi")),
    _rand_seed(getParam<unsigned int>("rand_seed")),
    _rve_type(getParam<MooseEnum>("rve_type")),
    _block_zero(getParam<bool>("use_zero_based_block_indexing")),
    _ngrain(isParamValid("ngrain") ? getParam<unsigned int>("ngrain")
                                   : getParam<unsigned int>("nvoronoi")),
    _mesh(_fe_problem.mesh()),
    _nprop(getParam<unsigned int>("nprop")),
    _nvoronoi(isParamValid("ngrain") ? getParam<unsigned int>("ngrain")
                                     : getParam<unsigned int>("nvoronoi")),
    _nblock(getParam<unsigned int>("nblock"))
{
  if (!_use_random_tesselation && parameters.isParamSetByUser("rand_seed"))
    paramError("rand_seed",
               "Random seeds should only be provided if random tesselation is desired");

  Point mesh_min, mesh_max;
  for (unsigned int i : make_range(Moose::dim))
  {
    mesh_min(i) = _mesh.getMinInDimension(i);
    mesh_max(i) = _mesh.getMaxInDimension(i);
  }
  _bounding_box = MooseUtils::buildBoundingBox(mesh_min, mesh_max);

  readData();
}

void
PropertyReadFile::readData()
{
  if (_read_type == ReadTypeEnum::ELEMENT && _mesh.getMesh().allow_renumbering())
    mooseWarning("CSV data is sorted by element, but mesh element renumbering is on, be careful!");

  _reader.setFormatFlag(MooseUtils::DelimitedFileReader::FormatFlag::ROWS);
  _reader.read();

  unsigned int nobjects = 0;

  switch (_read_type)
  {
    case ReadTypeEnum::ELEMENT:
      nobjects = _mesh.nElem();
      break;

    case ReadTypeEnum::NODE:
      nobjects = _mesh.nNodes();
      break;

    case ReadTypeEnum::VORONOI:
      if (_nvoronoi <= 0)
        paramError("nvoronoi", "Provide non-zero number of voronoi tesselations/grains.");
      nobjects = _nvoronoi;
      break;

    // TODO Delete after Grizzly update see idaholab/Grizzly#182
    case ReadTypeEnum::GRAIN:
      if (_nvoronoi <= 0)
        paramError("ngrain", "Provide non-zero number of voronoi tesselations/grains.");
      nobjects = _nvoronoi;
      break;

    case ReadTypeEnum::BLOCK:
      if (_nblock <= 0)
        paramError("nblock", "Provide non-zero number of blocks.");
      nobjects = _nblock;
      break;
  }

  // make sure the data from file has enough rows and columns
  if (_reader.getData().size() < nobjects)
    mooseError(
        "Data in ", _prop_file_name, " does not have enough rows for ", nobjects, " objects.");
  if (_reader.getData().size() > nobjects)
    mooseWarning("Data size in ",
                 _prop_file_name,
                 " is larger than ",
                 nobjects,
                 " objects, some data will not be used.");
  for (unsigned int i = 0; i < nobjects; i++)
    if (_reader.getData(i).size() < _nprop)
      mooseError("Row ", i, " in ", _prop_file_name, " has number of data less than ", _nprop);

  if (_read_type == ReadTypeEnum::VORONOI || _read_type == ReadTypeEnum::GRAIN)
    initVoronoiCenterPoints();
}

void
PropertyReadFile::initVoronoiCenterPoints()
{
  _center.resize(_nvoronoi);

  // Generate a random tesselation
  if (_use_random_tesselation)
  {
    MooseRandom::seed(_rand_seed);
    for (const auto i : make_range(_nvoronoi))
      for (const auto j : make_range(Moose::dim))
        _center[i](j) = _bounding_box.min()(j) +
                        MooseRandom::rand() * (_bounding_box.max() - _bounding_box.min())(j);
  }
  // Read tesselation from file
  else
  {
    for (const auto i : make_range(_nvoronoi))
      for (const auto j : make_range(_mesh.dimension()))
        _center[i](j) = _reader.getData(i)[j];
  }
}

Real
PropertyReadFile::getData(const Elem * const elem, const unsigned int prop_num) const
{
  if (prop_num > _nprop)
    paramError(
        "nprop", "Property number ", prop_num, " greater than total number of properties ", _nprop);

  Real data = 0.0;
  switch (_read_type)
  {
    case ReadTypeEnum::ELEMENT:
      data = getElementData(elem, prop_num);
      break;

    case ReadTypeEnum::VORONOI:
      data = getVoronoiData(elem->vertex_average(), prop_num);
      break;

    // TODO: Delete after Grizzly update
    case ReadTypeEnum::GRAIN:
      data = getVoronoiData(elem->vertex_average(), prop_num);
      break;

    case ReadTypeEnum::BLOCK:
      data = getBlockData(elem, prop_num);
      break;

    case ReadTypeEnum::NODE:
      mooseError(
          "PropertyReadFile has data sorted by node, it should note be retrieved by element");
      break;
  }
  return data;
}

Real
PropertyReadFile::getElementData(const Elem * elem, unsigned int prop_num) const
{
  unsigned int jelem = elem->id();
  if (jelem >= _mesh.nElem())
    mooseError("Element ID ",
               jelem,
               " greater than than total number of element in mesh: ",
               _mesh.nElem(),
               ". Elements should be numbered consecutively.");
  return _reader.getData(jelem)[prop_num];
}

Real
PropertyReadFile::getNodeData(const Node * const node, const unsigned int prop_num) const
{
  unsigned int jnode = node->id();
  if (jnode >= _mesh.nNodes())
    mooseError("Node ID ",
               jnode,
               " greater than than total number of nodes in mesh: ",
               _mesh.nNodes(),
               ". Nodes should be numbered consecutively.");
  return _reader.getData(jnode)[prop_num];
}

Real
PropertyReadFile::getBlockData(const Elem * elem, unsigned int prop_num) const
{
  unsigned int offset = 0;
  if (!_block_zero)
    offset = 1;

  unsigned int elem_subdomain_id = elem->subdomain_id();
  if (elem_subdomain_id >= _nblock + offset)
    paramError("nblock",
               "Element block id ",
               elem_subdomain_id,
               " greater than than total number of blocks in mesh: ",
               _nblock,
               ". Blocks should be numbered consecutively.");
  return _reader.getData(elem_subdomain_id - offset)[prop_num];
}

Real
PropertyReadFile::getVoronoiData(const Point & point, const unsigned int prop_num) const
{
  Real min_dist = std::numeric_limits<Real>::max();
  unsigned int ivoronoi = 0;

  for (const auto i : make_range(_nvoronoi))
  {
    Real dist = 0.0;
    switch (_rve_type)
    {
      case 0:
        // Calculates minimum periodic distance when "periodic" is specified
        // for rve_type
        dist = minPeriodicDistance(_center[i], point);
        break;

      default:
        // Calculates minimum distance when nothing is specified
        // for rve_type
        Point dist_vec = _center[i] - point;
        dist = dist_vec.norm();
    }

    if (dist < min_dist)
    {
      min_dist = dist;
      ivoronoi = i;
    }
  }
  return _reader.getData(ivoronoi)[prop_num];
}

// TODO: this should probably use the built-in min periodic distance!
Real
PropertyReadFile::minPeriodicDistance(const Point & c, const Point & p) const
{
  Point dist_vec = c - p;
  Real min_dist = dist_vec.norm();

  Real fac[3] = {-1.0, 0.0, 1.0};
  for (unsigned int i = 0; i < 3; i++)
    for (unsigned int j = 0; j < 3; j++)
      for (unsigned int k = 0; k < 3; k++)
      {
        Point p1;
        p1(0) = p(0) + fac[i] * (_bounding_box.max() - _bounding_box.min())(0);
        p1(1) = p(1) + fac[j] * (_bounding_box.max() - _bounding_box.min())(1);
        p1(2) = p(2) + fac[k] * (_bounding_box.max() - _bounding_box.min())(2);

        dist_vec = c - p1;
        Real dist = dist_vec.norm();

        if (dist < min_dist)
          min_dist = dist;
      }

  return min_dist;
}
