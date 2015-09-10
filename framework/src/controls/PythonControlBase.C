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

// MOOSE includes
#include "PythonControlBase.h"
#include "Function.h"

template<>
InputParameters validParams<PythonControlBase<> >()
{
  InputParameters params = validParams<Control>();
  params.addParam<std::string>("python_function", "function", "The name of the function, within the given file, to be executed.");
  params.addParam<FileName>("python_module", "control.py", "The python file (.py) that contains the python function to execute.");
  params.addParam<PostprocessorName>("monitor", 0.0, "A postprocessor to pass to the function for monitoring the simulation.");

  params.addRequiredParam<std::string>("parameter", "The input parameter(s) to control. Specify a single parameter name and all parameters in all objects matching the name will be updated");

  return params;
}
