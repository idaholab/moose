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
  // Coefficients and error range vectors for Thermocouple Type B in mV and degreeC
  const std::vector<Real> coeff_d_thermo_type_B_0_291_to_2_431 =
  {
    9.8423321e1, 6.9971500e2, -8.4765304e2, 1.0052644e3,
    -8.3345952e2, 4.5508542e2, -1.5523037e2, 2.9886750e1,
    -2.4742860e0
  };
  const std::vector<Real> coeff_d_thermo_type_B_2_431_to_13_82 =
  {
    2.1315071e2, 2.8510504e2, -5.2742887e1, 9.9160804e0,
    -1.2965303e0, 1.1195870e-1, -6.0625199e-3, 1.8661696e-4,
    -2.4878585e-6
  };
  const std::vector<Real> error_range_thermo_type_B_0_291_to_2_431 =
  {
    -0.02, 0.03
  };
  const std::vector<Real> error_range_thermo_type_B_2_431_to_13_82 =
  {
    -0.01, 0.02
  };
  
  // Coefficients and error range vectors for Thermocouple Type E in mV and degreeC
  const std::vector<Real> coeff_d_thermo_type_E_minus_8_825_to_0 =
  {
    0.0000000e0, 1.6977288e1, -4.3514970e-1, -1.5859697e-1,
    -9.2502871e-2, -2.6084314e-2, -4.1360199e-3, -3.4034030e-4,
    -1.1564890e-5, 0.0000000e0
  };
  const std::vector<Real> coeff_d_thermo_type_E_0_to_76_373 =
  {
    0.0000000e0, 1.7057035e1, -2.3301759e-1, 6.5435585e-3,
    -7.3562749e-5, -1.7896001e-6, 8.4036165e-8, -1.3735879e-9,
    1.0629823e-11, -3.2447087e-14
  };
  const std::vector<Real> error_range_thermo_type_E_minus_8_825_to_0 =
  {
    -0.01, 0.03
  };
  const std::vector<Real> error_range_thermo_type_E_0_to_76_373 =
  {
    -0.02, 0.02
  };

  // Coefficients and error range vectors for Thermocouple Type J in mV and degreeC
  const std::vector<Real> coeff_d_thermo_type_J_minus_8_095_to_0 =
  {
    0.0000000e0, 1.9528268e1, -1.2286185e0, -1.0752178e0,
    -5.9086933e-1, -1.7256713e-1, -2.8131513e-2, -2.3963370e-3,
    -8.3823321e-5
  };
  const std::vector<Real> coeff_d_thermo_type_J_0_to_42_919 =
  {
    0.0000000e0, 1.978425e1, -2.001204e-1, 1.036969e-2,
    -2.549687e-4, 3.585153e-6, -5.344285e-8, 5.099890e-10,
    0.0000000e0
  };
  const std::vector<Real> coeff_d_thermo_type_J_42_919_to_69_553 =
  {
    -3.11358187e3, 3.00543684e2, -9.94773230e0, 1.70276630e-1,
    -1.43033468e-3, 4.73886084e-6, 0.0000000e0, 0.0000000e0,
    0.0000000e0
  };
  const std::vector<Real> error_range_thermo_type_J_minus_8_095_to_0 =
  {
    -0.05, 0.03
  };
  const std::vector<Real> error_range_thermo_type_J_0_to_42_919 =
  {
    -0.04, 0.04
  };
  const std::vector<Real> error_range_thermo_type_J_42_919_to_69_553 =
  {
    -0.04, 0.03
  };

  // Coefficients and error range vectors for Thermocouple Type K in mV and degreeC
  const std::vector<Real> coeff_d_thermo_type_K_minus_5_891_to_0 =
  {
    0.0000000e0, 2.5173462e1, -1.1662878e0, -1.0833638e0,
    -8.9773540e-1, -3.7342377e-1, -8.6632643e-2, -1.0450598e-2,
    -5.1920577e-4, 0.0000000e0
  };
  const std::vector<Real> coeff_d_thermo_type_K_0_to_20_644 =
  {
    0.0000000e0, 2.508355e1, 7.860106e-2, -2.503131e-1,
    8.315270e-2, -1.228034e-2, 9.804036e-4, -4.413030e-5,
    1.057734e-6, -1.052755e-8
  };
  const std::vector<Real> coeff_d_thermo_type_K_20_644_to_54_886 =
  {
    -1.318058e2, 4.830222e1, -1.646031e0, 5.464731e-2,
    -9.650715e-4, 8.802193e-6, -3.110810e-8, 0.0000000e0,
    0.0000000e0
  };
  const std::vector<Real> error_range_thermo_type_K_minus_5_891_to_0 =
  {
    -0.02, 0.04
  };
  const std::vector<Real> error_range_thermo_type_K_0_to_20_644 =
  {
    -0.05, 0.04
  };
  const std::vector<Real> error_range_thermo_type_K_20_644_to_54_886 =
  {
    -0.05, 0.06
  };

  // Coefficients and error range vectors for Thermocouple Type N in mV and degreeC
  const std::vector<Real> coeff_d_thermo_type_N_minus_3_990_to_0 =
  {
    0.0000000e0, 3.8436847e1, 1.1010485e0, 5.2229312e0,
    7.2060525e0, 5.8488586e0, 2.7754916e0, 7.7075166e-1,
    1.1582665e-1, 7.3138868e-3
  };
 const std::vector<Real> coeff_d_thermo_type_N_0_to_20_613 =
  {
    0.0000000e0, 3.86896e1, -1.08267e0, 4.70205e-2,
    -2.12169e-6, -1.17272e-4, 5.39280e-6, -7.98156e-8,
    0.0000000e0, 0.0000000e0
  };
  const std::vector<Real> coeff_d_thermo_type_N_20_613_to_47_513 =
  {
    1.972485e1, 3.300943e1, -3.915159e-1, 9.855391e-3,
    -1.274371e-4, 7.767022e-7, 0.0000000e0, 0.0000000e0,
    0.0000000e0
  };
  const std::vector<Real> error_range_thermo_type_N_minus_3_990_to_0 =
  {
    -0.02, 0.03
  };
  const std::vector<Real> error_range_thermo_type_N_0_to_20_613 =
  {
    -0.02, 0.03
  };
  const std::vector<Real> error_range_thermo_type_N_20_613_to_47_513 =
  {
    -0.04, 0.02
  };

  // Coefficients and error range vectors for Thermocouple Type R in mV and degreeC
  const std::vector<Real> coeff_d_thermo_type_R_minus_0_226_to_1_923 =
  {
    0.0000000e0, 1.334584505e1, -9.3835290e1, 1.3068619e2,
    -2.2703580e2, 3.5145659e2, -3.8953900e2, 2.8239471e2,
    -1.2607281e2, 3.1353611e1, -3.3187769e0
  };
  const std::vector<Real> coeff_d_thermo_type_R_1_923_to_13_228 =
  {
    1.8891380e2, 1.472644573e2, -1.844024844e1, 4.031129726e0,
    -6.249428360e-1, 6.468412046e-2, -4.458750426e-3, 1.994710149e-4,
    -5.313401790e-6, 6.481976217e-8
  };
  const std::vector<Real> coeff_d_thermo_type_R_11_361_to_19_739 =
  {
    -8.199599416e1, 1.553962042e2, -8.342197663e0, 4.279433549e-1,
    -1.191577910e-2, 1.492290091e-4, 0.0, 0.0, 0.0, 0.0
  };
  const std::vector<Real> coeff_d_thermo_type_R_19_739_to_21_103 =
  {
    3.406177836e4, -7.023729171e3, 5.582903813e2, -1.952394635e1,
    2.560740231e-1, 0.0, 0.0, 0.0, 0.0, 0.0
  };
  const std::vector<Real> error_range_thermo_type_R_minus_0_226_to_1_923 =
  {
    -0.02, 0.02
  };
  const std::vector<Real> error_range_thermo_type_R_1_923_to_13_228 =
  {
    -0.005, 0.005
  };
  const std::vector<Real> error_range_thermo_type_R_11_361_to_19_739 =
  {
    -0.0005, 0.001
  };
  const std::vector<Real> error_range_thermo_type_R_19_739_to_21_103 =
  {
    -0.001, 0.002
  };

  // Coefficients and error range vectors for Thermocouple Type S in mV and degreeC
  const std::vector<Real> coeff_d_thermo_type_S_minus_0_235_to_1_874 =
  {
    0.00000000e0, 1.291507177e1, -8.00504062e1, 1.02237430e2,
    -1.52248592e2, 1.88821343e2, -1.59085941e2, 8.23027880e1,
    -2.34181944e1, 2.79786260e0
  };
  const std::vector<Real> coeff_d_thermo_type_S_1_874_to_11_950 =
  {
    1.84949460e2, 1.466298863e2, -1.534713402e1, 3.145945973e0,
    -4.163257839e-1, 3.187963771e-2, -1.291637500e-3, 2.183475087e-5,
    -1.447379511e-7, 8.211272125e-9
  };
  const std::vector<Real> coeff_d_thermo_type_S_10_332_to_17_536 =
  {
    -8.087801117e1, 1.621573104e2, -8.536869453e0, 4.719686976e-1,
    -1.441693666e-2, 2.081618890e-4, 0.0, 0.0, 0.0, 0.0
  };
  const std::vector<Real> coeff_d_thermo_type_S_17_536_to_18_693 =
  {
    5.333875126e4, -1.235892298e4, 1.092657613e3, -4.265693686e1,
    6.247205420e-1, 0.0, 0.0, 0.0, 0.0, 0.0
  };
  const std::vector<Real> error_range_thermo_type_S_minus_0_235_to_1_874 =
  {
    -0.02, 0.02
  };
  const std::vector<Real> error_range_thermo_type_S_1_874_to_11_950 =
  {
    -0.01, 0.01
  };
  const std::vector<Real> error_range_thermo_type_S_10_332_to_17_536 =
  {
    -0.0002, 0.0002
  };
  const std::vector<Real> error_range_thermo_type_S_17_536_to_18_693 =
  {
    -0.002, 0.002
  };

  // Coefficients and error range vectors for Thermocouple Type S in mV and degreeC
  const std::vector<Real> coeff_d_thermo_type_T_minus_5_603_to_0 =
  {
    0.0000000e0, 2.5949192e1, -2.1316967e-1, 7.9018692e-1,
    4.2527777e-1, 1.3304473e-1, 2.0241446e-2, 1.2668171e-3
  };
  const std::vector<Real> coeff_d_thermo_type_T_0_to_20_872 =
  {
    0.000000e0, 2.592800e1, -7.602961e-1, 4.637791e-2,
    -2.165394e-3, 6.048144e-5, -7.293422e-7, 0.0
  };
  const std::vector<Real> error_range_thermo_type_T_minus_5_603_to_0 =
  {
    -0.02, 0.04
  };
  const std::vector<Real> error_range_thermo_type_T_0_to_20_872 =
  {
    -0.03, 0.03
  };
  };