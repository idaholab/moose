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
   * Initializes and checks validity of ReactorMeshParams mesh generator object
   * @param name of ReactorMeshParams mesh generator
   */
  void initializeReactorMeshParams(const std::string reactor_param_name);

  /**
   * Initializes and checks validity of ReactorMeshParams mesh generator object.
   * This overloaded function should be called if iniitalizeReactorMeshParams
   * has already been called before
   */
  void initializeReactorMeshParams();

  /**
   * Updates values of ReactorMeshParams object meta data for use by future
   * RGMB mesh generators
   */
  void updateReactorMeshParams();

  /**
   * Returns block id associated with block name and region id. If pairing
   * does not exist, a unique block id is generated.
   * @param block name
   * @param region id
   * @return block id
   */
  dof_id_type getBlockId(const std::string block_name, const dof_id_type region_id);

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
  const T & getReactorParam(const std::string param_name);

  ///The ReactorMeshParams object that is storing the reactor global information for this reactor geometry mesh
  MeshGeneratorName _reactor_params;

  ///Tracker to assign new block ids in RGMB mesh generators.
  ///Increments by 1 each time a new block id is requested
  subdomain_id_type _current_block_id;

  ///Map between RGMB element block names, block ids, and region ids
  std::map<std::string, std::pair<subdomain_id_type, dof_id_type>> _name_id_map;

  ///Prefix to use for block names if block names not provided by user
  const std::string _block_name_prefix = "RGMB_BLOCK_";
};
