//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementPropertyReadFile.h"
#include "MooseRandom.h"
#include "MooseMesh.h"

#include <fstream>

registerMooseObject("TensorMechanicsApp", ElementPropertyReadFile);

InputParameters
ElementPropertyReadFile::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription("User Object to read property data from an external file and assign "
                             "to elements.");
  params.addParam<FileName>("prop_file_name", "", "Name of the property file name");
  params.addRequiredParam<unsigned int>("nprop", "Number of tabulated property values");
  params.addParam<unsigned int>("ngrain", 0, "Number of grains");
  params.addParam<unsigned int>("nblock", 0, "Number of blocks");
  params.addParam<MooseEnum>("read_type",
                             MooseEnum("element grain block"),
                             "Type of property distribution: element:element by element property "
                             "grain:voronoi grain structure "
                             "block:by mesh block");
  params.addParam<unsigned int>("rand_seed", 2000, "random seed");
  params.addParam<MooseEnum>(
      "rve_type",
      MooseEnum("periodic none", "none"),
      "Periodic or non-periodic grain distribution: Default is non-periodic");
  params.addParam<bool>("blocks_zero_numbered", true, "Are the blocks numbered starting at zero?");
  return params;
}

ElementPropertyReadFile::ElementPropertyReadFile(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _prop_file_name(getParam<FileName>("prop_file_name")),
    _reader(_prop_file_name),
    _nprop(getParam<unsigned int>("nprop")),
    _ngrain(getParam<unsigned int>("ngrain")),
    _nblock(getParam<unsigned int>("nblock")),
    _read_type(getParam<MooseEnum>("read_type").getEnum<ReadType>()),
    _rand_seed(getParam<unsigned int>("rand_seed")),
    _rve_type(getParam<MooseEnum>("rve_type")),
    _block_zero(getParam<bool>("blocks_zero_numbered")),
    _mesh(_fe_problem.mesh())
{
  _nelem = _mesh.nElem();

  for (unsigned int i = 0; i < LIBMESH_DIM; i++)
  {
    _bottom_left(i) = _mesh.getMinInDimension(i);
    _top_right(i) = _mesh.getMaxInDimension(i);
    _range(i) = _top_right(i) - _bottom_left(i);
  }

  _max_range = _range(0);
  for (unsigned int i = 1; i < LIBMESH_DIM; i++)
    if (_range(i) > _max_range)
      _max_range = _range(i);

  readData();
}

void
ElementPropertyReadFile::readData()
{
  _reader.setFormatFlag(MooseUtils::DelimitedFileReader::FormatFlag::ROWS);
  _reader.read();

  unsigned int nobjects = 0;

  switch (_read_type)
  {
    case ReadType::ELEMENT:
      nobjects = _nelem;
      break;

    case ReadType::GRAIN:
      if (_ngrain <= 0)
        paramError("ngrain", "Provide non-zero number of grains.");
      nobjects = _ngrain;
      break;

    case ReadType::BLOCK:
      if (_nblock <= 0)
        paramError("nblock", "Provide non-zero number of blocks.");
      nobjects = _nblock;
      break;
  }

  // make sure the data from file has enough rows and columns
  if (_reader.getData().size() < nobjects)
    mooseError(
        "Data in ", _prop_file_name, " does not have enough rows for ", nobjects, " objects.");
  for (unsigned int i = 0; i < nobjects; i++)
    if (_reader.getData(i).size() < _nprop)
      mooseError("Row ", i, " in ", _prop_file_name, " has number of data less than ", _nprop);

  if (_read_type == ReadType::GRAIN)
    initGrainCenterPoints();
}

void
ElementPropertyReadFile::initGrainCenterPoints()
{
  _center.resize(_ngrain);
  MooseRandom::seed(_rand_seed);
  for (unsigned int i = 0; i < _ngrain; i++)
    for (unsigned int j = 0; j < LIBMESH_DIM; j++)
      _center[i](j) = _bottom_left(j) + MooseRandom::rand() * _range(j);
}

Real
ElementPropertyReadFile::getData(const Elem * elem, unsigned int prop_num) const
{
  if (prop_num >= _nprop)
    paramError("nprop",
               "Property number ",
               prop_num,
               " greater than than total number of properties ",
               _nprop);

  Real data = 0.0;
  switch (_read_type)
  {
    case ReadType::ELEMENT:
      data = getElementData(elem, prop_num);
      break;

    case ReadType::GRAIN:
      data = getGrainData(elem, prop_num);
      break;

    case ReadType::BLOCK:
      data = getBlockData(elem, prop_num);
      break;
  }
  return data;
}

Real
ElementPropertyReadFile::getElementData(const Elem * elem, unsigned int prop_num) const
{
  unsigned int jelem = elem->id();
  if (jelem >= _nelem)
    mooseError("Element ID", jelem, " greater than than total number of element in mesh ", _nelem);
  return _reader.getData(jelem)[prop_num];
}

Real
ElementPropertyReadFile::getBlockData(const Elem * elem, unsigned int prop_num) const
{
  unsigned int offset = 0;
  if (!_block_zero)
    offset = 1;

  unsigned int elem_subdomain_id = elem->subdomain_id();
  if (elem_subdomain_id >= _nblock + offset)
    paramError("nblock",
               "Element block id ",
               elem_subdomain_id,
               " greater than than total number of blocks in mesh ",
               _nblock);
  return _reader.getData(elem_subdomain_id - offset)[prop_num];
}

Real
ElementPropertyReadFile::getGrainData(const Elem * elem, unsigned int prop_num) const
{
  Point centroid = elem->vertex_average();
  Real min_dist = _max_range;
  unsigned int igrain = 0;

  for (unsigned int i = 0; i < _ngrain; ++i)
  {
    Real dist = 0.0;
    switch (_rve_type)
    {
      case 0:
        // Calculates minimum periodic distance when "periodic" is specified
        // for rve_type
        dist = minPeriodicDistance(_center[i], centroid);
        break;

      default:
        // Calculates minimum distance when nothing is specified
        // for rve_type
        Point dist_vec = _center[i] - centroid;
        dist = dist_vec.norm();
    }

    if (dist < min_dist)
    {
      min_dist = dist;
      igrain = i;
    }
  }
  return _reader.getData(igrain)[prop_num];
}

// TODO: this should probably use the built-in min periodic distance!
Real
ElementPropertyReadFile::minPeriodicDistance(Point c, Point p) const
{
  Point dist_vec = c - p;
  Real min_dist = dist_vec.norm();

  Real fac[3] = {-1.0, 0.0, 1.0};
  for (unsigned int i = 0; i < 3; i++)
    for (unsigned int j = 0; j < 3; j++)
      for (unsigned int k = 0; k < 3; k++)
      {
        Point p1;
        p1(0) = p(0) + fac[i] * _range(0);
        p1(1) = p(1) + fac[j] * _range(1);
        p1(2) = p(2) + fac[k] * _range(2);

        dist_vec = c - p1;
        Real dist = dist_vec.norm();

        if (dist < min_dist)
          min_dist = dist;
      }

  return min_dist;
}
