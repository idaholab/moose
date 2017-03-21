#include "DeformedGrainMaterial.h"
#include "GrainTrackerInterface.h"

template <>
InputParameters
validParams<DeformedGrainMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredCoupledVarWithAutoBuild(
      "v", "var_name_base", "op_num", "Array of coupled variables");
  params.addRequiredParam<unsigned int>("deformed_grain_num",
                                        "Number of OP representing deformed grains");
  params.addParam<Real>("length_scale", 1.0e-9, "Length scale in m, where default is nm");
  params.addParam<Real>("int_width", 4.0, "Diffuse Interface width in length_scale unit");
  params.addParam<Real>("time_scale", 1.0e-6, "Time scale in sec, where default is micro sec");
  params.addParam<Real>("GBMobility", 2.0e-13, "GB mobility input in m^4/(J*s)");
  params.addParam<Real>("GBE", 1.0, "Grain boundary energy in J/m^2");
  params.addParam<Real>("Disloc_Den", 9.0e15, "Dislocation Density in m^-2");
  params.addParam<Real>("Elas_Mod", 2.50e10, "Elastic Modulus in J/m^3");
  params.addParam<Real>("Burg_vec", 3.0e-10, "Length of Burger Vector in m");
  params.addRequiredParam<UserObjectName>("grain_tracker",
                                          "The GrainTracker UserObject to get values from.");
  return params;
}

DeformedGrainMaterial::DeformedGrainMaterial(const InputParameters & parameters)
  : Material(parameters),
    _op_num(coupledComponents("v")),
    _vals(_op_num),
    _length_scale(getParam<Real>("length_scale")),
    _int_width(getParam<Real>("int_width")),
    _time_scale(getParam<Real>("time_scale")),
    _GBMobility(getParam<Real>("GBMobility")),
    _GBE(getParam<Real>("GBE")),
    _Disloc_Den(getParam<Real>("Disloc_Den")),
    _Elas_Mod(getParam<Real>("Elas_Mod")),
    _Burg_vec(getParam<Real>("Burg_vec")),
    _kappa(declareProperty<Real>("kappa_op")),
    _gamma(declareProperty<Real>("gamma_asymm")),
    _L(declareProperty<Real>("L")),
    _mu(declareProperty<Real>("mu")),
    _tgrad_corr_mult(declareProperty<Real>("tgrad_corr_mult")),
    _beta(declareProperty<Real>("beta")),
    _Disloc_Den_i(declareProperty<Real>("Disloc_Den_i")),
    _rho_eff(declareProperty<Real>("rho_eff")),
    _Def_Eng(declareProperty<Real>("Def_Eng")),
    _deformed_grain_num(getParam<unsigned int>("deformed_grain_num")),
    _grain_tracker(getUserObject<GrainTrackerInterface>("grain_tracker")),
    _kb(8.617343e-5),     // Boltzmann constant in eV/K
    _JtoeV(6.24150974e18) // Joule to eV conversion
{
  if (_op_num == 0)
    mooseError("Model requires op_num > 0");

  for (unsigned int i = 0; i < _op_num; ++i)
    _vals[i] = &coupledValue("v", i);
}

void
DeformedGrainMaterial::computeQpProperties()
{
  _Disloc_Den_i[_qp] = _Disloc_Den * (_length_scale * _length_scale);

  Real rho_i;
  Real rho0 = 0.0;
  Real SumEtai2 = 0.0;
  for (unsigned int i = 0; i < _op_num; ++i)
    SumEtai2 += (*_vals[i])[_qp] * (*_vals[i])[_qp];

  // calculate effective dislocation density and assign zero dislocation densities to undeformed
  // grains
  const auto & op_to_grains = _grain_tracker.getVarToFeatureVector(_current_elem->id());

  // loop over active OPs
  bool one_active = false;
  for (auto op_index = beginIndex(op_to_grains); op_index < op_to_grains.size(); ++op_index)
  {
    if (op_to_grains[op_index] == FeatureFloodCount::invalid_id)
      continue;

    one_active = true;
    auto grain_id = op_to_grains[op_index];

    if (grain_id >= _deformed_grain_num)
      rho_i = 0.0;
    else
      rho_i = _Disloc_Den_i[_qp];
    rho0 += rho_i * (*_vals[op_index])[_qp] * (*_vals[op_index])[_qp];
  }

  if (!one_active && _t_step > 0)
    mooseError("No active order parameters");

  _rho_eff[_qp] = rho0 / SumEtai2;
  if (_rho_eff[_qp] < 1e-9)
  {
    _rho_eff[_qp] = 0.0;
    _Disloc_Den_i[_qp] = 0.0;
  }

  _beta[_qp] = 0.5 * _Elas_Mod * _Burg_vec * _Burg_vec * _JtoeV * _length_scale;

  // Compute the deformation energy
  _Def_Eng[_qp] = _beta[_qp] * _rho_eff[_qp];

  const Real sigma = _GBE * _JtoeV * (_length_scale * _length_scale);
  const Real length_scale4 = _length_scale * _length_scale * _length_scale * _length_scale;
  const Real M_GB = _GBMobility * _time_scale / (_JtoeV * length_scale4);

  _L[_qp] = 4.0 / 3.0 * M_GB / _int_width;
  _kappa[_qp] = 3.0 / 4.0 * sigma * _int_width;
  _gamma[_qp] = 1.5;
  _mu[_qp] = 6.0 * sigma / _int_width;
  _tgrad_corr_mult[_qp] = _mu[_qp] * 9.0 / 8.0;
}
