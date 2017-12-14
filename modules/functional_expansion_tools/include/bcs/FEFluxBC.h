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

#ifndef FEFLUXBC_H
#define FEFLUXBC_H

// MOOSE includes
#include "FunctionNeumannBC.h"

// Forward declarations
class FEFluxBC;

template <>
InputParameters validParams<FEFluxBC>();

/// Defines an FE-based BC that strongly encourages the gradients to match
class FEFluxBC : public FunctionNeumannBC
{
public:
  FEFluxBC(const InputParameters & parameters);
};

#endif // FEFLUXBC_H
