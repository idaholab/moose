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

#ifndef NODALROTATIONALINERTIA_H
#define NODALROTATIONALINERTIA_H

#include "NodalKernel.h"
#include "RankTwoTensor.h"

// Forward Declarations
class NodalRotationalInertia;

template <>
InputParameters validParams<NodalRotationalInertia>();

/**
 * Calculates the inertial torque and inertia proportional damping
 * for nodal rotational inertia
 */
class NodalRotationalInertia : public NodalKernel
{
public:
  NodalRotationalInertia(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  virtual Real computeQpJacobian() override;

  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// Auxiliary system object
  AuxiliarySystem & _aux_sys;

  /// Number of coupled rotational variables
  unsigned int _nrot;

  /// Value of rotational displacements
  std::vector<const VariableValue *> _rot;

  /// Old value of rotational displacements
  std::vector<const VariableValue *> _rot_old;

  /// Variable numbers for rotational velocity aux variables
  std::vector<unsigned int> _rot_vel_num;

  /// Variable numbers for rotational acceleration aux variables
  std::vector<unsigned int> _rot_accel_num;

  /// Variable numbers for rotational variables
  std::vector<unsigned int> _rot_variables;

  /// Current acceleration of the node
  std::vector<Real> _rot_accel;

  /// Current velocity of the node
  std::vector<Real> _rot_vel;

  /// Old velocity of the node
  std::vector<Real> _rot_vel_old;

  /// Newmark time integration parameter
  const Real & _beta;

  /// Newmark time integration parameter
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

#endif /* NODALROTATIOANLINERTIA_H */
