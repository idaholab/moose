#include "TensorMechanicsPlasticWeakPlaneTensileN.h"
#include "RotationMatrix.h" // for rotVecToZ

template<>
InputParameters validParams<TensorMechanicsPlasticWeakPlaneTensileN>()
{
  InputParameters params = validParams<TensorMechanicsPlasticModel>();
  params.addRequiredRangeCheckedParam<Real>("tensile_strength", "tensile_strength>=0", "Weak plane tensile strength");
  params.addParam<Real>("tensile_strength_residual", "Tenile strength at internal_parameter = limit.  If not given, this defaults to tensile_strength, ie, perfect plasticity");
  params.addRangeCheckedParam<Real>("tensile_strength_limit", 1, "tensile_strength_limit>0", "Tensile strength = cubic between tensile_strength (at zero internal parameter) and tensile_strength_residual (at internal_parameter = tensile_strength_limit).  Set to zero for perfect plasticity");
  params.addRequiredParam<RealVectorValue>("normal_vector", "The normal vector to the weak plane");
  params.addClassDescription("Associative weak-plane tensile plasticity with hardening/softening, with specified, fixed normal vector.  (WeakPlaneTensile combined with specifying N in the Material might be preferable to you.)");

  return params;
}

TensorMechanicsPlasticWeakPlaneTensileN::TensorMechanicsPlasticWeakPlaneTensileN(const std::string & name,
                                                         InputParameters parameters) :
    TensorMechanicsPlasticModel(name, parameters),
    _tension_cutoff(getParam<Real>("tensile_strength")),
    _tension_cutoff_residual(parameters.isParamValid("tensile_strength_residual") ? getParam<Real>("tensile_strength_residual") : _tension_cutoff),
    _tension_cutoff_limit(getParam<Real>("tensile_strength_limit")),
    _half_tension_cutoff_limit(0.5*_tension_cutoff_limit),
    _alpha_tension_cutoff((_tension_cutoff - _tension_cutoff_residual)/4.0/std::pow(_half_tension_cutoff_limit, 3)),
    _beta_tension_cutoff(-3.0*_alpha_tension_cutoff*std::pow(_half_tension_cutoff_limit, 2)),
    _input_n(getParam<RealVectorValue>("normal_vector")),
    _df_dsig(RankTwoTensor())
{
  if (_input_n.size() == 0)
     mooseError("Weak-plane normal vector must not have zero length");
   else
     _input_n /= _input_n.size();
  _rot = RotationMatrix::rotVecToZ(_input_n);

  for (unsigned i = 0 ; i < 3 ; ++i)
    for (unsigned j = 0 ; j < 3 ; ++j)
      _df_dsig(i, j) = _rot(2, i)*_rot(2, j);
}


Real
TensorMechanicsPlasticWeakPlaneTensileN::yieldFunction(const RankTwoTensor & stress, const Real & intnl) const
{
  Real s22 = 0;
  for (unsigned i = 0 ; i < 3 ; ++i)
    for (unsigned j = 0 ; j < 3 ; ++j)
      s22 += _rot(2, i)*_rot(2, j)*stress(i, j);
  return s22 - tensile_strength(intnl);
}

RankTwoTensor
TensorMechanicsPlasticWeakPlaneTensileN::dyieldFunction_dstress(const RankTwoTensor & /*stress*/, const Real & /*intnl*/) const
{
  return _df_dsig;
}


Real
TensorMechanicsPlasticWeakPlaneTensileN::dyieldFunction_dintnl(const RankTwoTensor & /*stress*/, const Real & intnl) const
{
  return -dtensile_strength(intnl);
}

RankTwoTensor
TensorMechanicsPlasticWeakPlaneTensileN::flowPotential(const RankTwoTensor & /*stress*/, const Real & /*intnl*/) const
{
  return _df_dsig;
}

RankFourTensor
TensorMechanicsPlasticWeakPlaneTensileN::dflowPotential_dstress(const RankTwoTensor & /*stress*/, const Real & /*intnl*/) const
{
  return RankFourTensor();
}

RankTwoTensor
TensorMechanicsPlasticWeakPlaneTensileN::dflowPotential_dintnl(const RankTwoTensor & /*stress*/, const Real & /*intnl*/) const
{
  return RankTwoTensor();
}


Real
TensorMechanicsPlasticWeakPlaneTensileN::tensile_strength(const Real internal_param) const
{
  if (internal_param <= 0)
    return _tension_cutoff;
  else if (internal_param >= _tension_cutoff_limit)
    return _tension_cutoff_residual;
  else
    return _alpha_tension_cutoff*std::pow(internal_param - _half_tension_cutoff_limit, 3) + _beta_tension_cutoff*(internal_param - _half_tension_cutoff_limit) + 0.5*(_tension_cutoff + _tension_cutoff_residual);
}

Real
TensorMechanicsPlasticWeakPlaneTensileN::dtensile_strength(const Real internal_param) const
{
  if (internal_param <= 0)
    return 0.0;
  else if (internal_param >= _tension_cutoff_limit)
    return 0.0;
  else
    return 3*_alpha_tension_cutoff*std::pow(internal_param - _half_tension_cutoff_limit, 2) + _beta_tension_cutoff;
}
