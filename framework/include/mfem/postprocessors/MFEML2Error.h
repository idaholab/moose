#pragma once
#include "MFEMPostprocessor.h"
#include "MFEMGeneralUserObject.h"

/*
 * Compute the L2 error for a variable.
 */
class MFEML2Error : public MFEMPostprocessor
{
public:
  static InputParameters validParams();

  MFEML2Error(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;

  /**
   * Get the L2 Error.
   */
  virtual PostprocessorValue getValue() const override final;

private:
  VariableName _var_name;
  FunctionName _coeff_name;
  std::shared_ptr<mfem::Coefficient> _coeff;
  mfem::GridFunction & _var;
};
