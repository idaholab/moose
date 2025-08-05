#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "FunctionParserUtils.h"
#include "MFEMGeneralUserObject.h"

struct MFEMProblemData;
/**
 * Declares parsed functions based on names and values prescribed by input parameters.
 */
class MFEMParsedFunction : public MFEMGeneralUserObject, public FunctionParserUtils<false>
{
public:
  static InputParameters validParams();

  MFEMParsedFunction(const InputParameters & parameters);
  virtual ~MFEMParsedFunction();

protected:
  /// function expression
  std::string _function;
  /// function variables
  const std::vector<std::string> & _var_names;
  unsigned int _num_props;
  /// import coordinates and time
  const bool _use_xyzt;
  /// coordinate and time variable names
  const std::vector<std::string> _xyzt;
  MFEMProblemData & _problem_data;
  /// function parser object for the resudual and on-diagonal Jacobian
  SymFunctionPtr _func_F;
};

#endif
