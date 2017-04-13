/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
