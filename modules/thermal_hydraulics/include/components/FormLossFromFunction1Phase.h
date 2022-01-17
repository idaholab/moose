#pragma once

#include "FormLoss1PhaseBase.h"

/**
 * Component for prescribing a form loss over a 1-phase flow channel given by a function
 */
class FormLossFromFunction1Phase : public FormLoss1PhaseBase
{
public:
  FormLossFromFunction1Phase(const InputParameters & params);

  virtual void addMooseObjects() override;

public:
  static InputParameters validParams();
};
