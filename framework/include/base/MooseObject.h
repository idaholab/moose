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
#include "MooseUtils.h"
#include "ParallelParamObject.h"
#include "InputParameters.h"
#include "ConsoleStreamInterface.h"
#include "Registry.h"
#include "MooseObjectParameterName.h"

#define usingMooseObjectMembers                                                                    \
  usingMooseBaseMembers;                                                                           \
  usingMooseBaseParameterInterfaceMembers;                                                         \
  using MooseObject::enabled

/**
 * Every object that can be built by the factory should be derived from this class.
 */
class MooseObject : public ParallelParamObject, public std::enable_shared_from_this<MooseObject>
{
public:
  static InputParameters validParams();

  MooseObject(const InputParameters & parameters);

  virtual ~MooseObject() = default;

  /**
   * Return the enabled status of the object.
   */
  virtual bool enabled() const { return _enabled; }

  /**
   * Get another shared pointer to this object that has the same ownership group. Wrapper around
   * shared_from_this().
   */
  std::shared_ptr<MooseObject> getSharedPtr();
  std::shared_ptr<const MooseObject> getSharedPtr() const;

protected:
  /// Reference to the "enable" InputParameters, used by Controls for toggling on/off MooseObjects
  const bool & _enabled;

  // Base classes have the same name for that attribute, pick one
  using MooseBase::_app;
};
