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
#include "InputParameters.h"

#include <string>

class MooseApp;

/**
 * The Interface used to retrieve mesh meta data (attributes) set by the MeshGenerator system.
 * MOOSE objects should avoid retrieving and casting MeshGenerator objects since they are not
 * re-created during a recover operation. This data is read from files early during the simulation
 * setup an d can be used to make decisions about how to setup the rest of the problem.
 */
class MeshMetaDataInterface
{
public:
  MeshMetaDataInterface(MooseApp & app);

protected:
  /**
   * Method for determining whether a property with the given type and name exists in the mesh
   * meta-data store.
   */
  template <typename T>
  bool hasMeshProperty(const std::string & name) const;

  /**
   * Method for retrieving a property with the given type and name exists in the mesh meta-data
   * store. This method will throw an error if the property does not exist.
   */
  template <typename T>
  T getMeshProperty(const std::string & name) const;

  /**
   * Returns the total number of mesh attributes stored in the meta-data store.
   */
  std::size_t metaDataSize() const { return _mgi_mesh_props.n_parameters(); }

  /**
   * Convenience method reporting whether any attributes are stored in the meta-data store or not.
   * If there are no attributed stores, no file is written through the Checkpoint format.
   */
  bool metaDataEmpty() const { return metaDataSize() == 0; }

private:
  /**
   * MeshGenerators are the only objects that may write to the mesh meta-data store. We use a friend
   * declaration to grant access to the MeshGenerators to access the meta-data.
   */
  friend class MeshGenerator;
  Parameters & metaData() const { return _mgi_mesh_props; }

  /// A writable reference to the meta-data store.
  Parameters & _mgi_mesh_props;
};

template <typename T>
bool
MeshMetaDataInterface::hasMeshProperty(const std::string & name) const
{
  return _mgi_mesh_props.have_parameter<T>(name);
}

template <typename T>
T
MeshMetaDataInterface::getMeshProperty(const std::string & name) const
{
  if (!hasMeshProperty<T>(name))
    mooseError("Property \"",
               name,
               "\" with type \"",
               demangle(typeid(T).name()),
               "\" doesn't exist in this mesh meta-data instance:\n",
               _mgi_mesh_props);

  return _mgi_mesh_props.get<T>(name);
}
