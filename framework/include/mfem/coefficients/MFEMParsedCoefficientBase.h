#pragma once

#include "InputParameters.h"

/**
 * Base class for MFEMParsedCoefficient
 * to declare and read the input parameters.
 */
class MFEMParsedCoefficientBase
{
public:
  static InputParameters validParams();

  MFEMParsedCoefficientBase(const InputParameters & parameters);

protected:
  /// function expression
  std::string _function;

  /// constant vectors
  std::vector<std::string> _constant_names;
  std::vector<std::string> _constant_expressions;

  /// mfem object names
  std::vector<std::string> _mfem_coefficient_names;
  std::vector<std::string> _mfem_gridfunction_names;
};
