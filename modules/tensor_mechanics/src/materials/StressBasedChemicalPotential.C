/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "StressBasedChemicalPotential.h"

template <>
InputParameters
validParams<StressBasedChemicalPotential>()
{
  InputParameters params = validParams<Material>();
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
    _has_coupled_c(isCoupled("c"))
{
  if (_has_coupled_c)
  {
    _dchemical_potential = &declarePropertyDerivative<Real>(
        getParam<MaterialPropertyName>("property_name"), getVar("c", 0)->name());
    _dprefactor_dc = &getMaterialPropertyDerivative<Real>("prefactor_name", getVar("c", 0)->name());
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
