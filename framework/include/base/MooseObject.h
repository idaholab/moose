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
#include "InputParameters.h"
#include "ConsoleStreamInterface.h"
#include "Registry.h"
#include "MooseUtils.h"
#include "DataFileInterface.h"
#include "MooseObjectParameterName.h"

#include "libmesh/parallel_object.h"

#define usingMooseObjectMembers                                                                    \
  using MooseObject::isParamValid;                                                                 \
  using MooseObject::isParamSetByUser;                                                             \
  using MooseObject::paramError

// helper macro to explicitly instantiate AD classes
#define adBaseClass(X)                                                                             \
  template class X<RESIDUAL>;                                                                      \
  template class X<JACOBIAN>

/**
 * Every object that can be built by the factory should be derived from this class.
 */
class MooseObject : public MooseBase,
                    public MooseBaseParameterInterface,
                    public MooseBaseErrorInterface,
                    public libMesh::ParallelObject,
                    public DataFileInterface<MooseObject>
{
public:
  static InputParameters validParams();

  MooseObject(const InputParameters & parameters);

  virtual ~MooseObject() = default;

  /**
   * Return the enabled status of the object.
   */
  virtual bool enabled() const { return _enabled; }

protected:
  /// Reference to the "enable" InputParameters, used by Controls for toggling on/off MooseObjects
  const bool & _enabled;

  // Base classes have the same name for that attribute, pick one
  using MooseBase::_app;
};
