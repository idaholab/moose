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
