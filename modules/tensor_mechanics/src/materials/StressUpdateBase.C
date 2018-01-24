//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StressUpdateBase.h"

#include "MooseMesh.h"

template <>
InputParameters
validParams<StressUpdateBase>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("Calculates an admissible state (stress that lies on or within the "
                             "yield surface, plastic strains, internal parameters, etc).  This "
                             "class is intended to be a parent class for classes with specific "
                             "constitutive models.");
  // The return stress increment classes are intended to be iterative materials, so must set compute
  // = false for all inheriting classes
  params.set<bool>("compute") = false;
  params.suppressParameter<bool>("compute");
  return params;
}

StressUpdateBase::StressUpdateBase(const InputParameters & parameters) : Material(parameters) {}

void
StressUpdateBase::setQp(unsigned int qp)
{
  _qp = qp;
}

void
StressUpdateBase::propagateQpStatefulProperties()
{
  mooseError(
      "propagateQpStatefulProperties called: it needs to be implemented by your inelastic model");
}

Real
StressUpdateBase::computeTimeStepLimit()
{
  return std::numeric_limits<Real>::max();
}
