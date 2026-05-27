//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IsotropicCauchyStressTest.h"

registerMooseObject("SolidMechanicsTestApp", IsotropicCauchyStressTest);

InputParameters
IsotropicCauchyStressTest::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Test-only hypoelastic isotropic Cauchy stress with analytic "
                             "dsigma/d(spatial_velocity_increment), used to exercise "
                             "ComputeLagrangianCauchyCustomStress with the Jacobian tester.");
  params.addRequiredParam<Real>("lambda", "First Lame parameter.");
  params.addRequiredParam<Real>("mu", "Shear modulus.");
  params.addParam<MaterialPropertyName>(
      "sigma_name", "test_cauchy", "Name to publish the Cauchy stress as.");
  params.addParam<MaterialPropertyName>("dsigma_d_dL_name",
                                        "test_dcauchy_d_dL",
                                        "Name to publish dsigma/d(spatial_velocity_increment) as.");
  params.addParam<MaterialPropertyName>("spatial_velocity_increment",
                                        "spatial_velocity_increment",
                                        "Strain-calc's spatial velocity gradient increment.");
  return params;
}

IsotropicCauchyStressTest::IsotropicCauchyStressTest(const InputParameters & parameters)
  : Material(parameters),
    _lambda(getParam<Real>("lambda")),
    _mu(getParam<Real>("mu")),
    _dL(getMaterialProperty<RankTwoTensor>("spatial_velocity_increment")),
    _sigma(declareProperty<RankTwoTensor>(getParam<MaterialPropertyName>("sigma_name"))),
    _sigma_old(getMaterialPropertyOld<RankTwoTensor>(getParam<MaterialPropertyName>("sigma_name"))),
    _dsigma_d_dL(
        declareProperty<RankFourTensor>(getParam<MaterialPropertyName>("dsigma_d_dL_name")))
{
}

void
IsotropicCauchyStressTest::initQpStatefulProperties()
{
  _sigma[_qp].zero();
}

void
IsotropicCauchyStressTest::computeQpProperties()
{
  const RankTwoTensor I = RankTwoTensor::Identity();

  // dd = sym(dL) (the symmetric part of the spatial-velocity-gradient increment).
  const RankTwoTensor dd = 0.5 * (_dL[_qp] + _dL[_qp].transpose());
  // Linear isotropic elastic stress update on dd. (Hypoelastic; ignores rotation for the
  // purposes of this Jacobian test.)
  _sigma[_qp] = _sigma_old[_qp] + 2.0 * _mu * dd + _lambda * dd.trace() * I;

  // dsigma_ij/d(dL)_pq:
  //   dsigma/d(dd) = 2 mu I^{sym} + lambda I (x) I    (isotropic elastic tangent)
  //   d(dd)/d(dL)  = 0.5 (delta_ip delta_jq + delta_iq delta_jp)
  //   so dsigma_ij/d(dL)_pq = mu (delta_ip delta_jq + delta_iq delta_jp)
  //                          + lambda delta_ij delta_pq.
  RankFourTensor & dS = _dsigma_d_dL[_qp];
  dS.zero();
  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      for (unsigned int p = 0; p < 3; ++p)
        for (unsigned int q = 0; q < 3; ++q)
        {
          Real v = (i == j && p == q) ? _lambda : 0.0;
          if (i == p && j == q)
            v += _mu;
          if (i == q && j == p)
            v += _mu;
          dS(i, j, p, q) = v;
        }
}
