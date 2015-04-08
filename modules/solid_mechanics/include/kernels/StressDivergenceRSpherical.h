/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef STRESSDIVERGENCERSPHERICAL_H
#define STRESSDIVERGENCERSPHERICAL_H

#include "Kernel.h"

//Forward Declarations
class SymmElasticityTensor;
class SymmTensor;

class StressDivergenceRSpherical : public Kernel
{
public:

  StressDivergenceRSpherical(const std::string & name, InputParameters parameters);
  virtual ~StressDivergenceRSpherical() {}

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  MaterialProperty<SymmTensor> & _stress;
  MaterialProperty<SymmElasticityTensor> & _Jacobian_mult;
  MaterialProperty<SymmTensor> & _d_stress_dT;

private:
  const unsigned int _component;
  const bool _temp_coupled;
  const unsigned int _temp_var;
};

template<>
InputParameters validParams<StressDivergenceRSpherical>();

#endif //STRESSDIVERGENCERSPHERICAL_H
