//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MATTESTNEUMANNBC_H
#define MATTESTNEUMANNBC_H

#include "NeumannBC.h"

class MatTestNeumannBC;

template <>
InputParameters validParams<MatTestNeumannBC>();

/**
 * Neumann boundary condition for testing BoundaryRestrictable class
 */
class MatTestNeumannBC : public NeumannBC
{
public:
  MatTestNeumannBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  const std::string _prop_name;

  const MaterialProperty<Real> * _value;
};

#endif /* MATTESTNEUMANNBC_H */
