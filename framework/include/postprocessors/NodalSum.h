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

#ifndef NODALSUM_H
#define NODALSUM_H

#include "NodalVariablePostprocessor.h"

// Forward Declarations
class NodalSum;

template <>
InputParameters validParams<NodalSum>();

/**
 * Computes a sum of the nodal values of the coupled variable.
 */
class NodalSum : public NodalVariablePostprocessor
{
public:
  NodalSum(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual Real getValue() override;

  void threadJoin(const UserObject & y) override;

protected:
  Real _sum;
};

#endif // NODALSUM_H
