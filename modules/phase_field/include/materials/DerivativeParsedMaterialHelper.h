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
class DerivativeParsedMaterialHelper : public ParsedMaterialHelper
{
public:
  DerivativeParsedMaterialHelper(const std::string & name,
                                 InputParameters parameters,
                                 VariableNameMappingMode map_mode = USE_PARAM_NAMES);

  virtual ~DerivativeParsedMaterialHelper();

protected:
  virtual void computeProperties();

  virtual void functionsPostParse();

  void assembleDerivatives();

  struct QueueItem;
  typedef std::pair<MaterialProperty<Real> *,ADFunction *> Derivative;

  /// The requested derivatives of the free energy
  std::vector<Derivative> _derivatives;

  /// maximum derivative order
  unsigned int _derivative_order;
};

struct DerivativeParsedMaterialHelper::QueueItem {
  QueueItem() : _F(NULL), _dargs(0) {}
  QueueItem(ADFunction * F) : _F(F), _dargs(0) {}
  QueueItem(const QueueItem & rhs) : _F(rhs._F), _dargs(rhs._dargs) {}

  ADFunction * _F;
  std::vector<unsigned int> _dargs;
};

#endif // DERIVATIVEPARSEDMATERIALHELPER_H
