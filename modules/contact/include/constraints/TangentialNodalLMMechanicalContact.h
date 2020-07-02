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

class TangentialNodalLMMechanicalContact : public NodeFaceConstraint
{
public:
  static InputParameters validParams();

  TangentialNodalLMMechanicalContact(const InputParameters & parameters);

protected:
  virtual Real computeQpSecondaryValue() override;

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned jvar) override;

  virtual Real computeQpResidual(Moose::ConstraintType type) override;
  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type) override;
  virtual Real computeQpOffDiagJacobian(Moose::ConstraintJacobianType type, unsigned jvar) override;

  const Real & _contact_pressure;
  const unsigned _contact_pressure_id;

  const Real & _disp_x_dot;
  const Real & _disp_y_dot;

  const unsigned _disp_y_id;

  const VariableValue & _du_dot_du;

  const Real _mu;
  const Real _epsilon;

  const MooseEnum _ncp_type;
};
