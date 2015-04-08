/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef DERIVATIVEPARSEDMATERIALHELPER_H
#define DERIVATIVEPARSEDMATERIALHELPER_H

#include "DerivativeFunctionMaterialBase.h"
#include "ParsedMaterialHelper.h"
#include "libmesh/fparser_ad.hh"

// Forward Declarations
class DerivativeParsedMaterialHelper;

template<>
InputParameters validParams<DerivativeParsedMaterialHelper>();

/**
 * Helper class to perform the auto derivative taking.
 */
class DerivativeParsedMaterialHelper : public ParsedMaterialHelper<DerivativeFunctionMaterialBase>
{
public:
  DerivativeParsedMaterialHelper(const std::string & name,
                                 InputParameters parameters,
                                 VariableNameMappingMode map_mode = USE_PARAM_NAMES);

  virtual ~DerivativeParsedMaterialHelper();

protected:
  virtual void computeProperties();

  virtual void functionsPostParse();

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
