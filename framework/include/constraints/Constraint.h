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
#include "NeighborResidualObject.h"
#include "GeometricSearchInterface.h"

namespace libMesh
{
class DofMap;
}

/**
 * Base class for all Constraint types
 */
class Constraint : public NeighborResidualObject, protected GeometricSearchInterface
{
public:
  static InputParameters validParams();

  Constraint(const InputParameters & parameters);

  /**
   * @return Whether this constraint adds exact constraints instead of a residual
   */
  virtual bool usesExactConstraints() const { return false; }

  /**
   * Add the exact constraints of this object. Every rank must call this. Each rank adds the rows
   * of the dependent nodes that it owns, and libMesh sends each row to the other ranks that need
   * it.
   * @param dof_map The DofMap that receives the exact constraints
   */
  virtual void addExactConstraints(libMesh::DofMap & /*dof_map*/) const {}

  /**
   * @return Whether the coefficients of the exact constraints changed since the last call. This
   * method is not const, because an implementation can record the coefficients that it compares.
   */
  virtual bool exactConstraintsChanged() { return false; }

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
