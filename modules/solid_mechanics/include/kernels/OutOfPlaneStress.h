/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef OUTOFPLANESTRESS_H
#define OUTOFPLANESTRESS_H

#include "Kernel.h"

// Forward Declarations
class OutOfPlaneStress;
class SymmElasticityTensor;
class SymmTensor;

template <>
InputParameters validParams<OutOfPlaneStress>();

class OutOfPlaneStress : public Kernel
{
public:
  OutOfPlaneStress(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const MaterialProperty<SymmTensor> & _stress;
  const MaterialProperty<SymmElasticityTensor> & _Jacobian_mult;
  const MaterialProperty<SymmTensor> & _d_stress_dT;

private:
  const bool _xdisp_coupled;
  const bool _ydisp_coupled;
  const bool _temp_coupled;

  const unsigned int _xdisp_var;
  const unsigned int _ydisp_var;
  const unsigned int _temp_var;
};
#endif // OUTOFPLANESTRESS_H
