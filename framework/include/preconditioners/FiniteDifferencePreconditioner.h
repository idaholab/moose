//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
