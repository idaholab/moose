#pragma once

#include "FunctionParserUtils.h"
#include "MFEMCoefficient.h"
#include "PlatypusUtils.h"
#include "auxsolvers.hpp"
#include "libmesh/fparser_ad.hh"
#include "libmesh/quadrature.h"

#define usingMFEMParsedCoefficientHelperMembers()                                                  \
  usingFunctionParserUtilsMembers();                                                               \
  using typename MFEMParsedCoefficientHelper::VariableNameMappingMode;                             \
  using typename MFEMParsedCoefficientHelper::MatPropDescriptorList;                               \
  using MFEMParsedCoefficientHelper::functionParse;                                                \
  using MFEMParsedCoefficientHelper::functionsPostParse;                                           \
  using MFEMParsedCoefficientHelper::functionsOptimize;                                            \
  using MFEMParsedCoefficientHelper::_func_F;                                                      \
  using MFEMParsedCoefficientHelper::_symbol_names;                                                \
  using MFEMParsedCoefficientHelper::_gridfunction_names;                                          \
  using MFEMParsedCoefficientHelper::_coefficient_names;                                           \
  using MFEMParsedCoefficientHelper::_map_mode

/**
 * Helper class to perform the parsing and optimization of the
 * function expression.
 */
class MFEMParsedCoefficientHelper : public MFEMCoefficient,
                                    public hephaestus::CoupledCoefficient,
                                    public FunctionParserUtils<false>
{
public:
  enum class VariableNameMappingMode
  {
    USE_MOOSE_NAMES,
    USE_PARAM_NAMES
  };

  MFEMParsedCoefficientHelper(const InputParameters & parameters, VariableNameMappingMode map_mode);

  static InputParameters validParams();

  void functionParse(const std::string & function_expression);
  void functionParse(const std::string & function_expression,
                     const std::vector<std::string> & constant_names,
                     const std::vector<std::string> & constant_expressions);
  void functionParse(const std::string & function_expression,
                     const std::vector<std::string> & constant_names,
                     const std::vector<std::string> & constant_expressions,
                     const std::vector<std::string> & _mfem_coefficient_names);
  void functionParse(const std::string & function_expression,
                     const std::vector<std::string> & constant_names,
                     const std::vector<std::string> & constant_expressions,
                     const std::vector<std::string> & _mfem_coefficient_names,
                     const std::vector<std::string> & _mfem_gridfunction_names);
  void Init(const hephaestus::GridFunctions & variables,
            hephaestus::Coefficients & coefficients) override;

  double Eval(mfem::ElementTransformation & trans, const mfem::IntegrationPoint & ip) override;

  std::shared_ptr<mfem::Coefficient> getCoefficient() const override
  {
    return PlatypusUtils::dynamic_const_cast<mfem::Coefficient>(getSharedPtr());
  }

protected:
  usingFunctionParserUtilsMembers(false);

  // run FPOptimizer on the parsed function
  virtual void functionsOptimize();

  /// The undiffed free energy function parser object.
  SymFunctionPtr _func_F;

  /**
   * Symbol names used in the expression (depends on the map_mode).
   * We distinguish "symbols" i.e. FParser placeholder names from "variables", which
   * are MOOSE solution objects
   */
  std::vector<std::string> _symbol_names;

  std::vector<std::string> _gridfunction_names;
  std::vector<mfem::ParGridFunction *> _gridfunctions;

  std::vector<std::string> _coefficient_names;
  std::vector<mfem::Coefficient *> _coefficients;
};
