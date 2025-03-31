#ifdef MFEM_ENABLED

#pragma once
#include "MFEMPostprocessor.h"
#include "MFEMGeneralUserObject.h"

/*
 * Compute the L2 error for a vector variable.
 */
class MFEMVectorL2Error : public MFEMPostprocessor
{
public:
  static InputParameters validParams();

  MFEMVectorL2Error(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;

  /**
   * Get the L2 Error.
   */
  virtual PostprocessorValue getValue() const override final;

private:
  const VariableName & _var_name;
  const FunctionName & _coeff_name;
  mfem::VectorCoefficient & _vec_coeff;
  mfem::GridFunction & _var;
};

#endif
