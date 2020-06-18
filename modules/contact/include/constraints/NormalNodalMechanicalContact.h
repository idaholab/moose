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
#include "NodeFaceConstraint.h"

// Forward Declarations

class NormalNodalMechanicalContact : public NodeFaceConstraint
{
public:
  static InputParameters validParams();

  NormalNodalMechanicalContact(const InputParameters & parameters);

  void computeJacobian() override;
  void computeOffDiagJacobian(unsigned int jvar) override;

protected:
  virtual Real computeQpSecondaryValue() override;

  virtual Real computeQpResidual(Moose::ConstraintType type) override;
  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type) override;
  virtual Real computeQpOffDiagJacobian(Moose::ConstraintJacobianType type, unsigned jvar) override;

  const Real & _lambda;
  const unsigned _lambda_id;
  const Real _epsilon;
  const MooseEnum _component;
};
