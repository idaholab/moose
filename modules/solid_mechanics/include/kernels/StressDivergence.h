/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef STRESSDIVERGENCE_H
#define STRESSDIVERGENCE_H

#include "Kernel.h"

// Forward Declarations
class ColumnMajorMatrix;
class StressDivergence;
class SymmElasticityTensor;
class SymmTensor;

template <>
InputParameters validParams<StressDivergence>();

class StressDivergence : public Kernel
{
public:
  StressDivergence(const InputParameters & parameters);

protected:
  virtual void computeResidual();
  virtual void computeJacobian();
  virtual void computeOffDiagJacobian(unsigned int jvar);

  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const MaterialProperty<SymmTensor> & _stress_older;
  const MaterialProperty<SymmTensor> & _stress_old;
  const MaterialProperty<SymmTensor> & _stress;
  const MaterialProperty<SymmElasticityTensor> & _Jacobian_mult;
  const MaterialProperty<SymmTensor> & _d_stress_dT;

private:
  const unsigned int _component;

  const bool _xdisp_coupled;
  const bool _ydisp_coupled;
  const bool _zdisp_coupled;
  const bool _temp_coupled;

  const unsigned int _xdisp_var;
  const unsigned int _ydisp_var;
  const unsigned int _zdisp_var;
  const unsigned int _temp_var;
  const Real _zeta;
  const Real _alpha;
  std::vector<std::vector<Real>> _avg_grad_test;
  std::vector<std::vector<Real>> _avg_grad_phi;
  bool _volumetric_locking_correction;
};
#endif // STRESSDIVERGENCE_H
