//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MooseBase.h"
#include "MooseBaseParameterInterface.h"
#include "MooseBaseErrorInterface.h"
#include "DataFileInterface.h"

#include "libmesh/parallel_object.h"

/**
 * Base class shared by both Action and MooseObject.
 */
class ParallelParamObject : public MooseBase,
                            public MooseBaseParameterInterface,
                            public MooseBaseErrorInterface,
                            public libMesh::ParallelObject,
                            public DataFileInterface
{
public:
  ParallelParamObject(const std::string & type,
                      const std::string & name,
                      MooseApp & app,
                      const InputParameters & params);

  virtual ~ParallelParamObject() = default;
};
