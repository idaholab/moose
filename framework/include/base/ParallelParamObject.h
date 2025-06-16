//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MooseBase.h"
#include "DataFileInterface.h"

#include "libmesh/parallel_object.h"

class Factory;
class ActionFactory;

/**
 * Base class shared by both Action and MooseObject.
 */
class ParallelParamObject : public MooseBase,
                            public libMesh::ParallelObject,
                            public DataFileInterface
{
public:
  static InputParameters validParams();

  ParallelParamObject(const InputParameters & params);

  virtual ~ParallelParamObject() = default;

protected:
  /// The Factory associated with the MooseApp
  Factory & _factory;

  /// Builds Actions
  ActionFactory & _action_factory;
};
