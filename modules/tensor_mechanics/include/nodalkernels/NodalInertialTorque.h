/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef NODALINERTIALTORQUE_H
#define NODALINERTIALTORQUE_H

#include "NodalKernel.h"
#include "RankTwoTensor.h"

// Forward Declarations
class NodalInertialTorque;

template <>
InputParameters validParams<NodalInertialTorque>();

/**
 * Calculates the inertial force and mass proportional damping for nodal inertia
 */
class NodalInertialTorque : public NodalKernel
{
public:
  NodalInertialTorque(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  virtual Real computeQpJacobian() override;

  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  AuxiliarySystem & _aux_sys;

  unsigned int _nrot;

  /// Value of rotational displacements
  std::vector<const VariableValue *> _rot;

  /// Old value of rotational displacements
  std::vector<const VariableValue *> _rot_old;

  /// Variable number for rotational velocity variables
  std::vector<unsigned int> _rot_vel_num;

  /// Variable number for rotational acceleration variables
  std::vector<unsigned int> _rot_accel_num;

  std::vector<unsigned int> _rot_variables;

  /// Newmark beta time integration parameters - beta and gamma
  const Real & _beta;
  const Real & _gamma;

  /// Mass proportional Rayliegh damping
  const Real & _eta;

  /// HHT time integration parameter
  const Real & _alpha;

  /// Component along which torque is applied
  const unsigned int _component;

  /// Moment of inertia tensor in global coordinate system
  RankTwoTensor _inertia;
};

#endif /* NODALINERTIALTORQUE_H */
