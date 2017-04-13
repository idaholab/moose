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

// Standard includes
#include <math.h>

// MOOSE includes
#include "BasicOutput.h"
#include "MooseApp.h"
#include "OversampleOutput.h"

// Define the four possible validParams methods
template <>
InputParameters
validParams<BasicOutput<OversampleOutput>>()
{
  // Get the parameters from the parent object
  InputParameters params = validParams<OversampleOutput>();
  return params;
}

template <>
InputParameters
validParams<BasicOutput<FileOutput>>()
{
  // Get the parameters from the parent object
  InputParameters params = validParams<FileOutput>();
  return params;
}

template <>
InputParameters
validParams<BasicOutput<PetscOutput>>()
{
  // Get the parameters from the parent object
  InputParameters params = validParams<PetscOutput>();
  return params;
}

template <>
InputParameters
validParams<BasicOutput<Output>>()
{
  // Get the parameters from the parent object
  InputParameters params = validParams<Output>();
  return params;
}

// The generic output method used for Output, PetscOutput, and FileOutput base classes
template <class OutputBase>
void
BasicOutput<OutputBase>::outputStep(const ExecFlagType & type)
{
  // Output is not allowed
  if (!OutputBase::_allow_output && type != EXEC_FORCED)
    return;

  // If recovering disable output of initial condition, it was already output
  if (type == EXEC_INITIAL && OutputBase::_app.isRecovering())
    return;

  // Return if the current output is not on the desired interval
  if (type != EXEC_FINAL && !OutputBase::onInterval())
    return;

  // Call the output method
  if (OutputBase::shouldOutput(type))
    output(type);
}

// OversampleOutput template specialization
template <>
void
BasicOutput<OversampleOutput>::outputStep(const ExecFlagType & type)
{
  // Output is not allowed
  if (!_allow_output && type != EXEC_FORCED)
    return;

  // If recovering disable output of initial condition, it was already output
  if (type == EXEC_INITIAL && _app.isRecovering())
    return;

  // Return if the current output is not on the desired interval
  if (type != EXEC_FINAL && !onInterval())
    return;

  // Call the output method (this has the file checking built in b/c OversampleOutput is a
  // FileOutput)
  if (shouldOutput(type))
  {
    updateOversample();
    output(type);
  }
}

// Instantiate the four possible template classes
template class BasicOutput<Output>;
template class BasicOutput<PetscOutput>;
template class BasicOutput<FileOutput>;
template class BasicOutput<OversampleOutput>;
