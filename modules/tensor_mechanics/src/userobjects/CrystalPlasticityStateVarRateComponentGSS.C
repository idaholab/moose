/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CrystalPlasticityStateVarRateComponentGSS.h"

template <>
InputParameters
validParams<CrystalPlasticityStateVarRateComponentGSS>()
{
  InputParameters params = validParams<CrystalPlasticityStateVarRateComponent>();
  params.addParam<std::string>(
      "uo_slip_rate_name",
      "Name of slip rate property: Same as slip rate user object specified in input file.");
  params.addParam<std::string>("uo_state_var_name",
                               "Name of state variable property: Same as "
                               "state variable user object specified in input "
                               "file.");
  params.addParam<FileName>(
      "slip_sys_hard_prop_file_name",
      "",
      "Name of the file containing the values of hardness evolution parameters");
  params.addParam<std::vector<Real>>("hprops", "Hardening properties");
  params.addClassDescription("Phenomenological constitutive model state variable evolution rate "
                             "component base class.  Override the virtual functions in your class");
  return params;
}

CrystalPlasticityStateVarRateComponentGSS::CrystalPlasticityStateVarRateComponentGSS(
    const InputParameters & parameters)
  : CrystalPlasticityStateVarRateComponent(parameters),
    _mat_prop_slip_rate(
        getMaterialProperty<std::vector<Real>>(parameters.get<std::string>("uo_slip_rate_name"))),
    _mat_prop_state_var(
        getMaterialProperty<std::vector<Real>>(parameters.get<std::string>("uo_state_var_name"))),
    _slip_sys_hard_prop_file_name(getParam<FileName>("slip_sys_hard_prop_file_name")),
    _hprops(getParam<std::vector<Real>>("hprops"))
{
}

bool
CrystalPlasticityStateVarRateComponentGSS::calcStateVariableEvolutionRateComponent(
    unsigned int qp, std::vector<Real> & val) const
{
  val.assign(_variable_size, 0.0);

  Real r = _hprops[0];
  Real h0 = _hprops[1];
  Real tau_sat = _hprops[2];

  DenseVector<Real> hb(_variable_size);
  Real qab;
  Real a = _hprops[3]; // Kalidindi

  for (unsigned int i = 0; i < _variable_size; ++i)
    hb(i) = h0 * std::pow(std::abs(1.0 - _mat_prop_state_var[qp][i] / tau_sat), a) *
            copysign(1.0, 1.0 - _mat_prop_state_var[qp][i] / tau_sat);

  for (unsigned int i = 0; i < _variable_size; ++i)
    for (unsigned int j = 0; j < _variable_size; ++j)
    {
      unsigned int iplane, jplane;
      iplane = i / 3;
      jplane = j / 3;

      if (iplane == jplane) // Kalidindi
        qab = 1.0;
      else
        qab = r;

      val[i] += std::abs(_mat_prop_slip_rate[qp][j]) * qab * hb(j);
    }

  return true;
}
