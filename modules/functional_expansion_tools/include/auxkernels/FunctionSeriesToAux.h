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

#ifndef FUNCTIONSERIESTOAUX_H
#define FUNCTIONSERIESTOAUX_H

// MOOSE includes
#include "FunctionAux.h"

// Forward declarations
class FunctionSeriesToAux;

template <>
InputParameters validParams<FunctionSeriesToAux>();

/**
 * Specialization of FunctionAux that is designed to work specifically with FEs,
 * namely that it is always processed at timestep_begin
 */
class FunctionSeriesToAux : public FunctionAux
{
public:
  FunctionSeriesToAux(const InputParameters & parameters);
};

#endif // FUNCTIONSERIESTOAUX_H
