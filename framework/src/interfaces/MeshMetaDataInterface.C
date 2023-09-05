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
                                       const std::string & generator_name,
                                       const std::type_info & type) const
{
  return hasMeshPropertyInternal(data_name, generator_name, type, false) != DOES_NOT_HAVE;
}

std::string
MeshMetaDataInterface::meshPropertyName(const std::string & data_name,
                                        const std::string & generator_name)
{
  return std::string(SYSTEM) + "/" + generator_name + "/" + data_name;
}

std::string
MeshMetaDataInterface::meshPropertyPrefix(const std::string &) const
{
  mooseError(
      "This object does not support obtaining a mesh property without a generator_name.\n\nThis "
      "capability is upcoming.");
}

const RestartableDataValue &
MeshMetaDataInterface::getMeshPropertyInternal(const std::string & data_name,
                                               const std::string & generator_name) const
{
  return _meta_data_app.getRestartableMetaData(
      meshPropertyName(data_name, generator_name), MooseApp::MESH_META_DATA, 0);
}

MeshMetaDataInterface::HasMeshProperty
MeshMetaDataInterface::hasMeshPropertyInternal(const std::string & data_name,
                                               const std::string & generator_name,
                                               const std::type_info & type,
                                               const bool error_on_type) const
{
  const auto name = meshPropertyName(data_name, generator_name);

  if (const auto query_data =
          _meta_data_app.queryRestartableMetaData(name, MooseApp::MESH_META_DATA))
  {
    if (query_data->typeId() == type)
      return HAS_LOADED;
    else if (error_on_type)
      mooseErrorInternal("While querying mesh property '",
                         generator_name,
                         "/",
                         data_name,
                         "' with type '",
                         MooseUtils::prettyCppType(type),
                         "',\nthe property exists with different type '",
                         query_data->type(),
                         "'");
  }
  else if (_meta_data_app.getLateRestartableDataRestorer(MooseApp::MESH_META_DATA)
               .isRestorable(name, type))
    return HAS_NOT_LOADED;

  return DOES_NOT_HAVE;
}
