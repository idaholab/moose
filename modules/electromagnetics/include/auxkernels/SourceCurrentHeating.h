//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

class SourceCurrentHeating : public AuxKernel
{
public:
  static InputParameters validParams();

  SourceCurrentHeating(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

private:
  const VectorVariableValue & _E_real;
  const VectorVariableValue & _E_imag;
  const Function & _source_real;
  const Function & _source_imag;
};
