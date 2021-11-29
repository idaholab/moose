//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MaterialStdVectorAuxBase.h"

/**
 * AuxKernel for outputting a std::vector material-property component to an AuxVariable
 */
class MaterialStdVectorRealGradientAux : public MaterialStdVectorAuxBase<RealGradient>
{
public:
  static InputParameters validParams();

  MaterialStdVectorRealGradientAux(const InputParameters & parameters);

protected:
  virtual Real getRealValue() override;

  /// component of the real gradient to be extracted
  unsigned int _component;
};
