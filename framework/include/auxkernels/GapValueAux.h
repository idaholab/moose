//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef GAPVALUEAUX_H
#define GAPVALUEAUX_H

#include "AuxKernel.h"

// Forward Declarations
class GapValueAux;
class PenetrationLocator;

template <>
InputParameters validParams<GapValueAux>();

class GapValueAux : public AuxKernel
{
public:
  GapValueAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  PenetrationLocator & _penetration_locator;

  MooseVariable & _moose_var;

  const NumericVector<Number> *& _serialized_solution;

  const DofMap & _dof_map;

  const bool _warnings;
};

#endif // GAPVALUEAUX_H
