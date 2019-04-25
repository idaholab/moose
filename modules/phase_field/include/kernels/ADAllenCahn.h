//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADAllenCahnBase.h"

// Forward Declarations
template <ComputeStage>
class ADAllenCahn;

declareADValidParams(ADAllenCahn);

/**
 * ADAllenCahn uses the Free Energy function and derivatives
 * provided by a DerivativeParsedMaterial to computer the
 * residual for the bulk part of the Allen-Cahn equation.
 */
template <ComputeStage compute_stage>
class ADAllenCahn : public ADAllenCahnBase<compute_stage, Real>
{
public:
  ADAllenCahn(const InputParameters & parameters);

protected:
  virtual ADReal computeDFDOP();

  const MaterialPropertyName _f_name;
  const ADMaterialProperty(Real) & _dFdEta;

  usingAllenCahnBaseMembers(Real);
};

