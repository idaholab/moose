#pragma once

#include "DerivativeParsedMaterialHelper.h"
#include "ExpressionBuilder.h"

// Forward Declarations
class EBCoupledVarTest;

template <>
InputParameters validParams<EBCoupledVarTest>();

/**
 * Grain boundary energy parameters for isotropic uniform grain boundary energies
 */
class EBCoupledVarTest : public DerivativeParsedMaterialHelper,
		                     public ExpressionBuilder
{
public:
  EBCoupledVarTest(const InputParameters & parameters);

protected:
  const unsigned int _op_num;
  std::vector<EBTerm> _vals;
};
