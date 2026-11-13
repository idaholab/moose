//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalConstraint.h"

class EqualValueNodalConstraint : public NodalConstraint
{
public:
  static InputParameters validParams();

  EqualValueNodalConstraint(const InputParameters & parameters);
  virtual ~EqualValueNodalConstraint();

  virtual void meshChanged() override;

protected:
  /**
   * Rebuild the constrained node lists and the ghosting they require. Called again whenever the
   * mesh changes, since node ownership and the locally available elements may both have changed.
   */
  void updateConstrainedNodes();

  virtual Real computeQpResidual(Moose::ConstraintType type);
  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type);

  Real _penalty;
};
