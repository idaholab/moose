//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputePlasticHeatEnergy.h"

registerMooseObject("TensorMechanicsApp", ComputePlasticHeatEnergy);

InputParameters
ComputePlasticHeatEnergy::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
  params.addClassDescription("Plastic heat energy density = stress * plastic_strain_rate");
  return params;
}

ComputePlasticHeatEnergy::ComputePlasticHeatEnergy(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _plastic_strain(getMaterialProperty<RankTwoTensor>("plastic_strain")),
    _plastic_strain_old(getMaterialPropertyOld<RankTwoTensor>("plastic_strain")),
    _stress(getMaterialProperty<RankTwoTensor>(_base_name + "stress")),
    _Jacobian_mult(getMaterialProperty<RankFourTensor>(_base_name + "Jacobian_mult")),
    _elasticity_tensor(getMaterialProperty<RankFourTensor>(_base_name + "elasticity_tensor")),
    _plastic_heat(declareProperty<Real>(_base_name + "plastic_heat")),
    _dplastic_heat_dstrain(declareProperty<RankTwoTensor>(_base_name + "dplastic_heat_dstrain"))
{
}

void
ComputePlasticHeatEnergy::computeQpProperties()
{
  _plastic_heat[_qp] =
      _stress[_qp].doubleContraction(_plastic_strain[_qp] - _plastic_strain_old[_qp]) / _dt;
  if (_fe_problem.currentlyComputingJacobian())
  {
    if (_plastic_strain[_qp] == _plastic_strain_old[_qp])
      // no plastic deformation, so _elasticity_tensor = _Jacobian_mult
      _dplastic_heat_dstrain[_qp] = RankTwoTensor();
    else
    {
      _dplastic_heat_dstrain[_qp] =
          (_plastic_strain[_qp] - _plastic_strain_old[_qp]).initialContraction(_Jacobian_mult[_qp]);
      _dplastic_heat_dstrain[_qp] += _stress[_qp];
      _dplastic_heat_dstrain[_qp] -=
          _stress[_qp].initialContraction(_elasticity_tensor[_qp].invSymm() * _Jacobian_mult[_qp]);
      _dplastic_heat_dstrain[_qp] /= _dt;
    }
  }
}
