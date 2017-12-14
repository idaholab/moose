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

#ifndef FEVALUEPENALTYBC_H
#define FEVALUEPENALTYBC_H

// MOOSE includes
#include "FunctionPenaltyDirichletBC.h"

// Forward declarations
class FEValuePenaltyBC;

template <>
InputParameters validParams<FEValuePenaltyBC>();

/// Defines an FE-based BC that strongly encourages the values to match
class FEValuePenaltyBC : public FunctionPenaltyDirichletBC
{
public:
  FEValuePenaltyBC(const InputParameters & parameters);
};

#endif // FEVALUEPENALTYBC_H
