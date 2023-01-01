//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeCrystalPlasticityThermalEigenstrain.h"

registerMooseObject("TensorMechanicsApp", ComputeCrystalPlasticityThermalEigenstrain);

InputParameters
ComputeCrystalPlasticityThermalEigenstrain::validParams()
{
  InputParameters params = ComputeCrystalPlasticityEigenstrainBase::validParams();

  params.addCoupledVar("temperature", "Coupled temperature variable");

  // Let's check the range of the parameter here
  params.addRequiredRangeCheckedParam<std::vector<Real>>(
      "thermal_expansion_coefficients",
      "thermal_expansion_coefficients_size=1 | thermal_expansion_coefficients_size=3 | "
      "thermal_expansion_coefficients_size=6 | thermal_expansion_coefficients_size=9",
      "Vector of values defining the constant second order thermal expansion coefficients, "
      "depending on the degree of anisotropy, this should be of size 1, 3, 6 or 9");

  return params;
}

ComputeCrystalPlasticityThermalEigenstrain::ComputeCrystalPlasticityThermalEigenstrain(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<ComputeCrystalPlasticityEigenstrainBase>(parameters),
    _temperature(coupledValue("temperature")),
    _temperature_old(coupledValueOld("temperature")),
    _ddeformation_gradient_dT(isCoupledConstant("temperature")
                                  ? nullptr
                                  : &declarePropertyDerivative<RankTwoTensor>(
                                        _deformation_gradient_name, coupledName("temperature", 0))),
    _thermal_expansion_coefficients(getParam<std::vector<Real>>("thermal_expansion_coefficients")),
    _lattice_thermal_expansion_coefficients(declareProperty<RankTwoTensor>(
        _eigenstrain_name +
        "_lattice_thermal_expansion_coefficients")) // avoid duplicated material name by including
                                                    // the eigenstrain name this coeff corresponds
                                                    // to
{
}

void
ComputeCrystalPlasticityThermalEigenstrain::initQpStatefulProperties()
{
  ComputeCrystalPlasticityEigenstrainBase::initQpStatefulProperties();
  // rotate the thermal deforamtion gradient for crystals based on Euler angles
  _lattice_thermal_expansion_coefficients[_qp] =
      _thermal_expansion_coefficients.rotated(_crysrot[_qp]);
}

void
ComputeCrystalPlasticityThermalEigenstrain::computeQpDeformationGradient()
{
  // compute the deformation gradient due to thermal expansion
  Real dtheta = (_temperature[_qp] - _temperature_old[_qp]) * _substep_dt / _dt;
  RankTwoTensor residual_equivalent_thermal_expansion_increment =
      RankTwoTensor::Identity() - dtheta * _lattice_thermal_expansion_coefficients[_qp];
  _deformation_gradient[_qp] =
      residual_equivalent_thermal_expansion_increment.inverse() * _deformation_gradient_old[_qp];

  // compute the derivative of deformation gradient w.r.t temperature
  if (_ddeformation_gradient_dT)
    (*_ddeformation_gradient_dT)[_qp] =
        _lattice_thermal_expansion_coefficients[_qp] * _deformation_gradient[_qp];
}
