/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef STRESSDIVERGENCERZ_H
#define STRESSDIVERGENCERZ_H

#include "Kernel.h"

// Forward Declarations
class StressDivergenceRZ;
class SymmElasticityTensor;
class SymmTensor;

template <>
InputParameters validParams<StressDivergenceRZ>();

class StressDivergenceRZ : public Kernel
{
public:
  StressDivergenceRZ(const InputParameters & parameters);

protected:
  virtual void computeResidual();
  virtual void computeJacobian();
  virtual void computeOffDiagJacobian(unsigned int jvar);

  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  Real calculateJacobian(unsigned int ivar, unsigned int jvar);

  const MaterialProperty<SymmTensor> & _stress;
  const MaterialProperty<SymmElasticityTensor> & _Jacobian_mult;
  const MaterialProperty<SymmTensor> & _d_stress_dT;

private:
  const unsigned int _component;

  const bool _rdisp_coupled;
  const bool _zdisp_coupled;
  const bool _temp_coupled;
  const unsigned int _rdisp_var;
  const unsigned int _zdisp_var;
  const unsigned int _temp_var;
  std::vector<std::vector<Real>> _avg_grad_test;
  std::vector<std::vector<Real>> _avg_grad_phi;
  bool _volumetric_locking_correction;
};
#endif // STRESSDIVERGENCERZ_H
