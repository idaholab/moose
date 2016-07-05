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

  virtual Real computeQpNonlocalJacobian(dof_id_type dof_index);
  virtual Real computeQpNonlocalOffDiagJacobian(unsigned int jvar, dof_id_type dof_index);


  /**
   * Calculates jacobian entry for variable c seperately
   * can be used for both on/off diagonal jacobian corrsponding to c
   * can be used with both split/non-split version of CH kernels
   */
  virtual Real computeCVarJacobianEntry(dof_id_type jdof);
  virtual Real computeCVarNonlocalJacobianEntry(dof_id_type jdof);
  virtual Real computeEtaVarJacobianEntry(dof_id_type jdof, unsigned int var_num);
  virtual Real computeEtaVarNonlocalJacobianEntry(dof_id_type jdof, unsigned int var_num);
};

#endif //MULTIGRAINRIGIDBODYMOTION_H
