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
