//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HyperElasticPhaseFieldIsoDamage.h"
#include "libmesh/utility.h"

registerMooseObject("TensorMechanicsApp", HyperElasticPhaseFieldIsoDamage);

InputParameters
HyperElasticPhaseFieldIsoDamage::validParams()
{
  InputParameters params = FiniteStrainHyperElasticViscoPlastic::validParams();
  params.addParam<bool>("numerical_stiffness", false, "Flag for numerical stiffness");
  params.addParam<Real>("damage_stiffness", 1e-8, "Avoid zero after complete damage");
  params.addParam<Real>("zero_tol", 1e-12, "Tolerance for numerical zero");
  params.addParam<Real>(
      "zero_perturb", 1e-8, "Perturbation value when strain value less than numerical zero");
  params.addParam<Real>("perturbation_scale_factor", 1e-5, "Perturbation scale factor");
  params.addRequiredCoupledVar("c", "Damage variable");
  params.addParam<bool>(
      "use_current_history_variable", false, "Use the current value of the history variable.");
  params.addParam<MaterialPropertyName>(
      "F_name", "E_el", "Name of material property storing the elastic energy");
  params.addClassDescription(
      "Computes damaged stress and energy in the intermediate configuration assuming isotropy");

  return params;
}

HyperElasticPhaseFieldIsoDamage::HyperElasticPhaseFieldIsoDamage(const InputParameters & parameters)
  : FiniteStrainHyperElasticViscoPlastic(parameters),
    _num_stiffness(getParam<bool>("numerical_stiffness")),
    _kdamage(getParam<Real>("damage_stiffness")),
    _use_current_hist(getParam<bool>("use_current_history_variable")),
    _l(getMaterialProperty<Real>("l")),
    _gc(getMaterialProperty<Real>("gc_prop")),
    _zero_tol(getParam<Real>("zero_tol")),
    _zero_pert(getParam<Real>("zero_perturb")),
    _pert_val(getParam<Real>("perturbation_scale_factor")),
    _c(coupledValue("c")),
    _save_state(false),
    _dstress_dc(
        declarePropertyDerivative<RankTwoTensor>(_base_name + "stress", coupledName("c", 0))),
    _etens(LIBMESH_DIM),
    _F(declareProperty<Real>(getParam<MaterialPropertyName>("F_name"))),
    _dFdc(declarePropertyDerivative<Real>(getParam<MaterialPropertyName>("F_name"),
                                          coupledName("c", 0))),
    _d2Fdc2(declarePropertyDerivative<Real>(
        getParam<MaterialPropertyName>("F_name"), coupledName("c", 0), coupledName("c", 0))),
    _d2Fdcdstrain(declareProperty<RankTwoTensor>("d2Fdcdstrain")),
    _hist(declareProperty<Real>("hist")),
    _hist_old(getMaterialPropertyOld<Real>("hist"))
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
  for (const auto i : make_range(Moose::dim))
    for (const auto j : make_range(Moose::dim))
      for (const auto k : make_range(Moose::dim))
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

  std::vector<Real> eigval;
  RankTwoTensor evec;
  _ee.symmetricEigenvaluesEigenvectors(eigval, evec);

  for (const auto i : make_range(Moose::dim))
    _etens[i] = RankTwoTensor::selfOuterProduct(evec.column(i));

  Real etr = 0.0;
  for (const auto i : make_range(Moose::dim))
    etr += eigval[i];

  Real etrpos = (std::abs(etr) + etr) / 2.0;
  Real etrneg = (std::abs(etr) - etr) / 2.0;

  RankTwoTensor pk2pos, pk2neg;

  for (const auto i : make_range(Moose::dim))
  {
    pk2pos += _etens[i] * (lambda * etrpos + 2.0 * mu * (std::abs(eigval[i]) + eigval[i]) / 2.0);
    pk2neg += _etens[i] * (lambda * etrneg + 2.0 * mu * (std::abs(eigval[i]) - eigval[i]) / 2.0);
  }

  _pk2_tmp = pk2pos * xfac - pk2neg;

  if (_save_state)
  {
    std::vector<Real> epos(LIBMESH_DIM), eneg(LIBMESH_DIM);
    for (const auto i : make_range(Moose::dim))
    {
      epos[i] = (std::abs(eigval[i]) + eigval[i]) / 2.0;
      eneg[i] = (std::abs(eigval[i]) - eigval[i]) / 2.0;
    }

    // sum squares of epos and eneg
    Real pval(0.0), nval(0.0);
    for (const auto i : make_range(Moose::dim))
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

    Real hist_variable = _hist_old[_qp];
    if (_use_current_hist)
      hist_variable = _hist[_qp];

    // Elastic free energy density
    _F[_qp] = hist_variable * xfac - G0_neg + _gc[_qp] / (2 * _l[_qp]) * c * c;

    // derivative of elastic free energy density wrt c
    _dFdc[_qp] = -hist_variable * 2.0 * (1.0 - c) * (1 - _kdamage) + _gc[_qp] / _l[_qp] * c;

    // 2nd derivative of elastic free energy density wrt c
    _d2Fdc2[_qp] = hist_variable * 2.0 * (1 - _kdamage) + _gc[_qp] / _l[_qp];

    _dG0_dee = pk2pos;

    _dpk2_dc = -pk2pos * 2.0 * (1.0 - c);
  }
}

void
HyperElasticPhaseFieldIsoDamage::computeNumStiffness()
{
  RankTwoTensor ee_tmp;

  for (const auto i : make_range(Moose::dim))
    for (unsigned int j = i; j < LIBMESH_DIM; ++j)
    {
      Real ee_pert = _zero_pert;
      if (std::abs(_ee(i, j)) > _zero_tol)
        ee_pert = _pert_val * std::abs(_ee(i, j));

      ee_tmp = _ee;
      _ee(i, j) += ee_pert;

      computeDamageStress();

      for (const auto k : make_range(Moose::dim))
        for (const auto l : make_range(Moose::dim))
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

  // 2nd derivative wrt c and strain = 0.0 if we used the previous step's history varible
  if (_use_current_hist)
    _d2Fdcdstrain[_qp] =
        -_df_dstretch_inc.innerProductTranspose(dG0_df) * 2.0 * (1.0 - _c[_qp]) * (1 - _kdamage);

  usingTensorIndices(i_, j_, k_, l_);
  _dstress_dc[_qp] = _fe.times<i_, k_, j_, l_>(_fe) * _dpk2_dc;
}
