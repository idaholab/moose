//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DamageBase.h"

template <>
InputParameters
validParams<DamageBase>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription(
      "Base class for damage models for use in conjunction with "
      "ComputeMultipleInelasticStress. The damage model updates the "
      "stress and Jacobian multiplier at the end of the stress computation.");
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
  // The damage materials are designed to be called by another model, and not
  // called directly by MOOSE, so set compute=false.
  params.set<bool>("compute") = false;
  params.suppressParameter<bool>("compute");
  return params;
}

DamageBase::DamageBase(const InputParameters & parameters)
  : Material(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : "")
{
}

void
DamageBase::setQp(unsigned int qp)
{
  _qp = qp;
}

void
DamageBase::updateDamage()
{
}

Real
DamageBase::computeTimeStepLimit()
{
  return std::numeric_limits<Real>::max();
}

void
DamageBase::finiteStrainRotation(const RankTwoTensor & /*rotation_increment*/)
{
}
