//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SINGLEMATRIXPRECONDITIONER_H
#define SINGLEMATRIXPRECONDITIONER_H

#include "MoosePreconditioner.h"

class SingleMatrixPreconditioner;

template <>
InputParameters validParams<SingleMatrixPreconditioner>();

/**
 * Single matrix preconditioner.
 */
class SingleMatrixPreconditioner : public MoosePreconditioner
{
public:
  SingleMatrixPreconditioner(const InputParameters & params);
};

#endif /* SINGLEMATRIXPRECONDITIONER_H */
