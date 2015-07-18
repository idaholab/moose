#include "FlowRateModel.h"

template<>
InputParameters validParams<FlowRateModel>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addParam<Real>("reference_flow_rate",0.001,"Reference flow rate for rate dependent flow");
  params.addParam<Real>("flow_rate_exponent",10,"Power law exponent in flow rate equation");
  params.addParam<Real>("flow_rate_tol", 1e3, "Tolerance for flow rate");
  params.addRequiredParam<UserObjectName>("flow_stress_user_object","User object for flow stress");
  params.addClassDescription("User object to evaluate flow rate and update associated internal variables");

  return params;
}

FlowRateModel::FlowRateModel(const std::string & name,
                             InputParameters parameters) :
    GeneralUserObject(name, parameters),
    _ref_flow_rate(getParam<Real>("reference_flow_rate")),
    _flow_rate_exponent(getParam<Real>("flow_rate_exponent")),
    _flow_rate_tol(getParam<Real>("flow_rate_tol")),
    _flow_stress_uo(getUserObject<TensorMechanicsHardeningModel>("flow_stress_user_object"))
{
  //Sets number of internal variables used by the user object
  _num_internal_var = 1;
}

unsigned int
FlowRateModel::numInternalVar() const
{
  return _num_internal_var;
}

bool
FlowRateModel::computeFlowRate(Real & flow_rate, const RankTwoTensor & pk2, const RankTwoTensor & ce, const std::vector<Real> & internal_var, const unsigned int start_index, const unsigned int size) const
{
  if (size != 1)
    mooseError("FlowRateModel Error: This user object is for J2 Plasticity and requires one internal variable");

  RankTwoTensor pk2_dev = computePK2Deviatoric(pk2, ce);
  Real eqv_stress = computeEqvStress(pk2_dev, ce);

  Real sigy = _flow_stress_uo.value(internal_var[start_index]);
  flow_rate = std::pow(eqv_stress/sigy, _flow_rate_exponent) * _ref_flow_rate;

  if (flow_rate > _flow_rate_tol)
  {
#ifdef DEBUG
    mooseWarning("Flow rate greater than " << _flow_rate_tol << " " << flow_rate << " " << eqv_stress << " " << sigy );
#endif
    return false;
  }

  return true;
}

bool
FlowRateModel::computeDflowrateDstress(RankTwoTensor & dflowrate_dstress, const RankTwoTensor & pk2, const RankTwoTensor & ce, const std::vector<Real> & internal_var, const unsigned int start_index, const unsigned int size) const
{
  RankTwoTensor pk2_dev = computePK2Deviatoric(pk2, ce);
  Real eqv_stress = computeEqvStress(pk2_dev, ce);

  Real sigy = _flow_stress_uo.value(internal_var[start_index]);
  Real dflowrate_dseqv = _ref_flow_rate * _flow_rate_exponent * std::pow(eqv_stress/sigy,_flow_rate_exponent-1) * 1/sigy;

  if (dflowrate_dseqv > _flow_rate_tol)
  {
#ifdef DEBUG
    mooseWarning("dflowrate_dseqv greater than " << _flow_rate_tol << " " << dflowrate_dseqv << " " << eqv_stress << " " << sigy );
#endif
    return false;
  }

  RankTwoTensor tau = pk2_dev * ce;
  RankTwoTensor dseqv_dpk2dev;
  dseqv_dpk2dev.zero();
  if (eqv_stress > 0)
    dseqv_dpk2dev = 3/(2 * eqv_stress) * tau * ce;

  RankTwoTensor ce_inv = ce.inverse();

  RankFourTensor dpk2dev_dpk2;
  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
        for (unsigned int l = 0; l < LIBMESH_DIM; ++l)
        {
          dpk2dev_dpk2(i, j, k, l) = 0.0;
          if (i==k && j==l)
            dpk2dev_dpk2(i, j, k, l) = 1.0;
          dpk2dev_dpk2(i, j, k, l) -= ce_inv(i, j) * ce(k, l)/3.0;
        }

  dflowrate_dstress = dflowrate_dseqv * dpk2dev_dpk2.transposeMajor() * dseqv_dpk2dev;
  return true;
}

bool
FlowRateModel::computeDflowrateDinternalvar(std::vector<Real> & dflowrate_dq, const RankTwoTensor & pk2, const RankTwoTensor & ce, const std::vector<Real> & internal_var, const unsigned int start_index, const unsigned int size) const
{
  if (size != 1)
    mooseError("FlowRateModel Error: This user object is for J2 Plasticity and requires one internal variable");

  RankTwoTensor pk2_dev = computePK2Deviatoric(pk2, ce);
  Real eqv_stress = computeEqvStress(pk2_dev, ce);

  Real sigy = _flow_stress_uo.value(internal_var[start_index]);
  Real dsigy_dint = _flow_stress_uo.derivative(internal_var[start_index]);

  Real dflowrate_dsigy = - _ref_flow_rate * _flow_rate_exponent * std::pow(eqv_stress/sigy,_flow_rate_exponent) * 1/sigy;

  dflowrate_dq[start_index] = dflowrate_dsigy * dsigy_dint;
  return true;
}

bool
FlowRateModel::updateInternalVar(std::vector<Real> & q, std::vector<Real> & dq_dflowrate, const Real flow_rate, const Real dt, const std::vector<Real> & q_old, const unsigned int start_index, const unsigned int size) const
{
  if (size != 1)
    mooseError("FlowRateModel Error: This user object is for J2 Plasticity and requires one internal variable");

  q[start_index] = q_old[start_index] + flow_rate * dt;
  dq_dflowrate[start_index] = dt;

  return true;
}


bool
FlowRateModel::computeFlowDirection(RankTwoTensor & flow_dirn, const RankTwoTensor & pk2, const RankTwoTensor & ce, const std::vector<Real> & internal_var, const unsigned int start_index, const unsigned int size) const
{
  RankTwoTensor pk2_dev = computePK2Deviatoric(pk2, ce);
  Real eqv_stress = computeEqvStress(pk2_dev, ce);

  flow_dirn.zero();
  if (eqv_stress > 0.0)
    flow_dirn = 3.0/(2.0 * eqv_stress) * ce * pk2_dev * ce;

  return true;
}

RankTwoTensor
FlowRateModel::computePK2Deviatoric(const RankTwoTensor & pk2, const RankTwoTensor & ce) const
{
  return pk2 - (pk2.doubleContraction(ce) * ce.inverse())/3.0;
}

Real
FlowRateModel::computeEqvStress(const RankTwoTensor & pk2_dev, const RankTwoTensor & ce) const
{
  RankTwoTensor sdev = pk2_dev * ce;
  Real val = sdev.doubleContraction(sdev.transpose());
  return std::pow(3.0 * val/2.0, 0.5);
}
