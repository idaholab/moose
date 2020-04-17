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

/**
 * ADAllenCahn uses the Free Energy function and derivatives
 * provided by a DerivativeParsedMaterial to computer the
 * residual for the bulk part of the Allen-Cahn equation.
 */
class ADAllenCahn : public ADAllenCahnBase<Real>
{
public:
  static InputParameters validParams();

  ADAllenCahn(const InputParameters & parameters);

protected:
  virtual ADReal computeDFDOP();

  const MaterialPropertyName _f_name;
  const ADMaterialProperty<Real> & _dFdEta;
};
