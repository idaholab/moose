#pragma once

#include "FormLoss1PhaseBase.h"

class FormLossFromFunction1Phase;

template <>
InputParameters validParams<FormLossFromFunction1Phase>();

/**
 * Component for prescribing a form loss over a 1-phase flow channel given by a function
 */
class FormLossFromFunction1Phase : public FormLoss1PhaseBase
{
public:
  FormLossFromFunction1Phase(const InputParameters & params);

  virtual void addMooseObjects() override;
};
