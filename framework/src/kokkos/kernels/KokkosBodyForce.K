//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosBodyForce.h"

registerMooseObjectAliased("MooseApp", KokkosBodyForceKernel, "KokkosBodyForce");

InputParameters
KokkosBodyForceKernel::validParams()
{
  InputParameters params = KokkosBodyForce::validParams();
  params.addClassDescription("Demonstrates the two ways that scalar values can be introduced "
                             "into kernels, e.g. (controllable) constants and postprocessors. "
                             "Implements the weak form $(\\psi_i, -f)$.");
  return params;
}

KokkosBodyForceKernel::KokkosBodyForceKernel(const InputParameters & parameters)
  : KokkosBodyForce(parameters)
{
}
