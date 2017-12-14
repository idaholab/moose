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

#ifndef FEVALUEBC_H
#define FEVALUEBC_H

// MOOSE includes
#include "FunctionDirichletBC.h"

// Forward declarations
class FEValueBC;

template <>
InputParameters validParams<FEValueBC>();

/// Defines an FE-based boundary condition that forces the values to match
class FEValueBC : public FunctionDirichletBC
{
public:
  FEValueBC(const InputParameters & parameters);
};

#endif // FEVALUEBC_H
