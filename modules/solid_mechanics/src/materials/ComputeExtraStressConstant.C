//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeExtraStressConstant.h"

registerMooseObject("TensorMechanicsApp", ComputeExtraStressConstant);

InputParameters
ComputeExtraStressConstant::validParams()
{
  InputParameters params = ComputeExtraStressBase::validParams();
  params.addClassDescription("Computes a constant extra stress that is added to the stress "
                             "calculated by the constitutive model");
  params.addRequiredParam<std::vector<Real>>("extra_stress_tensor",
                                             "Vector of values defining the constant extra stress "
                                             "to add, in order 11, 22, 33, 23, 13, 12");
  params.addParam<MaterialPropertyName>(
      "prefactor", 1.0, "Name of material property defining additional constant prefactor");
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
