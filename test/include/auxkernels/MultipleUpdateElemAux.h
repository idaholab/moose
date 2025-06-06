//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

/**
 * Aux kernel that updated values of coupled variables
 */
class MultipleUpdateElemAux : public AuxKernel
{
public:
  static InputParameters validParams();

  MultipleUpdateElemAux(const InputParameters & parameters);
  virtual ~MultipleUpdateElemAux();

protected:
  virtual void compute();
  virtual Real computeValue();
  virtual void computeVarValues(std::vector<Real> & values);

  unsigned int _n_vars;
  std::vector<MooseWritableVariable *> _vars;

  const bool _use_compute_value;
};
