//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"
#include "MooseError.h"

#include "libmesh/parameters.h"

class MooseMesh;

class MeshMetaDataInterface
{
public:
  MeshMetaDataInterface(MooseMesh * mesh_ptr = nullptr);

  void setMeshMetaData(MooseMesh & mesh);

protected:
  template <typename T>
  bool hasMeshProperty(const std::string & name) const;

  template <typename T>
  T getMeshProperty(const std::string & name) const;

private:
  Parameters * _mgi_mesh_props;
};

template <typename T>
bool
MeshMetaDataInterface::hasMeshProperty(const std::string & name) const
{
  mooseAssert(_mgi_mesh_props, "Mesh meta data store not initialized");

  return _mgi_mesh_props->have_parameter<T>(name);
}

template <typename T>
T
MeshMetaDataInterface::getMeshProperty(const std::string & name) const
{
  if (hasMeshProperty<T>(name))
    mooseError("Property ", name, " doesn't exist in this instance.");

  return _mgi_mesh_props->get<T>(name);
}
