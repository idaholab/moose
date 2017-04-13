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

#ifndef NODALL2NORM_H
#define NODALL2NORM_H

#include "NodalVariablePostprocessor.h"

// Forward Declarations
class NodalL2Norm;

template <>
InputParameters validParams<NodalL2Norm>();

/**
 * Computes the "nodal" L2-norm of the coupled variable, which is
 * defined by summing the square of its value at every node and taking
 * the square root.
 */
class NodalL2Norm : public NodalVariablePostprocessor
{
public:
  NodalL2Norm(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual Real getValue() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  Real _sum_of_squares;
};

#endif
