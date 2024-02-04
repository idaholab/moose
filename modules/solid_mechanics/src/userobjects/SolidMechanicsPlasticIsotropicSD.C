//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolidMechanicsPlasticIsotropicSD.h"

registerMooseObject("SolidMechanicsApp", SolidMechanicsPlasticIsotropicSD);
registerMooseObjectRenamed("SolidMechanicsApp",
                           TensorMechanicsPlasticIsotropicSD,
                           "01/01/2025 00:00",
                           SolidMechanicsPlasticIsotropicSD);

InputParameters
SolidMechanicsPlasticIsotropicSD::validParams()
{
  InputParameters params = SolidMechanicsPlasticJ2::validParams();
  params.addRequiredParam<Real>("b", "A constant to model the influence of pressure");
  params.addParam<Real>(
      "c", 0.0, "A constant to model the influence of strength differential effect");
  params.addParam<bool>("associative", true, "Flag for flow-rule, true if not specified");
  params.addClassDescription("IsotropicSD plasticity for pressure sensitive materials and also "
                             "models the strength differential effect");
  return params;
}

SolidMechanicsPlasticIsotropicSD::SolidMechanicsPlasticIsotropicSD(
    const InputParameters & parameters)
  : SolidMechanicsPlasticJ2(parameters),
    _b(getParam<Real>("b")),
    _c(getParam<Real>("c")),
    _associative(getParam<bool>("associative"))
{
  _a = 1.0 / (_b + std::pow(1.0 / std::sqrt(27.0) - _c / 27.0, 1.0 / 3.0));
  for (unsigned i = 0; i < 3; ++i)
    for (unsigned j = 0; j < 3; ++j)
      for (unsigned k = 0; k < 3; ++k)
        for (unsigned l = 0; l < 3; ++l)
          _h(i, j, k, l) = ((i == k) * (j == l) - 1.0 / 3.0 * (i == j) * (k == l));
}

Real
SolidMechanicsPlasticIsotropicSD::dphi_dj2(const Real j2, const Real j3) const
{
  return std::pow(j2, 1.0 / 2.0) / (2 * std::pow(std::pow(j2, 3.0 / 2.0) - _c * j3, 2.0 / 3.0));
}

Real
SolidMechanicsPlasticIsotropicSD::dphi_dj3(const Real j2, const Real j3) const
{
  return -_c / (3 * std::pow(std::pow(j2, 3.0 / 2.0) - _c * j3, 2.0 / 3.0));
}

Real
SolidMechanicsPlasticIsotropicSD::dfj2_dj2(const Real j2, const Real j3) const
{
  return std::pow(j2, -1.0 / 2.0) / (4 * std::pow(std::pow(j2, 3.0 / 2.0) - _c * j3, 2.0 / 3.0)) -
         j2 / (2 * std::pow(std::pow(j2, 3.0 / 2.0) - _c * j3, 5.0 / 3.0));
}

Real
SolidMechanicsPlasticIsotropicSD::dfj2_dj3(const Real j2, const Real j3) const
{
  return _c * std::pow(j2, 1.0 / 2.0) /
         (3 * std::pow(std::pow(j2, 3.0 / 2.0) - _c * j3, 5.0 / 3.0));
}

Real
SolidMechanicsPlasticIsotropicSD::dfj3_dj2(const Real j2, const Real j3) const
{
  return _c * std::pow(j2, 1.0 / 2.0) /
         (3 * std::pow(std::pow(j2, 3.0 / 2.0) - _c * j3, 5.0 / 3.0));
}

Real
SolidMechanicsPlasticIsotropicSD::dfj3_dj3(const Real j2, const Real j3) const
{
  return -_c * _c * 2.0 / (9 * std::pow(std::pow(j2, 3.0 / 2.0) - _c * j3, 5.0 / 3.0));
}

RankTwoTensor
SolidMechanicsPlasticIsotropicSD::dI_sigma() const
{
  return RankTwoTensor(RankTwoTensor::initIdentity);
}

RankTwoTensor
SolidMechanicsPlasticIsotropicSD::dj2_dSkl(const RankTwoTensor & stress) const
{
  RankTwoTensor a;
  const Real trace = stress.trace();
  for (unsigned i = 0; i < 3; ++i)
    for (unsigned j = 0; j < 3; ++j)
      a(i, j) = (trace - stress(i, j)) * -1 * (i == j) + stress(i, j) * (i != j);

  return a;
}

Real
SolidMechanicsPlasticIsotropicSD::yieldFunction(const RankTwoTensor & stress, Real intnl) const
{
  return _a * (_b * stress.trace() +
               std::pow(std::pow(stress.secondInvariant(), 1.5) - _c * stress.thirdInvariant(),
                        1.0 / 3.0)) -
         yieldStrength(intnl);
}

RankTwoTensor
SolidMechanicsPlasticIsotropicSD::dyieldFunction_dstress(const RankTwoTensor & stress,
                                                         Real /*intnl*/) const
{
  const RankTwoTensor sDev = stress.deviatoric();
  const Real j2 = stress.secondInvariant();
  const Real j3 = stress.thirdInvariant();
  return _a * (_b * dI_sigma() + dphi_dj2(j2, j3) * _h.innerProductTranspose(dj2_dSkl(sDev)) +
               dphi_dj3(j2, j3) * _h.innerProductTranspose(sDev.ddet()));
}

RankFourTensor
SolidMechanicsPlasticIsotropicSD::dflowPotential_dstress(const RankTwoTensor & stress,
                                                         Real /*intnl*/) const
{
  if (_associative)
  {
    const RankTwoTensor sDev = stress.deviatoric();
    const RankTwoTensor dj2 = dj2_dSkl(sDev);
    const RankTwoTensor dj3 = sDev.ddet();
    const Real j2 = stress.secondInvariant();
    const Real j3 = stress.thirdInvariant();
    return _a * (dfj2_dj2(j2, j3) *
                     _h.innerProductTranspose(dj2).outerProduct(_h.innerProductTranspose(dj2)) +
                 dfj2_dj3(j2, j3) *
                     _h.innerProductTranspose(dj2).outerProduct(_h.innerProductTranspose(dj3)) +
                 dfj3_dj2(j2, j3) *
                     _h.innerProductTranspose(dj3).outerProduct(_h.innerProductTranspose(dj2)) +
                 dfj3_dj3(j2, j3) *
                     _h.innerProductTranspose(dj3).outerProduct(_h.innerProductTranspose(dj3)));
  }
  else
    return SolidMechanicsPlasticJ2::dflowPotential_dstress(stress, 0);
}

RankTwoTensor
SolidMechanicsPlasticIsotropicSD::flowPotential(const RankTwoTensor & stress, Real intnl) const
{
  if (_associative)
    return dyieldFunction_dstress(stress, intnl);
  else
    return SolidMechanicsPlasticJ2::dyieldFunction_dstress(stress, intnl);
}
