//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CartesianMeshGenerator.h"
#include "CastUniquePointer.h"

// libMesh includes
#include "libmesh/mesh_generation.h"
#include "libmesh/unstructured_mesh.h"
#include "libmesh/replicated_mesh.h"
#include "libmesh/point.h"
#include "libmesh/elem.h"
#include "libmesh/node.h"

registerMooseObject("MooseApp", CartesianMeshGenerator);

InputParameters
CartesianMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  MooseEnum dims("1=1 2 3");
  params.addRequiredParam<MooseEnum>("dim", dims, "The dimension of the mesh to be generated");

  params.addRequiredParam<std::vector<Real>>("dx", "Intervals in the X direction");
  params.addParam<std::vector<unsigned int>>(
      "ix", "Number of grids in all intervals in the X direction (default to all one)");
  params.addParam<std::vector<Real>>(
      "dy", "Intervals in the Y direction (required when dim>1 otherwise ignored)");
  params.addParam<std::vector<unsigned int>>(
      "iy", "Number of grids in all intervals in the Y direction (default to all one)");
  params.addParam<std::vector<Real>>(
      "dz", "Intervals in the Z direction (required when dim>2 otherwise ignored)");
  params.addParam<std::vector<unsigned int>>(
      "iz", "Number of grids in all intervals in the Z direction (default to all one)");
  params.addParam<std::vector<unsigned int>>("subdomain_id", "Block IDs (default to all zero)");
  params.addClassDescription("This CartesianMeshGenerator creates a non-uniform Cartesian mesh.");
  return params;
}

CartesianMeshGenerator::CartesianMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _dim(getParam<MooseEnum>("dim")),
    _dx(getParam<std::vector<Real>>("dx"))
{
  // get all other parameters if provided and check their sizes
  if (isParamValid("ix"))
  {
    _ix = getParam<std::vector<unsigned int>>("ix");
    if (_ix.size() != _dx.size())
      mooseError("ix must be in the same size of dx");
    for (unsigned int i = 0; i < _ix.size(); ++i)
      if (_ix[i] == 0)
        mooseError("ix cannot be zero");
  }
  else
    _ix = std::vector<unsigned int>(_dx.size(), 1);

  for (unsigned int i = 0; i < _dx.size(); ++i)
    if (_dx[i] <= 0)
      mooseError("dx must be greater than zero");

  if (isParamValid("dy"))
  {
    _dy = getParam<std::vector<Real>>("dy");
    for (unsigned int i = 0; i < _dy.size(); ++i)
      if (_dy[i] <= 0)
        mooseError("dy must be greater than zero");
  }

  if (isParamValid("iy"))
  {
    _iy = getParam<std::vector<unsigned int>>("iy");
    if (_iy.size() != _dy.size())
      mooseError("iy must be in the same size of dy");
    for (unsigned int i = 0; i < _iy.size(); ++i)
      if (_iy[i] == 0)
        mooseError("iy cannot be zero");
  }
  else
    _iy = std::vector<unsigned int>(_dy.size(), 1);

  if (isParamValid("dz"))
  {
    _dz = getParam<std::vector<Real>>("dz");
    for (unsigned int i = 0; i < _dz.size(); ++i)
      if (_dz[i] <= 0)
        mooseError("dz must be greater than zero");
  }

  if (isParamValid("iz"))
  {
    _iz = getParam<std::vector<unsigned int>>("iz");
    if (_iz.size() != _dz.size())
      mooseError("iz must be in the same size of dz");
    for (unsigned int i = 0; i < _iz.size(); ++i)
      if (_iz[i] == 0)
        mooseError("iz cannot be zero");
  }
  else
    _iz = std::vector<unsigned int>(_dz.size(), 1);

  if (isParamValid("subdomain_id"))
  {
    _subdomain_id = getParam<std::vector<unsigned int>>("subdomain_id");
    if (isParamValid("dz") && isParamValid("dy"))
    {
      if (_subdomain_id.size() != _dx.size() * _dy.size() * _dz.size())
        mooseError("subdomain_id must be in the size of product of sizes of dx, dy and dz");
    }
    else if (isParamValid("dy"))
    {
      if (_subdomain_id.size() != _dx.size() * _dy.size())
        mooseError("subdomain_id must be in the size of product of sizes of dx and dy");
    }
    else
    {
      if (_subdomain_id.size() != _dx.size())
        mooseError("subdomain_id must be in the size of product of sizes of dx");
    }
  }
  else
  {
    if (isParamValid("dz"))
      _subdomain_id = std::vector<unsigned int>(_dx.size() * _dy.size() * _dz.size(), 0);
    else if (isParamValid("dy"))
      _subdomain_id = std::vector<unsigned int>(_dx.size() * _dy.size(), 0);
    else
      _subdomain_id = std::vector<unsigned int>(_dx.size(), 0);
  }

  // do dimension checks and expand block IDs for all sub-grids
  switch (_dim)
  {
    case 1:
    {
      _nx = 0;
      for (unsigned int i = 0; i < _dx.size(); ++i)
        _nx += _ix[i];
      _ny = 1;
      _nz = 1;

      std::vector<unsigned int> new_id;
      for (unsigned int i = 0; i < _dx.size(); ++i)
        for (unsigned int ii = 0; ii < _ix[i]; ++ii)
          new_id.push_back(_subdomain_id[i]);

      _subdomain_id = new_id;

      if (isParamValid("dy"))
        mooseWarning("dy provided for 1D");
      if (isParamValid("iy"))
        mooseWarning("iy provided for 1D");
      if (isParamValid("dz"))
        mooseWarning("dz provided for 1D");
      if (isParamValid("iz"))
        mooseWarning("iz provided for 1D");
      break;
    }
    case 2:
    {
      _nx = 0;
      for (unsigned int i = 0; i < _dx.size(); ++i)
        _nx += _ix[i];
      _ny = 0;
      for (unsigned int i = 0; i < _dy.size(); ++i)
        _ny += _iy[i];
      _nz = 1;

      std::vector<unsigned int> new_id;
      for (unsigned int j = 0; j < _dy.size(); ++j)
        for (unsigned int jj = 0; jj < _iy[j]; ++jj)
          for (unsigned int i = 0; i < _dx.size(); ++i)
            for (unsigned int ii = 0; ii < _ix[i]; ++ii)
              new_id.push_back(_subdomain_id[j * _dx.size() + i]);

      _subdomain_id = new_id;

      if (!isParamValid("dy"))
        mooseError("dy is not provided for 2D");
      if (isParamValid("dz"))
        mooseWarning("dz provided for 2D");
      if (isParamValid("iz"))
        mooseWarning("iz provided for 2D");
      break;
    }
    case 3:
    {
      _nx = 0;
      for (unsigned int i = 0; i < _dx.size(); ++i)
        _nx += _ix[i];
      _ny = 0;
      for (unsigned int i = 0; i < _dy.size(); ++i)
        _ny += _iy[i];
      _nz = 0;
      for (unsigned int i = 0; i < _dz.size(); ++i)
        _nz += _iz[i];

      std::vector<unsigned int> new_id;
      for (unsigned int k = 0; k < _dz.size(); ++k)
        for (unsigned int kk = 0; kk < _iz[k]; ++kk)
          for (unsigned int j = 0; j < _dy.size(); ++j)
            for (unsigned int jj = 0; jj < _iy[j]; ++jj)
              for (unsigned int i = 0; i < _dx.size(); ++i)
                for (unsigned int ii = 0; ii < _ix[i]; ++ii)
                  new_id.push_back(_subdomain_id[k * _dx.size() * _dy.size() + j * _dx.size() + i]);

      _subdomain_id = new_id;

      if (!isParamValid("dy"))
        mooseError("dy is not provided for 3D");
      if (!isParamValid("dz"))
        mooseError("dz is not provided for 3D");
      break;
    }
  }
}

std::unique_ptr<MeshBase>
CartesianMeshGenerator::generate()
{
  auto mesh = buildMeshBaseObject();

  // switching on MooseEnum to generate the reference mesh
  // Note: element type is fixed
  switch (_dim)
  {
    case 1:
      MeshTools::Generation::build_line(static_cast<UnstructuredMesh &>(*mesh), _nx, 0, _nx, EDGE2);
      break;
    case 2:
      MeshTools::Generation::build_square(
          static_cast<UnstructuredMesh &>(*mesh), _nx, _ny, 0, _nx, 0, _ny, QUAD4);
      break;
    case 3:
      MeshTools::Generation::build_cube(
          static_cast<UnstructuredMesh &>(*mesh), _nx, _ny, _nz, 0, _nx, 0, _ny, 0, _nz, HEX8);
      break;
  }

  // assign block IDs
  MeshBase::element_iterator el = mesh->active_elements_begin();
  MeshBase::element_iterator el_end = mesh->active_elements_end();
  for (; el != el_end; ++el)
  {
    const Point p = (*el)->vertex_average();
    unsigned int ix = std::floor(p(0));
    unsigned int iy = std::floor(p(1));
    unsigned int iz = std::floor(p(2));
    unsigned int i = iz * _nx * _ny + iy * _nx + ix;
    (*el)->subdomain_id() = _subdomain_id[i];
  }

  // adjust node coordinates
  switch (_dim)
  {
    case 1:
    {
      Real base;

      std::vector<Real> mapx;
      // Note: the starting coordinate is zero
      base = 0;
      mapx.push_back(base);
      for (unsigned int i = 0; i < _dx.size(); ++i)
      {
        for (unsigned int j = 1; j <= _ix[i]; ++j)
          mapx.push_back(base + _dx[i] / _ix[i] * j);
        base += _dx[i];
      }

      MeshBase::node_iterator node = mesh->active_nodes_begin();
      MeshBase::node_iterator node_end = mesh->active_nodes_end();
      for (; node != node_end; ++node)
      {
        unsigned int i = (*(*node))(0) + 0.5;
        (*(*node))(0) = mapx.at(i);
      }
      break;
    }
    case 2:
    {
      Real base;

      std::vector<Real> mapx;
      base = 0;
      mapx.push_back(base);
      for (unsigned int i = 0; i < _dx.size(); ++i)
      {
        for (unsigned int j = 1; j <= _ix[i]; ++j)
          mapx.push_back(base + _dx[i] / _ix[i] * j);
        base += _dx[i];
      }

      std::vector<Real> mapy;
      base = 0;
      mapy.push_back(base);
      for (unsigned int i = 0; i < _dy.size(); ++i)
      {
        for (unsigned int j = 1; j <= _iy[i]; ++j)
          mapy.push_back(base + _dy[i] / _iy[i] * j);
        base += _dy[i];
      }

      MeshBase::node_iterator node = mesh->active_nodes_begin();
      MeshBase::node_iterator node_end = mesh->active_nodes_end();
      for (; node != node_end; ++node)
      {
        unsigned int i = (*(*node))(0) + 0.5;
        (*(*node))(0) = mapx.at(i);
        unsigned int j = (*(*node))(1) + 0.5;
        (*(*node))(1) = mapy.at(j);
      }
      break;
    }
    case 3:
    {
      Real base;

      std::vector<Real> mapx;
      base = 0;
      mapx.push_back(base);
      for (unsigned int i = 0; i < _dx.size(); ++i)
      {
        for (unsigned int j = 1; j <= _ix[i]; ++j)
          mapx.push_back(base + _dx[i] / _ix[i] * j);
        base += _dx[i];
      }

      std::vector<Real> mapy;
      base = 0;
      mapy.push_back(base);
      for (unsigned int i = 0; i < _dy.size(); ++i)
      {
        for (unsigned int j = 1; j <= _iy[i]; ++j)
          mapy.push_back(base + _dy[i] / _iy[i] * j);
        base += _dy[i];
      }

      std::vector<Real> mapz;
      base = 0;
      mapz.push_back(base);
      for (unsigned int i = 0; i < _dz.size(); ++i)
      {
        for (unsigned int j = 1; j <= _iz[i]; ++j)
          mapz.push_back(base + _dz[i] / _iz[i] * j);
        base += _dz[i];
      }

      MeshBase::node_iterator node = mesh->active_nodes_begin();
      MeshBase::node_iterator node_end = mesh->active_nodes_end();
      for (; node != node_end; ++node)
      {
        unsigned int i = (*(*node))(0) + 0.5;
        (*(*node))(0) = mapx.at(i);
        unsigned int j = (*(*node))(1) + 0.5;
        (*(*node))(1) = mapy.at(j);
        unsigned int k = (*(*node))(2) + 0.5;
        (*(*node))(2) = mapz.at(k);
      }
      break;
    }
  }

  mesh->prepare_for_use();
  return dynamic_pointer_cast<MeshBase>(mesh);
}
