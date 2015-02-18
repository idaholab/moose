#ifndef PARSEDMATERIAL_H
#define PARSEDMATERIAL_H

#include "ParsedMaterialHelper.h"
#include "ParsedMaterialBase.h"

// Forward Declarations
class ParsedMaterial;

template<>
InputParameters validParams<ParsedMaterial>();

/**
 * FunctionMaterialBase child class to evaluate a parsed function. The function
 * can access non-linear and aux variables (unlike MooseParsedFunction).
 */
class ParsedMaterial : public ParsedMaterialHelper, public ParsedMaterialBase
{
public:
  ParsedMaterial(const std::string & name,
                 InputParameters parameters);
};

#endif //PARSEDMATERIAL_H
