#pragma once

#include "FormLoss1PhaseBase.h"

class FormLossFromExternalApp1Phase;

template <>
InputParameters validParams<FormLossFromExternalApp1Phase>();

/**
 * A component for prescribing a form loss computed by an external application
 */
class FormLossFromExternalApp1Phase : public FormLoss1PhaseBase
{
public:
  FormLossFromExternalApp1Phase(const InputParameters & params);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;

protected:
  /// Name of the variable that stores the distributed form loss coefficient
  VariableName _K_prime_var_name;
};
