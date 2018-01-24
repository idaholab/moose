/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "TensorMechanicsPlasticWeakPlaneTensileN.h"
#include "RotationMatrix.h" // for rotVecToZ

template <>
InputParameters
validParams<TensorMechanicsPlasticWeakPlaneTensileN>()
{
  InputParameters params = validParams<TensorMechanicsPlasticWeakPlaneTensile>();
  params.addRequiredParam<RealVectorValue>("normal_vector", "The normal vector to the weak plane");
  params.addClassDescription("Associative weak-plane tensile plasticity with hardening/softening, "
                             "with specified, fixed normal vector.  (WeakPlaneTensile combined "
                             "with specifying N in the Material might be preferable to you.)");

  return params;
}

TensorMechanicsPlasticWeakPlaneTensileN::TensorMechanicsPlasticWeakPlaneTensileN(
    const InputParameters & parameters)
  : TensorMechanicsPlasticWeakPlaneTensile(parameters),
    _input_n(getParam<RealVectorValue>("normal_vector")),
    _df_dsig(RankTwoTensor())
{
  // cannot check the following for all values of strength, but this is a start
  if (_strength.value(0) < 0)
    mooseError("Weak plane tensile strength must not be negative");
  if (_input_n.norm() == 0)
    mooseError("Weak-plane normal vector must not have zero length");
  else
    _input_n /= _input_n.norm();
  _rot = RotationMatrix::rotVecToZ(_input_n);

  for (unsigned i = 0; i < 3; ++i)
    for (unsigned j = 0; j < 3; ++j)
      _df_dsig(i, j) = _rot(2, i) * _rot(2, j);
}

Real
TensorMechanicsPlasticWeakPlaneTensileN::yieldFunction(const RankTwoTensor & stress,
                                                       Real intnl) const
{
  Real s22 = 0;
  for (unsigned i = 0; i < 3; ++i)
    for (unsigned j = 0; j < 3; ++j)
      s22 += _rot(2, i) * _rot(2, j) * stress(i, j);
  return s22 - tensile_strength(intnl);
}

RankTwoTensor
TensorMechanicsPlasticWeakPlaneTensileN::dyieldFunction_dstress(const RankTwoTensor & /*stress*/,
                                                                Real /*intnl*/) const
{
  return _df_dsig;
}

Real
TensorMechanicsPlasticWeakPlaneTensileN::dyieldFunction_dintnl(const RankTwoTensor & /*stress*/,
                                                               Real intnl) const
{
  return -dtensile_strength(intnl);
}

RankTwoTensor
TensorMechanicsPlasticWeakPlaneTensileN::flowPotential(const RankTwoTensor & /*stress*/,
                                                       Real /*intnl*/) const
{
  return _df_dsig;
}

RankFourTensor
TensorMechanicsPlasticWeakPlaneTensileN::dflowPotential_dstress(const RankTwoTensor & /*stress*/,
                                                                Real /*intnl*/) const
{
  return RankFourTensor();
}

RankTwoTensor
TensorMechanicsPlasticWeakPlaneTensileN::dflowPotential_dintnl(const RankTwoTensor & /*stress*/,
                                                               Real /*intnl*/) const
{
  return RankTwoTensor();
}

std::string
TensorMechanicsPlasticWeakPlaneTensileN::modelName() const
{
  return "WeakPlaneTensileN";
}
