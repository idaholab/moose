//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TANGENTIALLMCONSTRAINT_H
#define TANGENTIALLMCONSTRAINT_H

// MOOSE includes
#include "NodeFaceConstraint.h"

// Forward Declarations
class TangentialLMConstraint;

template <>
InputParameters validParams<TangentialLMConstraint>();

class TangentialLMConstraint : public NodeFaceConstraint
{
public:
  TangentialLMConstraint(const InputParameters & parameters);

protected:
  virtual Real computeQpSlaveValue() override;

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned jvar) override;

  virtual Real computeQpResidual(Moose::ConstraintType type) override;
  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type) override;
  virtual Real computeQpOffDiagJacobian(Moose::ConstraintJacobianType type, unsigned jvar) override;

  const VariableValue & _contact_pressure;
  const unsigned _contact_pressure_id;
  const VariableValue & _vel_x;
  const unsigned _vel_x_id;
  const VariableValue & _vel_y;
  const unsigned _vel_y_id;
  const VariableValue & _vel_z;
  const unsigned _vel_z_id;

  const Real _mu;
  const Real _lambda;
  const Real _epsilon;
};

#endif
