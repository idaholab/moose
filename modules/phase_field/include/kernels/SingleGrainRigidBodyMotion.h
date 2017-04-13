/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SINGLEGRAINRIGIDBODYMOTION_H
#define SINGLEGRAINRIGIDBODYMOTION_H

#include "GrainRigidBodyMotionBase.h"

// Forward Declarations
class SingleGrainRigidBodyMotion;

template <>
InputParameters validParams<SingleGrainRigidBodyMotion>();

class SingleGrainRigidBodyMotion : public GrainRigidBodyMotionBase
{
public:
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

#endif // SINGLEGRAINRIGIDBODYMOTION_H
