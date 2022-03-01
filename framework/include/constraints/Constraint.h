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
#include "NeighborResidualObject.h"
#include "GeometricSearchInterface.h"

/**
 * Base class for all Constraint types
 */
class Constraint : public NeighborResidualObject, protected GeometricSearchInterface
{
public:
  static InputParameters validParams();

  Constraint(const InputParameters & parameters);

  virtual bool addCouplingEntriesToJacobian() { return true; }
  virtual void subdomainSetup() override final
  {
    mooseError("subdomain setup for constraints is not implemented");
  }

  virtual void residualEnd() {}

protected:
  unsigned int _i, _j;
  unsigned int _qp;
};

#define usingConstraintMembers                                                                     \
  usingMooseObjectMembers;                                                                         \
  usingUserObjectInterfaceMembers;                                                                 \
  usingTaggingInterfaceMembers;                                                                    \
  using Constraint::_i;                                                                            \
  using Constraint::_qp;                                                                           \
  using Constraint::_tid
