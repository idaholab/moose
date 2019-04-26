//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADSplitCHCRes.h"
#include "DerivativeMaterialPropertyNameInterface.h"

// Forward Declarations
template <ComputeStage>
class ADSplitCHParsed;

declareADValidParams(ADSplitCHParsed);

/**
 * ADSplitCHParsed uses the Free Energy function and derivatives
 * provided by an ADMaterial. Derivatives w.r.t DOFs provided by the MOOSE AD
 * system are required for a correct Jacobian to be formed.
 */
template <ComputeStage compute_stage>
class ADSplitCHParsed : public ADSplitCHCRes<compute_stage>,
                        public DerivativeMaterialPropertyNameInterface
{
public:
  ADSplitCHParsed(const InputParameters & parameters);

protected:
  virtual ADReal computeDFDC();

  /// name of the free energy function
  const MaterialPropertyName _f_name;

  /// chemical potential property
  const ADMaterialProperty(Real) & _dFdc;

  usingSplitCHCResMembers;
};

