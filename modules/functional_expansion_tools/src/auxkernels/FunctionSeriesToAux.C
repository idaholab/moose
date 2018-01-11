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

// Module includes
#include "FunctionSeries.h"
#include "FunctionSeriesToAux.h"

template <>
InputParameters
validParams<FunctionSeriesToAux>()
{
  InputParameters params = validParams<FunctionAux>();

  params.addClassDescription("AuxKernel to convert a functional expansion"
                             " (Functions object, type = FunctionSeries) to an AuxVariable");

  // Force this AuxKernel to execute at "timestep_begin"
  params.set<ExecFlagEnum>("execute_on", true) = EXEC_TIMESTEP_BEGIN;
  // Don't let the user change the execution time
  params.suppressParameter<ExecFlagEnum>("execute_on");

  return params;
}

FunctionSeriesToAux::FunctionSeriesToAux(const InputParameters & parameters)
  : FunctionAux(parameters)
{
  FunctionSeries * validate = dynamic_cast<FunctionSeries *>(&_func);

  if (!validate)
    paramError("function", "'function' must be a FunctionSeries type");
}
