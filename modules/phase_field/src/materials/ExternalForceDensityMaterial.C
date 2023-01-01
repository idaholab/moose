//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExternalForceDensityMaterial.h"
#include "Function.h"

registerMooseObject("PhaseFieldApp", ExternalForceDensityMaterial);

InputParameters
ExternalForceDensityMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Providing external applied force density to grains");
  params.addParam<FunctionName>("force_x", 0.0, "The forcing function in x direction.");
  params.addParam<FunctionName>("force_y", 0.0, "The forcing function in y direction.");
  params.addParam<FunctionName>("force_z", 0.0, "The forcing function in z direction.");
  params.addRequiredCoupledVarWithAutoBuild(
      "etas", "var_name_base", "op_num", "Array of coupled order parameters");
  params.addCoupledVar("c", "Concentration field");
  params.addParam<Real>("k", 1.0, "stiffness constant multiplier");
  return params;
}

ExternalForceDensityMaterial::ExternalForceDensityMaterial(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _force_x(getFunction("force_x")),
    _force_y(getFunction("force_y")),
    _force_z(getFunction("force_z")),
    _c(coupledValue("c")),
    _c_name(coupledName("c", 0)),
    _k(getParam<Real>("k")),
    _op_num(coupledComponents(
        "etas")), // determine number of grains from the number of names passed in.
    _vals(coupledValues("etas")),
    _vals_name(coupledNames("etas")),
    _dF(declareProperty<std::vector<RealGradient>>("force_density_ext")),
    _dFdc(isCoupledConstant(_c_name) ? nullptr
                                     : &declarePropertyDerivative<std::vector<RealGradient>>(
                                           "force_density_ext", _c_name)),
    _dFdeta(_op_num)
{
  // Loop through grains and load derivatives
  for (unsigned int i = 0; i < _op_num; ++i)
    if (!isCoupledConstant(_vals_name[i]))
      _dFdeta[i] =
          &declarePropertyDerivative<std::vector<RealGradient>>("force_density_ext", _vals_name[i]);
}

void
ExternalForceDensityMaterial::computeQpProperties()
{
  _dF[_qp].resize(_op_num);
  if (_dFdc)
    (*_dFdc)[_qp].resize(_op_num);

  for (unsigned int i = 0; i < _op_num; ++i)
  {
    _dF[_qp][i](0) = _k * _c[_qp] * _force_x.value(_t, _q_point[_qp]) * (*_vals[i])[_qp];
    _dF[_qp][i](1) = _k * _c[_qp] * _force_y.value(_t, _q_point[_qp]) * (*_vals[i])[_qp];
    _dF[_qp][i](2) = _k * _c[_qp] * _force_z.value(_t, _q_point[_qp]) * (*_vals[i])[_qp];

    if (_dFdc)
    {
      (*_dFdc)[_qp][i](0) = _k * _force_x.value(_t, _q_point[_qp]) * (*_vals[i])[_qp];
      (*_dFdc)[_qp][i](1) = _k * _force_y.value(_t, _q_point[_qp]) * (*_vals[i])[_qp];
      (*_dFdc)[_qp][i](2) = _k * _force_z.value(_t, _q_point[_qp]) * (*_vals[i])[_qp];
    }
  }

  for (unsigned int i = 0; i < _op_num; ++i)
  {
    if (_dFdeta[i])
    {
      (*_dFdeta[i])[_qp].resize(_op_num);
      for (unsigned int j = 0; j < _op_num; ++j)
      {
        (*_dFdeta[i])[_qp][j](0) = _k * _c[_qp] * _force_x.value(_t, _q_point[_qp]);
        (*_dFdeta[i])[_qp][j](1) = _k * _c[_qp] * _force_y.value(_t, _q_point[_qp]);
        (*_dFdeta[i])[_qp][j](2) = _k * _c[_qp] * _force_z.value(_t, _q_point[_qp]);
      }
    }
  }
}
