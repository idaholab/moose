//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NODALNORMALCONTACTTEST_H
#define NODALNORMALCONTACTTEST_H

// MOOSE includes
#include "NodeFaceConstraint.h"

// Forward Declarations
class NodalNormalContactTest;

template <>
InputParameters validParams<NodalNormalContactTest>();

class NodalNormalContactTest : public NodeFaceConstraint
{
public:
  NodalNormalContactTest(const InputParameters & parameters);

protected:
  virtual Real computeQpSlaveValue() override;

  virtual Real computeQpResidual(Moose::ConstraintType type) override;
  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type) override;
  virtual Real computeQpOffDiagJacobian(Moose::ConstraintJacobianType type, unsigned jvar) override;

  const Real & _lambda;
  const unsigned _lambda_id;
  const Real _epsilon;
  const MooseEnum _component;
};

#endif
