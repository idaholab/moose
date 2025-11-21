//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ThreadedGeneralUserObject.h"
#include "SubChannelMesh.h"
#include "SubChannel1PhaseProblem.h"

/**
 * Base class for SCM closures
 */
class SCMClosureBase : public ThreadedGeneralUserObject
{
public:
  static InputParameters validParams();

  SCMClosureBase(const InputParameters & parameters);

  virtual void execute() final {}
  virtual void initialize() final {}
  virtual void finalize() final {}

  virtual void threadJoin(const UserObject &) final {}
  virtual void subdomainSetup() final {}

protected:
  /// Reference to the subchannel mesh
  const SubChannelMesh & _subchannel_mesh;
  /// Reference to the subchannel problem
  const SubChannel1PhaseProblem * const _scm_problem;
};
