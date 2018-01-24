//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NODALNORMALBC_H
#define NODALNORMALBC_H

#include "NodalBC.h"

class NodalNormalBC;

template <>
InputParameters validParams<NodalNormalBC>();

/**
 * This is a base class to enforce strong boundary condition with a normal defined at a node
 *
 * NOTE: This class will not compute the normal! It is computed in a user object subsystem.
 */
class NodalNormalBC : public NodalBC
{
public:
  NodalNormalBC(const InputParameters & parameters);

  virtual void computeResidual(NumericVector<Number> & residual) override;

protected:
  const VariableValue & _nx;
  const VariableValue & _ny;
  const VariableValue & _nz;
  /// Normal at the node (it is pre-computed by user object subsystem)
  Point _normal;
};

#endif /* NODALNORMALBC_H */
