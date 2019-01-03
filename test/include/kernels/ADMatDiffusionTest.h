//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#ifndef ADMATDIFFUSIONTEST_H
#define ADMATDIFFUSIONTEST_H

#include "ADKernel.h"
#include "MaterialProperty.h"

// Forward Declaration
template <ComputeStage compute_stage>
class ADMatDiffusionTest;

declareADValidParams(ADMatDiffusionTest);

template <ComputeStage compute_stage>
class ADMatDiffusionTest : public ADKernel<compute_stage>
{
public:
  ADMatDiffusionTest(const InputParameters & parameters);

protected:
  virtual ADResidual computeQpResidual();

  const ADMaterialProperty(Real) & _ad_diff_from_ad_prop;
  const MaterialProperty<Real> & _regular_diff_from_ad_prop;
  const ADMaterialProperty(Real) & _ad_diff_from_regular_prop;
  const MaterialProperty<Real> & _regular_diff_from_regular_prop;
  const MooseEnum _prop_to_use;

  usingKernelMembers;
};

#endif // ADMATDIFFUSIONTEST_H
