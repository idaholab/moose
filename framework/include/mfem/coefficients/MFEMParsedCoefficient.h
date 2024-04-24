#pragma once

#include "MFEMParsedCoefficientHelper.h"
#include "MFEMParsedCoefficientBase.h"

/**
 * MFEMCoefficient child class to evaluate a parsed function. The function
 * can access non-linear and aux variables (unlike MooseParsedFunction).
 */
class MFEMParsedCoefficient : public MFEMParsedCoefficientHelper, public MFEMParsedCoefficientBase
{
public:
  static InputParameters validParams();

  MFEMParsedCoefficient(const InputParameters & parameters);
};
