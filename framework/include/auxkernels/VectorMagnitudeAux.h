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

template<>
InputParameters validParams<VectorMagnitudeAux>();

/**
 *
 */
class VectorMagnitudeAux : public AuxKernel
{
public:
  VectorMagnitudeAux(const std::string & name, InputParameters parameters);
  virtual ~VectorMagnitudeAux();

protected:
  virtual Real computeValue();

  VariableValue & _x;
  VariableValue & _y;
  VariableValue & _z;
};

#endif /* VECTORMAGNITUDEAUX_H */
