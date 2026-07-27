//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StVenantKirchhoffPK2Test.h"

registerMooseObject("SolidMechanicsTestApp", StVenantKirchhoffPK2Test);

InputParameters
StVenantKirchhoffPK2Test::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Test-only Saint-Venant-Kirchhoff PK2 stress with analytic dPK2/dF, "
                             "used to exercise ComputeLagrangianStressCustomPK2 with the Jacobian "
                             "tester.");
  params.addRequiredParam<Real>("lambda", "First Lame parameter.");
  params.addRequiredParam<Real>("mu", "Shear modulus.");
  params.addParam<MaterialPropertyName>(
      "pk2_name", "test_pk2", "Name to publish the PK2 stress as.");
  params.addParam<MaterialPropertyName>(
      "dpk2_dF_name", "test_dpk2_dF", "Name to publish dPK2/dF as.");
  params.addParam<MaterialPropertyName>(
      "deformation_gradient",
      "deformation_gradient",
      "Strain-calc's deformation gradient (F-bar-stabilized when F-bar is on).");
  return params;
}

StVenantKirchhoffPK2Test::StVenantKirchhoffPK2Test(const InputParameters & parameters)
  : Material(parameters),
    _lambda(getParam<Real>("lambda")),
    _mu(getParam<Real>("mu")),
    _F(getMaterialProperty<RankTwoTensor>("deformation_gradient")),
    _pk2(declareProperty<RankTwoTensor>(getParam<MaterialPropertyName>("pk2_name"))),
    _dpk2_dF(declareProperty<RankFourTensor>(getParam<MaterialPropertyName>("dpk2_dF_name")))
{
}

void
StVenantKirchhoffPK2Test::computeQpProperties()
{
  const RankTwoTensor I = RankTwoTensor::Identity();
  const RankTwoTensor & F = _F[_qp];

  // E_kl = 0.5 (F_mk F_ml - delta_kl).
  const RankTwoTensor E = 0.5 * (F.transpose() * F - I);
  // PK2 = 2 mu E + lambda tr(E) I.
  _pk2[_qp] = 2.0 * _mu * E + _lambda * E.trace() * I;

  // dPK2_ij/dF_pq = mu (delta_iq F_pj + F_pi delta_jq) + lambda delta_ij F_pq.
  RankFourTensor & dS = _dpk2_dF[_qp];
  dS.zero();
  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      for (unsigned int p = 0; p < 3; ++p)
        for (unsigned int q = 0; q < 3; ++q)
        {
          Real v = _lambda * (i == j ? 1.0 : 0.0) * F(p, q);
          if (i == q)
            v += _mu * F(p, j);
          if (j == q)
            v += _mu * F(p, i);
          dS(i, j, p, q) = v;
        }
}
