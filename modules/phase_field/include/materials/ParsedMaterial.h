#ifndef PARSEDMATERIAL_H
#define PARSEDMATERIAL_H

#include "ParsedMaterialHelper.h"
#include "ParsedMaterialBase.h"

// Forward Declarations
class ParsedMaterial;

template<>
InputParameters validParams<ParsedMaterial>();

/**
 * DerivativeFunctionMaterialBase child class to evaluate a parsed function for the
 * free energy and automatically provide all derivatives.
 * This requires the autodiff patch (https://github.com/libMesh/libmesh/pull/238)
 * to Function Parser in libmesh.
 */
class ParsedMaterial : public ParsedMaterialHelper, public ParsedMaterialBase
{
public:
  ParsedMaterial(const std::string & name,
                 InputParameters parameters);
};

#endif //PARSEDMATERIAL_H
