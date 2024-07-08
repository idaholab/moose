//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

//* Won't converge. Going to try explicit

#include "ComputeSimoHughesJ2PlasticHeatEnergy.h"
#include <cmath>

registerMooseObject("SolidMechanicsApp", ComputeSimoHughesJ2PlasticHeatEnergy);

InputParameters
ComputeSimoHughesJ2PlasticHeatEnergy::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
  params.addClassDescription("Plastic heat energy density = stress * plastic_strain_rate");
  return params;
}

ComputeSimoHughesJ2PlasticHeatEnergy::ComputeSimoHughesJ2PlasticHeatEnergy(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _ep_name(_base_name + "effective_plastic_strain"),
    _ep(getMaterialProperty<Real>(_ep_name)),
    _ep_old(getMaterialPropertyOld<Real>(_ep_name)),
    _F(getMaterialProperty<RankTwoTensor>(_base_name + "deformation_gradient")),
    _cauchy_stress(getMaterialProperty<RankTwoTensor>(_base_name + "cauchy_stress")),
    _plastic_heat(declareProperty<Real>(_base_name + "plastic_heat"))
{
}

void
ComputeSimoHughesJ2PlasticHeatEnergy::computeQpProperties()
{
  auto J = _F[_qp].det();
  auto cauchy_dev = _cauchy_stress[_qp].deviatoric();
  auto cauchy_dev_norm = cauchy_dev.norm(); // Frobenius norm
  auto s_eff = std::sqrt(3.0 / 2.0) * J * cauchy_dev_norm;
  _plastic_heat[_qp] = s_eff * (_ep[_qp] - _ep_old[_qp]) / _dt;

  // We are not currently computing Jacobian because the plastic strain is being lagged.
}
