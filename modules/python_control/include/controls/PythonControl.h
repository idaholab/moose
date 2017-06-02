#ifndef PYTHONCONTROL_H
#define PYTHONCONTROL_H

#include "Control.h"
#include <Python.h>

class PythonControl;

template <>
InputParameters validParams<PythonControl>();

/**
 * Allows a MOOSE application to be controlled with a python program.
 * Takes two different functions: execute(controlled, postprocessor) and
 * keep_going(controlled, postprocessor)
 * keep_going returns a boolean and if it is false, the simulation
 * will stop if Transient is used.
 */
class PythonControl : public Control
{
public:
  /**
   * Constructor
   *
   * @param parameters The parameters object holding data for the class to use.
   */
  PythonControl(const InputParameters & parameters);

  /**
   * Runs the python functions.
   */
  virtual void execute() override;

private:
  /**
   * If called, the simulation should finish.
   */
  void stopGoing();

  /// A list of the variables that are controlled (can be changed)
  std::vector<std::string> _controlled_vars;

  /// A list of the variables that give information to the python functions
  std::vector<std::string> _postprocessor_vars;

  /// This is the python module that the functions are in.
  PyObject * _python_module;

  /// These are the execute function and the keep going function.
  PyObject *_execute_function, *_keep_going_function;
};

#endif /* PYTHONCONTROL_H */
