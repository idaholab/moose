//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef DERIVATIVEPARSEDMATERIAL_H
#define DERIVATIVEPARSEDMATERIAL_H

#include "DerivativeParsedMaterialHelper.h"
#include "ParsedMaterialBase.h"

// Forward Declarations
class DerivativeParsedMaterial;

template <>
InputParameters validParams<DerivativeParsedMaterial>();

/**
 * DerivativeFunctionMaterialBase child class to evaluate a parsed function (for
 * example a free energy) and automatically provide all derivatives.
 */
class DerivativeParsedMaterial : public DerivativeParsedMaterialHelper, public ParsedMaterialBase
{
public:
  DerivativeParsedMaterial(const InputParameters & parameters);
};

#endif // DERIVATIVEPARSEDMATERIAL_H
