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

template <>
InputParameters validParams<DerivativeParsedMaterialHelper>();

/**
 * Helper class to perform the auto derivative taking.
 */
class DerivativeParsedMaterialHelper : public ParsedMaterialHelper
{
public:
  DerivativeParsedMaterialHelper(const InputParameters & parameters,
                                 VariableNameMappingMode map_mode = USE_PARAM_NAMES);

protected:
  virtual void computeProperties();

  virtual void functionsPostParse();
  void assembleDerivatives();
  MatPropDescriptorList::iterator findMatPropDerivative(const FunctionMaterialPropertyDescriptor &);

  struct QueueItem;
  struct Derivative;

  /// The requested derivatives of the free energy
  std::vector<Derivative> _derivatives;

  /// variable base name for the dynamically material property derivatives
  const std::string _dmatvar_base;

  /// next available variable number for automatically created material property derivative variables
  unsigned int _dmatvar_index;

  /// maximum derivative order
  unsigned int _derivative_order;
};

struct DerivativeParsedMaterialHelper::QueueItem
{
  QueueItem() : _dargs(0) {}
  QueueItem(ADFunctionPtr & F) : _F(F), _dargs(0) {}
  QueueItem(const QueueItem & rhs) : _F(rhs._F), _dargs(rhs._dargs) {}

  ADFunctionPtr _F;
  std::vector<unsigned int> _dargs;
};

struct DerivativeParsedMaterialHelper::Derivative
{
  MaterialProperty<Real> * first;
  ADFunctionPtr second;
  std::vector<VariableName> darg_names;
};

#endif // DERIVATIVEPARSEDMATERIALHELPER_H
