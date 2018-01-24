//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ZEROINTERFACE_H
#define ZEROINTERFACE_H

#include "MooseTypes.h"
#include "MooseVariableBase.h"
#include "libmesh/libmesh_common.h"

class FEProblemBase;
class InputParameters;

/**
 * Interface to bring zero values inside objects
 *
 * It brings the following kind of zeroes:
 * - Real _real_zero
 * - VariableValue _zero
 * - VariableGradient _grad_zero
 * - VariableSecond _second_zero
 * - VariablePhiSecond _second_phi_zero
 */
class ZeroInterface
{
public:
  ZeroInterface(const InputParameters & parameters);

protected:
  FEProblemBase & _zi_feproblem;
  THREAD_ID _zi_tid;
  const Real & _real_zero;
  const VariableValue & _zero;
  const VariableGradient & _grad_zero;
  const VariableSecond & _second_zero;
  const VariablePhiSecond & _second_phi_zero;
};

#endif /* ZEROINTERFACE_H */
