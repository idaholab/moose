/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "HyperElasticPhaseFieldIsoDamage.h"
#include "libmesh/utility.h"

template <>
InputParameters
validParams<HyperElasticPhaseFieldIsoDamage>()
{
  InputParameters params = validParams<FiniteStrainHyperElasticViscoPlastic>();
  params.addParam<bool>("numerical_stiffness", false, "Flag for numerical stiffness");
  params.addParam<Real>("damage_stiffness", 1e-8, "Avoid zero after complete damage");
  params.addParam<Real>("zero_tol", 1e-12, "Tolerance for numerical zero");
  params.addParam<Real>(
      "zero_perturb", 1e-8, "Perturbation value when strain value less than numerical zero");
  params.addParam<Real>("perturbation_scale_factor", 1e-5, "Perturbation scale factor");
  params.addRequiredCoupledVar("c", "Damage variable");
  params.addClassDescription(
      "Computes damaged stress and energy in the intermediate configuration assuming isotropy");

  return params;
}

HyperElasticPhaseFieldIsoDamage::HyperElasticPhaseFieldIsoDamage(const InputParameters & parameters)
  : FiniteStrainHyperElasticViscoPlastic(parameters),
    _num_stiffness(getParam<bool>("numerical_stiffness")),
    _kdamage(getParam<Real>("damage_stiffness")),
    _zero_tol(getParam<Real>("zero_tol")),
    _zero_pert(getParam<Real>("zero_perturb")),
    _pert_val(getParam<Real>("perturbation_scale_factor")),
    _c(coupledValue("c")),
    _save_state(false),
    _G0(declareProperty<Real>(_base_name + "G0")),
    _dG0_dstrain(declareProperty<RankTwoTensor>(_base_name + "dG0_dstrain")),
    _dstress_dc(
        declarePropertyDerivative<RankTwoTensor>(_base_name + "stress", getVar("c", 0)->name())),
    _etens(LIBMESH_DIM)
{
}

void
HyperElasticPhaseFieldIsoDamage::computePK2StressAndDerivative()
{
  computeElasticStrain();

  _save_state = true;
  computeDamageStress();
  _pk2[_qp] = _pk2_tmp;

  _save_state = false;
  if (_num_stiffness)
    computeNumStiffness();

  if (_num_stiffness)
    _dpk2_dce = _dpk2_dee * _dee_dce;

  _dce_dfe.zero();
  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
      {
        _dce_dfe(i, j, k, i) = _dce_dfe(i, j, k, i) + _fe(k, j);
        _dce_dfe(i, j, k, j) = _dce_dfe(i, j, k, j) + _fe(k, i);
      }

  _dpk2_dfe = _dpk2_dce * _dce_dfe;
}

void
HyperElasticPhaseFieldIsoDamage::computeDamageStress()
{
  Real lambda = _elasticity_tensor[_qp](0, 0, 1, 1);
  Real mu = _elasticity_tensor[_qp](0, 1, 0, 1);

  Real c = _c[_qp];
  Real xfac = Utility::pow<2>(1.0 - c) + _kdamage;

  std::vector<Real> w;
  RankTwoTensor evec;
  _ee.symmetricEigenvaluesEigenvectors(w, evec);

  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    _etens[i].vectorOuterProduct(evec.column(i), evec.column(i));

  Real etr = 0.0;
  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    etr += w[i];

  Real etrpos = (std::abs(etr) + etr) / 2.0;
  Real etrneg = (std::abs(etr) - etr) / 2.0;

  RankTwoTensor pk2pos, pk2neg;

  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
  {
    pk2pos += _etens[i] * (lambda * etrpos + 2.0 * mu * (std::abs(w[i]) + w[i]) / 2.0);
    pk2neg += _etens[i] * (lambda * etrneg + 2.0 * mu * (std::abs(w[i]) - w[i]) / 2.0);
  }

  _pk2_tmp = pk2pos * xfac - pk2neg;

  if (_save_state)
  {
    std::vector<Real> epos(LIBMESH_DIM);
    for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
      epos[i] = (std::abs(w[i]) + w[i]) / 2.0;

    _G0[_qp] = 0.0;
    for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
      _G0[_qp] += Utility::pow<2>(epos[i]);
    _G0[_qp] *= mu;
    _G0[_qp] += lambda * Utility::pow<2>(etrpos) / 2.0;

    _dG0_dee = pk2pos;
    _dpk2_dc = -pk2pos * (2.0 * (1.0 - c));
  }
}

void
HyperElasticPhaseFieldIsoDamage::computeNumStiffness()
{
  RankTwoTensor ee_tmp;

  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = i; j < LIBMESH_DIM; ++j)
    {
      Real ee_pert = _zero_pert;
      if (std::abs(_ee(i, j)) > _zero_tol)
        ee_pert = _pert_val * std::abs(_ee(i, j));

      ee_tmp = _ee;
      _ee(i, j) += ee_pert;

      computeDamageStress();

      for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
        for (unsigned int l = 0; l < LIBMESH_DIM; ++l)
        {
          _dpk2_dee(k, l, i, j) = (_pk2_tmp(k, l) - _pk2[_qp](k, l)) / ee_pert;
          _dpk2_dee(k, l, j, i) = (_pk2_tmp(k, l) - _pk2[_qp](k, l)) / ee_pert;
        }
      _ee = ee_tmp;
    }
}

void
HyperElasticPhaseFieldIsoDamage::computeQpJacobian()
{
  FiniteStrainHyperElasticViscoPlastic::computeQpJacobian();

  RankTwoTensor dG0_dce = _dee_dce.innerProductTranspose(_dG0_dee);
  RankTwoTensor dG0_dfe = _dce_dfe.innerProductTranspose(dG0_dce);
  RankTwoTensor dG0_df = _dfe_df.innerProductTranspose(dG0_dfe);

  _dG0_dstrain[_qp] = _df_dstretch_inc.innerProductTranspose(dG0_df);
  _dstress_dc[_qp] = _fe.mixedProductIkJl(_fe) * _dpk2_dc;
}
