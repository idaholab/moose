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
  const MaterialProperty<Real> & _density;
  unsigned int _nrot;
  unsigned int _ndisp;
  std::vector<unsigned int> _rot_num;
  std::vector<unsigned int> _disp_num;
  std::vector<unsigned int> _vel_num;
  std::vector<unsigned int> _accel_num;
  std::vector<unsigned int> _rot_vel_num;
  std::vector<unsigned int> _rot_accel_num;
  std::vector<unsigned int> _disp_var;
  std::vector<unsigned int> _rot_var;

  const VariableValue & _area;
  const VariableValue & _Ay;
  const VariableValue & _Az;
  const VariableValue & _Iy;
  const VariableValue & _Iz;

  const Real _beta;
  const Real _gamma;
  const MaterialProperty<Real> & _eta;
  const Real _alpha;

  std::string _base_name;
  const MaterialProperty<RankTwoTensor> & _original_local_config;
  const MaterialProperty<Real> & _original_length;
  const unsigned int _component;
};

#endif // INERTIALFORCEBEAM_H
