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

class CheckActiveMatProp : public AuxKernel
{
public:
  static InputParameters validParams();

  CheckActiveMatProp(const InputParameters & parameters);

protected:
  virtual Real computeValue();
  const MaterialPropertyName _prop_name;
  FEProblemBase & _problem;
  const MaterialData & _data;
  const unsigned int _prop_id;
};
