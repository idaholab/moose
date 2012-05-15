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

class FiniteDifferencePreconditioner;

template<>
InputParameters validParams<FiniteDifferencePreconditioner>();

/**
 * Finite difference preconditioner.
 */
class FiniteDifferencePreconditioner : public MoosePreconditioner
{
public:
  FiniteDifferencePreconditioner(const std::string & name, InputParameters params);
  virtual ~FiniteDifferencePreconditioner();
};

#endif /* FINITEDIFFERENCEPRECONDITIONER_H */
