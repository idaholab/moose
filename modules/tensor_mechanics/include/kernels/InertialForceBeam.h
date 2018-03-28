/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INERTIALFORCEBEAM_H
#define INERTIALFORCEBEAM_H

#include "Kernel.h"
#include "Material.h"
#include "RankTwoTensor.h"

// Forward Declarations
class InertialForceBeam;

template <>
InputParameters validParams<InertialForceBeam>();

class InertialForceBeam : public Kernel
{
public:
  InertialForceBeam(const InputParameters & parameters);

protected:
  virtual void computeResidual() override;
  virtual Real computeQpResidual() override { return 0.0; };

  virtual void computeJacobian() override;

  virtual void computeOffDiagJacobian(unsigned int jvar) override;

private:
  /// Density of the beam
  const MaterialProperty<Real> & _density;

  /// Number of coupled rotational variables
  unsigned int _nrot;

  /// Number of coupled displacement variables
  unsigned int _ndisp;

  /// Variable numbers corresponding to rotational variables
  std::vector<unsigned int> _rot_num;

  /// Variable numbers corresponding to displacement variables
  std::vector<unsigned int> _disp_num;

  /// Variable numbers corresponding to velocity aux variables
  std::vector<unsigned int> _vel_num;

  /// Variable numbers corresponding to acceleraion aux variables
  std::vector<unsigned int> _accel_num;

  /// Variable numbers corresponding to rotational velocity aux variables
  std::vector<unsigned int> _rot_vel_num;

  /// Variable numbers corresponding to rotational acceleration aux variables
  std::vector<unsigned int> _rot_accel_num;

  /// Coupled variable for beam cross-sectional area
  const VariableValue & _area;

  /// Coupled variable for first moment of area of beam in y direction, i.e., integral of y*dA over the cross-section
  const VariableValue & _Ay;

  /// Coupled variable for first moment of area of beam in z direction, i.e., integral of z*dA over the cross-section
  const VariableValue & _Az;

  /// Coupled variable for second moment of area of beam in y direction, i.e., integral of y^2*dA over the cross-section
  const VariableValue & _Iy;

  /// Coupled variable for second momemnt of area of beam in z dirextion, i.e., integral of z^2*dA over the cross-section
  const VariableValue & _Iz;

  /// Newmark time integration parameter
  const Real _beta;

  /// Newmark time integraion parameter
  const Real _gamma;

  /// Mass proportional Rayleigh damping parameter
  const MaterialProperty<Real> & _eta;

  /// HHT time integration parameter
  const Real _alpha;

  /// Rotational transformation from global to initial beam local coordinate system
  const MaterialProperty<RankTwoTensor> & _original_local_config;

  /// Initial length of beam
  const MaterialProperty<Real> & _original_length;

  /// Direction along which residual is calculated
  const unsigned int _component;
};

#endif // INERTIALFORCEBEAM_H
