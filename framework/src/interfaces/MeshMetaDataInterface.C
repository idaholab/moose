//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MeshMetaDataInterface.h"
#include "MooseApp.h"
#include "MeshGenerator.h"

MeshMetaDataInterface::MeshMetaDataInterface(const MooseObject * moose_object)
  : _meta_data_app(moose_object->getMooseApp())
{
}

MeshMetaDataInterface::MeshMetaDataInterface(MooseApp & moose_app) : _meta_data_app(moose_app) {}

RestartableDataValue &
MeshMetaDataInterface::registerMetaDataOnApp(const std::string & name,
                                             std::unique_ptr<RestartableDataValue> data)
{
  return _meta_data_app.registerRestartableData(
      name, std::move(data), 0, true, MooseApp::MESH_META_DATA);
}

bool
MeshMetaDataInterface::hasMeshProperty(const std::string & data_name,
                                       const std::string & prefix) const
{
  return _meta_data_app.hasRestartableMetaData(meshPropertyName(prefix, data_name),
                                               MooseApp::MESH_META_DATA);
}

std::string
MeshMetaDataInterface::meshPropertyName(const std::string & prefix, const std::string & data_name)
{
  return std::string(SYSTEM) + "/" + prefix + "/" + data_name;
}
