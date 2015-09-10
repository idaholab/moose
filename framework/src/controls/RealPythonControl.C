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

#include "RealPythonControl.h"

template<>
InputParameters validParams<RealPythonControl>()
{
  InputParameters params = validParams<PythonControlBase<> >();
  return params;
}

RealPythonControl::RealPythonControl(const InputParameters & parameters) :
    PythonControlBase(parameters)
{
}

PyObject*
RealPythonControl::buildPythonArguments()
{
  /// Creates a tuple of three doubles (control paramter, time, postprocessor)
  return Py_BuildValue("(ddd)", _parameter, _t, _monitor);
}

Real
RealPythonControl::getPythonResult(PyObject * result)
{
  /// Converts the return value from python into a double
  return PyFloat_AsDouble(result);
}
