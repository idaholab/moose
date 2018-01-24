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

#ifndef FINITEDIFFERENCEPRECONDITIONER_H
#define FINITEDIFFERENCEPRECONDITIONER_H

#include "MoosePreconditioner.h"
#include "MooseEnum.h"

class FiniteDifferencePreconditioner;

template <>
InputParameters validParams<FiniteDifferencePreconditioner>();

/**
 * Finite difference preconditioner.
 */
class FiniteDifferencePreconditioner : public MoosePreconditioner
{
public:
  FiniteDifferencePreconditioner(const InputParameters & params);
  MooseEnum & finiteDifferenceType() { return _finite_difference_type; }

private:
  MooseEnum _finite_difference_type;
};

#endif /* FINITEDIFFERENCEPRECONDITIONER_H */
