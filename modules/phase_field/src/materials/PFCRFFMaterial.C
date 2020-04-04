//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PFCRFFMaterial.h"

registerMooseObject("PhaseFieldApp", PFCRFFMaterial);

InputParameters
PFCRFFMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredParam<unsigned int>(
      "num_L", "specifies the number of complex L variables will be solved for");
  return params;
}

PFCRFFMaterial::PFCRFFMaterial(const InputParameters & parameters)
  : Material(parameters),
    _M(declareProperty<Real>("M")),
    _alpha_R_0(declareProperty<Real>("alpha_R_0")),
    _alpha_I_0(declareProperty<Real>("alpha_I_0")),
    _A_R_0(declareProperty<Real>("A_R_0")),
    _A_I_0(declareProperty<Real>("A_I_0")),
    _alpha_R_1(declareProperty<Real>("alpha_R_1")),
    _alpha_I_1(declareProperty<Real>("alpha_I_1")),
    _A_R_1(declareProperty<Real>("A_R_1")),
    _A_I_1(declareProperty<Real>("A_I_1")),
    _alpha_R_2(declareProperty<Real>("alpha_R_2")),
    _alpha_I_2(declareProperty<Real>("alpha_I_2")),
    _A_R_2(declareProperty<Real>("A_R_2")),
    _A_I_2(declareProperty<Real>("A_I_2")),
    _alpha_R_3(declareProperty<Real>("alpha_R_3")),
    _alpha_I_3(declareProperty<Real>("alpha_I_3")),
    _A_R_3(declareProperty<Real>("A_R_3")),
    _A_I_3(declareProperty<Real>("A_I_3")),
    _alpha_R_4(declareProperty<Real>("alpha_R_4")),
    _alpha_I_4(declareProperty<Real>("alpha_I_4")),
    _A_R_4(declareProperty<Real>("A_R_4")),
    _A_I_4(declareProperty<Real>("A_I_4")),
    _num_L(getParam<unsigned int>("num_L"))
{
}

void
PFCRFFMaterial::computeQpProperties()
{
  // Mobility
  _M[_qp] = 1.0;

  // Alpha and A constants
  if (_num_L == 3)
  {
    // alpha constants
    _alpha_R_0[_qp] = 2.352788316033853;
    _alpha_I_0[_qp] = 0.0;
    _alpha_R_1[_qp] = -4.371217046300305;
    _alpha_I_1[_qp] = 6.153993830413678;
    _alpha_R_2[_qp] = -4.371217046300305;
    _alpha_I_2[_qp] = -6.153993830413678;

    // A constants
    _A_R_0[_qp] = -1.254832460194660e2;
    _A_I_0[_qp] = 4.141043034348927e-15;
    _A_R_1[_qp] = 24.798843718179786;
    _A_I_1[_qp] = 37.678064436502760;
    _A_R_2[_qp] = 24.798843718179786;
    _A_I_2[_qp] = -37.678064436502760;
  }
  else if (_num_L == 5)
  {
    // alpha constants
    _alpha_R_0[_qp] = 2.429134088464706;
    _alpha_I_0[_qp] = 0.0;
    _alpha_R_1[_qp] = -3.972333899872749;
    _alpha_I_1[_qp] = 6.499130135847140;
    _alpha_R_2[_qp] = -3.972333899872749;
    _alpha_I_2[_qp] = -6.499130135847140;
    _alpha_R_3[_qp] = -18.943264072194637;
    _alpha_I_3[_qp] = 9.349446845430961;
    _alpha_R_4[_qp] = -18.943264072194637;
    _alpha_I_4[_qp] = -9.349446845430961;

    // A constants
    _A_R_0[_qp] = -1.282478656880326e02;
    _A_I_0[_qp] = 9.910190130869531e-15;
    _A_R_1[_qp] = 34.212475550662354;
    _A_I_1[_qp] = 42.274652746493430;
    _A_R_2[_qp] = 34.212475550666770;
    _A_I_2[_qp] = -42.274652746496530;
    _A_R_3[_qp] = 10.501019149011636;
    _A_I_3[_qp] = -2.363585468012575;
    _A_R_4[_qp] = 10.501019149026910;
    _A_I_4[_qp] = 2.363585467971611;
  }
}

/*
  _alpha_R_0[_qp] =  2.412;
    _alpha_I_0[_qp] =   0.0;
    _alpha_R_1[_qp] =  -18.62;
    _alpha_I_1[_qp] =   9.968;
    _alpha_R_2[_qp] =  -51.8;
    _alpha_I_2[_qp] =  -18.58;
    _alpha_R_3[_qp] =   -104.6;
    _alpha_I_3[_qp] =   -19.21;
    _alpha_R_4[_qp] =  -3.88;
    _alpha_I_4[_qp] =   6.545;

    // A constants
    _A_R_0[_qp] = -63.1;
    _A_I_0[_qp] =  0.0;
    _A_R_1[_qp] = 12.52;
    _A_I_1[_qp] =  -3.607;
    _A_R_2[_qp] = 3.88;
    _A_I_2[_qp] = 0.7762;
    _A_R_3[_qp] = 0.9984;
    _A_I_3[_qp] = 0.1591;
    _A_R_4[_qp] = 36.7;
    _A_I_4[_qp] = 42.66;

  else if (_num_L == 5)
  {
    // alpha constants
    _alpha_R_0[_qp] = 2.4887266073084095552303551812656223773956298828125;
    _alpha_I_0[_qp] =  0.0;
    _alpha_R_1[_qp] = -19.2733746470461682065433706156909465789794921875;
    _alpha_I_1[_qp] =  -9.18277447910810451503493823111057281494140625;
    _alpha_R_2[_qp] = -19.2733746470461682065433706156909465789794921875;
    _alpha_I_2[_qp] = 9.18277447910810451503493823111057281494140625;
    _alpha_R_3[_qp] =  -3.8695517424123173633176975272363051772117614746094;
    _alpha_I_3[_qp] = 6.7955256217773678528715208813082426786422729492188;
    _alpha_R_4[_qp] =  -3.8695517424123173633176975272363051772117614746094;
    _alpha_I_4[_qp] =  -6.7955256217773678528715208813082426786422729492188;

    // A constants
    _A_R_0[_qp] = -133.2927098034036816898151300847530364990234375;
    _A_I_0[_qp] =  0.0;
    _A_R_1[_qp] = 10.728194854990965367846911249216645956039428710938;
    _A_I_1[_qp] =  2.5027746492227604946378960448782891035079956054688;
    _A_R_2[_qp] = 10.728194854990965367846911249216645956039428710938;
    _A_I_2[_qp] = -2.5027746492227604946378960448782891035079956054688;
    _A_R_3[_qp] = 35.9742594324134188354946672916412353515625;
    _A_I_3[_qp] = 45.6070815722133602321264334022998809814453125;
    _A_R_4[_qp] = 35.9742594324134188354946672916412353515625;
    _A_I_4[_qp] = -45.6070815722133602321264334022998809814453125;
  }

*/
