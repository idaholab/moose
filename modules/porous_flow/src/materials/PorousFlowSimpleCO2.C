/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowSimpleCO2.h"

template<>
InputParameters validParams<PorousFlowSimpleCO2>()
{
  InputParameters params = validParams<PorousFlowFluidPropertiesBase>();
  params.addClassDescription("This Material calculates fluid properties for CO2");
  return params;
}

PorousFlowSimpleCO2::PorousFlowSimpleCO2(const InputParameters & parameters) :
    PorousFlowFluidPropertiesBase(parameters),

    _density_nodal(declareProperty<Real>("PorousFlow_fluid_phase_density" + _phase)),
    _density_nodal_old(declarePropertyOld<Real>("PorousFlow_fluid_phase_density" + _phase)),
    _ddensity_nodal_dp(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density" + _phase, _pressure_variable_name)),
    _ddensity_nodal_dt(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density" + _phase, _temperature_variable_name)),
    _density_qp(declareProperty<Real>("PorousFlow_fluid_phase_density_qp" + _phase)),
    _ddensity_qp_dp(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density_qp" + _phase, _pressure_variable_name)),
    _ddensity_qp_dt(declarePropertyDerivative<Real>("PorousFlow_fluid_phase_density_qp" + _phase, _temperature_variable_name)),
    _viscosity_nodal(declareProperty<Real>("PorousFlow_viscosity" + _phase)),
    _dviscosity_nodal_dt(declarePropertyDerivative<Real>("PorousFlow_viscosity" + _phase, _temperature_variable_name)),
    _Mco2(44.0e-3),
    _critical_pressure(7.3773e6),
    _critical_temperature(30.9782)
{
}

void
PorousFlowSimpleCO2::initQpStatefulProperties()
{
  _density_nodal[_qp] = density(_porepressure_nodal[_qp][_phase_num], _temperature_nodal[_qp]);
}

void
PorousFlowSimpleCO2::computeQpProperties()
{
  /// Density and derivatives wrt pressure and temperature at the nodes
  _density_nodal[_qp] = density(_porepressure_nodal[_qp][_phase_num], _temperature_nodal[_qp]);
  _ddensity_nodal_dp[_qp] = dDensity_dP(_porepressure_nodal[_qp][_phase_num], _temperature_nodal[_qp]);
  _ddensity_nodal_dt[_qp] = dDensity_dT(_porepressure_nodal[_qp][_phase_num], _temperature_nodal[_qp]);

  /// Density and derivatives wrt pressure and temperature at the qps
  _density_qp[_qp] = density(_porepressure_qp[_qp][_phase_num], _temperature_qp[_qp]);
  _ddensity_qp_dp[_qp] = dDensity_dP(_porepressure_qp[_qp][_phase_num], _temperature_qp[_qp]);
  _ddensity_qp_dt[_qp] = dDensity_dT(_porepressure_qp[_qp][_phase_num], _temperature_qp[_qp]);

  /// Viscosity and derivative wrt temperature at the nodes
  _viscosity_nodal[_qp] = viscosity(_porepressure_nodal[_qp][_phase_num], _temperature_nodal[_qp], _density_nodal[_qp]);
  _dviscosity_nodal_dt[_qp] = dViscosity_dT(_porepressure_nodal[_qp][_phase_num], _temperature_nodal[_qp], _density_nodal[_qp]);
}

Real
PorousFlowSimpleCO2::density(Real pressure, Real temperature) const
{
  Real rho;

  if (pressure <= _critical_pressure)
    rho = gasDensity(pressure, temperature);

  else // if (pressure > _critical_pressure)
    rho = supercriticalDensity(pressure, temperature);

  return rho;
}

Real
PorousFlowSimpleCO2::viscosity(Real pressure, Real temperature, Real density) const
{
  Real mu;

  if (pressure <= _critical_pressure)
    mu = gasViscosity(temperature, density);

  else // if (pressure > _critical_pressure)
    mu = supercriticalViscosity(pressure, temperature);

  return mu;
}

Real
PorousFlowSimpleCO2::dDensity_dP(Real pressure, Real temperature) const
{
  Real drho;

  if (pressure <= _critical_pressure)
    drho = dGasDensity_dP(pressure, temperature);

  else // if (pressure > _critical_pressure)
    drho = dSupercriticalDensity_dP(pressure, temperature);

  return drho;
}

Real
PorousFlowSimpleCO2::dDensity_dT(Real pressure, Real temperature) const
{
  Real drho;

  if (pressure <= _critical_pressure)
    drho = dGasDensity_dT(pressure, temperature);

  else // if (pressure > _critical_pressure)
    drho = dSupercriticalDensity_dT(pressure, temperature);

  return drho;
}

Real
PorousFlowSimpleCO2::gasViscosity(Real temperature, Real density) const
{
  /*
   * Viscosity of supercritical CO2. From Ouyang, New correlations for predicting
   * the density and viscosity of supercritical Carbon Dioxide under conditions
   * expected in Carbon Capture and Sequestration operations, The Open Petroleum
   * Engineering Journal, 4, 13-21 (2011)
   */
  const Real tk = temperature + _t_c2k;
  const Real tstar = tk / 251.196;
  const Real a[5] = {0.235156, -0.491266, 5.211155e-2, 5.347906e-2, -1.537102e-2};
  const Real d[5] = {0.4071119e-2, 0.7198037e-4, 0.2411697e-16, 0.2971072e-22, -0.1627888e-22};
  const int j[5] = {1, 1, 4, 1, 2};
  const int i[5] = {1, 2, 6, 8, 8};

  // Zero-denisty viscosity
  Real sum = 0.0;

  for (int n = 0; n < 5; ++n)
    sum += a[n] * std::pow(std::log(tstar), n);

  const Real theta = std::exp(sum);
  const Real mu0 = 1.00697 * std::sqrt(tk) / theta;

  Real b[5];
  for (unsigned int n = 0; n < 5; ++n)
    b[n] = d[n] /  std::pow(tstar, j[n] - 1);

  // Excess viscosity due to density
  Real mu = 0.0;

  for (unsigned int n = 0; n < 5; ++n)
    mu += b[n] * std::pow(density, i[n]);

  return (mu0 + mu) * 1e-6; // convert to Pa.s
}

Real
PorousFlowSimpleCO2::supercriticalViscosity(Real pressure, Real temperature) const
{
  const Real b0[5] = {-1.958098980443E+01, 1.123243298270E+00, -2.320378874100E-02,
    2.067060943050E-04, -6.740205984528E-07};
  const Real b1[5] = {4.187280585109E-02, -2.425666731623E-03, 5.051177210444E-05,
    -4.527585394282E-07, 1.483580144144E-09};
  const Real b2[5] = {-3.164424775231E-05, 1.853493293079E-06, -3.892243662924E-08,
    3.511599795831E-10, -1.156613338683E-12};
  const Real b3[5] = {1.018084854204E-08, -6.013995738056E-10, 1.271924622771E-11,
    -1.154170663233E-13, 3.819260251596E-16};
  const Real b4[5] = {-1.185834697489E-12, 7.052301533772E-14, -1.500321307714E-15,
    1.368104294236E-17, -4.545472651918E-20};

  const Real c0[5] = {1.856798626054E-02, 3.083186834281E-03, -1.004022090988E-04,
    8.331453343531E-07, -1.824126204417E-09};
  const Real c1[5] = {6.519276827948E-05, -3.174897980949E-06, 7.524167185714E-08,
    -6.141534284471E-10, 1.463896995503E-12};
  const Real c2[5] = {-1.310632653461E-08, 7.702474418324E-10, -1.830098887313E-11,
    1.530419648245E-13, -3.852361658746E-16};
  const Real c3[5] = {1.335772487425E-12, -8.113168443709E-14, 1.921794651400E-15,
    -1.632868926659E-17, 4.257160059035E-20};
  const Real c4[5] = {-5.047795395464E-17, 3.115707980951E-18, -7.370406590957E-20,
    6.333570782917E-22, -1.691344581198E-24};

  Real a0, a1, a2, a3, a4;

  const Real t1 = temperature;
  const Real t2 = t1 * t1;
  const Real t3 = t2 * t1;
  const Real t4 = t3 * t1;

  // Correlation uses pressure in psia
  const Real pa2psia = 1.45037738007e-4;
  const Real p1 = pressure * pa2psia;
  const Real p2 = p1 * p1;
  const Real p3 = p2 * p1;
  const Real p4 = p3 * p1;

  if (p1 <= 3000)
  {
    a0 = b0[0] + b0[1] * t1 + b0[2] * t2 + b0[3] * t3 + b0[4] * t4;
    a1 = b1[0] + b1[1] * t1 + b1[2] * t2 + b1[3] * t3 + b1[4] * t4;
    a2 = b2[0] + b2[1] * t1 + b2[2] * t2 + b2[3] * t3 + b2[4] * t4;
    a3 = b3[0] + b3[1] * t1 + b3[2] * t2 + b3[3] * t3 + b3[4] * t4;
    a4 = b4[0] + b4[1] * t1 + b4[2] * t2 + b4[3] * t3 + b4[4] * t4;
   }

  else // if (p1 > 3000)
  {
    a0 = c0[0] + c0[1] * t1 + c0[2] * t2 + c0[3] * t3 + c0[4] * t4;
    a1 = c1[0] + c1[1] * t1 + c1[2] * t2 + c1[3] * t3 + c1[4] * t4;
    a2 = c2[0] + c2[1] * t1 + c2[2] * t2 + c2[3] * t3 + c2[4] * t4;
    a3 = c3[0] + c3[1] * t1 + c3[2] * t2 + c3[3] * t3 + c3[4] * t4;
    a4 = c4[0] + c4[1] * t1 + c4[2] * t2 + c4[3] * t3 + c4[4] * t4;
  }

 const Real mu = a0 + a1 * p1 + a2 * p2 + a3 * p3 + a4 * p4;

 return mu * 1e-3; //cP to Pa.s
}

Real
PorousFlowSimpleCO2::partialDensity(Real temperature) const
{
  /*
   * Partial density of dissolved CO2. From Garcia, Density of aqueous
   * solutions of CO2, LBNL-49023 (2001).
   */
  const Real t2 = temperature * temperature;
  const Real t3 = t2 * temperature;

  const Real V = 37.51 - 9.585e-2 * temperature + 8.74e-4 * t2 - 5.044e-7 * t3;

  return 1.e6 * _Mco2 / V;
}

Real
PorousFlowSimpleCO2::supercriticalDensity(Real pressure, Real temperature) const
{
  /*
   * Density of supercritical CO2. From Ouyang, New correlations for predicting
   * the density and viscosity of supercritical Carbon Dioxide under conditions
   * expected in Carbon Capture and Sequestration operations, The Open Petroleum
   * Engineering Journal, 4, 13-21 (2011)
   */
  const Real b0[5] = {-2.148322085348e5, 1.168116599408e4, -2.302236659392e2,
    1.967428940167, -6.184842764145e-3};
  const Real b1[5] = {4.757146002428e2, -2.619250287624e1, 5.215134206837e-1,
    -4.494511089838e-3, 1.423058795982e-5};
  const Real b2[5] = {-3.713900186613e-1, 2.072488876536e-2, -4.169082831078e-4,
    3.622975674137e-6, -1.155050860329e-8};
  const Real b3[5] = {1.228907393482e-4, -6.930063746226e-6, 1.406317206628e-7,
    -1.230995287169e-9, 3.948417428040e-12};
  const Real b4[5] = {-1.466408011784e-8, 8.338008651366e-10, -1.704242447194e-11,
    1.500878861807e-13, -4.838826574173e-16};

  const Real c0[5] = {6.897382693936e2, 2.730479206931, -2.254102364542e-2,
    -4.651196146917e-3, 3.439702234956e-5};
  const Real c1[5] = {2.213692462613e-1, -6.547268255814e-3, 5.982258882656e-5,
    2.274997412526e-6, -1.888361337660e-8};
  const Real c2[5] = {-5.118724890479e-5, 2.019697017603e-6, -2.311332097185e-8,
    -4.079557404679e-10, 3.893599641874e-12};
  const Real c3[5] ={5.517971126745e-9, -2.415814703211e-10, 3.121603486524e-12,
    3.171271084870e-14, -3.560785550401e-16};
  const Real c4[5] = {-2.184152941323e-13, 1.010703706059e-14, -1.406620681883e-16,
    -8.957731136447e-19, 1.215810469539e-20};

  Real a0, a1, a2, a3, a4;

  const Real t1 = temperature;
  const Real t2 = t1 * t1;
  const Real t3 = t2 * t1;
  const Real t4 = t3 * t1;

  // Correlation uses pressure in psia
  const Real pa2psia = 1.45037738007e-4;
  const Real p1 = pressure * pa2psia;
  const Real p2 = p1 * p1;
  const Real p3 = p2 * p1;
  const Real p4 = p3 * p1;

  if (p1 <= 3000)
  {
     a0 = b0[0] + b0[1] * t1 + b0[2] * t2 + b0[3] * t3 + b0[4] * t4;
     a1 = b1[0] + b1[1] * t1 + b1[2] * t2 + b1[3] * t3 + b1[4] * t4;
     a2 = b2[0] + b2[1] * t1 + b2[2] * t2 + b2[3] * t3 + b2[4] * t4;
     a3 = b3[0] + b3[1] * t1 + b3[2] * t2 + b3[3] * t3 + b3[4] * t4;
     a4 = b4[0] + b4[1] * t1 + b4[2] * t2 + b4[3] * t3 + b4[4] * t4;
   }

   else // if (p1 > 3000)
   {
     a0 = c0[0] + c0[1] * t1 + c0[2] * t2 + c0[3] * t3 + c0[4] * t4;
     a1 = c1[0] + c1[1] * t1 + c1[2] * t2 + c1[3] * t3 + c1[4] * t4;
     a2 = c2[0] + c2[1] * t1 + c2[2] * t2 + c2[3] * t3 + c2[4] * t4;
     a3 = c3[0] + c3[1] * t1 + c3[2] * t2 + c3[3] * t3 + c3[4] * t4;
     a4 = c4[0] + c4[1] * t1 + c4[2] * t2 + c4[3] * t3 + c4[4] * t4;
   }

  return a0 + a1 * p1 + a2 * p2 + a3 * p3 + a4 * p4;
}

Real
PorousFlowSimpleCO2::gasDensity(Real pressure, Real temperature) const
{
  const Real tk = temperature + _t_c2k;
  const Real tc = std::pow((tk * 1.e-2), 10./3.);
  const Real pc = pressure * 1.e-6;

  const Real vc1 = 1.8882e-4 * tk;
  const Real vc2 = - pc * (8.24e-2 + 1.249e-2 * pc) / tc;

  return pc / (vc1 + vc2);
}

Real
PorousFlowSimpleCO2::dGasDensity_dP(Real pressure, Real temperature) const
{
  const Real tk = temperature + _t_c2k;
  const Real tc = std::pow((tk * 1.e-2), 10./3.);
  const Real pc = pressure * 1.e-6;

  const Real vc1 = 1.8882e-4 * tk;
  const Real vc2 = - pc * (8.24e-2 + 1.249e-2 * pc) / tc;
  const Real dvc2 = - (8.24e-2 + 2.498e-2 * pc) / tc;

  return (vc1 + vc2 - pc * dvc2) / ((vc1 + vc2) * (vc1 + vc2)) * 1e-6;
}

Real
PorousFlowSimpleCO2::dGasDensity_dT(Real pressure, Real temperature) const
{
  const Real tk = temperature + _t_c2k;
  const Real tc = std::pow((tk * 1.e-2), 10./3.);
  const Real pc = pressure * 1.e-6;

  const Real vc1 = 1.8882e-4 * tk;
  const Real vc2 = - pc * (8.24e-2 + 1.249e-2 * pc) / tc;
  const Real dtc = (0.1 / 3.0) * std::pow((tk * 1.e-2), 7./3.);
  const Real dvc1 = 1.8882e-4;
  const Real dvc2 = - vc2 / tc;

  return - pc / (vc1 + vc2) / (vc1 + vc2) * (dvc1 + dvc2 * dtc);
}

Real
PorousFlowSimpleCO2::dSupercriticalDensity_dP(Real pressure, Real temperature) const
{
  const Real b1[5] = {4.757146002428e2, -2.619250287624e1, 5.215134206837e-1,
    -4.494511089838e-3, 1.423058795982e-5};
  const Real b2[5] = {-3.713900186613e-1, 2.072488876536e-2, -4.169082831078e-4,
    3.622975674137e-6, -1.155050860329e-8};
  const Real b3[5] = {1.228907393482e-4, -6.930063746226e-6, 1.406317206628e-7,
    -1.230995287169e-9, 3.948417428040e-12};
  const Real b4[5] = {-1.466408011784e-8, 8.338008651366e-10, -1.704242447194e-11,
    1.500878861807e-13, -4.838826574173e-16};

  const Real c1[5] = {2.213692462613e-1, -6.547268255814e-3, 5.982258882656e-5,
    2.274997412526e-6, -1.888361337660e-8};
  const Real c2[5] = {-5.118724890479e-5, 2.019697017603e-6, -2.311332097185e-8,
    -4.079557404679e-10, 3.893599641874e-12};
  const Real c3[5] ={5.517971126745e-9, -2.415814703211e-10, 3.121603486524e-12,
    3.171271084870e-14, -3.560785550401e-16};
  const Real c4[5] = {-2.184152941323e-13, 1.010703706059e-14, -1.406620681883e-16,
    -8.957731136447e-19, 1.215810469539e-20};

  Real a1, a2, a3, a4;

  const Real t1 = temperature;
  const Real t2 = t1 * t1;
  const Real t3 = t2 * t1;
  const Real t4 = t3 * t1;

  // Correlation uses pressure in psia
  const Real pa2psia = 1.45037738007e-4;
  const Real p1 = pressure * pa2psia;
  const Real p2 = p1 * p1;
  const Real p3 = p2 * p1;

  if (p1 <= 3000)
  {
     a1 = b1[0] + b1[1] * t1 + b1[2] * t2 + b1[3] * t3 + b1[4] * t4;
     a2 = b2[0] + b2[1] * t1 + b2[2] * t2 + b2[3] * t3 + b2[4] * t4;
     a3 = b3[0] + b3[1] * t1 + b3[2] * t2 + b3[3] * t3 + b3[4] * t4;
     a4 = b4[0] + b4[1] * t1 + b4[2] * t2 + b4[3] * t3 + b4[4] * t4;
   }

   else // if (p1 > 3000)
   {
     a1 = c1[0] + c1[1] * t1 + c1[2] * t2 + c1[3] * t3 + c1[4] * t4;
     a2 = c2[0] + c2[1] * t1 + c2[2] * t2 + c2[3] * t3 + c2[4] * t4;
     a3 = c3[0] + c3[1] * t1 + c3[2] * t2 + c3[3] * t3 + c3[4] * t4;
     a4 = c4[0] + c4[1] * t1 + c4[2] * t2 + c4[3] * t3 + c4[4] * t4;
   }

  return (a1 + 2.0 * a2 * p1 + 3.0 * a3 * p2 + 4.0 * a4 * p3) * pa2psia;
}

Real
PorousFlowSimpleCO2::dSupercriticalDensity_dT(Real pressure, Real temperature) const
{
  const Real b0[5] = {-2.148322085348e5, 1.168116599408e4, -2.302236659392e2,
    1.967428940167, -6.184842764145e-3};
  const Real b1[5] = {4.757146002428e2, -2.619250287624e1, 5.215134206837e-1,
    -4.494511089838e-3, 1.423058795982e-5};
  const Real b2[5] = {-3.713900186613e-1, 2.072488876536e-2, -4.169082831078e-4,
    3.622975674137e-6, -1.155050860329e-8};
  const Real b3[5] = {1.228907393482e-4, -6.930063746226e-6, 1.406317206628e-7,
    -1.230995287169e-9, 3.948417428040e-12};
  const Real b4[5] = {-1.466408011784e-8, 8.338008651366e-10, -1.704242447194e-11,
    1.500878861807e-13, -4.838826574173e-16};

  const Real c0[5] = {6.897382693936e2, 2.730479206931, -2.254102364542e-2,
    -4.651196146917e-3, 3.439702234956e-5};
  const Real c1[5] = {2.213692462613e-1, -6.547268255814e-3, 5.982258882656e-5,
    2.274997412526e-6, -1.888361337660e-8};
  const Real c2[5] = {-5.118724890479e-5, 2.019697017603e-6, -2.311332097185e-8,
    -4.079557404679e-10, 3.893599641874e-12};
  const Real c3[5] ={5.517971126745e-9, -2.415814703211e-10, 3.121603486524e-12,
    3.171271084870e-14, -3.560785550401e-16};
  const Real c4[5] = {-2.184152941323e-13, 1.010703706059e-14, -1.406620681883e-16,
    -8.957731136447e-19, 1.215810469539e-20};

  Real a0, a1, a2, a3, a4;

  const Real t1 = temperature;
  const Real t2 = t1 * t1;
  const Real t3 = t2 * t1;

  // Correlation uses pressure in psia
  const Real pa2psia = 1.45037738007e-4;
  const Real p1 = pressure * pa2psia;
  const Real p2 = p1 * p1;
  const Real p3 = p2 * p1;
  const Real p4 = p3 * p1;

  if (p1 <= 3000)
  {
     a0 = b0[1]  + 2.0 * b0[2] * t1 + 3.0 * b0[3] * t2 + 4.0 * b0[4] * t3;
     a1 = b1[1]  + 2.0 * b1[2] * t1 + 3.0 * b1[3] * t2 + 4.0 * b1[4] * t3;
     a2 = b2[1]  + 2.0 * b2[2] * t1 + 3.0 * b2[3] * t2 + 4.0 * b2[4] * t3;
     a3 = b3[1]  + 2.0 * b3[2] * t1 + 3.0 * b3[3] * t2 + 4.0 * b3[4] * t3;
     a4 = b4[1]  + 2.0 * b4[2] * t1 + 3.0 * b4[3] * t2 + 4.0 * b4[4] * t3;
   }

   else // if (p1 > 3000)
   {
     a0 = c0[1]  + 2.0 * c0[2] * t1 + 3.0 * c0[3] * t2 + 4.0 * c0[4] * t3;
     a1 = c1[1]  + 2.0 * c1[2] * t1 + 3.0 * c1[3] * t2 + 4.0 * c1[4] * t3;
     a2 = c2[1]  + 2.0 * c2[2] * t1 + 3.0 * c2[3] * t2 + 4.0 * c2[4] * t3;
     a3 = c3[1]  + 2.0 * c3[2] * t1 + 3.0 * c3[3] * t2 + 4.0 * c3[4] * t3;
     a4 = c4[1]  + 2.0 * c4[2] * t1 + 3.0 * c4[3] * t2 + 4.0 * c4[4] * t3;
   }

  return a0 + a1 * p1 + a2 * p2 + a3 * p3 + a4 * p4;
}

Real
PorousFlowSimpleCO2::dViscosity_dDensity(Real pressure, Real temperature, Real density) const
{
  Real dmu_drho;

  if (pressure <= _critical_pressure)
    dmu_drho = dGasViscosity_dDensity(temperature, density);

  else // if (pressure > _critical_pressure)
    dmu_drho = dSupercriticalViscosity_dDensity(pressure, temperature);

  return dmu_drho;
}

Real
PorousFlowSimpleCO2::dGasViscosity_dDensity(Real temperature, Real density) const
{
  const Real tk = temperature + _t_c2k;
  const Real tstar = tk / 251.196;
  const Real d[5] = {0.4071119e-2, 0.7198037e-4, 0.2411697e-16, 0.2971072e-22, -0.1627888e-22};
  const int j[5] = {1, 1, 4, 1, 2};
  const int i[5] = {1, 2, 6, 8, 8};

  Real b[5];
  for (unsigned int n = 0; n < 5; ++n)
    b[n] = d[n] /  std::pow(tstar, j[n] - 1);

  // Excess viscosity due to density
  Real dmu = 0.0;

  for (unsigned int n = 0; n < 5; ++n)
    dmu += b[n] * i[n] * std::pow(density, i[n] - 1);

  return  dmu * 1e-6; // convert to Pa.s
}

Real
PorousFlowSimpleCO2::dSupercriticalViscosity_dDensity(Real pressure, Real temperature) const
{
  Real dmu_drho = 0.0;

  /**
   * The correlation for supercritical CO2 gives viscosity as a function of pressure. Therefore, The
   * derivative of viscosity wrt density is given by the chain rule.
   * Note that if d(density)/d(pressure) = 0, so should d(viscosity)/d(density)
   */
  const Real drho_dp = dSupercriticalDensity_dP(pressure, temperature);

  if (drho_dp != 0.0)
    dmu_drho += dSupercriticalViscosity_dP(pressure, temperature) / drho_dp;

  return dmu_drho;
}

Real
PorousFlowSimpleCO2::dSupercriticalViscosity_dP(Real pressure, Real temperature) const
{
  const Real b1[5] = {4.187280585109E-02, -2.425666731623E-03, 5.051177210444E-05,
    -4.527585394282E-07, 1.483580144144E-09};
  const Real b2[5] = {-3.164424775231E-05, 1.853493293079E-06, -3.892243662924E-08,
    3.511599795831E-10, -1.156613338683E-12};
  const Real b3[5] = {1.018084854204E-08, -6.013995738056E-10, 1.271924622771E-11,
    -1.154170663233E-13, 3.819260251596E-16};
  const Real b4[5] = {-1.185834697489E-12, 7.052301533772E-14, -1.500321307714E-15,
    1.368104294236E-17, -4.545472651918E-20};

  const Real c1[5] = {6.519276827948E-05, -3.174897980949E-06, 7.524167185714E-08,
    -6.141534284471E-10, 1.463896995503E-12};
  const Real c2[5] = {-1.310632653461E-08, 7.702474418324E-10, -1.830098887313E-11,
    1.530419648245E-13, -3.852361658746E-16};
  const Real c3[5] = {1.335772487425E-12, -8.113168443709E-14, 1.921794651400E-15,
    -1.632868926659E-17, 4.257160059035E-20};
  const Real c4[5] = {-5.047795395464E-17, 3.115707980951E-18, -7.370406590957E-20,
    6.333570782917E-22, -1.691344581198E-24};

  Real a1, a2, a3, a4;

  const Real t1 = temperature;
  const Real t2 = t1 * t1;
  const Real t3 = t2 * t1;
  const Real t4 = t3 * t1;

  // Correlation uses pressure in psia
  const Real pa2psia = 1.45037738007e-4;
  const Real p1 = pressure * pa2psia;
  const Real p2 = p1 * p1;
  const Real p3 = p2 * p1;

  if (p1 <= 3000)
  {
    a1 = b1[0] + b1[1] * t1 + b1[2] * t2 + b1[3] * t3 + b1[4] * t4;
    a2 = b2[0] + b2[1] * t1 + b2[2] * t2 + b2[3] * t3 + b2[4] * t4;
    a3 = b3[0] + b3[1] * t1 + b3[2] * t2 + b3[3] * t3 + b3[4] * t4;
    a4 = b4[0] + b4[1] * t1 + b4[2] * t2 + b4[3] * t3 + b4[4] * t4;
   }

  else // if (p1 > 3000)
  {
    a1 = c1[0] + c1[1] * t1 + c1[2] * t2 + c1[3] * t3 + c1[4] * t4;
    a2 = c2[0] + c2[1] * t1 + c2[2] * t2 + c2[3] * t3 + c2[4] * t4;
    a3 = c3[0] + c3[1] * t1 + c3[2] * t2 + c3[3] * t3 + c3[4] * t4;
    a4 = c4[0] + c4[1] * t1 + c4[2] * t2 + c4[3] * t3 + c4[4] * t4;
  }

 const Real dmu = a1 + 2.0 * a2 * p1 + 3.0 * a3 * p2 + 4.0 * a4 * p3;

 return dmu * pa2psia * 1e-3; // cP to Pa.s
}

Real
PorousFlowSimpleCO2::dViscosity_dT(Real /*pressure*/, Real /*temperature*/, Real /*density*/) const
{
  //TODO: implement
  return 0.0;
}

std::vector<Real>
PorousFlowSimpleCO2::henryConstants() const
{
  /*
   * Henry's law constant coefficients for dissolution of CO2 into water.
   * From Guidelines on the Henry's constant and vapour
   * liquid distribution constant for gases in H20 and D20 at high
   * temperatures, IAPWS (2004).
   */
  std::vector<Real> co2henry;
  co2henry.push_back(-8.55445);
  co2henry.push_back(4.01195);
  co2henry.push_back(9.52345);

  return co2henry;
}
