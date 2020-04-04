//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TensorMechanicsPlasticOrthotropic.h"
#include "RankFourTensor.h"
#include "libmesh/utility.h"

registerMooseObject("TensorMechanicsApp", TensorMechanicsPlasticOrthotropic);

InputParameters
TensorMechanicsPlasticOrthotropic::validParams()
{
  InputParameters params = TensorMechanicsPlasticIsotropicSD::validParams();
  params.addRequiredParam<std::vector<Real>>("c1", "The six coefficients of L prime");
  params.addRequiredParam<std::vector<Real>>("c2", "The six coefficients of L prime prime");
  params.addClassDescription("Orthotropic plasticity for pressure sensitive materials and also "
                             "models the strength differential effect");
  return params;
}

TensorMechanicsPlasticOrthotropic::TensorMechanicsPlasticOrthotropic(
    const InputParameters & parameters)
  : TensorMechanicsPlasticIsotropicSD(parameters),
    _c1(getParam<std::vector<Real>>("c1")),
    _c2(getParam<std::vector<Real>>("c2"))
{
  _c = 1.0;
  _l1(0, 0, 0, 0) = (_c1[1] + _c1[2]) / 3.0;
  _l1(0, 0, 1, 1) = -_c1[2] / 3.0;
  _l1(0, 0, 2, 2) = -_c1[1] / 3.0;
  _l1(1, 1, 0, 0) = -_c1[2] / 3.0;
  _l1(1, 1, 1, 1) = (_c1[0] + _c1[2]) / 3.0;
  _l1(1, 1, 2, 2) = -_c1[0] / 3.0;
  _l1(2, 2, 0, 0) = -_c1[1] / 3.0;
  _l1(2, 2, 1, 1) = -_c1[0] / 3.0;
  _l1(2, 2, 2, 2) = (_c1[0] + _c1[1]) / 3.0;
  _l1(0, 1, 1, 0) = _c1[5] / 2.0;
  _l1(0, 1, 0, 1) = _c1[5] / 2.0;
  _l1(1, 0, 1, 0) = _c1[5] / 2.0;
  _l1(1, 0, 0, 1) = _c1[5] / 2.0;
  _l1(0, 2, 0, 2) = _c1[4] / 2.0;
  _l1(0, 2, 2, 0) = _c1[4] / 2.0;
  _l1(2, 0, 2, 0) = _c1[4] / 2.0;
  _l1(2, 0, 0, 2) = _c1[4] / 2.0;
  _l1(1, 2, 2, 1) = _c1[3] / 2.0;
  _l1(1, 2, 1, 2) = _c1[3] / 2.0;
  _l1(2, 1, 1, 2) = _c1[3] / 2.0;
  _l1(2, 1, 2, 1) = _c1[3] / 2.0;

  _l2(0, 0, 0, 0) = (_c2[1] + _c2[2]) / 3.0;
  _l2(0, 0, 1, 1) = -_c2[2] / 3.0;
  _l2(0, 0, 2, 2) = -_c2[1] / 3.0;
  _l2(1, 1, 0, 0) = -_c2[2] / 3.0;
  _l2(1, 1, 1, 1) = (_c2[0] + _c2[2]) / 3.0;
  _l2(1, 1, 2, 2) = -_c2[0] / 3.0;
  _l2(2, 2, 0, 0) = -_c2[1] / 3.0;
  _l2(2, 2, 1, 1) = -_c2[0] / 3.0;
  _l2(2, 2, 2, 2) = (_c2[0] + _c2[1]) / 3.0;
  _l2(0, 1, 1, 0) = _c2[5] / 2.0;
  _l2(0, 1, 0, 1) = _c2[5] / 2.0;
  _l2(1, 0, 1, 0) = _c2[5] / 2.0;
  _l2(1, 0, 0, 1) = _c2[5] / 2.0;
  _l2(0, 2, 0, 2) = _c2[4] / 2.0;
  _l2(0, 2, 2, 0) = _c2[4] / 2.0;
  _l2(2, 0, 2, 0) = _c2[4] / 2.0;
  _l2(2, 0, 0, 2) = _c2[4] / 2.0;
  _l2(1, 2, 2, 1) = _c2[3] / 2.0;
  _l2(1, 2, 1, 2) = _c2[3] / 2.0;
  _l2(2, 1, 1, 2) = _c2[3] / 2.0;
  _l2(2, 1, 2, 1) = _c2[3] / 2.0;
}

Real
TensorMechanicsPlasticOrthotropic::yieldFunction(const RankTwoTensor & stress, Real intnl) const
{
  const RankTwoTensor j2prime = _l1 * stress;
  const RankTwoTensor j3prime = _l2 * stress;
  return _b * stress.trace() +
         std::pow(std::pow(-j2prime.generalSecondInvariant(), 3.0 / 2.0) - j3prime.det(),
                  1.0 / 3.0) -
         yieldStrength(intnl);
}

RankTwoTensor
TensorMechanicsPlasticOrthotropic::dyieldFunction_dstress(const RankTwoTensor & stress,
                                                          Real /*intnl*/) const
{
  const RankTwoTensor j2prime = _l1 * stress;
  const RankTwoTensor j3prime = _l2 * stress;
  const Real j2 = -j2prime.generalSecondInvariant();
  const Real j3 = j3prime.det();
  return _b * dI_sigma() + dphi_dj2(j2, j3) * _l1.innerProductTranspose(dj2_dSkl(j2prime)) +
         dphi_dj3(j2, j3) * _l2.innerProductTranspose(j3prime.ddet());
}

RankFourTensor
TensorMechanicsPlasticOrthotropic::dflowPotential_dstress(const RankTwoTensor & stress,
                                                          Real /*intnl*/) const
{
  if (_associative)
  {
    const RankTwoTensor j2prime = _l1 * stress;
    const RankTwoTensor j3prime = _l2 * stress;
    const RankTwoTensor dj2 = dj2_dSkl(j2prime);
    const RankTwoTensor dj3 = j3prime.ddet();
    const Real j2 = -j2prime.generalSecondInvariant();
    const Real j3 = j3prime.det();
    const RankFourTensor dr =
        dfj2_dj2(j2, j3) *
            _l1.innerProductTranspose(dj2).outerProduct(_l1.innerProductTranspose(dj2)) +
        dfj2_dj3(j2, j3) *
            _l1.innerProductTranspose(dj2).outerProduct(_l2.innerProductTranspose(dj3)) +
        dfj3_dj2(j2, j3) *
            _l2.innerProductTranspose(dj3).outerProduct(_l1.innerProductTranspose(dj2)) +
        dfj3_dj3(j2, j3) *
            _l2.innerProductTranspose(dj3).outerProduct(_l2.innerProductTranspose(dj3));
    const RankTwoTensor r = _b * dI_sigma() +
                            dphi_dj2(j2, j3) * _l1.innerProductTranspose(dj2_dSkl(j2prime)) +
                            dphi_dj3(j2, j3) * _l2.innerProductTranspose(j3prime.ddet());
    const Real norm = r.L2norm();
    return dr / norm - (r / Utility::pow<3>(norm)).outerProduct(dr.innerProductTranspose(r));
  }
  else
    return TensorMechanicsPlasticJ2::dflowPotential_dstress(stress, 0);
}

RankTwoTensor
TensorMechanicsPlasticOrthotropic::flowPotential(const RankTwoTensor & stress, Real intnl) const
{
  if (_associative)
  {
    const RankTwoTensor a = dyieldFunction_dstress(stress, intnl);
    return a / a.L2norm();
  }
  else
    return TensorMechanicsPlasticJ2::dyieldFunction_dstress(stress, intnl);
}
