/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef PARSEDMATERIAL_H
#define PARSEDMATERIAL_H

#include "ParsedMaterialHelper.h"
#include "FunctionMaterialBase.h"
#include "ParsedMaterialBase.h"

// Forward Declarations
class ParsedMaterial;

template <>
InputParameters validParams<ParsedMaterial>();

/**
 * FunctionMaterialBase child class to evaluate a parsed function. The function
 * can access non-linear and aux variables (unlike MooseParsedFunction).
 */
class ParsedMaterial : public ParsedMaterialHelper, public ParsedMaterialBase
{
public:
  ParsedMaterial(const InputParameters & parameters);
};

#endif // PARSEDMATERIAL_H
