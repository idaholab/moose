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

#include "GeneralPostprocessor.h"
#include "FunctionInterface.h"

class Function;

//Forward Declarations
class ScalarL2Error;

template<>
InputParameters validParams<ScalarL2Error>();

class ScalarL2Error :
  public GeneralPostprocessor
{
public:
  ScalarL2Error(const std::string & name, InputParameters parameters);

  virtual void initialize();
  virtual void execute();

  /**
   * Get the L2 Error.
   */
  virtual Real getValue();

protected:
  MooseVariableScalar & _var;
  Function & _func;
};

#endif //SCALARL2ERROR_H
