//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef STICKYBC_H
#define STICKYBC_H

#include "NodalBC.h"

class StickyBC;

template <>
InputParameters validParams<StickyBC>();

/**
 * Sticky-type boundary condition, where if
 * the old variable value exceeds the bounds provided
 * u is fixed (ala Dirichlet) to the old value
 */
class StickyBC : public NodalBC
{
public:
  StickyBC(const InputParameters & parameters);

protected:
  virtual bool shouldApply() override;
  virtual Real computeQpResidual() override;

  // old value of the variable
  const VariableValue & _u_old;
  /// The minimum bound
  const Real _min_value;
  /// The maximum bound
  const Real _max_value;
};

#endif /* STICKYBC_H */
