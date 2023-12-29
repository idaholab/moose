//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeLinearElasticPFFractureStress.h"
#include "MathUtils.h"

registerMooseObject("TensorMechanicsApp", ComputeLinearElasticPFFractureStress);

InputParameters
ComputeLinearElasticPFFractureStress::validParams()
{
  InputParameters params = ComputePFFractureStressBase::validParams();
  params.addClassDescription("Computes the stress and free energy derivatives for the phase field "
                             "fracture model, with small strain");
  MooseEnum Decomposition("strain_spectral strain_vol_dev stress_spectral none", "none");
  params.addParam<MooseEnum>("decomposition_type",
                             Decomposition,
                             "Decomposition approaches.  Choices are: " +
                                 Decomposition.getRawNames());
  return params;
}

ComputeLinearElasticPFFractureStress::ComputeLinearElasticPFFractureStress(
    const InputParameters & parameters)
  : ComputePFFractureStressBase(parameters),
    GuaranteeConsumer(this),
    _decomposition_type(getParam<MooseEnum>("decomposition_type").getEnum<Decomposition_type>())
{
}

void
ComputeLinearElasticPFFractureStress::initialSetup()
{
  if ((_decomposition_type == Decomposition_type::strain_vol_dev ||
       _decomposition_type == Decomposition_type::strain_spectral) &&
      !hasGuaranteedMaterialProperty(_elasticity_tensor_name, Guarantee::ISOTROPIC))
    mooseError("Decomposition approach of strain_vol_dev and strain_spectral can only be used with "
               "isotropic elasticity tensor materials, use stress_spectral for anistropic "
               "elasticity tensor materials");
}

void
ComputeLinearElasticPFFractureStress::computeStrainSpectral(Real & F_pos, Real & F_neg)
{
  // Isotropic elasticity is assumed and should be enforced
  const Real lambda = _elasticity_tensor[_qp](0, 0, 1, 1);
  const Real mu = _elasticity_tensor[_qp](0, 1, 0, 1);

  RankTwoTensor I2(RankTwoTensor::initIdentity);

  // Compute eigenvectors and eigenvalues of mechanical strain and projection tensor
  RankTwoTensor eigvec;
  std::vector<Real> eigval(LIBMESH_DIM);
  RankFourTensor Ppos =
      _mechanical_strain[_qp].positiveProjectionEigenDecomposition(eigval, eigvec);
  RankFourTensor I4sym(RankFourTensor::initIdentitySymmetricFour);

  // Calculate tensors of outerproduct of eigen vectors
  std::vector<RankTwoTensor> etens(LIBMESH_DIM);

  for (const auto i : make_range(Moose::dim))
    etens[i] = RankTwoTensor::selfOuterProduct(eigvec.column(i));

  // Separate out positive and negative eigen values
  std::vector<Real> epos(LIBMESH_DIM), eneg(LIBMESH_DIM);
  for (const auto i : make_range(Moose::dim))
  {
    epos[i] = (std::abs(eigval[i]) + eigval[i]) / 2.0;
    eneg[i] = -(std::abs(eigval[i]) - eigval[i]) / 2.0;
  }

  // Seprate positive and negative sums of all eigenvalues
  Real etr = 0.0;
  for (const auto i : make_range(Moose::dim))
    etr += eigval[i];

  const Real etrpos = (std::abs(etr) + etr) / 2.0;
  const Real etrneg = -(std::abs(etr) - etr) / 2.0;

  // Calculate the tensile (postive) and compressive (negative) parts of stress
  RankTwoTensor stress0pos, stress0neg;
  for (const auto i : make_range(Moose::dim))
  {
    stress0pos += etens[i] * (lambda * etrpos + 2.0 * mu * epos[i]);
    stress0neg += etens[i] * (lambda * etrneg + 2.0 * mu * eneg[i]);
  }

  // sum squares of epos and eneg
  Real pval(0.0), nval(0.0);
  for (const auto i : make_range(Moose::dim))
  {
    pval += epos[i] * epos[i];
    nval += eneg[i] * eneg[i];
  }

  _stress[_qp] = stress0pos * _D[_qp] - _pressure[_qp] * I2 * _I[_qp] + stress0neg;

  // Energy with positive principal strains
  F_pos = lambda * etrpos * etrpos / 2.0 + mu * pval;
  F_neg = -lambda * etrneg * etrneg / 2.0 + mu * nval;

  // 2nd derivative wrt c and strain = 0.0 if we used the previous step's history varible
  if (_use_current_hist)
    _d2Fdcdstrain[_qp] = stress0pos * _dDdc[_qp];

  // Used in StressDivergencePFFracTensors off-diagonal Jacobian
  _dstress_dc[_qp] = stress0pos * _dDdc[_qp] - _pressure[_qp] * I2 * _dIdc[_qp];

  _Jacobian_mult[_qp] = (I4sym - (1 - _D[_qp]) * Ppos) * _elasticity_tensor[_qp];
}

void
ComputeLinearElasticPFFractureStress::computeStressSpectral(Real & F_pos, Real & F_neg)
{
  // Compute Uncracked stress
  RankTwoTensor stress = _elasticity_tensor[_qp] * _mechanical_strain[_qp];

  RankTwoTensor I2(RankTwoTensor::initIdentity);

  // Create the positive and negative projection tensors
  RankFourTensor I4sym(RankFourTensor::initIdentitySymmetricFour);
  std::vector<Real> eigval;
  RankTwoTensor eigvec;
  RankFourTensor Ppos = stress.positiveProjectionEigenDecomposition(eigval, eigvec);

  // Project the positive and negative stresses
  RankTwoTensor stress0pos = Ppos * stress;
  RankTwoTensor stress0neg = stress - stress0pos;

  // Compute the positive and negative elastic energies
  F_pos = (stress0pos).doubleContraction(_mechanical_strain[_qp]) / 2.0;
  F_neg = (stress0neg).doubleContraction(_mechanical_strain[_qp]) / 2.0;

  _stress[_qp] = stress0pos * _D[_qp] - _pressure[_qp] * I2 * _I[_qp] + stress0neg;

  // 2nd derivative wrt c and strain = 0.0 if we used the previous step's history varible
  if (_use_current_hist)
    _d2Fdcdstrain[_qp] = stress0pos * _dDdc[_qp];

  // Used in StressDivergencePFFracTensors off-diagonal Jacobian
  _dstress_dc[_qp] = stress0pos * _dDdc[_qp] - _pressure[_qp] * I2 * _dIdc[_qp];

  _Jacobian_mult[_qp] = (I4sym - (1 - _D[_qp]) * Ppos) * _elasticity_tensor[_qp];
}

void
ComputeLinearElasticPFFractureStress::computeStrainVolDev(Real & F_pos, Real & F_neg)
{
  // Isotropic elasticity is assumed and should be enforced
  const Real lambda = _elasticity_tensor[_qp](0, 0, 1, 1);
  const Real mu = _elasticity_tensor[_qp](0, 1, 0, 1);
  const Real k = lambda + 2.0 * mu / LIBMESH_DIM;

  RankTwoTensor I2(RankTwoTensor::initIdentity);
  RankFourTensor I2I2 = I2.outerProduct(I2);

  RankFourTensor Jacobian_pos, Jacobian_neg;
  RankTwoTensor strain0vol, strain0dev;
  RankTwoTensor stress0pos, stress0neg;
  Real strain0tr, strain0tr_neg, strain0tr_pos;

  strain0dev = _mechanical_strain[_qp].deviatoric();
  strain0vol = _mechanical_strain[_qp] - strain0dev;
  strain0tr = _mechanical_strain[_qp].trace();
  strain0tr_neg = std::min(strain0tr, 0.0);
  strain0tr_pos = strain0tr - strain0tr_neg;
  stress0neg = k * strain0tr_neg * I2;
  stress0pos = _elasticity_tensor[_qp] * _mechanical_strain[_qp] - stress0neg;
  // Energy with positive principal strains
  RankTwoTensor strain0dev2 = strain0dev * strain0dev;
  F_pos = 0.5 * k * strain0tr_pos * strain0tr_pos + mu * strain0dev2.trace();
  F_neg = 0.5 * k * strain0tr_neg * strain0tr_neg;

  _stress[_qp] = stress0pos * _D[_qp] - _pressure[_qp] * I2 * _I[_qp] + stress0neg;

  // 2nd derivative wrt c and strain = 0.0 if we used the previous step's history varible
  if (_use_current_hist)
    _d2Fdcdstrain[_qp] = stress0pos * _dDdc[_qp];

  // Used in StressDivergencePFFracTensors off-diagonal Jacobian
  _dstress_dc[_qp] = stress0pos * _dDdc[_qp] - _pressure[_qp] * I2 * _dIdc[_qp];

  if (strain0tr < 0)
    Jacobian_neg = k * I2I2;
  Jacobian_pos = _elasticity_tensor[_qp] - Jacobian_neg;
  _Jacobian_mult[_qp] = _D[_qp] * Jacobian_pos + Jacobian_neg;
}

void
ComputeLinearElasticPFFractureStress::computeQpStress()
{
  Real F_pos, F_neg;
  RankTwoTensor I2(RankTwoTensor::initIdentity);

  switch (_decomposition_type)
  {
    case Decomposition_type::strain_spectral:
      computeStrainSpectral(F_pos, F_neg);
      break;
    case Decomposition_type::strain_vol_dev:
      computeStrainVolDev(F_pos, F_neg);
      break;
    case Decomposition_type::stress_spectral:
      computeStressSpectral(F_pos, F_neg);
      break;
    default:
    {
      RankTwoTensor stress = _elasticity_tensor[_qp] * _mechanical_strain[_qp];
      F_pos = stress.doubleContraction(_mechanical_strain[_qp]) / 2.0;
      F_neg = 0.0;
      if (_use_current_hist)
        _d2Fdcdstrain[_qp] = stress * _dDdc[_qp];

      _stress[_qp] = _D[_qp] * stress - _pressure[_qp] * I2 * _I[_qp];
      _dstress_dc[_qp] = stress * _dDdc[_qp] - _pressure[_qp] * I2 * _dIdc[_qp];
      _Jacobian_mult[_qp] = _D[_qp] * _elasticity_tensor[_qp];
    }
  }

  // // Assign history variable
  Real hist_variable = _H_old[_qp];
  if (_use_snes_vi_solver)
  {
    _H[_qp] = F_pos;

    if (_use_current_hist)
      hist_variable = _H[_qp];
  }
  else
  {
    if (F_pos > _H_old[_qp])
      _H[_qp] = F_pos;
    else
      _H[_qp] = _H_old[_qp];

    if (_use_current_hist)
      hist_variable = _H[_qp];

    if (hist_variable < _barrier[_qp])
      hist_variable = _barrier[_qp];
  }

  // Elastic free energy density
  _E[_qp] =
      hist_variable * _D[_qp] + F_neg - _pressure[_qp] * _mechanical_strain[_qp].trace() * _I[_qp];
  _dEdc[_qp] =
      hist_variable * _dDdc[_qp] - _pressure[_qp] * _mechanical_strain[_qp].trace() * _dIdc[_qp];
  _d2Ed2c[_qp] = hist_variable * _d2Dd2c[_qp] -
                 _pressure[_qp] * _mechanical_strain[_qp].trace() * _d2Id2c[_qp];
}
