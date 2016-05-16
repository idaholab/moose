/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ExternalForceDensityMaterial.h"
#include "Function.h"

template<>
InputParameters validParams<ExternalForceDensityMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("Providing external applied force density to grains");
  params.addParam<FunctionName>("force_x", 0.0, "The forcing function in x direction.");
  params.addParam<FunctionName>("force_y", 0.0, "The forcing function in y direction.");
  params.addParam<FunctionName>("force_z", 0.0, "The forcing function in z direction.");
  params.addRequiredCoupledVarWithAutoBuild("etas", "var_name_base", "op_num", "Array of coupled order parameters");
  params.addCoupledVar("c", "Concentration field");
  params.addParam<Real>("k", 1.0, "stiffness constant multiplier");
  return params;
}

ExternalForceDensityMaterial::ExternalForceDensityMaterial(const InputParameters & parameters) :
   DerivativeMaterialInterface<Material>(parameters),
   _force_x(getFunction("force_x")),
   _force_y(getFunction("force_y")),
   _force_z(getFunction("force_z")),
   _c(coupledValue("c")),
   _c_name(getVar("c", 0)->name()),
   _k(getParam<Real>("k")),
   _ncrys(coupledComponents("etas")), //determine number of grains from the number of names passed in.
   _vals(_ncrys), //Size variable arrays
   _dF(declareProperty<std::vector<RealGradient> >("force_density_ext")),
   _dFdc(declarePropertyDerivative<std::vector<RealGradient> >("force_density_ext", _c_name))
{
  //Loop through grains and load coupled variables into the arrays
  for (unsigned int i = 0; i < _ncrys; ++i)
    _vals[i] = &coupledValue("etas", i);
}

void
ExternalForceDensityMaterial::computeQpProperties()
{
  _dF[_qp].resize(_ncrys);
  _dFdc[_qp].resize(_ncrys);

  for (unsigned int i = 0; i < _ncrys; ++i)
  {
    _dF[_qp][i](0) = _k * _c[_qp] * _force_x.value(_t, _q_point[_qp]) * (*_vals[i])[_qp];
    _dF[_qp][i](1) = _k * _c[_qp] * _force_y.value(_t, _q_point[_qp]) * (*_vals[i])[_qp];
    _dF[_qp][i](2) = _k * _c[_qp] * _force_z.value(_t, _q_point[_qp]) * (*_vals[i])[_qp];

    _dFdc[_qp][i](0) = _k * _force_x.value(_t, _q_point[_qp]) * (*_vals[i])[_qp];
    _dFdc[_qp][i](1) = _k * _force_y.value(_t, _q_point[_qp]) * (*_vals[i])[_qp];
    _dFdc[_qp][i](2) = _k * _force_z.value(_t, _q_point[_qp]) * (*_vals[i])[_qp];
  }
}
