//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GrainRigidBodyMotionBase.h"

// Forward Declarations

class SingleGrainRigidBodyMotion : public GrainRigidBodyMotionBase
{
public:
  static InputParameters validParams();

  SingleGrainRigidBodyMotion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int /*jvar*/);

  virtual Real computeQpNonlocalJacobian(dof_id_type /*dof_index*/);
  virtual Real computeQpNonlocalOffDiagJacobian(unsigned int /*jvar*/, dof_id_type /*dof_index*/);

  virtual void calculateAdvectionVelocity();
  virtual void getUserObjectJacobian(unsigned int jvar, dof_id_type dof_index);

  /// Grain number for the kernel to be applied
  unsigned int _op_index;
};
