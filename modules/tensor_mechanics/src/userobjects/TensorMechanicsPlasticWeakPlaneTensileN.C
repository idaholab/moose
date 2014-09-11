#include "TensorMechanicsPlasticWeakPlaneTensileN.h"
#include <math.h> // for M_PI
#include "RotationMatrix.h" // for rotVecToZ

template<>
InputParameters validParams<TensorMechanicsPlasticWeakPlaneTensileN>()
{
  InputParameters params = validParams<TensorMechanicsPlasticModel>();
  params.addRequiredRangeCheckedParam<Real>("tensile_strength", "tensile_strength>=0", "Weak plane tensile strength");
  params.addParam<Real>("tensile_strength_residual", "Tenile strength at infinite hardening.  If not given, this defaults to tensile_strength, ie, perfect plasticity");
  params.addRangeCheckedParam<Real>("tensile_strength_rate", 0, "tensile_strength_rate>=0", "Tensile strength = tensile_strenght_residual + (tensile_strength - tensile_strength_residual)*exp(-tensile_rate*plasticstrain).  Set to zero for perfect plasticity");
  params.addRequiredParam<RealVectorValue>("normal_vector", "The normal vector to the weak plane");
  params.addClassDescription("Associative weak-plane tensile plasticity with hardening/softening, with specified, fixed normal vector.  This plasticity model is best used for experimental, debugging purposes only.  WeakPlaneTensile combined with specifying N in the Material is preferred.");

  return params;
}

TensorMechanicsPlasticWeakPlaneTensileN::TensorMechanicsPlasticWeakPlaneTensileN(const std::string & name,
                                                         InputParameters parameters) :
    TensorMechanicsPlasticModel(name, parameters),
    _tension_cutoff(getParam<Real>("tensile_strength")),
    _tension_cutoff_residual(parameters.isParamValid("tensile_strength_residual") ? getParam<Real>("tensile_strength_residual") : _tension_cutoff),
    _tension_cutoff_rate(getParam<Real>("tensile_strength_rate")),
    _input_n(getParam<RealVectorValue>("normal_vector"))
{
  if (_input_n.size() == 0)
     mooseError("Weak-plane normal vector must not have zero length");
   else
     _input_n /= _input_n.size();
  _rot = RotationMatrix::rotVecToZ(_input_n);

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
  RankTwoTensor df_dsig;
  for (unsigned i = 0 ; i < 3 ; ++i)
    for (unsigned j = 0 ; j < 3 ; ++j)
      df_dsig(i, j) = _rot(2, i)*_rot(2, j);
  return df_dsig;
}


Real
TensorMechanicsPlasticWeakPlaneTensileN::dyieldFunction_dintnl(const RankTwoTensor & /*stress*/, const Real & intnl) const
{
  return -dtensile_strength(intnl);
}

RankTwoTensor
TensorMechanicsPlasticWeakPlaneTensileN::flowPotential(const RankTwoTensor & /*stress*/, const Real & /*intnl*/) const
{
  RankTwoTensor df_dsig;
  for (unsigned i = 0 ; i < 3 ; ++i)
    for (unsigned j = 0 ; j < 3 ; ++j)
      df_dsig(i, j) = _rot(2, i)*_rot(2, j);
  return df_dsig;
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
  return _tension_cutoff_residual + (_tension_cutoff - _tension_cutoff_residual)*std::exp(-_tension_cutoff_rate*internal_param);
}

Real
TensorMechanicsPlasticWeakPlaneTensileN::dtensile_strength(const Real internal_param) const
{
  return -_tension_cutoff_rate*(_tension_cutoff - _tension_cutoff_residual)*std::exp(-_tension_cutoff_rate*internal_param);
}
