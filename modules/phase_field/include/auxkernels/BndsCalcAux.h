//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef BNDSCALCAUX_H
#define BNDSCALCAUX_H

#include "AuxKernel.h"

// Forward Declarations
class BndsCalcAux;

template <>
InputParameters validParams<BndsCalcAux>();

/**
 * Visualize the location of grain boundaries in a polycrystalline simulation.
 */
class BndsCalcAux : public AuxKernel
{
public:
  BndsCalcAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  const unsigned int _op_num;
  std::vector<const VariableValue *> _vals;
};

#endif // BNDSCALCAUX_H
