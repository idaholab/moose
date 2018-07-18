//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#ifndef ADMATDIFFUSION_H
#define ADMATDIFFUSION_H

#include "ADKernel.h"
#include "MaterialProperty.h"

// Forward Declaration
template <ComputeStage compute_stage>
class ADMatDiffusion;

declareADValidParams(ADMatDiffusion);

template <ComputeStage compute_stage>
class ADMatDiffusion : public ADKernel<compute_stage>
{
public:
  ADMatDiffusion(const InputParameters & parameters);

protected:
  virtual ADResidual computeQpResidual();

  const ADMaterialPropertyType(Real) & _ad_diff_from_ad_prop;
  const MaterialProperty<Real> & _regular_diff_from_ad_prop;
  const ADMaterialPropertyType(Real) & _ad_diff_from_regular_prop;
  const MaterialProperty<Real> & _regular_diff_from_regular_prop;
  const MooseEnum _prop_to_use;

  usingKernelMembers;
};

#endif // ADMATDIFFUSION_H
