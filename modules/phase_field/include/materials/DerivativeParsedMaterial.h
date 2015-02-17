#ifndef DERIVATIVEPARSEDMATERIAL_H
#define DERIVATIVEPARSEDMATERIAL_H

#include "DerivativeParsedMaterialHelper.h"
#include "ParsedMaterialBase.h"

// Forward Declarations
class DerivativeParsedMaterial;

template<>
InputParameters validParams<DerivativeParsedMaterial>();

/**
 * DerivativeFunctionMaterialBase child class to evaluate a parsed function for the
 * free energy and automatically provide all derivatives.
 * This requires the autodiff patch (https://github.com/libMesh/libmesh/pull/238)
 * to Function Parser in libmesh.
 */
class DerivativeParsedMaterial : public DerivativeParsedMaterialHelper, public ParsedMaterialBase
{
public:
  DerivativeParsedMaterial(const std::string & name,
                           InputParameters parameters);
};

#endif // DERIVATIVEPARSEDMATERIAL_H
