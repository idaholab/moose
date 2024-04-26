//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementSubdomainModifier.h"
#include "MooseEnum.h"

class ModifyElementSubdomainByVar : public ElementSubdomainModifier
{
public:
  static InputParameters validParams();

  ModifyElementSubdomainByVar(const InputParameters & parameters);

protected:
  virtual SubdomainID computeSubdomainID() override;

private:
  const VariableValue & _v;
};
