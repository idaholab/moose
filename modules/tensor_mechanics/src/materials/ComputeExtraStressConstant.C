/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ComputeExtraStressConstant.h"

template <>
InputParameters
validParams<ComputeExtraStressConstant>()
{
  InputParameters params = validParams<ComputeExtraStressBase>();
  params.addClassDescription("Computes a constant extra stress that is added to the stress "
                             "calculated by the constitutive model");
  params.addRequiredParam<std::vector<Real>>("extra_stress_tensor",
                                             "Vector of values defining the constant extra stress "
                                             "to add, in order 11, 22, 33, 23, 13, 12");
  params.addParam<MaterialPropertyName>(
      "prefactor", 1.0, "Name of material defining additional constant prefactor");
  return params;
}

ComputeExtraStressConstant::ComputeExtraStressConstant(const InputParameters & parameters)
  : ComputeExtraStressBase(parameters), _prefactor(getMaterialProperty<Real>("prefactor"))
{
  _extra_stress_tensor.fillFromInputVector(getParam<std::vector<Real>>("extra_stress_tensor"));
}

void
ComputeExtraStressConstant::computeQpExtraStress()
{
  _extra_stress[_qp] = _extra_stress_tensor * _prefactor[_qp];
}
