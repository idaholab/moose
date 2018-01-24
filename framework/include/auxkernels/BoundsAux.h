//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef BOUNDSAUX_H
#define BOUNDSAUX_H

#include "AuxKernel.h"

// Forward Declarations
class BoundsAux;

template <>
InputParameters validParams<BoundsAux>();

/**
 * Fills in the "bounds vectors" to provide an upper and lower bound for the variable that is
 * coupled in.
 * Doesn't actually calculate an auxiliary value although it must take an auxiliary variable as
 * input.
 *
 * This MUST be run on a Nodal Auxiliary Variable!
 */
class BoundsAux : public AuxKernel
{
public:
  /**
   * Factory constructor.
   */
  BoundsAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  NumericVector<Number> & _upper_vector;
  NumericVector<Number> & _lower_vector;
  unsigned int _bounded_variable_id;
  bool _upper_valid;
  bool _lower_valid;
  Real _upper;
  Real _lower;
};

#endif // BOUNDSAUX_H
