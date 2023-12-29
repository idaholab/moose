//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StressBasedChemicalPotential.h"

registerMooseObject("TensorMechanicsApp", StressBasedChemicalPotential);

InputParameters
StressBasedChemicalPotential::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Chemical potential from stress");
  params.addRequiredParam<MaterialPropertyName>("property_name",
                                                "Name of stress based chemical potential");
  params.addRequiredParam<MaterialPropertyName>("stress_name", "Name of stress property variable");
  params.addRequiredParam<MaterialPropertyName>("direction_tensor_name",
                                                "Name of direction tensor variable");
  params.addRequiredParam<MaterialPropertyName>("prefactor_name", "Name of prefactor variable");
  params.addCoupledVar("c", "Concentration variable");
  return params;
}

StressBasedChemicalPotential::StressBasedChemicalPotential(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _chemical_potential(declareProperty<Real>(getParam<MaterialPropertyName>("property_name"))),
    _stress_old(getMaterialPropertyOld<RankTwoTensor>("stress_name")),
    _direction_tensor(getMaterialProperty<RealTensorValue>("direction_tensor_name")),
    _prefactor(getMaterialProperty<Real>("prefactor_name")),
    _has_coupled_c(isCoupled("c") && !isCoupledConstant("c"))
{
  if (_has_coupled_c)
  {
    _dchemical_potential = &declarePropertyDerivative<Real>(
        getParam<MaterialPropertyName>("property_name"), coupledName("c", 0));
    _dprefactor_dc = &getMaterialPropertyDerivative<Real>("prefactor_name", coupledName("c", 0));
  }
}

void
StressBasedChemicalPotential::initQpStatefulProperties()
{
  _chemical_potential[_qp] = 0.0;

  if (_has_coupled_c)
    (*_dchemical_potential)[_qp] = 0.0;
}

void
StressBasedChemicalPotential::computeQpProperties()
{
  RankTwoTensor direction_tensor_rank_two = _direction_tensor[_qp];
  _chemical_potential[_qp] =
      -_stress_old[_qp].doubleContraction(direction_tensor_rank_two) * _prefactor[_qp];

  if (_has_coupled_c)
    (*_dchemical_potential)[_qp] =
        -_stress_old[_qp].doubleContraction(direction_tensor_rank_two) * (*_dprefactor_dc)[_qp];
}
