//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SmearedCrackSofteningBase.h"

#include "MooseMesh.h"

InputParameters
SmearedCrackSofteningBase::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Calculates the softening behavior in a given crack direction. This "
                             "class is intended to be used with ComputeSmearedCrackingStress.");
  // These models are to be called by another model, so set compute=false
  params.set<bool>("compute") = false;
  params.suppressParameter<bool>("compute");
  return params;
}

SmearedCrackSofteningBase::SmearedCrackSofteningBase(const InputParameters & parameters)
  : Material(parameters)
{
}
