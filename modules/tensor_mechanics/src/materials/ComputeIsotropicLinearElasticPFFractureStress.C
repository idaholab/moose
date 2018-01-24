//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeIsotropicLinearElasticPFFractureStress.h"

template <>
InputParameters
validParams<ComputeIsotropicLinearElasticPFFractureStress>()
{
  InputParameters params = validParams<ComputeStressBase>();
  params.addClassDescription("Computes the stress and free energy derivatives for the phase field "
                             "fracture model, with linear isotropic elasticity");
  params.addRequiredCoupledVar("c", "Order parameter for damage");
  params.addParam<Real>("kdamage", 1e-6, "Stiffness of damaged matrix");
  params.addParam<MaterialPropertyName>(
      "F_name", "E_el", "Name of material property storing the elastic energy");
  return params;
}

ComputeIsotropicLinearElasticPFFractureStress::ComputeIsotropicLinearElasticPFFractureStress(
    const InputParameters & parameters)
  : ComputeStressBase(parameters),
    _c(coupledValue("c")),
    _kdamage(getParam<Real>("kdamage")),
    _l(getMaterialProperty<Real>("l")),
    _gc(getMaterialProperty<Real>("gc_prop")),
    _F(declareProperty<Real>(getParam<MaterialPropertyName>("F_name"))),
    _dFdc(declarePropertyDerivative<Real>(getParam<MaterialPropertyName>("F_name"),
                                          getVar("c", 0)->name())),
    _d2Fdc2(declarePropertyDerivative<Real>(
        getParam<MaterialPropertyName>("F_name"), getVar("c", 0)->name(), getVar("c", 0)->name())),
    _d2Fdcdstrain(declareProperty<RankTwoTensor>("d2Fdcdstrain")),
    _dstress_dc(
        declarePropertyDerivative<RankTwoTensor>(_base_name + "stress", getVar("c", 0)->name())),
    _hist(declareProperty<Real>("hist")),
    _hist_old(getMaterialPropertyOld<Real>("hist"))
{
}

void
ComputeIsotropicLinearElasticPFFractureStress::initQpStatefulProperties()
{
  _hist[_qp] = 0.0;
}

void
ComputeIsotropicLinearElasticPFFractureStress::computeQpStress()
{
  const Real c = _c[_qp];

  // Compute cfactor to have c > 1 behave the same as c = 1
  Real cfactor = 0.0;
  if (c < 1.0)
    cfactor = 1.0;

  // Isotropic elasticity is assumed and should be enforced
  const Real lambda = _elasticity_tensor[_qp](0, 0, 1, 1);
  const Real mu = _elasticity_tensor[_qp](0, 1, 0, 1);

  // Compute eigenvectors and eigenvalues of mechanical strain
  RankTwoTensor eigvec;
  std::vector<Real> eigval(LIBMESH_DIM);
  _mechanical_strain[_qp].symmetricEigenvaluesEigenvectors(eigval, eigvec);

  // Calculate tensors of outerproduct of eigen vectors
  std::vector<RankTwoTensor> etens(LIBMESH_DIM);

  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
        etens[i](j, k) = eigvec(j, i) * eigvec(k, i);

  // Separate out positive and negative eigen values
  std::vector<Real> epos(LIBMESH_DIM), eneg(LIBMESH_DIM);
  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
  {
    epos[i] = (std::abs(eigval[i]) + eigval[i]) / 2.0;
    eneg[i] = (std::abs(eigval[i]) - eigval[i]) / 2.0;
  }

  // Seprate positive and negative sums of all eigenvalues
  Real etr = 0.0;
  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    etr += eigval[i];

  const Real etrpos = (std::abs(etr) + etr) / 2.0;
  const Real etrneg = (std::abs(etr) - etr) / 2.0;

  // Calculate the tensile (postive) and compressive (negative) parts of stress
  RankTwoTensor stress0pos, stress0neg;
  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
  {
    stress0pos += etens[i] * (lambda * etrpos + 2.0 * mu * epos[i]);
    stress0neg += etens[i] * (lambda * etrneg + 2.0 * mu * eneg[i]);
  }

  // sum squares of epos and eneg
  Real pval(0.0), nval(0.0);
  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
  {
    pval += epos[i] * epos[i];
    nval += eneg[i] * eneg[i];
  }

  // Energy with positive principal strains
  const Real G0_pos = lambda * etrpos * etrpos / 2.0 + mu * pval;
  const Real G0_neg = lambda * etrneg * etrneg / 2.0 + mu * nval;

  // Assign history variable and derivative
  if (G0_pos > _hist_old[_qp])
    _hist[_qp] = G0_pos;
  else
    _hist[_qp] = _hist_old[_qp];

  // Damage associated with positive component of stress
  _stress[_qp] =
      stress0pos * cfactor * ((1.0 - c) * (1.0 - c) * (1 - _kdamage) + _kdamage) - stress0neg;

  // Elastic free energy density
  _F[_qp] = _hist[_qp] * cfactor * ((1.0 - c) * (1.0 - c) * (1 - _kdamage) + _kdamage) - G0_neg +
            _gc[_qp] / (2 * _l[_qp]) * c * c;

  // derivative of elastic free energy density wrt c
  _dFdc[_qp] = -_hist[_qp] * 2.0 * cfactor * (1.0 - c) * (1 - _kdamage) + _gc[_qp] / _l[_qp] * c;

  // 2nd derivative of elastic free energy density wrt c
  _d2Fdc2[_qp] = _hist[_qp] * 2.0 * cfactor * (1 - _kdamage) + _gc[_qp] / _l[_qp];

  // 2nd derivative wrt c and strain. Note that I am ignoring the history variable here, but this
  // approach gets the fastest convergence
  _d2Fdcdstrain[_qp] = -stress0pos * cfactor * (1.0 - c) * (1 - _kdamage);

  // Used in StressDivergencePFFracTensors off-diagonal Jacobian
  _dstress_dc[_qp] = -stress0pos * 2.0 * cfactor * (1.0 - c) * (1 - _kdamage);

  // Estimate Jacobian mult, exact when c = 0 and VERY APPROXIMATE when c = 1
  _Jacobian_mult[_qp] = _elasticity_tensor[_qp];
}
