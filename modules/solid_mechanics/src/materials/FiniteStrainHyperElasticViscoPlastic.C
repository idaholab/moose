//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FiniteStrainHyperElasticViscoPlastic.h"
#include "libmesh/utility.h"

registerMooseObject("TensorMechanicsApp", FiniteStrainHyperElasticViscoPlastic);

InputParameters
FiniteStrainHyperElasticViscoPlastic::validParams()
{
  InputParameters params = ComputeStressBase::validParams();
  params.addParam<Real>(
      "resid_abs_tol", 1e-10, "Absolute Tolerance for flow rate residual equation");
  params.addParam<Real>(
      "resid_rel_tol", 1e-6, "Relative Tolerance for flow rate residual equation");
  params.addParam<unsigned int>("maxiters", 50, "Maximum iteration for flow rate update");
  params.addParam<unsigned int>("max_substep_iteration", 1, "Maximum number of substep iteration");
  params.addParam<std::vector<UserObjectName>>(
      "flow_rate_user_objects",
      "List of User object names that computes flow rate and derivatives");
  params.addParam<std::vector<UserObjectName>>(
      "strength_user_objects",
      "List of User object names that computes strength variables and derivatives");
  params.addParam<std::vector<UserObjectName>>(
      "internal_var_user_objects",
      "List of User object names that integrates internal variables and computes derivatives");
  params.addParam<std::vector<UserObjectName>>(
      "internal_var_rate_user_objects",
      "List of User object names that computes internal variable rates and derivatives");
  params.addClassDescription("Material class for hyper-elastic viscoplatic flow: Can handle "
                             "multiple flow models defined by flowratemodel type user objects");

  return params;
}

FiniteStrainHyperElasticViscoPlastic::FiniteStrainHyperElasticViscoPlastic(
    const InputParameters & parameters)
  : ComputeStressBase(parameters),
    _resid_abs_tol(getParam<Real>("resid_abs_tol")),
    _resid_rel_tol(getParam<Real>("resid_rel_tol")),
    _maxiters(getParam<unsigned int>("maxiters")),
    _max_substep_iter(getParam<unsigned int>("max_substep_iteration")),
    _flow_rate_uo_names(isParamValid("flow_rate_user_objects")
                            ? getParam<std::vector<UserObjectName>>("flow_rate_user_objects")
                            : std::vector<UserObjectName>(0)),
    _strength_uo_names(isParamValid("strength_user_objects")
                           ? getParam<std::vector<UserObjectName>>("strength_user_objects")
                           : std::vector<UserObjectName>(0)),
    _int_var_uo_names(isParamValid("internal_var_user_objects")
                          ? getParam<std::vector<UserObjectName>>("internal_var_user_objects")
                          : std::vector<UserObjectName>(0)),
    _int_var_rate_uo_names(
        isParamValid("internal_var_rate_user_objects")
            ? getParam<std::vector<UserObjectName>>("internal_var_rate_user_objects")
            : std::vector<UserObjectName>(0)),
    _pk2_prop_name(_base_name + "pk2"),
    _pk2(declareProperty<RankTwoTensor>(_pk2_prop_name)),
    _fp(declareProperty<RankTwoTensor>(_base_name + "fp")),
    _fp_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "fp")),
    _ce(declareProperty<RankTwoTensor>(_base_name + "ce")),
    _elasticity_tensor_name(_base_name + "elasticity_tensor"),
    _elasticity_tensor(getMaterialPropertyByName<RankFourTensor>(_elasticity_tensor_name)),
    _deformation_gradient(getMaterialProperty<RankTwoTensor>(_base_name + "deformation_gradient")),
    _deformation_gradient_old(
        getMaterialPropertyOld<RankTwoTensor>(_base_name + "deformation_gradient")),
    _rotation_increment(getMaterialProperty<RankTwoTensor>(_base_name + "rotation_increment"))
{
  initUOVariables();

  initJacobianVariables();

  _dflow_rate.resize(_num_flow_rate_uos);
  _flow_rate.resize(_num_flow_rate_uos);
  _resid.resize(_num_flow_rate_uos);

  _flow_dirn.resize(_num_flow_rate_uos);
}

void
FiniteStrainHyperElasticViscoPlastic::initUOVariables()
{
  initNumUserObjects(_flow_rate_uo_names, _num_flow_rate_uos);
  initNumUserObjects(_strength_uo_names, _num_strength_uos);
  initNumUserObjects(_int_var_uo_names, _num_int_var_uos);
  initNumUserObjects(_int_var_rate_uo_names, _num_int_var_rate_uos);

  initProp(_flow_rate_uo_names, _num_flow_rate_uos, _flow_rate_prop);
  initProp(_strength_uo_names, _num_strength_uos, _strength_prop);
  initProp(_int_var_uo_names, _num_int_var_uos, _int_var_stateful_prop);
  initProp(_int_var_rate_uo_names, _num_int_var_rate_uos, _int_var_rate_prop);

  initPropOld(_int_var_uo_names, _num_int_var_uos, _int_var_stateful_prop_old);

  initUserObjects(_flow_rate_uo_names, _num_flow_rate_uos, _flow_rate_uo);
  initUserObjects(_strength_uo_names, _num_strength_uos, _strength_uo);
  initUserObjects(_int_var_uo_names, _num_int_var_uos, _int_var_uo);
  initUserObjects(_int_var_rate_uo_names, _num_int_var_rate_uos, _int_var_rate_uo);

  _int_var_old.resize(_num_int_var_uos, 0.0);
}

void
FiniteStrainHyperElasticViscoPlastic::initNumUserObjects(
    const std::vector<UserObjectName> & uo_names, unsigned int & uo_num)
{
  uo_num = uo_names.size();
}

template <typename T>
void
FiniteStrainHyperElasticViscoPlastic::initProp(const std::vector<UserObjectName> & uo_names,
                                               unsigned int uo_num,
                                               std::vector<MaterialProperty<T> *> & uo_prop)
{
  uo_prop.resize(uo_num);
  for (unsigned int i = 0; i < uo_num; ++i)
    uo_prop[i] = &declareProperty<T>(uo_names[i]);
}

template <typename T>
void
FiniteStrainHyperElasticViscoPlastic::initPropOld(
    const std::vector<UserObjectName> & uo_names,
    unsigned int uo_num,
    std::vector<const MaterialProperty<T> *> & uo_prop_old)
{
  uo_prop_old.resize(uo_num);
  for (unsigned int i = 0; i < uo_num; ++i)
    uo_prop_old[i] = &getMaterialPropertyOld<T>(uo_names[i]);
}

template <typename T>
void
FiniteStrainHyperElasticViscoPlastic::initUserObjects(const std::vector<UserObjectName> & uo_names,
                                                      unsigned int uo_num,
                                                      std::vector<const T *> & uo)
{
  uo.resize(uo_num);

  if (uo_num == 0)
    mooseError("Specify atleast one user object of type", typeid(T).name());

  for (unsigned int i = 0; i < uo_num; ++i)
    uo[i] = &getUserObjectByName<T>(uo_names[i]);
}

void
FiniteStrainHyperElasticViscoPlastic::initJacobianVariables()
{
  _dintvarrate_dflowrate.resize(_num_flow_rate_uos);
  for (unsigned int i = 0; i < _num_flow_rate_uos; ++i)
    _dintvarrate_dflowrate[i].resize(_num_int_var_rate_uos);

  _dintvar_dflowrate_tmp.resize(_num_flow_rate_uos);
  for (unsigned int i = 0; i < _num_flow_rate_uos; ++i)
    _dintvar_dflowrate_tmp[i].resize(_num_int_var_uos);

  _dintvarrate_dintvar.resize(_num_int_var_rate_uos, _num_int_var_uos);
  _dintvar_dintvarrate.resize(_num_int_var_uos, _num_int_var_rate_uos);
  _dintvar_dflowrate.resize(_num_int_var_uos, _num_flow_rate_uos);
  _dintvar_dintvar.resize(_num_int_var_uos, _num_int_var_uos);
  _dstrength_dintvar.resize(_num_strength_uos, _num_int_var_uos);
  _dflowrate_dstrength.resize(_num_flow_rate_uos, _num_strength_uos);
  _dintvar_dintvar_x.resize(_num_int_var_uos);

  _dpk2_dflowrate.resize(_num_flow_rate_uos);
  _dflowrate_dpk2.resize(_num_flow_rate_uos);
  _dfpinv_dflowrate.resize(_num_flow_rate_uos);

  _jac.resize(_num_flow_rate_uos, _num_flow_rate_uos);

  computeDeeDce();
}

void
FiniteStrainHyperElasticViscoPlastic::initQpStatefulProperties()
{
  _stress[_qp].zero();
  _ce[_qp].zero();
  _pk2[_qp].zero();
  _fp[_qp].setToIdentity();

  for (unsigned int i = 0; i < _num_flow_rate_uos; ++i)
    (*_flow_rate_prop[i])[_qp] = 0.0;

  for (unsigned int i = 0; i < _num_strength_uos; ++i)
    (*_strength_prop[i])[_qp] = 0.0;

  for (unsigned int i = 0; i < _num_int_var_uos; ++i)
  {
    (*_int_var_stateful_prop[i])[_qp] = 0.0;
    // TODO: remove this nasty const_cast if you can figure out how
    const_cast<MaterialProperty<Real> &>(*_int_var_stateful_prop_old[i])[_qp] = 0.0;
  }

  for (unsigned int i = 0; i < _num_int_var_rate_uos; ++i)
    (*_int_var_rate_prop[i])[_qp] = 0.0;
}

void
FiniteStrainHyperElasticViscoPlastic::computeQpStress()
{
  bool converge;
  RankTwoTensor delta_dfgrd = _deformation_gradient[_qp] - _deformation_gradient_old[_qp];
  unsigned int num_substep = 1;
  unsigned int substep_iter = 1;

  saveOldState();

  do
  {
    preSolveQp();

    converge = true;
    _dt_substep = _dt / num_substep;

    for (unsigned int istep = 0; istep < num_substep; ++istep)
    {
      _dfgrd_tmp = (istep + 1.0) * delta_dfgrd / num_substep + _deformation_gradient_old[_qp];
      if (!solveQp())
      {
        converge = false;
        substep_iter++;
        num_substep *= 2;
        break;
      }
    }

    if (substep_iter > _max_substep_iter)
      mooseError("Constitutive failure with substepping at quadrature point ",
                 _q_point[_qp](0),
                 " ",
                 _q_point[_qp](1),
                 " ",
                 _q_point[_qp](2));
  } while (!converge);

  postSolveQp();
}

void
FiniteStrainHyperElasticViscoPlastic::saveOldState()
{
  for (unsigned int i = 0; i < _num_int_var_uos; ++i)
    _int_var_old[i] = (*_int_var_stateful_prop_old[i])[_qp];
}

void
FiniteStrainHyperElasticViscoPlastic::preSolveQp()
{
  _fp_tmp_old_inv = _fp_old[_qp].inverse();

  // TODO: remove this nasty const_cast if you can figure out how
  for (unsigned int i = 0; i < _num_int_var_uos; ++i)
    (*_int_var_stateful_prop[i])[_qp] =
        const_cast<MaterialProperty<Real> &>(*_int_var_stateful_prop_old[i])[_qp] = _int_var_old[i];

  _dpk2_dce = _elasticity_tensor[_qp] * _dee_dce;
}

bool
FiniteStrainHyperElasticViscoPlastic::solveQp()
{
  preSolveFlowrate();
  if (!solveFlowrate())
    return false;
  postSolveFlowrate();

  return true;
}

void
FiniteStrainHyperElasticViscoPlastic::postSolveQp()
{
  recoverOldState();

  _stress[_qp] = _fe * _pk2[_qp] * _fe.transpose() / _fe.det();
  _fp[_qp] = _fp_tmp_inv.inverse();

  computeQpJacobian();
}

void
FiniteStrainHyperElasticViscoPlastic::recoverOldState()
{
  // TODO: remove this nasty const_cast if you can figure out how
  for (unsigned int i = 0; i < _num_int_var_uos; ++i)
    const_cast<MaterialProperty<Real> &>(*_int_var_stateful_prop_old[i])[_qp] = _int_var_old[i];
}

void
FiniteStrainHyperElasticViscoPlastic::preSolveFlowrate()
{
  for (unsigned int i = 0; i < _num_flow_rate_uos; ++i)
  {
    _flow_rate(i) = 0.0;
    (*_flow_rate_prop[i])[_qp] = 0.0;
  }

  for (unsigned int i = 0; i < _num_int_var_uos; ++i)
    (*_int_var_stateful_prop[i])[_qp] = (*_int_var_stateful_prop_old[i])[_qp];

  _fp_tmp_inv = _fp_tmp_old_inv;
  _fe = _dfgrd_tmp * _fp_tmp_inv;
}

bool
FiniteStrainHyperElasticViscoPlastic::solveFlowrate()
{
  Real resid0, rnorm;
  unsigned int iter = 0;

#ifdef DEBUG
  std::vector<Real> rnormst(_maxiters + 1), flowratest(_maxiters + 1);
#endif

  if (!computeFlowRateResidual())
    return false;

  rnorm = computeNorm(_resid.get_values());
  resid0 = rnorm;

#ifdef DEBUG
  rnormst[iter] = rnorm;
  flowratest[iter] = computeNorm(_flow_rate.get_values());
#endif

  while (rnorm > _resid_abs_tol && rnorm > _resid_rel_tol * resid0 && iter < _maxiters)
  {
    computeFlowRateJacobian();

    updateFlowRate();

    computeElasticPlasticDeformGrad();

    if (!computeFlowRateResidual())
      return false;

    rnorm = computeNorm(_resid.get_values());
    iter++;

#ifdef DEBUG
    rnormst[iter] = rnorm;
    flowratest[iter] = computeNorm(_flow_rate.get_values());
#endif
  }

  if (iter == _maxiters && rnorm > _resid_abs_tol && rnorm > _resid_rel_tol * resid0)
    return false;

  return true;
}

void
FiniteStrainHyperElasticViscoPlastic::postSolveFlowrate()
{
  _fp_tmp_old_inv = _fp_tmp_inv;

  // TODO: remove this nasty const_cast if you can figure out how
  for (unsigned int i = 0; i < _num_int_var_uos; ++i)
    const_cast<MaterialProperty<Real> &>(*_int_var_stateful_prop_old[i])[_qp] =
        (*_int_var_stateful_prop[i])[_qp];
}

bool
FiniteStrainHyperElasticViscoPlastic::computeFlowRateResidual()
{
  if (!computeIntVarRates())
    return false;

  if (!computeIntVar())
    return false;

  if (!computeStrength())
    return false;

  computeElasticRightCauchyGreenTensor();
  computePK2StressAndDerivative();

  if (!computeFlowRateFunction())
    return false;

  if (!computeFlowDirection())
    return false;

  _resid += _flow_rate;

  return true;
}

void
FiniteStrainHyperElasticViscoPlastic::computeFlowRateJacobian()
{
  computeIntVarRateDerivatives();
  computeIntVarDerivatives();
  computeStrengthDerivatives();

  for (unsigned int i = 0; i < _num_flow_rate_uos; ++i)
    for (unsigned int j = 0; j < _num_strength_uos; ++j)
      _flow_rate_uo[i]->computeDerivative(_qp, _strength_uo_names[j], _dflowrate_dstrength(i, j));

  for (unsigned int i = 0; i < _num_flow_rate_uos; ++i)
    _flow_rate_uo[i]->computeTensorDerivative(_qp, _pk2_prop_name, _dflowrate_dpk2[i]);

  computeDpk2Dfpinv();

  for (unsigned int i = 0; i < _num_flow_rate_uos; ++i)
  {
    _dfpinv_dflowrate[i] = -_fp_tmp_old_inv * _flow_dirn[i] * _dt_substep;
    _dpk2_dflowrate[i] = _dpk2_dfpinv * _dfpinv_dflowrate[i];
  }

  DenseMatrix<Real> dflowrate_dflowrate;
  dflowrate_dflowrate = _dflowrate_dstrength;
  dflowrate_dflowrate.right_multiply(_dstrength_dintvar);
  dflowrate_dflowrate.right_multiply(_dintvar_dflowrate);

  _jac.zero();
  for (unsigned int i = 0; i < _num_flow_rate_uos; ++i)
    for (unsigned int j = 0; j < _num_flow_rate_uos; ++j)
    {
      if (i == j)
        _jac(i, j) = 1;
      _jac(i, j) -= dflowrate_dflowrate(i, j);
      _jac(i, j) -= _dflowrate_dpk2[i].doubleContraction(_dpk2_dflowrate[j]);
    }
}

bool
FiniteStrainHyperElasticViscoPlastic::computeFlowDirection()
{
  for (unsigned i = 0; i < _num_flow_rate_uos; ++i)
  {
    if (!_flow_rate_uo[i]->computeDirection(_qp, _flow_dirn[i]))
      return false;
  }
  return true;
}

bool
FiniteStrainHyperElasticViscoPlastic::computeFlowRateFunction()
{
  Real val = 0;
  for (unsigned i = 0; i < _num_flow_rate_uos; ++i)
  {
    if (_flow_rate_uo[i]->computeValue(_qp, val))
      _resid(i) = -val;
    else
      return false;
  }
  return true;
}

void
FiniteStrainHyperElasticViscoPlastic::computePK2StressAndDerivative()
{
  computeElasticStrain();
  _pk2[_qp] = _elasticity_tensor[_qp] * _ee;

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
FiniteStrainHyperElasticViscoPlastic::computeElasticStrain()
{
  RankTwoTensor iden(RankTwoTensor::initIdentity);
  _ee = 0.5 * (_ce[_qp] - iden);
}

void
FiniteStrainHyperElasticViscoPlastic::computeDeeDce()
{
  _dee_dce.zero();

  for (const auto i : make_range(Moose::dim))
    for (const auto j : make_range(Moose::dim))
      _dee_dce(i, j, i, j) = 0.5;
}

void
FiniteStrainHyperElasticViscoPlastic::computeElasticRightCauchyGreenTensor()
{
  _ce[_qp] = _fe.transpose() * _fe;
}

void
FiniteStrainHyperElasticViscoPlastic::computeElasticPlasticDeformGrad()
{
  RankTwoTensor iden(RankTwoTensor::initIdentity);

  RankTwoTensor val;
  for (unsigned int i = 0; i < _num_flow_rate_uos; ++i)
    val += _flow_rate(i) * _flow_dirn[i] * _dt_substep;

  _fp_tmp_inv = _fp_tmp_old_inv * (iden - val);
  _fp_tmp_inv = std::pow(_fp_tmp_inv.det(), -1.0 / 3.0) * _fp_tmp_inv;
  _fe = _dfgrd_tmp * _fp_tmp_inv;
}

void
FiniteStrainHyperElasticViscoPlastic::computeDpk2Dfpinv()
{
  for (const auto i : make_range(Moose::dim))
    for (const auto j : make_range(Moose::dim))
      for (const auto k : make_range(Moose::dim))
        _dfe_dfpinv(i, j, k, j) = _dfgrd_tmp(i, k);

  _dpk2_dfpinv = _dpk2_dce * _dce_dfe * _dfe_dfpinv;
}

Real
FiniteStrainHyperElasticViscoPlastic::computeNorm(const std::vector<Real> & var)
{
  Real val = 0.0;
  for (unsigned int i = 0; i < var.size(); ++i)
    val += Utility::pow<2>(var[i]);
  return std::sqrt(val);
}

void
FiniteStrainHyperElasticViscoPlastic::updateFlowRate()
{
  _jac.lu_solve(_resid, _dflow_rate);
  _flow_rate -= _dflow_rate;

  for (unsigned int i = 0; i < _num_flow_rate_uos; ++i)
    (*_flow_rate_prop[i])[_qp] = _flow_rate(i);
}

void
FiniteStrainHyperElasticViscoPlastic::computeQpJacobian()
{
  usingTensorIndices(i_, j_, k_, l_);
  _tan_mod = _fe.times<i_, k_, j_, l_>(_fe) * _dpk2_dfe;
  _pk2_fet = _pk2[_qp] * _fe.transpose();
  _fe_pk2 = _fe * _pk2[_qp];

  for (const auto i : make_range(Moose::dim))
    for (const auto j : make_range(Moose::dim))
      for (const auto l : make_range(Moose::dim))
      {
        _tan_mod(i, j, i, l) += _pk2_fet(l, j);
        _tan_mod(i, j, j, l) += _fe_pk2(i, l);
      }

  _tan_mod /= _fe.det();

  for (const auto i : make_range(Moose::dim))
    for (const auto j : make_range(Moose::dim))
      for (const auto l : make_range(Moose::dim))
        _dfe_df(i, j, i, l) = _fp_tmp_inv(l, j);

  for (const auto i : make_range(Moose::dim))
    for (const auto j : make_range(Moose::dim))
      for (const auto k : make_range(Moose::dim))
        for (const auto l : make_range(Moose::dim))
          _df_dstretch_inc(i, j, k, l) =
              _rotation_increment[_qp](i, k) * _deformation_gradient_old[_qp](l, j);

  _Jacobian_mult[_qp] = _tan_mod * _dfe_df * _df_dstretch_inc;
}

bool
FiniteStrainHyperElasticViscoPlastic::computeIntVarRates()
{
  Real val = 0;
  for (unsigned int i = 0; i < _num_int_var_rate_uos; ++i)
  {
    if (_int_var_rate_uo[i]->computeValue(_qp, val))
      (*_int_var_rate_prop[i])[_qp] = val;
    else
      return false;
  }
  return true;
}

bool
FiniteStrainHyperElasticViscoPlastic::computeIntVar()
{
  Real val = 0;
  for (unsigned int i = 0; i < _num_int_var_uos; ++i)
  {
    if (_int_var_uo[i]->computeValue(_qp, _dt_substep, val))
      (*_int_var_stateful_prop[i])[_qp] = val;
    else
      return false;
  }
  return true;
}

bool
FiniteStrainHyperElasticViscoPlastic::computeStrength()
{
  Real val = 0;
  for (unsigned int i = 0; i < _num_strength_uos; ++i)
  {
    if (_strength_uo[i]->computeValue(_qp, val))
      (*_strength_prop[i])[_qp] = val;
    else
      return false;
  }
  return true;
}

void
FiniteStrainHyperElasticViscoPlastic::computeIntVarRateDerivatives()
{
  Real val = 0;

  for (unsigned int i = 0; i < _num_int_var_rate_uos; ++i)
    for (unsigned int j = 0; j < _num_flow_rate_uos; ++j)
    {
      _int_var_rate_uo[i]->computeDerivative(_qp, _flow_rate_uo_names[j], val);
      _dintvarrate_dflowrate[j](i) = val;
    }
}

void
FiniteStrainHyperElasticViscoPlastic::computeIntVarDerivatives()
{
  Real val = 0;

  for (unsigned int i = 0; i < _num_int_var_uos; ++i)
    for (unsigned int j = 0; j < _num_int_var_rate_uos; ++j)
    {
      _int_var_uo[i]->computeDerivative(_qp, _dt_substep, _int_var_rate_uo_names[j], val);
      _dintvar_dintvarrate(i, j) = val;
    }

  _dintvar_dintvar.zero();

  for (unsigned int i = 0; i < _num_int_var_uos; ++i)
    for (unsigned int j = 0; j < _num_int_var_uos; ++j)
    {
      if (i == j)
        _dintvar_dintvar(i, j) = 1;
      for (unsigned int k = 0; k < _num_int_var_rate_uos; ++k)
        _dintvar_dintvar(i, j) -= _dintvar_dintvarrate(i, k) * _dintvarrate_dintvar(k, j);
    }

  for (unsigned int i = 0; i < _num_flow_rate_uos; ++i)
    _dintvar_dintvarrate.vector_mult(_dintvar_dflowrate_tmp[i], _dintvarrate_dflowrate[i]);

  for (unsigned int i = 0; i < _num_flow_rate_uos; ++i)
  {
    _dintvar_dintvar_x.zero();
    _dintvar_dintvar.lu_solve(_dintvar_dflowrate_tmp[i], _dintvar_dintvar_x);
    for (unsigned int j = 0; j < _num_int_var_uos; ++j)
      _dintvar_dflowrate(j, i) = _dintvar_dintvar_x(j);
  }
}

void
FiniteStrainHyperElasticViscoPlastic::computeStrengthDerivatives()
{
  Real val = 0;

  for (unsigned int i = 0; i < _num_strength_uos; ++i)
    for (unsigned int j = 0; j < _num_int_var_uos; ++j)
    {
      _strength_uo[i]->computeDerivative(_qp, _int_var_uo_names[j], val);
      _dstrength_dintvar(i, j) = val;
    }
}
