//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernel.h"

// Forward Declaration
template <ComputeStage>
class ADCHSplitChemicalPotential;

declareADValidParams(ADCHSplitChemicalPotential);

/**
 * Solves chemical potential in a weak sense (mu-mu_prop=0).  Can be coupled to Cahn-Hilliard
 * equation to solve species diffusion.  Allows spatial derivative of chemical potential when
 * coupled to material state such as stress, etc.  Can be used to model species diffusion mediated
 * creep
 **/
template <ComputeStage compute_stage>
class ADCHSplitChemicalPotential : public ADKernel<compute_stage>
{
public:
  ADCHSplitChemicalPotential(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual();

  // Chemical potential property evaluated at material points
  const ADMaterialProperty(Real) & _chemical_potential;

  usingKernelMembers;
};
