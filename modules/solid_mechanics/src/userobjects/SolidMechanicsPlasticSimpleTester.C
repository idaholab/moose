//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolidMechanicsPlasticSimpleTester.h"
#include "RankFourTensor.h"

registerMooseObject("SolidMechanicsApp", SolidMechanicsPlasticSimpleTester);
registerMooseObjectRenamed("SolidMechanicsApp",
                           TensorMechanicsPlasticSimpleTester,
                           "01/01/2025 00:00",
                           SolidMechanicsPlasticSimpleTester);

InputParameters
SolidMechanicsPlasticSimpleTester::validParams()
{
  InputParameters params = SolidMechanicsPlasticModel::validParams();
  params.addRequiredParam<Real>("a",
                                "Yield function = a*stress_yy + b*stress_zz + c*stress_xx + "
                                "d*(stress_xy + stress_yx)/2 + e*(stress_xz + stress_zx)/2 + "
                                "f*(stress_yz + stress_zy)/2 - strength");
  params.addRequiredParam<Real>("b",
                                "Yield function = a*stress_yy + b*stress_zz + c*stress_xx + "
                                "d*(stress_xy + stress_yx)/2 + e*(stress_xz + stress_zx)/2 + "
                                "f*(stress_yz + stress_zy)/2 - strength");
  params.addParam<Real>("c",
                        0,
                        "Yield function = a*stress_yy + b*stress_zz + c*stress_xx + "
                        "d*(stress_xy + stress_yx)/2 + e*(stress_xz + stress_zx)/2 + "
                        "f*(stress_yz + stress_zy)/2 - strength");
  params.addParam<Real>("d",
                        0,
                        "Yield function = a*stress_yy + b*stress_zz + c*stress_xx + "
                        "d*(stress_xy + stress_yx)/2 + e*(stress_xz + stress_zx)/2 + "
                        "f*(stress_yz + stress_zy)/2 - strength");
  params.addParam<Real>("e",
                        0,
                        "Yield function = a*stress_yy + b*stress_zz + c*stress_xx + "
                        "d*(stress_xy + stress_yx)/2 + e*(stress_xz + stress_zx)/2 + "
                        "f*(stress_yz + stress_zy)/2 - strength");
  params.addParam<Real>("f",
                        0,
                        "Yield function = a*stress_yy + b*stress_zz + c*stress_xx + "
                        "d*(stress_xy + stress_yx)/2 + e*(stress_xz + stress_zx)/2 + "
                        "f*(stress_yz + stress_zy)/2 - strength");
  params.addRequiredParam<Real>("strength",
                                "Yield function = a*stress_yy + b*stress_zz + "
                                "c*stress_xx + d*(stress_xy + stress_yx)/2 + "
                                "e*(stress_xz + stress_zx)/2 + f*(stress_yz + "
                                "stress_zy)/2 - strength");
  params.addClassDescription("Class that can be used for testing multi-surface plasticity models.  "
                             "Yield function = a*stress_yy + b*stress_zz + c*stress_xx + "
                             "d*(stress_xy + stress_yx)/2 + e*(stress_xz + stress_zx)/2 + "
                             "f*(stress_yz + stress_zy)/2 - strength");

  return params;
}

SolidMechanicsPlasticSimpleTester::SolidMechanicsPlasticSimpleTester(
    const InputParameters & parameters)
  : SolidMechanicsPlasticModel(parameters),
    _a(getParam<Real>("a")),
    _b(getParam<Real>("b")),
    _c(getParam<Real>("c")),
    _d(getParam<Real>("d")),
    _e(getParam<Real>("e")),
    _f(getParam<Real>("f")),
    _strength(getParam<Real>("strength"))
{
}

Real
SolidMechanicsPlasticSimpleTester::yieldFunction(const RankTwoTensor & stress, Real /*intnl*/) const
{
  return _a * stress(1, 1) + _b * stress(2, 2) + _c * stress(0, 0) +
         _d * (stress(0, 1) + stress(1, 0)) / 2.0 + _e * (stress(0, 2) + stress(2, 0)) / 2.0 +
         _f * (stress(1, 2) + stress(2, 1)) / 2.0 - _strength;
}

RankTwoTensor
SolidMechanicsPlasticSimpleTester::dyieldFunction_dstress(const RankTwoTensor & /*stress*/,
                                                          Real /*intnl*/) const
{
  RankTwoTensor df_dsig;
  df_dsig(1, 1) = _a;
  df_dsig(2, 2) = _b;
  df_dsig(0, 0) = _c;
  df_dsig(0, 1) = _d / 2.0;
  df_dsig(1, 0) = _d / 2.0;
  df_dsig(0, 2) = _e / 2.0;
  df_dsig(2, 0) = _e / 2.0;
  df_dsig(1, 2) = _f / 2.0;
  df_dsig(2, 1) = _f / 2.0;
  return df_dsig;
}

Real
SolidMechanicsPlasticSimpleTester::dyieldFunction_dintnl(const RankTwoTensor & /*stress*/,
                                                         Real /*intnl*/) const
{
  return 0.0;
}

RankTwoTensor
SolidMechanicsPlasticSimpleTester::flowPotential(const RankTwoTensor & stress, Real intnl) const
{
  return dyieldFunction_dstress(stress, intnl);
}

RankFourTensor
SolidMechanicsPlasticSimpleTester::dflowPotential_dstress(const RankTwoTensor & /*stress*/,
                                                          Real /*intnl*/) const
{
  return RankFourTensor();
}

RankTwoTensor
SolidMechanicsPlasticSimpleTester::dflowPotential_dintnl(const RankTwoTensor & /*stress*/,
                                                         Real /*intnl*/) const
{
  return RankTwoTensor();
}

std::string
SolidMechanicsPlasticSimpleTester::modelName() const
{
  return "SimpleTester";
}
