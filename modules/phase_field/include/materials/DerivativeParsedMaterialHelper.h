#ifndef DERIVATIVEPARSEDMATERIALHELPER_H
#define DERIVATIVEPARSEDMATERIALHELPER_H

#include "DerivativeBaseMaterial.h"
#include "ParsedMaterialHelper.h"
#include "libmesh/fparser_ad.hh"

// Forward Declarations
class DerivativeParsedMaterialHelper;

template<>
InputParameters validParams<DerivativeParsedMaterialHelper>();

/**
 * Helper class to perform the bulk of the bulk of the auto derivative taking.
 */
class DerivativeParsedMaterialHelper : public DerivativeFunctionMaterialBase, public ParsedMaterialHelper
{
public:
  DerivativeParsedMaterialHelper(const std::string & name,
                                 InputParameters parameters,
                                 VariableNameMappingMode map_mode = USE_PARAM_NAMES);

protected:
  virtual void computeProperties();

  void functionsDerivative();
  void functionsOptimize();

  /// The first derivatives of the free energy (function parser objects).
  std::vector<ADFunction *> _func_dF;

  /// The second derivatives of the free energy (function parser objects).
  std::vector<std::vector<ADFunction *> > _func_d2F;

  /// The third derivatives of the free energy (function parser objects).
  std::vector<std::vector<std::vector<ADFunction *> > > _func_d3F;
};

#endif // DERIVATIVEPARSEDMATERIALHELPER_H
