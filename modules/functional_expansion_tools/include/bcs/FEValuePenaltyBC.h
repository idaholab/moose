//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FEVALUEPENALTYBC_H
#define FEVALUEPENALTYBC_H

// MOOSE includes
#include "FunctionPenaltyDirichletBC.h"

// Forward declarations
class FEValuePenaltyBC;

template <>
InputParameters validParams<FEValuePenaltyBC>();

/**
 * Defines an FE-based BC that strongly encourages the values to match
 */
class FEValuePenaltyBC : public FunctionPenaltyDirichletBC
{
public:
  FEValuePenaltyBC(const InputParameters & parameters);
};

#endif // FEVALUEPENALTYBC_H
