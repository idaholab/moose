/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef MULTIGRAINRIGIDBODYMOTION_H
#define MULTIGRAINRIGIDBODYMOTION_H

#include "GrainRigidBodyMotionBase.h"

//Forward Declarations
class MultiGrainRigidBodyMotion;

template<>
InputParameters validParams<MultiGrainRigidBodyMotion>();

class MultiGrainRigidBodyMotion : public GrainRigidBodyMotionBase
{
public:
  MultiGrainRigidBodyMotion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /**
   * Calculates jacobian entry for variable c seperately
   * can be used for both on/off diagonal jacobian corrsponding to c
   * can be used with both split/non-split version of CH kernels
   */
  virtual Real computeCVarJacobianEntry();
};

#endif //MULTIGRAINRIGIDBODYMOTION_H
