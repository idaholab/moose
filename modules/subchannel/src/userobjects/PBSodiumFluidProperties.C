/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

#include "PBSodiumFluidProperties.h"

registerMooseObject("SubChannelApp", PBSodiumFluidProperties);

// Use the array to initialize the const static vector
const Real sodium_T[] = {
    388.15,  398.15,  408.15,  418.15,  428.15,  438.15,  448.15,  458.15,  468.15,  478.15,
    488.15,  498.15,  508.15,  518.15,  528.15,  538.15,  548.15,  558.15,  568.15,  578.15,
    588.15,  598.15,  608.15,  618.15,  628.15,  638.15,  648.15,  658.15,  668.15,  678.15,
    688.15,  698.15,  708.15,  718.15,  728.15,  738.15,  748.15,  758.15,  768.15,  778.15,
    788.15,  798.15,  808.15,  818.15,  828.15,  838.15,  848.15,  858.15,  868.15,  878.15,
    888.15,  898.15,  908.15,  918.15,  928.15,  938.15,  948.15,  958.15,  968.15,  978.15,
    988.15,  998.15,  1008.15, 1018.15, 1028.15, 1038.15, 1048.15, 1058.15, 1068.15, 1078.15,
    1088.15, 1098.15, 1108.15, 1118.15, 1128.15, 1138.15, 1148.15};
// sodium temperature vector corresponding to _e_vec enthalpy vector
const std::vector<Real>
    PBSodiumFluidProperties::_temperature_vec(sodium_T,
                                              sodium_T + sizeof(sodium_T) / sizeof(sodium_T[0]));

const Real sodium_e[] = {
    492755,      505884,      518995,      532089,      545166,      558227,      571270,
    584298,      597309,      610305,      623286,      636252,      649203,      662139,
    675061,      687970,      700865,      713747,      726616,      739472,      752316,
    765148,      777969,      790778,      803576,      816363,      829140,      841907,
    854665,      867413,      880151,      892882,      905603,      918317,      931023,
    943721,      956412,      969097,      981775,      994447,      1.00711e+06, 1.01977e+06,
    1.03243e+06, 1.04508e+06, 1.05773e+06, 1.07037e+06, 1.08301e+06, 1.09565e+06, 1.10828e+06,
    1.12091e+06, 1.13354e+06, 1.14617e+06, 1.15879e+06, 1.17142e+06, 1.18404e+06, 1.19667e+06,
    1.20929e+06, 1.22191e+06, 1.23454e+06, 1.24717e+06, 1.25979e+06, 1.27243e+06, 1.28506e+06,
    1.29769e+06, 1.31033e+06, 1.32298e+06, 1.33562e+06, 1.34828e+06, 1.36093e+06, 1.3736e+06,
    1.38626e+06, 1.39894e+06, 1.41162e+06, 1.42431e+06, 1.43701e+06, 1.44971e+06, 1.46243e+06};
// sodium enthalpy vector corresponding to _temperature_vec temperature vector
const std::vector<Real>
    PBSodiumFluidProperties::_e_vec(sodium_e, sodium_e + sizeof(sodium_e) / sizeof(sodium_e[0]));

InputParameters
PBSodiumFluidProperties::validParams()
{
  InputParameters params = SinglePhaseFluidProperties::validParams();
  params.addParam<Real>("p_0", 1.e5, "Reference pressure");
  params.addClassDescription(
      "Class that provides the methods that realize the equations of state for Liquid Sodium");
  return params;
}

PBSodiumFluidProperties::PBSodiumFluidProperties(const InputParameters & parameters)
  : SinglePhaseFluidProperties(parameters), _p_0(getParam<Real>("p_0"))
{
  _T0 = 628.15;
  // sodium boiling temprature
  _Tmax = 1154.55;
  _Tmin = 373.15;
  _H0 = cp_from_p_T(_p_0, _T0) * _T0;

  _Cp_Tmax = cp_from_p_T(_p_0, _Tmax);
  _Cp_Tmin = cp_from_p_T(_p_0, _Tmin);
  _H_Tmax = h_from_p_T(_p_0, _Tmax);
  _H_Tmin = h_from_p_T(_p_0, _Tmin);
}

Real
PBSodiumFluidProperties::rho_from_p_T(Real /*pressure*/, Real temperature) const
{
  Real A12 = 1.00423e3;
  Real A13 = -0.21390;
  Real A14 = -1.1046e-5;
  return (A12 + A13 * temperature + A14 * temperature * temperature);
}

void
PBSodiumFluidProperties::rho_from_p_T(
    Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  rho = rho_from_p_T(pressure, temperature);
  drho_dp = 0;
  Real A13 = -0.21390;
  Real A14 = -1.1046e-5;
  drho_dT = (A13 + 2.0 * A14 * temperature);
}

Real
PBSodiumFluidProperties::h_from_p_T(Real /*pressure*/, Real temperature) const
{
  if (temperature > _Tmax + 1.e-3)
    return _H_Tmax + _Cp_Tmax * (temperature - _Tmax);
  else if (temperature < _Tmin - 1.e-3)
    return _H_Tmin + _Cp_Tmin * (temperature - _Tmin);
  else
    return _H0 + F_enthalpy(temperature) - F_enthalpy(_T0);
}

Real
PBSodiumFluidProperties::beta_from_p_T(Real /*pressure*/, Real temperature) const
{
  Real A42 = 2.5156e-6;
  Real A43 = 0.79919;
  Real A44 = -6.9716e2;
  Real A45 = 3.3140e5;
  Real A46 = -7.0502e7;
  Real A47 = 5.4920e9;
  Real dt = 2503.3 - temperature;
  return (A42 + A43 / dt + A44 / dt / dt + A45 / (dt * dt * dt) + A46 / (dt * dt * dt * dt) +
          A47 / (dt * dt * dt * dt * dt));
}

Real
PBSodiumFluidProperties::cv_from_p_T(Real pressure, Real temperature) const
{
  // Consistent with SAM model cv is assumed to be equal to cp
  // cv is currentl not being used in subchannel algorithm.
  return cp_from_p_T(pressure, temperature);
}

Real
PBSodiumFluidProperties::cp_from_p_T(Real /*pressure*/, Real temperature) const
{
  if (temperature < 388.15)
  {
    temperature = 388.15;
    _console << "Warning - minimum temperature in cp caluclation bounded to 388.15 K \n";
  }
  if (temperature > 1148.15)
  {
    temperature = 1148.15;
    _console << "Warning - maximum temperature bounded in cp calculation to 1148.15 \n";
  }
  temperature = temperature_correction(temperature);
  Real A28 = 7.3898e5;
  Real A29 = 3.154e5;
  Real A30 = 1.1340e3;
  Real A31 = -2.2153e-1;
  Real A32 = 1.1156e-4;
  Real dt = 2503.3 - temperature;
  return (A28 / dt / dt + A29 / dt + A30 + A31 * dt + A32 * dt * dt);
}

void
PBSodiumFluidProperties::cp_from_p_T(
    Real pressure, Real temperature, Real & cp, Real & dcp_dp, Real & dcp_dT) const
{
  cp = cp_from_p_T(pressure, temperature);
  dcp_dp = 0;
  Real A28 = 7.3898e5;
  Real A29 = 3.154e5;
  Real A31 = -2.2153e-1;
  Real A32 = 1.1156e-4;
  Real dt = 2503.3 - temperature;
  if (temperature < _Tmax && temperature > _Tmin)
    dcp_dT = (2 * A28 / dt / dt / dt + A29 / dt / dt - A31 - 2 * A32 * dt);
  else
    dcp_dT = 0.;
}

Real
PBSodiumFluidProperties::mu_from_p_T(Real /*pressure*/, Real temperature) const
{
  Real A52 = 3.6522e-5;
  Real A53 = 0.16626;
  Real A54 = -4.56877e1;
  Real A55 = 2.8733e4;
  return (A52 + A53 / temperature + A54 / temperature / temperature +
          A55 / (temperature * temperature * temperature));
}

Real
PBSodiumFluidProperties::mu_from_rho_T(Real /*rho*/, Real temperature) const
{
  Real A52 = 3.6522e-5;
  Real A53 = 0.16626;
  Real A54 = -4.56877e1;
  Real A55 = 2.8733e4;
  return (A52 + A53 / temperature + A54 / temperature / temperature +
          A55 / (temperature * temperature * temperature));
}

Real
PBSodiumFluidProperties::k_from_p_T(Real /*pressure*/, Real temperature) const
{
  if (temperature < 388.15)
  {
    temperature = 388.15;
    _console << "Warning - minimum temperature in thermal conductivity caluclation bounded to "
                "388.15 K \n";
  }
  if (temperature > 1148.15)
  {
    temperature = 1148.15;
    _console << "Warning - maximum temperature bounded in thermal conductivity calculation to "
                "1148.15 \n";
  }
  Real A48 = 1.1045e2;
  Real A49 = -6.5112e-2;
  Real A50 = 1.5430e-5;
  Real A51 = -2.4617e-9;
  return (A48 + A49 * temperature + A50 * temperature * temperature +
          A51 * temperature * temperature * temperature);
}

Real
PBSodiumFluidProperties::F_enthalpy(Real temperature) const
{
  Real A28 = 7.3898e5;
  Real A29 = 3.154e5;
  Real A30 = 1.1340e3;
  Real A31 = -2.2153e-1;
  Real A32 = 1.1156e-4;
  Real dt = 2503.3 - temperature;

  return -(-A28 / dt + A29 * std::log(dt) + A30 * dt + 0.5 * A31 * dt * dt +
           1.0 / 3 * A32 * dt * dt * dt);
}

Real
PBSodiumFluidProperties::temperature_correction(Real & temperature) const
{
  if (temperature > _Tmax)
    return _Tmax;
  else if (temperature < _Tmin)
    return _Tmin;
  else
    return temperature;
}

Real
PBSodiumFluidProperties::T_from_p_h(Real temperature, Real enthalpy) const
{
  // the algorithm were made fully compliant with the enthalpy correlations above.
  // Consistent with the approach in SAM, it ignores that sodium boiling.
  // This part will be revisited in future.
  if (enthalpy > _H_Tmax)
  {
    temperature = (enthalpy - _H_Tmax) / _Cp_Tmax + _Tmax;
  }
  else if (enthalpy < _H_Tmin)
  {
    temperature = (enthalpy - _H_Tmin) / _Cp_Tmin + _Tmin;
  }
  else
  {
    for (unsigned int i = 0; i < _e_vec.size() - 1; i++)
    {
      if (enthalpy > _e_vec[i] && enthalpy <= _e_vec[i + 1])
      {
        temperature = _temperature_vec[i] + (enthalpy - _e_vec[i]) / (_e_vec[i + 1] - _e_vec[i]) *
                                                (_temperature_vec[i + 1] - _temperature_vec[i]);
        break;
      }
    }
  }
  return temperature;
}
