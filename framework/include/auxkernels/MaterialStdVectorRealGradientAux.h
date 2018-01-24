//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MATERIALSTDVECTORREALGRADIENTAUX_H
#define MATERIALSTDVECTORREALGRADIENTAUX_H

// MOOSE includes
#include "MaterialStdVectorAuxBase.h"

// Forward declarations
class MaterialStdVectorRealGradientAux;

template <>
InputParameters validParams<MaterialStdVectorRealGradientAux>();

/**
 * AuxKernel for outputting a std::vector material-property component to an AuxVariable
 */
class MaterialStdVectorRealGradientAux : public MaterialStdVectorAuxBase<RealGradient>
{
public:
  MaterialStdVectorRealGradientAux(const InputParameters & parameters);

protected:
  virtual Real getRealValue() override;

  /// component of the real gradient to be extracted
  unsigned int _component;
};

#endif // MATERIALSTDVECTORREALGRADIENTAUX_H
