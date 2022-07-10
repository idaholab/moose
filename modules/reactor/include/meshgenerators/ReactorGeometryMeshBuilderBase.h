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
   * @param input mesh
   * @param extra element id name
   * @param whether extra element integer should already exist in mesh
   * @return extra element integer
   */
  unsigned int
  getElemIntegerFromMesh(MeshBase & input_mesh, std::string id_name, bool should_exist = false);

  /**
   * Initializes and checks validity of ReactorMeshParams mesh generator object
   * @param name of ReactorMeshParams mesh generator
   */
  void initializeReactorMeshParams(const std::string reactor_param_name);

  /**
   * Checks whether parameter is defined in ReactorMeshParams metadata
   * @param name of ReactorMeshParams parameter
   * @return whether parameter is defined in ReactorMeshParams metadata
   */
  bool hasReactorParam(const std::string param_name);

  /**
   * Returns refernece of parameter in ReactorMeshParams object
   * @param name of ReactorMeshParams parameter
   * @return reference to parameter is defined in ReactorMeshParams metadata
   */
  template <typename T>
  const T & getReactorParam(const std::string & param_name);

  /**
   * Updates the block names and ids of the element in an input mesh according
   * to a map of name-id pairings
   * @param input mesh
   * @param iterator to mesh element
   * @param map of name-id pairs used in mesh
   * @param block name to set for element
   * @param next free block id to use if block name does not exist in map
   */
  void updateElementBlockNameId(MeshBase & input_mesh,
                                Elem * elem,
                                std::map<std::string, SubdomainID> & name_id_map,
                                std::string elem_block_name,
                                SubdomainID & next_free_id);

  ///The ReactorMeshParams object that is storing the reactor global information for this reactor geometry mesh
  MeshGeneratorName _reactor_params;

  ///Prefix to use for block names if block names not provided by user
  const std::string _block_name_prefix = "RGMB_BLOCK_";
};

template <typename T>
const T &
ReactorGeometryMeshBuilderBase::getReactorParam(const std::string & param_name)
{
  return getMeshProperty<T>(param_name, _reactor_params);
}
