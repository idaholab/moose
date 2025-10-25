//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"
#include "SetupInterface.h"
#include "TransientInterface.h"
#include "PostprocessorInterface.h"
#include "UserObjectInterface.h"
#include "Restartable.h"
#include "MeshChangedInterface.h"
#include "ScalarCoupleable.h"

namespace Moose
{

class FunctionBase : public MooseObject,
                     public SetupInterface,
                     public TransientInterface,
                     public PostprocessorInterface,
                     public UserObjectInterface,
                     public Restartable,
                     public MeshChangedInterface,
                     public ScalarCoupleable
{
public:
  static InputParameters validParams();

  FunctionBase(const InputParameters & parameters);

#ifdef MOOSE_KOKKOS_ENABLED
  /**
   * Special constructor used for Kokkos functor copy during parallel dispatch
   */
  FunctionBase(const FunctionBase & object, const Moose::Kokkos::FunctorCopy & key);
#endif

  virtual ~FunctionBase();
};

}
