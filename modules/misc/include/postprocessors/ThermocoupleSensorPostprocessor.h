//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

//#include "GeneralSensorPostprocessor.h"

/**
 * A thermocouple Postprocessor
  */
class ThermocoupleSensorPostprocessor : public GeneralSensorPostprocessor
{
public:
  static InputParameters validParams();

  ThermocoupleSensorPostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override {}
  using Postprocessor::getValue;
  virtual PostprocessorValue getValue() const override; 

protected:
  string _thermocouple_type;
  Real sensor_value_old;

  // functions to evaluate emf from temp

  Real evaluateEMF(Real t_90, const std::vector<Real>& coefficients); 
  Real evaluateEMFTypeK(Real t90, const std::vector<Real>& coefficients, const std::vector<Real>& additionalCoefficients);

  // Coefficients for thermocouple types (constants) for conversion from temp to emf
  // B type thermocouple
  // Coefficients for the temperature range 0 °C to 630.615 °C
  const std::vector<Real> coeff_C_thermo_type_B_0_to_630 = 
  {
    0.000000000000e0, -0.246508183460e-3, 0.590404211710e-5,
    -0.132579316360e-8, 0.156682919010e-11, -0.169445292400e-14,
    0.629903470940e-18
  };
  // Coefficients for the temperature range 630.615 °C to 1820 °C
  const std::vector<Real> coeff_C_thermo_type_B_630_to_1820 = 
  {
    -0.389381686210e1, 0.285717474700e-1, -0.848851047850e-4,
    0.157852801640e-6, -0.168353448640e-9, 0.111097940130e-12,
    -0.445154310330e-16, 0.989756408210e-20, -0.937913302890e-24
  };

  // E type thermocouple
  // Coefficients for the temperature range -270 °C to 0 °C
  const std::vector<Real> coeff_C_thermo_type_E_minus_270_to_0 = 
  {
    0.000000000000e0, 0.586655087080e-1, 0.454109771240e-4,
    -0.779980486860e-6, -0.258001608430e-7, -0.594525830570e-9,
    -0.932140586670e-11, -0.102876055340e-12, -0.803701236210e-15,
    -0.439794973910e-17, -0.164147763550e-19, -0.396736195160e-22,
    -0.558273287210e-25, -0.346578420130e-28
  };
  // Coefficients for the temperature range 0 °C to 1000 °C
  const std::vector<Real> coeff_C_thermo_type_E_0_to_1000 = 
  {
    0.000000000000e0, 0.586655087100e-1, 0.450322755820e-4,
    0.289084072120e-7, -0.330568966520e-9, 0.650244032700e-12,
    -0.191974955040e-15, -0.125366004970e-17, 0.214892175690e-20,
    -0.143880417820e-23, 0.359608994810e-27
  };

  // J type thermocouple
  // Coefficients for the temperature range -210 °C to 760 °C
  const std::vector<Real> coeff_C_thermo_type_J_minus_210_to_760 = 
  {
    0.000000000000e0, 0.503811878150e-1, 0.304758369300e-4,
    -0.856810657200e-7, 0.132281952950e-9, -0.170529583370e-12,
    0.209480906970e-15, -0.125383953360e-18, 0.156317256970e-22
  };
  // Coefficients for the temperature range 760 °C to 1200 °C
  const std::vector<Real> coeff_C_thermo_type_J_760_to_1200 = 
  {
    0.296456256810e3, -0.149761277860e1, 0.317871039240e-2,
    -0.318476867010e-5, 0.157208190040e-8, -0.306913690560e-12
  };

  // N type thermocouple
  // Coefficients for the temperature range -270 °C to 0 °C
  const std::vector<Real> coeff_C_thermo_type_N_minus_270_to_0 = 
  {
    0.000000000000e0, 0.261591059620e-1, 0.109574842280e-4,
    -0.938411115540e-7, -0.464120397590e-10, -0.263033577160e-11,
    -0.226534380030e-13, -0.760893007910e-16, -0.934196678350e-19
  };
  // Coefficients for the temperature range 0 °C to 1300 °C
  const std::vector<Real> coeff_C_thermo_type_N_0_to_1300 = 
  {
    0.000000000000e0, 0.259293946010e-1, 0.157101418800e-4,
    0.438256272370e-7, -0.252611697940e-9, 0.643118193390e-12,
    -0.100634715190e-14, 0.997453389920e-18, -0.608632456070e-21,
    0.208492293390e-24, -0.306821961510e-28
  };

  // R type thermocouple
  // Coefficients for the temperature range -50 °C to 1064.18 °C
  const std::vector<Real> coeff_C_thermo_type_R_minus_50_to_1064 = 
  {
    0.000000000000e0, 0.528961729765e-2, 0.139166589782e-4,
    -0.238855693017e-7, 0.356916001063e-10, -0.462347666298e-13,
    0.500777441034e-16, -0.373105886191e-19, 0.157716482367e-22,
    -0.281038625251e-26
  };
  // Coefficients for the temperature range 1064.18 °C to 1664.5 °C
  const std::vector<Real> coeff_C_thermo_type_R_1064_to_1664 = 
  {
    0.295157925316e1, -0.252061251332e-2, 0.159564501865e-4,
    -0.764085947576e-8, 0.205305291024e-11, -0.293359668173e-15,
    0
  };
  // Coefficients for the temperature range 1664.5 °C to 1768.1 °C
  const std::vector<Real> coeff_C_thermo_type_R_1664_to_1768 = 
  {
    0.152232118209e3, -0.268819888545e2, 0.171280280471e-3,
    -0.345895706453e-7, -0.934633971046e-14
  };

  // S type thermocouple
  // Coefficients for the temperature range -50 °C to 1064.18 °C
 const std::vector<Real> coeff_C_thermo_type_S_minus_50_to_1064 = 
 {
    0.000000000000e0, 0.540313308631e-2, 0.125934289740e-4,
    -0.232477968689e-7, 0.322028823036e-10, -0.331465196389e-13,
    0.255744251786e-16, -0.125068871393e-19, 0.271443176145e-23
  };
  // Coefficients for the temperature range 1064.18 °C to 1664.5 °C
  const std::vector<Real> coeff_C_thermo_type_S_1064_to_1664 = 
  {
    0.132900444085e1, 0.334509311344e-2, 0.654805192818e-5,
    -0.164856259209e-8, 0.129989605174e-13
  };
  // Coefficients for the temperature range 1664.5 °C to 1768.1 °C
  const std::vector<Real> coeff_C_thermo_type_S_1664_to_1768 = 
  {
    0.146628232636e3, -0.258430516752e0, 0.163693574641e-3,
    -0.330439046987e-7, -0.943223690612e-14
  };

  // T type thermocouple
  // Coefficients for the temperature range -270 °C to 0 °C
  const std::vector<Real> coeff_C_thermo_type_T_minus_270_to_0 = 
  {
    0.000000000000e0, 0.387481063640e-1, 0.441944343470e-4,
    0.118443231050e-6, 0.200329735540e-7, 0.901380195590e-9,
    0.226511565930e-10, 0.360711542050e-12, 0.384939398830e-14,
    0.282135219250e-16, 0.142515947790e-18, 0.487686622860e-21,
    0.107955392700e-23, 0.139450270620e-26, 0.797951539270e-30
  };
  // Coefficients for the temperature range 0 °C to 400 °C
  const std::vector<Real> coeff_C_thermo_type_T_0_to_400 = 
  {
    0.000000000000e0, 0.387481063640e-1, 0.332922278800e-4,
    0.206182434040e-6, -0.218822568460e-8, 0.109968809280e-10,
    -0.308157587720e-13, 0.454791352900e-16, -0.275129016730e-19,
  };

  // K type thermocouple ( has extra coefficients)
  // Coefficients for the temperature range -270 °C to 0 °C
  const std::vector<Real> coeff_C_thermo_type_K_minus_270_to_0 = 
  {
    0.000000000000e0, 0.394501280250e-1, 0.236223735980e-4,
    -0.328589067840e-6, -0.499048287770e-8, -0.675090591730e-10,
    -0.574103274280e-12, -0.310888728940e-14, -0.104516093650e-16,
    -0.198892668780e-19, -0.163226974860e-22
  };
  // Coefficients for the temperature range 0 °C to 1372 °C
  const std::vector<Real> coeff_C_thermo_type_K_0_to_1372 = 
  {
    -0.176004136860e-1, 0.389212049750e-1, 0.185587700320e-4,
    -0.994575928740e-7, 0.318409457190e-9, -0.560728448890e-12,
    0.560750590590e-15, -0.320207200030e-18, 0.971511471520e-22,
    -0.121047212750e-25
  };
  // Additional coefficients for the temperature range 0 °C to 1372 °C
 const std::vector<Real> coeff_A_thermo_type_K_0_to_1372 = 
 {
    0.118597600000e0, -0.118343200000e-3, 0.126968600000e3
  };

    // Coefficients for thermocouple types (constants) for conversion from emf to temp

};