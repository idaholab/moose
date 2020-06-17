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
#include "FEProblem.h"
#include "DisplacedProblem.h"

// Forward Declarations
class FEProblem;

class NormalNodalLMMechanicalContact : public NodeFaceConstraint
{
public:
  static InputParameters validParams();

  NormalNodalLMMechanicalContact(const InputParameters & parameters);

protected:
  virtual Real computeQpSecondaryValue() override;

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned jvar) override;

  virtual Real computeQpResidual(Moose::ConstraintType type) override;
  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type) override;
  virtual Real computeQpOffDiagJacobian(Moose::ConstraintJacobianType type, unsigned jvar) override;

  const unsigned _disp_y_id;
  const unsigned _disp_z_id;
  const Real _c;
  const Real _epsilon;

  const MooseEnum _ncp_type;

  const FEProblemBase & _fe_problem;

  const DisplacedProblem * const _displaced_problem;
};
