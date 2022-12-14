//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Output.h"
#include "PerfGraphInterface.h"

// System includes
#include <atomic>

// Forward declarations
class FEProblemBase;
class InputParameters;

/**
 * Class for storing and utilizing output objects
 */
class OutputWarehouse : protected PerfGraphInterface
{
public:
  /**
   * Class constructor
   */
  OutputWarehouse(MooseApp & app);

  /*
   * Class destructor
   * The OutputWarehouse deletes all output objects passed in via addOutput
   */
  virtual ~OutputWarehouse();

  /**
   * Adds an existing output object to the warehouse
   * @param output Pointer to the output object
   * It is the responsibility of the OutputWarehouse to delete the output objects
   * add using this method
   */
  void addOutput(std::shared_ptr<Output> output);

  /**
   * Get a complete set of all output object names
   * @return A set of output names for each output object
   *
   * Note, if this method is called prior to the creation of outputs in AddOutputAction it will
   * create the proxy list of names from the action system. The main use is for the OutputInterface,
   * specifically, when used with Postprocessors in the UserObjects block of the input file.
   * UserObjects are created prior to Outputs objects, but OutputInterface needs the list
   * of output names to operate correctly.
   *
   */
  const std::set<OutputName> & getOutputNames();

  /**
   * Returns true if the output object exists
   * @param name The name of the output object for which to test for existence within the warehouse
   */
  bool hasOutput(const std::string & name) const;

  /**
   * Calls the meshChanged method for every output object
   */
  void meshChanged();

  /**
   * Return the list of hidden variables for the given output name
   * @param output_name The name of the output object for which the variables should be returned
   * @param hide The set of variables to hide which is built by this method
   *
   * Objects inheriting from the OutputInterface have the ability to control the output of variables
   * associated with the objects (i.e., Marker elemental variable). This method returns a list
   * of variables that should be hidden for the supplied object name due to the 'outputs' parameter
   * being set by the object(s).
   *
   * This method is used by Output::initOutputList to populate the correct hide lists for the
   * output object, it is not intended for general use.
   */
  void buildInterfaceHideVariables(const std::string & output_name, std::set<std::string> & hide);

  /**
   * Calls the setFileNumber method for every FileOutput output object
   */
  void setFileNumbers(std::map<std::string, unsigned int> input, unsigned int offset = 0);

  /**
   * Extracts the file numbers from the output objects
   * @return Map of file numbers for the output objects
   */
  std::map<std::string, unsigned int> getFileNumbers();

  /**
   * Stores the common InputParameters object
   * @param params_ptr A pointer to the common parameters object to be stored
   *
   * @see CommonOutputAction
   */
  void setCommonParameters(const InputParameters * params_ptr);

  /**
   * Get a reference to the common output parameters
   * @return Pointer to the common InputParameters object
   */
  const InputParameters * getCommonParameters() const;

  /**
   * Return the sync times for all objects
   */
  std::set<Real> & getSyncTimes();

  /**
   * Test that the output names exist
   * @param names A vector of names to check
   * This method will produce an error if any of the supplied
   * names do not exist in the warehouse. Reserved names are not considered.
   */
  void checkOutputs(const std::set<OutputName> & names);

  /**
   * Return an Output object by name
   * @tparam T The Out put object type to return
   * @param name The name of the output object
   * @return A pointer to the output object
   */
  template <typename T>
  T * getOutput(const OutputName & name);

  /**
   * Return a vector of objects by names
   * @tparam T The Output object type to return
   * @param names A vector of names of the output object
   * @return A pointer to the output object
   */
  template <typename T>
  std::vector<T *> getOutputs(const std::vector<OutputName> & names);

  /**
   * Return a vector of objects of a given type
   * @tparam T The Output object type to return
   * @return A pointer to the output object
   */
  template <typename T>
  std::vector<T *> getOutputs() const;

  /**
   * Return a list of output objects with a given type
   * @tparam T The output object type
   * @return A vector of names
   */
  template <typename T>
  std::vector<OutputName> getOutputNames();

  /**
   * Return a set of reserved output names
   * @return A std::set of reserved names
   */
  const std::set<std::string> & getReservedNames() const;

  /**
   * Test if the given name is reserved
   * @param name The name to test
   * @return True if the name is reserved
   */
  bool isReservedName(const std::string & name);

  /**
   * Send current output buffer to Console output objects
   */
  void mooseConsole();

  /**
   * Send a buffer to Console output objects
   */
  void mooseConsole(std::ostringstream & buffer);

  /**
   * The buffered messages stream for Console objects
   * @return Reference to the stream storing cached messages from calls to _console
   */
  std::ostringstream & consoleBuffer() { return _console_buffer; }

  /**
   * Set if the outputs to Console before its construction are to be buffered or to screen directly
   * @param buffer Ture to buffer
   */
  void bufferConsoleOutputsBeforeConstruction(bool buffer)
  {
    _buffer_action_console_outputs = buffer;
  }

  /// Reset the output system
  void reset();

  /**
   * Calls the timestepSetup function for each of the output objects
   * @see FEProblemBase::solve()
   * timestepSetup is too early for solver setup where we try to hook up monitor,
   * change prefix, etc. SLEPc solver does not exist yet when we call timestepSetup.
   * Moved this method from the private region to the public region so we can call
   * this function from EigenProblem with no need to make EigenProblem as friend
   * of OutputWarehouse.
   */
  void solveSetup();

  /// The number of times something has been printed
  unsigned long long int numPrinted() const { return _num_printed; }

private:
  /**
   * Calls the outputStep method for each output object
   * @param type The type execution flag (see Moose.h)
   *
   * This is private, users should utilize FEProblemBase::outputStep()
   */
  void outputStep(ExecFlagType type);

  ///@{
  /**
   * Ability to enable/disable output calls
   * This is private, users should utilize FEProblemBase::allowOutput()
   * @see FEProblemBase::allowOutput()
   */
  void allowOutput(bool state);
  template <typename T>
  void allowOutput(bool state);
  ///@}

  /**
   * Indicates that the next call to outputStep should be forced
   * This is private, users should utilize FEProblemBase::forceOutput()
   * @see FEProblemBase::forceOutput()
   */
  void forceOutput();

  /**
   * We are using std::shared_ptr to handle the cleanup of the pointers at the end of execution.
   * This is necessary since several warehouses might be sharing a single instance of a MooseObject.
   */
  std::vector<std::shared_ptr<Output>> _all_ptrs;

  /**
   * Adds the file name to the map of filenames being output with an associated object
   * The main function of this object is to test that the same output file
   * does not already exist in another object to protect against output files overwriting each other
   *
   * @param obj_name Name of an FileOutput object
   * @param filename Name of an output file (extracted from filename() method of the objects)
   */
  void addOutputFilename(const OutputName & obj_name, const OutFileBase & filename);

  /**
   * Calls the initialSetup function for each of the output objects
   * @see FEProblemBase::initialSetup()
   */
  void initialSetup();

  /**
   * Calls the timestepSetup function for each of the output objects
   * @see FEProblemBase::timestepSetup()
   */
  void timestepSetup();

  /**
   * Calls the setup function for each of the output objects
   * @see FEProblemBase::customSetup(const ExecFlagType & exec_type)
   */
  void customSetup(const ExecFlagType & exec_type);

  /**
   * Calls the jacobianSetup function for each of the output objects
   * @see FEProblemBase::computeJacobian
   */
  void jacobianSetup();

  /**
   * Calls the residualSetup function for each of the output objects
   * @see FEProblemBase::computeResidualTyp
   */
  void residualSetup();

  /**
   * Calls the subdomainSetup function for each of the output objects
   * @see FEProblemBase::setupSubdomain
   */
  void subdomainSetup();

  /**
   * Insert variable names for hiding via the OutoutInterface
   * @param output_name The name of the output object on which the variable is to be hidden
   * @param variable_names The names of the variables to be hidden
   *
   * This is a private method used by the OutputInterface system, it is not intended for any
   * other purpose.
   */
  void addInterfaceHideVariables(const std::string & output_name,
                                 const std::set<std::string> & variable_names);

  /**
   * Sets the execution flag type
   *
   * This is a private method used by FEProblemBase, it is not intended for any other purpose
   */
  void setOutputExecutionType(ExecFlagType type);

  /**
   * If content exists in the buffer, write it.
   * This is used by Console to make sure PETSc related output does not dump
   * before buffered content. It is private because people shouldn't be messing with it.
   */
  void flushConsoleBuffer();

  /**
   * Resets the file base for all FileOutput objects
   */
  void resetFileBase();

  /// MooseApp
  MooseApp & _app;

  /// All instances of objects (raw pointers)
  std::vector<Output *> _all_objects;

  /// True to buffer console outputs in actions
  bool _buffer_action_console_outputs;

  /// A map of the output pointers
  std::map<OutputName, Output *> _object_map;

  /// A set of output names
  std::set<OutputName> _object_names;

  /// List of object names
  std::map<OutputName, std::set<OutFileBase>> _file_base_map;

  /// Pointer to the common InputParameters (@see CommonOutputAction)
  const InputParameters * _common_params_ptr;

  /// Sync times for all objects
  std::set<Real> _sync_times;

  /// Input file name for this output object
  std::string _input_file_name;

  /// Map of output name and AuxVariable names to be output (used by auto Material output)
  std::map<OutputName, std::set<AuxVariableName>> _material_output_map;

  /// List of all variable created by auto material output
  std::set<AuxVariableName> _all_material_output_variables;

  /// List of reserved names
  std::set<std::string> _reserved;

  /// The stream for holding messages passed to _console prior to Output object construction
  std::ostringstream _console_buffer;

  /// Storage for variables to hide as prescribed by the object via the OutputInterface
  std::map<std::string, std::set<std::string>> _interface_map;

  /// The current output execution flag
  ExecFlagType _output_exec_flag;

  /// Flag indicating that next call to outputStep is forced
  bool _force_output;

  /// Whether or not the last thing output by mooseConsole had a newline as the last character
  bool _last_message_ended_in_newline;

  /// What the last buffer was that was printed
  const std::ostringstream * _last_buffer;

  /// Number of times the stream has been printed to
  std::atomic<unsigned long long int> _num_printed;

  // Allow complete access:
  // FEProblemBase for calling initial, timestepSetup, outputStep, etc. methods
  friend class FEProblemBase;

  // MaterialOutputAction for calling addInterfaceHideVariables
  friend class MaterialOutputAction;

  // OutputInterface for calling addInterfaceHideVariables
  friend class OutputInterface;

  // Console for calling flushConsoleBuffer()
  friend class PetscOutput;

  // MooseApp for resetFileBase()
  friend class MooseApp;
};

template <typename T>
T *
OutputWarehouse::getOutput(const OutputName & name)
{
  // Check that the object exists
  if (!hasOutput(name))
    mooseError("An output object with the name '", name, "' does not exist.");

  // Attempt to cast the object to the correct type
  T * output = dynamic_cast<T *>(_object_map[name]);

  // Error if the cast fails
  if (output == NULL)
    mooseError("An output object with the name '", name, "' for the specified type does not exist");

  // Return the object
  return output;
}

template <typename T>
std::vector<T *>
OutputWarehouse::getOutputs(const std::vector<OutputName> & names)
{
  // The vector to output
  std::vector<T *> outputs;

  // Populate the vector
  for (std::vector<OutputName>::const_iterator it = names.begin(); it != names.end(); ++it)
    outputs.push_back(getOutput<T>(*it));

  // Return the objects
  return outputs;
}

template <typename T>
std::vector<T *>
OutputWarehouse::getOutputs() const
{
  // The vector to output
  std::vector<T *> outputs;

  // Populate the vector
  for (std::map<OutputName, Output *>::const_iterator it = _object_map.begin();
       it != _object_map.end();
       ++it)
  {
    T * output = dynamic_cast<T *>(it->second);
    if (output != NULL)
      outputs.push_back(output);
  }

  // Return the objects
  return outputs;
}

template <typename T>
std::vector<OutputName>
OutputWarehouse::getOutputNames()
{
  // The output vector
  std::vector<OutputName> names;

  // Loop through the objects and store the name if the type cast succeeds
  for (std::map<OutputName, Output *>::const_iterator it = _object_map.begin();
       it != _object_map.end();
       ++it)
  {
    T * output = dynamic_cast<T *>(it->second);
    if (output != NULL)
      names.push_back(it->first);
  }

  // Return the names
  return names;
}

template <typename T>
void
OutputWarehouse::allowOutput(bool state)
{
  std::vector<T *> outputs = getOutputs<T>();
  for (typename std::vector<T *>::iterator it = outputs.begin(); it != outputs.end(); ++it)
    (*it)->allowOutput(state);
}
