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
    _kdamage(getParam<Real>("kdamage")),
    _F(declareProperty<Real>(getParam<MaterialPropertyName>("F_name"))),
    _dFdc(declarePropertyDerivative<Real>(getParam<MaterialPropertyName>("F_name"),
                                          getVar("c", 0)->name())),
    _d2Fdc2(declarePropertyDerivative<Real>(
        getParam<MaterialPropertyName>("F_name"), getVar("c", 0)->name(), getVar("c", 0)->name())),
    _d2Fdcdstrain(declareProperty<RankTwoTensor>("d2Fdcdstrain")),
    _dstress_dc(
        declarePropertyDerivative<RankTwoTensor>(_base_name + "stress", getVar("c", 0)->name()))
{
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
  _mechanical_strain[_qp].symmetricEigenvaluesEigenvectors(eigval, eigvec);

  RankTwoTensor Ipos, Ineg, eigval_tensor;
  RankTwoTensor I(RankTwoTensor::initIdentity);
  eigval_tensor.fillFromInputVector(eigval);

  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    if (eigval[i] < 0.0)
      Ineg(i, i) = 1.0;

  Ipos = I - Ineg;

  // Creation of some critical fourth order tensors
  RankFourTensor QoQ = eigvec.mixedProductIkJl(eigvec);
  RankFourTensor QToQT = (eigvec.transpose()).mixedProductIkJl(eigvec.transpose());
  RankFourTensor IpoIp = Ipos.mixedProductIkJl(I);
  RankFourTensor InoIn = Ineg.mixedProductIkJl(I);

  const RankFourTensor Jpos = _Jacobian_mult[_qp] * QoQ * IpoIp * QToQT;
  const RankFourTensor Jneg = _Jacobian_mult[_qp] * QoQ * InoIn * QToQT;
  _Jacobian_mult[_qp] = cfactor * Jpos * ((1.0 - c) * (1.0 - c) + _kdamage) + Jneg;

  RankTwoTensor stress0pos = Jpos * _mechanical_strain[_qp];
  RankTwoTensor stress0neg = Jneg * _mechanical_strain[_qp];

  // Damage associated with positive component of stress
  _stress[_qp] = cfactor * stress0pos * ((1.0 - c) * (1.0 - c) + _kdamage) + stress0neg;
  // _stress[_qp] = _Jacobian_mult[_qp] * _mechanical_strain[_qp];

  // Energy with positive principal strains
  const Real G0_pos = stress0pos.doubleContraction(_mechanical_strain[_qp]) / 2.0;
  const Real G0_neg = stress0neg.doubleContraction(_mechanical_strain[_qp]) / 2.0;

  // Elastic free energy density
  _F[_qp] = cfactor * G0_pos * ((1.0 - c) * (1.0 - c) + _kdamage) + G0_neg;

  // derivative of elastic free energy density wrt c
  _dFdc[_qp] = -cfactor * G0_pos * 2.0 * (1.0 - c);

  // 2nd derivative of elastic free energy density wrt c
  _d2Fdc2[_qp] = cfactor * G0_pos * 2.0;

  // 2nd derivative wrt c and strain
  _d2Fdcdstrain[_qp] = -cfactor * stress0pos * (1.0 - c);

  // Used in StressDivergencePFFracTensors off-diagonal Jacobian
  _dstress_dc[_qp] = -cfactor * stress0pos * 2.0 * (1.0 - c);
}
