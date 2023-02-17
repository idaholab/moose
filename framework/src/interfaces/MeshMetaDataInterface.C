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

MeshMetaDataInterface::MeshMetaDataInterface(const MooseObject * moose_object)
  : _meta_data_app(moose_object->getMooseApp()), _meta_data_object(moose_object)
{
}

MeshMetaDataInterface::MeshMetaDataInterface(MooseApp & moose_app)
  : _meta_data_app(moose_app), _meta_data_object(nullptr)
{
}

bool
MeshMetaDataInterface::hasMeshProperty(const std::string & data_name,
                                       const std::string & prefix) const
{
  return _meta_data_app.hasRestartableMetaData(meshPropertyName(data_name, prefix),
                                               MooseApp::MESH_META_DATA);
}

std::string
MeshMetaDataInterface::meshPropertyName(const std::string & data_name, const std::string & prefix)
{
  return std::string(SYSTEM) + "/" + prefix + "/" + data_name;
}

std::string
MeshMetaDataInterface::meshPropertyPrefix(const std::string &) const
{
  mooseError("This object does not support obtaining a mesh property without a prefix.\n\nThis "
             "capability is upcoming.");
}

const RestartableDataValue &
MeshMetaDataInterface::getMeshPropertyInternal(const std::string & data_name,
                                               const std::string & prefix) const
{
  return _meta_data_app.getRestartableMetaData(
      meshPropertyName(data_name, prefix), MooseApp::MESH_META_DATA, 0);
}
