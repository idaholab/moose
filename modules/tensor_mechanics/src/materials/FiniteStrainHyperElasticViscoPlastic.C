#include "FiniteStrainHyperElasticViscoPlastic.h"

template<>
InputParameters validParams<FiniteStrainHyperElasticViscoPlastic>()
{
  InputParameters params = validParams<ComputeStressBase>();
  params.addParam<Real>("resid_abs_tol", 1e-10, "Absolute Tolerance for flow rate residual equation");
  params.addParam<Real>("resid_rel_tol", 1e-6, "Relative Tolerance for flow rate residual equation");
  params.addParam<unsigned int>("maxiters", 50, "Maximum iteration for flow rate update");
  params.addParam<unsigned int>("max_substep_iteration", 1, "Maximum number of substep iteration");
  params.addRequiredParam<UserObjectName>("stress_user_object", "User object that computes stress using elastic RightCauchyGreenTensor");
  params.addRequiredParam<std::vector<UserObjectName> >("flow_rate_user_objects", "List of User object names that computes flow rate and derivatives");
  params.addClassDescription("Material class for hyper-elastic visco-platic flow: Can handle multiple flow models defined by flowratemodel type user objects");

  return params;
}

FiniteStrainHyperElasticViscoPlastic::FiniteStrainHyperElasticViscoPlastic(const InputParameters & parameters) :
    ComputeStressBase(parameters),
    _resid_abs_tol(getParam<Real>("resid_abs_tol")),
    _resid_rel_tol(getParam<Real>("resid_rel_tol")),
    _maxiters(getParam<unsigned int>("maxiters")),
    _max_substep_iter(getParam<unsigned int>("max_substep_iteration")),
    _stress_uo(getUserObject<HyperElasticStress>("stress_user_object")),
    _num_flow_rate_uos(getParam<std::vector<UserObjectName> >("flow_rate_user_objects").size()),
    _fp(declareProperty<RankTwoTensor>("fp")),
    _fp_old(declarePropertyOld<RankTwoTensor>("fp")),
    _q(declareProperty< std::vector<Real> >("internal_var")),
    _q_old(declarePropertyOld< std::vector<Real> >("internal_var")),
    _deformation_gradient(getMaterialProperty<RankTwoTensor>("deformation gradient")),
    _deformation_gradient_old(getMaterialPropertyOld<RankTwoTensor>("deformation gradient"))
{
  if (_num_flow_rate_uos == 0)
    mooseError("Specify atleast one flow rate user object");

  _flow_rate_uo.resize(_num_flow_rate_uos);
  _q_end_index.resize(_num_flow_rate_uos, 0);

  _num_internal_var = 0;
  for (unsigned int i = 0; i < _num_flow_rate_uos; ++i)
  {
    _flow_rate_uo[i] = &getUserObjectByName<FlowRateModel>(parameters.get< std::vector<UserObjectName> >("flow_rate_user_objects")[i]);
    _num_internal_var += _flow_rate_uo[i]->numInternalVar();
    _q_end_index[i] = _num_internal_var;
  }

  _q_tmp.resize(_num_internal_var, 0.0);
  _q_tmp_old.resize(_num_internal_var, 0.0);

  _resid.resize(_num_flow_rate_uos, 0.0);
  _jac.resize(_num_flow_rate_uos * _num_flow_rate_uos, 0.0);

  _flow_rate.resize(_num_flow_rate_uos, 0.0);
  _flow_rate_func.resize(_num_flow_rate_uos, 0.0);
  _flow_dirn.resize(_num_flow_rate_uos);
  _dpk2_dflowrate.resize(_num_flow_rate_uos);

  _dq_dflowrate.resize(_num_internal_var, 0.0);
  _dflowrate_dpk2.resize(_num_flow_rate_uos);
  _dflowrate_dq.resize(_num_internal_var, 0.0);
}

void
FiniteStrainHyperElasticViscoPlastic::initQpStatefulProperties()
{
  _stress[_qp].zero();

  _fp[_qp].zero();
  _fp[_qp].addIa(1.0);

  _q[_qp].resize(_num_internal_var, 0.0);
  _q_old[_qp].resize(_num_internal_var, 0.0);
}

void
FiniteStrainHyperElasticViscoPlastic::computeQpStress()
{
  bool converge;
  RankTwoTensor delta_dfgrd = _deformation_gradient[_qp] - _deformation_gradient_old[_qp];
  unsigned int num_substep = 1;
  unsigned int substep_iter = 1;

  do
  {
    preSolveQp();

    converge = true;
    _dt_substep = _dt/num_substep;

    for (unsigned int istep = 0; istep < num_substep; ++istep)
    {
      _dfgrd_tmp = (istep+1) * delta_dfgrd/num_substep + _deformation_gradient_old[_qp];
      if (!solveQp())
      {
        converge = false;
        substep_iter++;
        num_substep*=2;
        break;
      }
    }

    if (substep_iter > _max_substep_iter)
      mooseError("Constitutive failure with substepping");
  }
  while (!converge);

  postSolveQp();
}


void
FiniteStrainHyperElasticViscoPlastic::preSolveQp()
{
  _fp_tmp_old_inv = _fp_old[_qp].inverse();

  for (unsigned int i = 0; i < _num_internal_var; ++i)
    _q_tmp_old[i] = _q_old[_qp][i];
}

bool
FiniteStrainHyperElasticViscoPlastic::solveQp()
{
  preSolveStress();
  if (!solveStress())
    return false;
  postSolveStress();

  return true;
}

void
FiniteStrainHyperElasticViscoPlastic::postSolveQp()
{
  _stress[_qp] = _fe * _pk2_tmp * _fe.transpose() / _fe.det();
  _fp[_qp] = _fp_tmp_inv.inverse();

  for (unsigned int i = 0; i < _num_internal_var; ++i)
    _q[_qp][i] = _q_tmp[i];

  computeQpJacobian();
}

void
FiniteStrainHyperElasticViscoPlastic::preSolveStress()
{
  for (unsigned int i = 0; i < _num_flow_rate_uos; ++i)
    _flow_rate[i] = 0.0;

  for (unsigned int i = 0; i < _num_internal_var; ++i)
  {
    _q_tmp[i] = _q_tmp_old[i];
    _dq_dflowrate[i] = 1.0;
  }

  _fp_tmp_inv = _fp_tmp_old_inv;
  _fe = _dfgrd_tmp * _fp_tmp_inv;
}

bool
FiniteStrainHyperElasticViscoPlastic::solveStress()
{
  Real resid0, rnorm;
  unsigned int iter = 0;

#ifdef DEBUG
  std::vector<Real> rnormst(_maxiters+1),flowratest(_maxiters+1);
#endif

  if (!computeFlowRateResidual())
    return false;

  rnorm = computeNorm(&_resid[0]);
  resid0 = rnorm;

#ifdef DEBUG
  rnormst[iter] = rnorm;
  flowratest[iter] = computeNorm(&_flow_rate[0]);
#endif

  while (rnorm > _resid_abs_tol && rnorm > _resid_rel_tol * resid0 && iter < _maxiters)
  {
    computeFlowRateJacobian();
    updateFlowRate();
    computeElasticPlasticDeformGrad();
    if (!updateInternalVar())
      return false;
    if (!computeFlowRateResidual())
      return false;

    rnorm = computeNorm(&_resid[0]);
    iter++;

#ifdef DEBUG
    rnormst[iter] = rnorm;
    flowratest[iter] = computeNorm(&_flow_rate[0]);
#endif
  }

  if (iter == _maxiters && rnorm > _resid_abs_tol && rnorm > _resid_rel_tol * resid0)
    return false;

  return true;
}

void
FiniteStrainHyperElasticViscoPlastic::postSolveStress()
{
  _fp_tmp_old_inv = _fp_tmp_inv;

  for (unsigned i = 0; i < _num_internal_var; ++i)
    _q_tmp_old[i] = _q_tmp[i];
}

bool
FiniteStrainHyperElasticViscoPlastic::computeFlowDirection()
{
  unsigned int start_index = 0;
  unsigned int size = 0;

  for (unsigned i = 0; i < _num_flow_rate_uos; ++i)
  {
    if (i > 0)
      start_index = _q_end_index[i-1];
    size = _q_end_index[i] - start_index;

    if (!_flow_rate_uo[i]->computeFlowDirection(_flow_dirn[i], _pk2_tmp, _ce, _q_tmp, start_index, size))
      return false;
  }

  return true;
}

bool
FiniteStrainHyperElasticViscoPlastic::computeFlowRate()
{
  unsigned int start_index = 0;
  unsigned int size = 0;

  for (unsigned i = 0; i < _num_flow_rate_uos; ++i)
  {
    if (i > 0)
      start_index = _q_end_index[i-1];
    size = _q_end_index[i] - start_index;

    if (!_flow_rate_uo[i]->computeFlowRate(_flow_rate_func[i], _pk2_tmp, _ce, _q_tmp, start_index, size))
      return false;

    if (!_flow_rate_uo[i]->computeDflowrateDinternalvar(_dflowrate_dq, _pk2_tmp, _ce, _q_tmp, start_index, size))
      return false;

    if (!_flow_rate_uo[i]->computeDflowrateDstress(_dflowrate_dpk2[i], _pk2_tmp, _ce, _q_tmp, start_index, size))
      return false;
  }

  return true;
}

void
FiniteStrainHyperElasticViscoPlastic::computePK2Stress()
{
  std::vector<Real> out_state_real;
  std::vector<Real> in_state;
  std::vector<RankTwoTensor> out_state_ranktwotensor;

  in_state.resize(_stress_uo.getNumStateIn(), 0.0);
  out_state_real.resize(_stress_uo.getNumStateOutReal(), 0.0);
  out_state_ranktwotensor.resize(_stress_uo.getNumStateOutRankTwoTensor());

  _stress_uo.computePK2Stress(_pk2_tmp, _dpk2_dce, out_state_real, out_state_ranktwotensor, _ce, _elasticity_tensor[_qp], in_state);
}

void
FiniteStrainHyperElasticViscoPlastic::computeElasticRightCauchyGreenTensor()
{
  _ce = _fe.transpose() * _fe;
}

bool
FiniteStrainHyperElasticViscoPlastic::computeFlowRateResidual()
{
  computeElasticRightCauchyGreenTensor();
  computePK2Stress();
  if (!computeFlowDirection())
    return false;

  if (!computeFlowRate())
    return false;

  for (unsigned int i = 0; i < _num_flow_rate_uos; ++i)
    _resid[i] = _flow_rate[i] - _flow_rate_func[i];

  return true;
}

void
FiniteStrainHyperElasticViscoPlastic::computeElasticPlasticDeformGrad()
{
  RankTwoTensor iden;
  iden.addIa(1.0);

  RankTwoTensor val;
  for (unsigned int i = 0; i < _num_flow_rate_uos; ++i)
    val += _flow_rate[i] * _flow_dirn[i] * _dt_substep;

  _fp_tmp_inv = _fp_tmp_old_inv * (iden - val);
  _fp_tmp_inv = std::pow(_fp_tmp_inv.det(),-1.0/3.0) * _fp_tmp_inv;
  _fe = _dfgrd_tmp * _fp_tmp_inv;
}

void
FiniteStrainHyperElasticViscoPlastic::computeFlowRateJacobian()
{
  computeDpk2Dfpinv();

  RankTwoTensor dfpinv_dflowrate;
  for (unsigned int i = 0; i < _num_flow_rate_uos; ++i)
  {
    dfpinv_dflowrate = -_fp_tmp_old_inv * _flow_dirn[i] * _dt_substep;
    _dpk2_dflowrate[i] = _dpk2_dfpinv * dfpinv_dflowrate;
  }

  for (unsigned int i = 0; i < _num_flow_rate_uos; ++i)
    for (unsigned int j = 0; j < _num_flow_rate_uos; ++j)
    {
      _jac[i * _num_flow_rate_uos + j] = 0.0;
      if (i == j)
        _jac[i * _num_flow_rate_uos + j] += (1.0 - _dflowrate_dq[i] * _dq_dflowrate[i]);
      _jac[i * _num_flow_rate_uos + j] -= _dflowrate_dpk2[i].doubleContraction(_dpk2_dflowrate[j]);
    }
}

void
FiniteStrainHyperElasticViscoPlastic::computeDpk2Dfpinv()
{
  RankFourTensor dce_dfe;
  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
      {
        dce_dfe(i,j,k,i) = dce_dfe(i,j,k,i) + _fe(k,j);
        dce_dfe(i,j,k,j) = dce_dfe(i,j,k,j) + _fe(k,i);
      }

  RankFourTensor dfe_dfpinv;
  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
        dfe_dfpinv(i,j,k,j) = _dfgrd_tmp(i,k);

  _dpk2_dfpinv = _dpk2_dce * dce_dfe * dfe_dfpinv;
}

bool
FiniteStrainHyperElasticViscoPlastic::updateInternalVar()
{
  unsigned int start_index = 0;
  unsigned int size = 0;

  for (unsigned int i = 0; i < _num_flow_rate_uos; ++i)
  {
    if (i > 0)
      start_index = _q_end_index[i-1];
    size = _q_end_index[i] - start_index;

    if (!_flow_rate_uo[i]->updateInternalVar(_q_tmp, _dq_dflowrate, _dt_substep, _flow_rate[i], _q_tmp_old, start_index, size))
      return false;
  }

  return true;
}

Real
FiniteStrainHyperElasticViscoPlastic::computeNorm(Real * resid)
{
  Real val = 0.0;
  for (unsigned int i = 0; i < _num_flow_rate_uos; ++i)
    val += std::pow(resid[i],2.0);
  return std::pow(val,0.5);
}

void
FiniteStrainHyperElasticViscoPlastic::updateFlowRate()
{
  if (_resid.size() == 1)
    _flow_rate[0] -= _resid[0]/_jac[0];
  else
  {
    DenseMatrix<Real> A(_num_flow_rate_uos, _num_flow_rate_uos);
    DenseVector<Real> r(_num_flow_rate_uos);
    DenseVector<Real> x(_num_flow_rate_uos);

    for (unsigned int i = 0; i < _num_flow_rate_uos; ++i)
      for (unsigned int j = 0; j < _num_flow_rate_uos; ++j)
        A(i,j) = _jac[i * _num_flow_rate_uos + j];

    for (unsigned int i = 0; i < _num_flow_rate_uos; ++i)
      r(i) = _resid[i];

    A.lu_solve(r,x);

    for (unsigned int i = 0; i < _num_flow_rate_uos; ++i)
      _flow_rate[i] -= x(i);
  }
}

void
FiniteStrainHyperElasticViscoPlastic::computeQpJacobian()
{
  RankTwoTensor pk2fet, fepk2;
  RankFourTensor dcedfe, dfedf, tan_mod;

  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
      {
        dcedfe(i,j,k,i) += _fe(k,j);
        dcedfe(i,j,k,j) += _fe(k,i);
      }

  tan_mod = _fe.mixedProductIkJl(_fe) * _dpk2_dce * dcedfe;
  pk2fet = _pk2_tmp * _fe.transpose();
  fepk2 = _fe * _pk2_tmp;

  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      for (unsigned int l = 0; l < LIBMESH_DIM; ++l)
      {
        tan_mod(i,j,i,l) += pk2fet(l,j);
        tan_mod(i,j,j,l) += fepk2(i,l);
      }

  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      for (unsigned int l = 0; l < LIBMESH_DIM; ++l)
        dfedf(i,j,i,l) =  _fp_tmp_inv(l,j);

  _Jacobian_mult[_qp] = tan_mod * dfedf;
}

// DEPRECATED
FiniteStrainHyperElasticViscoPlastic::FiniteStrainHyperElasticViscoPlastic(const std::string & name,
                                                                           InputParameters parameters) :
    ComputeStressBase(name, parameters),
    _resid_abs_tol(getParam<Real>("resid_abs_tol")),
    _resid_rel_tol(getParam<Real>("resid_rel_tol")),
    _maxiters(getParam<unsigned int>("maxiters")),
    _max_substep_iter(getParam<unsigned int>("max_substep_iteration")),
    _stress_uo(getUserObject<HyperElasticStress>("stress_user_object")),
    _num_flow_rate_uos(getParam<std::vector<UserObjectName> >("flow_rate_user_objects").size()),
    _fp(declareProperty<RankTwoTensor>("fp")),
    _fp_old(declarePropertyOld<RankTwoTensor>("fp")),
    _q(declareProperty< std::vector<Real> >("internal_var")),
    _q_old(declarePropertyOld< std::vector<Real> >("internal_var")),
    _deformation_gradient(getMaterialProperty<RankTwoTensor>("deformation gradient")),
    _deformation_gradient_old(getMaterialPropertyOld<RankTwoTensor>("deformation gradient"))
{
  if (_num_flow_rate_uos == 0)
    mooseError("Specify atleast one flow rate user object");

  _flow_rate_uo.resize(_num_flow_rate_uos);
  _q_end_index.resize(_num_flow_rate_uos, 0);

  _num_internal_var = 0;
  for (unsigned int i = 0; i < _num_flow_rate_uos; ++i)
  {
    _flow_rate_uo[i] = &getUserObjectByName<FlowRateModel>(parameters.get< std::vector<UserObjectName> >("flow_rate_user_objects")[i]);
    _num_internal_var += _flow_rate_uo[i]->numInternalVar();
    _q_end_index[i] = _num_internal_var;
  }

  _q_tmp.resize(_num_internal_var, 0.0);
  _q_tmp_old.resize(_num_internal_var, 0.0);

  _resid.resize(_num_flow_rate_uos, 0.0);
  _jac.resize(_num_flow_rate_uos * _num_flow_rate_uos, 0.0);

  _flow_rate.resize(_num_flow_rate_uos, 0.0);
  _flow_rate_func.resize(_num_flow_rate_uos, 0.0);
  _flow_dirn.resize(_num_flow_rate_uos);
  _dpk2_dflowrate.resize(_num_flow_rate_uos);

  _dq_dflowrate.resize(_num_internal_var, 0.0);
  _dflowrate_dpk2.resize(_num_flow_rate_uos);
  _dflowrate_dq.resize(_num_internal_var, 0.0);
}
