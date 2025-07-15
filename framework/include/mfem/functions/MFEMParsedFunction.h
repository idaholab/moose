#pragma once
#include "FunctionParserUtils.h"
#include "MFEMProblemData.h"
#include "MFEMGeneralUserObject.h"
/**
 * Declares material properties based on names and values prescribed by input parameters.
 *
 * This is identical in function to the GenericConstantMaterial in Moose.
 */
class MFEMParsedFunction : public MFEMGeneralUserObject, public FunctionParserUtils<false>
{
public:
  static InputParameters validParams();

  MFEMParsedFunction(const InputParameters & parameters);
  virtual ~MFEMParsedFunction();

  /**
   * Creates the parsed function.
   */
  virtual void initialSetup() override;

protected:
  const std::string & _prop_name;
  /// function expression
  std::string _function;
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
