//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
