//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#ifndef ADPHASEFIELDFRACTURE_H
#define ADPHASEFIELDFRACTURE_H

#include "ADKernel.h"

// Forward Declarations
template <ComputeStage>
class ADPhaseFieldFracture;

declareADValidParams(ADPhaseFieldFracture);

/**
 * Phase field fracture model that utilize MOOSE AD system
 */
template <ComputeStage compute_stage>
class ADPhaseFieldFracture : public ADKernel<compute_stage>
{
public:
  ADPhaseFieldFracture(const InputParameters & parameters);

protected:
  virtual ADResidual computeQpResidual() override;

  /// Critical energy release rate for fracture
  const MaterialProperty<Real> & _gc_prop;

  /// Characteristic length, controls damage zone thickness
  const MaterialProperty<Real> & _l;

  const ADMaterialProperty(Real) & _hist;

  const MaterialProperty<Real> & _hist_old;

  usingKernelMembers;
  //@}
};
#endif // ADPHASEFIELDFRACTURE_H
