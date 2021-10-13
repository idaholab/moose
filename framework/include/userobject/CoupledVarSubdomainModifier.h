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

class CoupledVarSubdomainModifier : public ElementSubdomainModifier
{
public:
  static InputParameters validParams();

  CoupledVarSubdomainModifier(const InputParameters & parameters);

protected:
  virtual SubdomainID computeSubdomainID() const override;

private:
  /// The coupled variable that defines the Subdomain ID
  const VariableValue & _v;
};
