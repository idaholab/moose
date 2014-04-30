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
  bool hasOutput(std::string name);

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
   * @return Reference to the common InputParameters object
   */
  InputParameters & getCommonParameters();

  /**
   * Return the sync times for all objects
   */
  std::set<Real> & getSyncTimes();

  /**
   * Call the init() method for each of the Outputters
   */
  void init();

  /**
   * Return an Output object by name
   * @tparam T The Output object type to return
   * @param The name of the output object
   * @return A pointer to the output object
   */
  template<typename T>
  T * getOutput(std::string name);

  /**
   * Return a vector of objects by names
   * @tparam T The Output object type to return
   * @param names A vector of names of the output object
   * @return A pointer to the output object
   */
  template<typename T>
  std::vector<T *> getOutputs(std::vector<std::string> names);

  /**
   * Return a vector of objects of a given type
   * @tparam T The Output object type to return
   * @return A pointer to the output object
   */
  template<typename T>
  std::vector<T *> getOutputs();

  /**
   * Return a list of output objects with a given type
   * @tparam T The output object type
   * @return A vector of names
   */
  template<typename T>
  std::vector<std::string> getOutputNames();


private:

  /**
   * Adds the file name to the list of filenames being output
   * The main function of this object is to test that the same output file
   * does not already exist to protect against output files overwriting each other
   * @param filename Name of an output file (extracted from filename() method of the objects)
   */
  void addOutputFilename(OutFileBase filename);

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

  /// The list of all output objects
  std::vector<Output *> _object_ptrs;

  /// A map of the output pointers
  std::map<std::string, Output *> _object_map;

  /// List of object names
  std::set<OutFileBase> _filenames;

  /// Pointer to the common InputParameters (@see CommonOutputAction)
  InputParameters * _common_params_ptr;

  /// True if multiple Console output objects are added to the warehouse
  bool _has_screen_console;

  /// Sync times for all objects
  std::set<Real> _sync_times;

  /// Input file name for this output object
  std::string _input_file_name;

  // Allow complete access to FEProblem for calling initial/timestepSetup functions
  friend class FEProblem;
};

template<typename T>
T *
OutputWarehouse::getOutput(std::string name)
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
OutputWarehouse::getOutputs(std::vector<std::string> names)
{
  // The vector to output
  std::vector<T *> outputs;

  // Populate the vector
  for (std::vector<std::string>::iterator it = names.begin(); it != names.end(); ++it)
    outputs.push_back(getOutput<T>(*it));

  // Return the objects
  return outputs;
}

template<typename T>
std::vector<T *>
OutputWarehouse::getOutputs()
{
  // The vector to output
  std::vector<T *> outputs;

  // Populate the vector
  for (std::map<std::string, Output *>::iterator it = _object_map.begin(); it != _object_map.end(); ++it)
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
std::vector<std::string>
OutputWarehouse::getOutputNames()
{
  // The output vector
  std::vector<std::string> names;

  // Loop through the objects and store the name if the type cast succeeds
  for (std::map<std::string, Output *>::iterator it = _object_map.begin(); it != _object_map.end(); ++it)
  {
    T * output = dynamic_cast<T*>(it->second);
    if (output != NULL)
      names.push_back(it->first);
  }

  // Return the names
  return names;
}

#endif // OUTPUTWAREHOUSE_H
