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

// Forward Declarations
class MaterialPropertyUserObject;

class MatPropUserObjectAux : public AuxKernel
{
public:
  static InputParameters validParams();

  MatPropUserObjectAux(const InputParameters & parameters);

  virtual ~MatPropUserObjectAux() {}

protected:
  virtual Real computeValue();

  const MaterialPropertyUserObject & _mat_uo;
};
