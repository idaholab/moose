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

/**
 * Coupled auxiliary value
 */
class StatefulAux : public AuxKernel
{
public:
  static InputParameters validParams();

  StatefulAux(const InputParameters & parameters);

  virtual ~StatefulAux() {}

protected:
  virtual Real computeValue();

  /// The coupled variable id
  unsigned int _coupled;
  /// The old coupled variable value
  const VariableValue & _coupled_val_old;
};
