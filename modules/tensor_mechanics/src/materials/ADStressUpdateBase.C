//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADStressUpdateBase.h"
#include "MooseMesh.h"
#include "InputParameters.h"
#include "Conversion.h"

defineADValidParams(
    ADStressUpdateBase,
    ADMaterial,
    params.addClassDescription("Calculates an admissible state (stress that lies on or within the "
                               "yield surface, plastic strains, internal parameters, etc).  This "
                               "class is intended to be a parent class for classes with specific "
                               "constitutive models.");
    params.addParam<std::string>(
        "base_name",
        "Optional parameter that defines a prefix for all material "
        "properties related to this stress update model. This allows for "
        "multiple models of the same type to be used without naming conflicts.");
    // The return stress increment classes are intended to be iterative materials, so must set
    // compute = false for all inheriting classes
    params.set<bool>("compute") = false;
    params.suppressParameter<bool>("compute");
    return params;);

template <ComputeStage compute_stage>
ADStressUpdateBase<compute_stage>::ADStressUpdateBase(const InputParameters & parameters)
  : ADMaterial<compute_stage>(parameters),
    _base_name(isParamValid("base_name") ? adGetParam<std::string>("base_name") + "_" : "")
{
}

template <ComputeStage compute_stage>
void
ADStressUpdateBase<compute_stage>::setQp(unsigned int qp)
{
  _qp = qp;
}

template <ComputeStage compute_stage>
void
ADStressUpdateBase<compute_stage>::propagateQpStatefulProperties()
{
  mooseError(
      "propagateQpStatefulProperties called: it needs to be implemented by your inelastic model");
}

template <ComputeStage compute_stage>
Real
ADStressUpdateBase<compute_stage>::computeTimeStepLimit()
{
  return std::numeric_limits<Real>::max();
}

// explicit instantiation is required for AD base classes
adBaseClass(ADStressUpdateBase);
