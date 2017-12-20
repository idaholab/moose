/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeLinearElasticPFFractureStress.h"

template <>
InputParameters
validParams<ComputeLinearElasticPFFractureStress>()
{
  InputParameters params = validParams<ComputeStressBase>();
  params.addClassDescription("Phase-field fracture model energy contribution to fracture for "
                             "elasticity and undamaged stress under compressive strain");
  params.addRequiredCoupledVar("c", "Order parameter for damage");
  params.addParam<Real>("kdamage", 1e-6, "Stiffness of damaged matrix");
  params.addParam<MaterialPropertyName>(
      "F_name", "E_el", "Name of material property storing the elastic energy");
  return params;
}

ComputeLinearElasticPFFractureStress::ComputeLinearElasticPFFractureStress(
    const InputParameters & parameters)
  : ComputeStressBase(parameters),
    _c(coupledValue("c")),
    _gc_prop(getMaterialProperty<Real>("gc_prop")),

    _l(getMaterialProperty<Real>("l")),

    _kdamage(getParam<Real>("kdamage")),
    _F(declareProperty<Real>(getParam<MaterialPropertyName>("F_name"))),
    _dFdc(declarePropertyDerivative<Real>(getParam<MaterialPropertyName>("F_name"),
                                          getVar("c", 0)->name())),
    _d2Fdc2(declarePropertyDerivative<Real>(
        getParam<MaterialPropertyName>("F_name"), getVar("c", 0)->name(), getVar("c", 0)->name())),
    _d2Fdcdstrain(declareProperty<RankTwoTensor>("d2Fdcdstrain")),
    _dstress_dc(
        declarePropertyDerivative<RankTwoTensor>(_base_name + "stress", getVar("c", 0)->name())),

    _dH0_pos_dstrain(declareProperty<RankTwoTensor>("dH0_pos_dstrain")),

    _H0_pos(declareProperty<Real>("H0_pos")),
    _H0_pos_old(getMaterialPropertyOld<Real>("H0_pos"))
{
}

void
ComputeLinearElasticPFFractureStress::initQpStatefulProperties()
{
  ComputeStressBase::initQpStatefulProperties();
  _H0_pos[_qp] = 0.0;
}

void
ComputeLinearElasticPFFractureStress::computeQpStress()
{
  const Real c = _c[_qp];

  // Compute usual Jacobian mult
  _Jacobian_mult[_qp] = _elasticity_tensor[_qp];

  // Zero out values when c > 1
  Real cfactor = 1.0;
  if (c > 1.0)
    cfactor = 0.0;

  // Compute eigenvectors and eigenvalues of ustress
  RankTwoTensor eigvec;
  std::vector<Real> eigval(LIBMESH_DIM);
  std::vector<Real> eigabs(LIBMESH_DIM);
  RankTwoTensor stresscal = _elasticity_tensor[_qp] * _mechanical_strain[_qp];
  stresscal.symmetricEigenvaluesEigenvectors(eigval, eigvec);
  RankTwoTensor Ipos, Ineg, eigval_tensor;
  RankTwoTensor I(RankTwoTensor::initIdentity);
  eigval_tensor.fillFromInputVector(eigval);
  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
  {
    if (eigval[i] < 0.0)
    {
      Ineg(i, i) = 1.0;
    }
  }

  Ipos = I - Ineg;

  RankFourTensor QoQ = eigvec.mixedProductIkJl(eigvec);
  RankFourTensor QToQT = eigvec.transpose().mixedProductIkJl(eigvec.transpose());
  RankFourTensor IpoIp = I.mixedProductIkJl(Ipos);
  RankFourTensor InoIn = I.mixedProductIkJl(Ineg);
  RankFourTensor Jpos = QoQ * IpoIp * QToQT * _elasticity_tensor[_qp];
  RankFourTensor Jneg = QoQ * InoIn * QToQT * _elasticity_tensor[_qp];

  RankTwoTensor eigpos = eigval_tensor * Ipos;
  RankTwoTensor eigneg = eigval_tensor * Ineg;
  RankTwoTensor stress0pos = eigvec * eigpos * (eigvec.transpose());
  RankTwoTensor stress0neg = eigvec * eigneg * (eigvec.transpose());

  Real G0_pos = (stress0pos).doubleContraction(_mechanical_strain[_qp]) / 2.0;
  Real G0_neg = (stress0neg).doubleContraction(_mechanical_strain[_qp]) / 2.0;

  _Jacobian_mult[_qp] =
      (cfactor * (1.0 - c) * (1.0 - c) * (1.0 - _kdamage) + _kdamage) * Jpos + Jneg; // cfactor

  if (G0_pos > _H0_pos_old[_qp])
  {
    _H0_pos[_qp] = G0_pos;
  }
  else
  {
    _H0_pos[_qp] = _H0_pos_old[_qp];
  }
  _dH0_pos_dstrain[_qp] = stress0pos;

  // Damage associated with positive component of stress
  _stress[_qp] =
      stress0pos * (cfactor * (1.0 - c) * (1.0 - c) * (1.0 - _kdamage) + _kdamage) + stress0neg;

  // Used in StressDivergencePFFracTensors Jacobian
  _dstress_dc[_qp] = -cfactor * stress0pos * 2.0 * (1.0 - c) * (1.0 - _kdamage);

  // Elastic free energy density
  _F[_qp] = G0_pos * (cfactor * (1.0 - c) * (1.0 - c) * (1.0 - _kdamage) + _kdamage) + G0_neg +
            _gc_prop[_qp] * c * c / 2.0 / _l[_qp];

  // derivative of elastic free energy density wrt c
  _dFdc[_qp] =
      -_H0_pos[_qp] * 2.0 * cfactor * (1.0 - c) * (1.0 - _kdamage) + _gc_prop[_qp] * c / _l[_qp];

  // 2nd derivative of elastic free energy density wrt c
  _d2Fdc2[_qp] = _H0_pos[_qp] * 2.0 * cfactor * (1.0 - _kdamage) + _gc_prop[_qp] / _l[_qp];

  // 2nd derivative wrt c and strain
  _d2Fdcdstrain[_qp] = -2.0 * _dH0_pos_dstrain[_qp] * cfactor * (1.0 - c) * (1.0 - _kdamage);

  // Used in StressDivergencePFFracTensors off-diagonal Jacobian
  _dstress_dc[_qp] = -stress0pos * 2.0 * cfactor * (1.0 - c) * (1.0 - _kdamage);
}
