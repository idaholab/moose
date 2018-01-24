/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
