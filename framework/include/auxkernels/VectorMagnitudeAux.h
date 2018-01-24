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
#ifndef VECTORMAGNITUDEAUX_H
#define VECTORMAGNITUDEAUX_H

#include "AuxKernel.h"

class VectorMagnitudeAux;

template <>
InputParameters validParams<VectorMagnitudeAux>();

/**
 * Computes the magnitude of a vector whose components are given by up
 * to three coupled variables.
 */
class VectorMagnitudeAux : public AuxKernel
{
public:
  VectorMagnitudeAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  const VariableValue & _x;
  const VariableValue & _y;
  const VariableValue & _z;
};

#endif /* VECTORMAGNITUDEAUX_H */
