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

#ifndef OUTPUTWAREHOUSE_H
#define OUTPUTWAREHOUSE_H

// Standard includes
#include <vector>

// MOOSE includes
#include "InputParameters.h"

// Forward declarations
class Output;
class Checkpoint;
class FEProblem;

/**
 * Class for storing and utilizing output objects
 */
class OutputWarehouse
{
public:

  /**
   * Class constructor
   */
  OutputWarehouse();

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
  void addOutput(Output * output);

  /**
   * Get a complete list of all output objects
   * @return A vector of pointers to each of the output objects
   */
  const std::vector<Output *> & getOutputs() const;

  /**
   * Returns true if the output object exists
   * @param name The name of the output object for which to test for existence within the warehouse
   */
  bool hasOutput(const std::string & name) const;

  /**
   * Calls the outputInitial method for each of the output objects
   */
  void outputInitial();

  /**
   * Calls the outputFailedStep method for each output object
   */
  void outputFailedStep();

  /**
   * Calls the outputStep method for each output object
   */
  void outputStep();

  /**
   * Calls the outputFinal method for each output object
   */
  void outputFinal();

  /**
   * Calls the meshChanged method for every output object
   */
  void meshChanged();

  /**
   * Calls the allowOutput method for every output object
   */
  void allowOutput(bool state);

  /**
   * Calls the forceOutput method for every output object
   */
  void forceOutput();

  /**
   * Creates a list of automatically generated material property AuxVariable to hide
   * @param name The name of the output object to compose the list of hidden variables
   * @param hide The vector to populate with the variables to hide
   *
   * The Material system has the ability to automatically generate AuxVariables for
   * material property outputting. This includes the ability to control which output
   * object the variables are written. This method extracts the list of AuxVariables
   * that should be hidden for the supplied output object name.
   *
   * @see Output::initOutputList
   */
  void  buildMaterialOutputHideList(const std::string & name, std::vector<std::string> & hide);

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
  void setCommonParameters(InputParameters * params_ptr);

  /**
   * Get a reference to the common output parameters
   * @return Pointer to the common InputParameters object
   */
  InputParameters * getCommonParameters();

  /**
   * Return the sync times for all objects
   */
  std::set<Real> & getSyncTimes();

  /**
   * Call the init() method for each of the Outputs
   */
  void init();

  /**
   * Test that the output names exist
   * @param A vector of names to check
   * This method will produce an error if any of the supplied
   * names do not exist in the warehouse. Reserved names are not considered.
   */
  void checkOutputs(const std::set<OutputName> & names);

  /**
   * Return an Output object by name
   * @tparam T The Output object type to return
   * @param The name of the output object
   * @return A pointer to the output object
   */
  template<typename T>
  T * getOutput(const OutputName & name);

  /**
   * Return a vector of objects by names
   * @tparam T The Output object type to return
   * @param names A vector of names of the output object
   * @return A pointer to the output object
   */
  template<typename T>
  std::vector<T *> getOutputs(const std::vector<OutputName> & names);

  /**
   * Return a vector of objects of a given type
   * @tparam T The Output object type to return
   * @return A pointer to the output object
   */
  template<typename T>
  std::vector<T *> getOutputs() const;

  /**
   * Return a list of output objects with a given type
   * @tparam T The output object type
   * @return A vector of names
   */
  template<typename T>
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
   * Sends the supplied message to Console output objects
   * @param message A string containing the message to write
   */
  void mooseConsole();

  /**
   * The multiapp level
   * @return A writable reference to the current number of levels from the master app
   */
  unsigned int & multiappLevel() { return _multiapp_level; }

  /**
   * The buffered messages stream for Console objectsc
   * @return Reference to the stream storing cached messages from calls to _console
   */
  std::ostringstream & consoleBuffer() { return _console_buffer; }

private:

  /**
   * Adds the file name to the list of filenames being output
   * The main function of this object is to test that the same output file
   * does not already exist to protect against output files overwriting each other
   * @param ptr Pointer to the Output object
   * @param filename Name of an output file (extracted from filename() method of the objects)
   */
  void addOutputFilename(const OutFileBase & filename);

  /**
   * Calls the initialSetup function for each of the output objects
   * @see FEProblem::initialSetup()
   */
  void initialSetup();

  /**
   * Calls the timestepSetup function for each of the output objects
   * @see FEProblem::timestepSetup()
   */
  void timestepSetup();

  /**
   * Method for populating and updating variables associated with automatic material output
   * @param outputs A vector output object names
   * @param variables A set of variables names to be output for the given output names
   *
   * This is a private function that is called by the friend class MaterialOutputAction, it is not
   * intended for any other purpose than automatic material property output control.
   */
  void updateMaterialOutput(const std::set<OutputName> & outputs, const std::set<AuxVariableName> & variables);

  /**
   * Method for setting the complete list of auto generated material property output AuxVariables
   * @param variables The set of variables to store as the complete list
   *
   * This complete list is compared with the output specific lists by getMaterialOutputHideList to
   * generate this variables that should be hidden for a given output object
   */
  void setMaterialOutputVariables(const std::set<AuxVariableName> & variables);

  /// The list of all output objects
  std::vector<Output *> _object_ptrs;

  /// A map of the output pointers
  std::map<OutputName, Output *> _object_map;

  /// List of object names
  std::set<OutFileBase> _file_base_set;

  /// Pointer to the common InputParameters (@see CommonOutputAction)
  InputParameters * _common_params_ptr;

  /// Sync times for all objects
  std::set<Real> _sync_times;

  /// Input file name for this output object
  std::string _input_file_name;

  /// Map of output name and AuxVariable names to be output (used by auto Material output)
  std::map<OutputName, std::set<AuxVariableName> > _material_output_map;

  /// List of all variable created by auto material output
  std::set<AuxVariableName> _all_material_output_variables;

  /// List of reserved names
  std::set<std::string> _reserved;

  /// Level of multiapp, the master is level 0. This used by the Console to indent output
  unsigned int _multiapp_level;

  /// Stream for holding messages passed to _console prior to Output object construction
  std::ostringstream _console_buffer;

  // Allow complete access to FEProblem for calling initial/timestepSetup functions
  friend class FEProblem;
  friend class MaterialOutputAction;
};

template<typename T>
T *
OutputWarehouse::getOutput(const OutputName & name)
{
  // Check that the object exists
  if (!hasOutput(name))
    mooseError("An output object with the name '" << name << "' does not exist.");

  // Attempt to cast the object to the correct type
  T * output = dynamic_cast<T*>(_object_map[name]);

  // Error if the cast fails
  if (output == NULL)
    mooseError("An output object with the name '" << name << "' for the specified type does not exist");

  // Return the object
  return output;
}

template<typename T>
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

template<typename T>
std::vector<T *>
OutputWarehouse::getOutputs() const
{
  // The vector to output
  std::vector<T *> outputs;

  // Populate the vector
  for (std::map<OutputName, Output *>::const_iterator it = _object_map.begin(); it != _object_map.end(); ++it)
  {
    T * output = dynamic_cast<T*>(it->second);
    if (output != NULL)
      outputs.push_back(output);
  }

  // Return the objects
  return outputs;
}

/**
 * Return a list of output objects with a given type
 * @tparam T The output object type
 * @return A vector of names
 */
template<typename T>
std::vector<OutputName>
OutputWarehouse::getOutputNames()
{
  // The output vector
  std::vector<OutputName> names;

  // Loop through the objects and store the name if the type cast succeeds
  for (std::map<OutputName, Output *>::const_iterator it = _object_map.begin(); it != _object_map.end(); ++it)
  {
    T * output = dynamic_cast<T*>(it->second);
    if (output != NULL)
      names.push_back(it->first);
  }

  // Return the names
  return names;
}

#endif // OUTPUTWAREHOUSE_H
