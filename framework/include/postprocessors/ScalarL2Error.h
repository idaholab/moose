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

#ifndef SCALARL2ERROR_H
#define SCALARL2ERROR_H

// MOOSE includes
#include "GeneralPostprocessor.h"

// Forward Declarations
class Function;
class ScalarL2Error;
class MooseVariableScalar;

template <>
InputParameters validParams<ScalarL2Error>();

/**
 * Postprocessor for computing the error in a scalar value relative to
 * a known Function's value.
 */
class ScalarL2Error : public GeneralPostprocessor
{
public:
  ScalarL2Error(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;

  /**
   * Get the L2 Error.
   */
  virtual Real getValue() override;

protected:
  MooseVariableScalar & _var;
  Function & _func;
};

#endif // SCALARL2ERROR_H
