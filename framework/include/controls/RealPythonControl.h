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

#ifndef REALPYTHONCONTROL_H
#define REALPYTHONCONTROL_H

// MOOSE includes
#include "PythonControlBase.h"

// Forward declarations
class RealPythonControl;

template<>
InputParameters validParams<RealPythonControl>();

/**
 * Python control object for Real parameters
 */
class RealPythonControl : public PythonControlBase<Real>
{
public:

  /**
   * Class constructor
   * @param parameters Input parameters for this Control object
   */
  RealPythonControl(const InputParameters & parameters);

protected:

  /**
   * Build the Python argument tuple
   */
  PyObject* buildPythonArguments();

  /**
   * Extract the result from Python
   */
  Real getPythonResult(PyObject * result);

};

#endif // PYTHONCONTROLBASE_H
