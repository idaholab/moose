//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGenerator.h"
#include "libmesh/elem.h"

/**
 * A base class that contains common members for Reactor Geometry Mesh Builder mesh generators.
 */
class ReactorGeometryMeshBuilderBase : public MeshGenerator
{
public:
  static InputParameters validParams();

  ReactorGeometryMeshBuilderBase(const InputParameters & parameters);

protected:
  /**
   * Initializes extra element integer from id name for a given mesh and throws an error
   * if it should exist but cannot be found within the mesh
   * @param input_mesh input mesh
   * @param extra_int_name extra element id name
   * @param should_exist whether extra element integer should already exist in mesh
   * @return extra element integer
   */
  unsigned int getElemIntegerFromMesh(MeshBase & input_mesh,
                                      std::string extra_int_name,
                                      bool should_exist = false);

  /**
   * Initializes and checks validity of ReactorMeshParams mesh generator object
   * @param reactor_param_name name of ReactorMeshParams mesh generator
   */
  void initializeReactorMeshParams(const std::string reactor_param_name);

  /**
   * Checks whether parameter is defined in ReactorMeshParams metadata
   * @param param_name name of ReactorMeshParams parameter
   * @return whether parameter is defined in ReactorMeshParams metadata
   */
  bool hasReactorParam(const std::string param_name);

  /**
   * Returns reference of parameter in ReactorMeshParams object
   * @param param_name name of ReactorMeshParams parameter
   * @return reference to parameter defined in ReactorMeshParams metadata
   */
  template <typename T>
  const T & getReactorParam(const std::string & param_name);

  /**
   * Updates the block names and ids of the element in an input mesh according
   * to a map of block name to block ids. Updates the map if the block name is not in the map
   * @param input_name input mesh
   * @param elem iterator to mesh element
   * @param name_id_map map of name-id pairs used in mesh
   * @param elem_block_name block name to set for element
   * @param next_free_id next free block id to use if block name does not exist in map
   */
  void updateElementBlockNameId(MeshBase & input_mesh,
                                Elem * elem,
                                std::map<std::string, SubdomainID> & name_id_map,
                                std::string elem_block_name,
                                SubdomainID & next_free_id);

  ///The ReactorMeshParams object that is storing the reactor global information for this reactor geometry mesh
  MeshGeneratorName _reactor_params;
};

template <typename T>
const T &
ReactorGeometryMeshBuilderBase::getReactorParam(const std::string & param_name)
{
  return getMeshProperty<T>(param_name, _reactor_params);
}
