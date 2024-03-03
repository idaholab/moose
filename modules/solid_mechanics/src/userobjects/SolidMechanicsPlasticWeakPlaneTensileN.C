//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolidMechanicsPlasticWeakPlaneTensileN.h"
#include "RotationMatrix.h" // for rotVecToZ
#include "RankFourTensor.h"

registerMooseObject("SolidMechanicsApp", SolidMechanicsPlasticWeakPlaneTensileN);
registerMooseObjectRenamed("SolidMechanicsApp",
                           TensorMechanicsPlasticWeakPlaneTensileN,
                           "01/01/2025 00:00",
                           SolidMechanicsPlasticWeakPlaneTensileN);

InputParameters
SolidMechanicsPlasticWeakPlaneTensileN::validParams()
{
  InputParameters params = SolidMechanicsPlasticWeakPlaneTensile::validParams();
  params.addRequiredParam<RealVectorValue>("normal_vector", "The normal vector to the weak plane");
  params.addClassDescription("Associative weak-plane tensile plasticity with hardening/softening, "
                             "with specified, fixed normal vector.  (WeakPlaneTensile combined "
                             "with specifying N in the Material might be preferable to you.)");

  return params;
}

SolidMechanicsPlasticWeakPlaneTensileN::SolidMechanicsPlasticWeakPlaneTensileN(
    const InputParameters & parameters)
  : SolidMechanicsPlasticWeakPlaneTensile(parameters),
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
SolidMechanicsPlasticWeakPlaneTensileN::yieldFunction(const RankTwoTensor & stress,
                                                      Real intnl) const
{
  Real s22 = 0;
  for (unsigned i = 0; i < 3; ++i)
    for (unsigned j = 0; j < 3; ++j)
      s22 += _rot(2, i) * _rot(2, j) * stress(i, j);
  return s22 - tensile_strength(intnl);
}

RankTwoTensor
SolidMechanicsPlasticWeakPlaneTensileN::dyieldFunction_dstress(const RankTwoTensor & /*stress*/,
                                                               Real /*intnl*/) const
{
  return _df_dsig;
}

Real
SolidMechanicsPlasticWeakPlaneTensileN::dyieldFunction_dintnl(const RankTwoTensor & /*stress*/,
                                                              Real intnl) const
{
  return -dtensile_strength(intnl);
}

RankTwoTensor
SolidMechanicsPlasticWeakPlaneTensileN::flowPotential(const RankTwoTensor & /*stress*/,
                                                      Real /*intnl*/) const
{
  return _df_dsig;
}

RankFourTensor
SolidMechanicsPlasticWeakPlaneTensileN::dflowPotential_dstress(const RankTwoTensor & /*stress*/,
                                                               Real /*intnl*/) const
{
  return RankFourTensor();
}

RankTwoTensor
SolidMechanicsPlasticWeakPlaneTensileN::dflowPotential_dintnl(const RankTwoTensor & /*stress*/,
                                                              Real /*intnl*/) const
{
  return RankTwoTensor();
}

std::string
SolidMechanicsPlasticWeakPlaneTensileN::modelName() const
{
  return "WeakPlaneTensileN";
}
