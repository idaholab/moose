/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
