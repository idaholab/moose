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

#ifndef PYTHONCONTROLBASE_H
#define PYTHONCONTROLBASE_H

// MOOSE includes
#include "Control.h"

// Include the Python header (this is standard in Python distributions)
#include <Python.h>

// Forward declarations
template<typename T = Real>
class PythonControlBase;

template<>
InputParameters validParams<PythonControlBase<> >();

/**
 * A base class for controlling MOOSE via python functions
 *
 * By default a function with the name 'function' that resides
 * in a module with the name 'control.py' located in the working directory
 * is called.
 *
 * This class has two methods that must be defined in the child class:
 * buildPythonArguments and getPythonResult.
 *
 * @see RealPythonControl
 */
template<typename T>
class PythonControlBase : public Control
{
public:

  /**
   * Class constructor
   * @param parameters Input parameters for this Control object
   */
  PythonControlBase(const InputParameters & parameters);

  /**
   * Class destructor
   */
  virtual ~PythonControlBase();

  /**
   * Build the tuple for use when calling the defined Python function
   */
  virtual PyObject* buildPythonArguments() = 0;

  /**
   * Convert the Python result to the correct type
   */
  virtual T getPythonResult(PyObject *) = 0;

  /**
   * Evaluate the function and set the parameter value
   */
  virtual void execute();

  ///@{
  /**
   * These methods are left intentionally empty
   */
  virtual void initialize(){}
  virtual void finalize(){}
  virtual void threadJoin(const UserObject & /*uo*/){}
  ///@}

protected:

  /// The postprocessor to pass to Python function as a monitor
  const PostprocessorValue & _monitor;

  /// A reference to the parameter to control
  T & _parameter;

private:

  ///@{
  /// Python objects for accessing and calling the user-defined function
  PyObject * _python_module_name;
  PyObject * _python_module;
  PyObject * _python_function;
  ///@}

};

template<typename T>
PythonControlBase<T>::PythonControlBase(const InputParameters & parameters) :
    Control(parameters),
    _monitor(getPostprocessorValue("monitor")),
    _parameter(getControllableParam<Real>("parameter"))
{

  // Extract the module name
  std::string input_module_name = getParam<FileName>("python_module");

  // Check that the file is executable
  MooseUtils::checkFileReadable(input_module_name);

  // Check the file extension
  if (!MooseUtils::hasExtension(input_module_name, "py"))
    mooseError("The file, \"" << input_module_name << "\", must have a *.py extension.");

  // Determine the file location
  std::pair<std::string, std::string> split_file = MooseUtils::splitFileName(input_module_name);
  if (split_file.first == ".")
    split_file.first = MooseUtils::getCurrentWorkingDir();

  // Strip the python extension
  split_file.second = split_file.second.substr(0, split_file.second.size()-3);

  // Initialize Python
  Py_Initialize();

  // Set the python system path to include the modules location
  std::ostringstream oss;
  oss << "import sys; sys.path.insert(0, '";
  oss << split_file.first;
  oss << "')";
  PyRun_SimpleString(oss.str().c_str());

  // From docs.python.org
  // "Reference counts are always manipulated explicitly. The normal way is to use the macro Py_INCREF()
  // to increment an object’s reference count by one, and Py_DECREF() to decrement it by one."

  // Get the Python module name
  _python_module_name = PyString_FromString(split_file.second.c_str());
  if (_python_module_name == NULL)
    mooseError("Failed to load the python module: " << split_file.second << ".");
  Py_INCREF(_python_module_name);

  // Import the module
  _python_module = PyImport_Import(_python_module_name);
  if (_python_module == NULL)
    mooseError("Failed to 'import' the python module: " << split_file.second << ".");
  Py_INCREF(_python_module);

  // Extract the function to call
  _python_function = PyObject_GetAttrString(_python_module, getParam<std::string>("python_function").c_str());
  if (_python_function == NULL)
    mooseError("Failed to access the python function: " << getParam<std::string>("python_function") << ".");
  Py_INCREF(_python_function);
}

template<typename T>
PythonControlBase<T>::~PythonControlBase()
{
  // Decrease python reference counts
  Py_DECREF(_python_function);
  Py_DECREF(_python_module);
  Py_DECREF(_python_module_name);

  // Finished using Python
  Py_Finalize();
}

template<typename T>
void
PythonControlBase<T>::execute()
{
  // Build the arguments to pass to the python function
  PyObject* args = buildPythonArguments();// Py_BuildValue("(ddd)", _parameter, _t, _monitor);
  if (args == NULL)
    mooseError("Failed got setup arguments to pass to python function.");
  Py_INCREF(args);

  // Invoke the function
  PyObject* result = PyObject_CallObject(_python_function, args);
  if (result == NULL)
    mooseError("Failed to execute the Python function.");

  // Convert the output to Real and update the controlled parameter
  _parameter = getPythonResult(result); //PyFloat_AsDouble(result);

  // Decrement the reference count for the temporaries created in this function
  Py_DECREF(result);
  Py_DECREF(args);
}

#endif // PYTHONCONTROLBASE_H
