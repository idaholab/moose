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

#ifndef ADVANCEDOUTPUT_H
#define ADVANCEDOUTPUT_H

// MOOSE includes
#include "OversampleOutput.h"
#include "MooseObject.h"
#include "Restartable.h"
#include "MooseTypes.h"
#include "MooseMesh.h"
#include "MeshChangedInterface.h"
#include "MooseApp.h"

// libMesh
#include "libmesh/equation_systems.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/mesh_function.h"

/**
 * A structure for storing the various lists that contain
 * the names of the items to be exported. An instance of this
 * struct exists for each of the output types (non-linear variables,
 * scalar variables, postprocessors, etc.)
 *
 * @see OutputDataWarehouse
 */
struct OutputData
{
  /// A list of all possible outputs
  std::vector<std::string> available;

  /// User-supplied list of outputs to display
  std::vector<std::string> show;

  /// User-supplied list of outputs to hide
  std::vector<std::string> hide;

  /// A list of the outputs to write
  std::vector<std::string> output;
};


/**
 * In newer versions of Clang calling operator[] on a map with a component that
 * has a default constructor is an error, thus utilizing a map directly to store
 * a MultiMooseEnum is not possible.
 *
 * This template class is a map wrapper that provides the basic map-like functionality
 * for accessing map types with operator[] by using find internally. It also produces
 * an error if the map key does not exists, this it provides a layer of protection not
 * available to maps operator[] in general.
 *
 * This class is used here to create two different warehouse containers below.
 *
 * @see OutputOnWarehouse OutputDataWarehouse
 */
template<typename T>
class OutputMapWrapper
{
public:
  /**
   * Constructor
   */
  OutputMapWrapper(){};

  /**
   * An map assessor that errors if the key is not found
   */
  T & operator[](const std::string & name)
  {
    // Locate the map entry, error if it is not found
    typename std::map<std::string, T>::iterator iter = _map.find(name);
    if (iter == _map.end())
      mooseError("Unknown map key " << name);
    return iter->second;
  }

  ///@{
  /**
   * Provide iterator and find access to the underlying map data
   */
  typename std::map<std::string, T>::iterator begin() { return _map.begin(); }
  typename std::map<std::string, T>::iterator end() { return _map.end(); }
  typename std::map<std::string, T>::iterator find(const std::string & name) { return _map.find(name); }
  ///@}

  /**
   * A method for testing of a key exists
   */
  bool contains(const std::string & name) { return find(name) != end(); }


protected:

  /// Data storage
  typename std::map<std::string, T> _map;
};


/**
 * A helper warehouse class for storing the "output_on" settings for
 * the various output types.
 *
 * In order to allow for new output types to be defined and to minimize
 * the number of member variables the "output_on" parameter for each of
 * the output types (e.g., output_postprocessors_on) are stored in a map.
 *
 * This allows for iterative access to these parameters, which makes creating
 * generic code (e.g., AdvancedOutput::shouldOutput) possible. However, MultiMooseEnum
 * has a private constructor, so calling operator[] on the map is a compile time error.
 *
 * To get around this and to provide a more robust storage structure, one that will error
 * if the wrong output name is given, this warehouse was created. For the purposes of the
 * AdvancedOutput object this warehouse functions exactly like a std::map, but provides
 * an operator[] that works with MultiMooseEnum and errors if called on an invalid key.
 *
 * @see OutputMapWrapper OutputDataWarehouse
 */
class OutputOnWarehouse : public OutputMapWrapper<MultiMooseEnum>
{
public:

  /**
   * Constructor
   * @param output_on The general "output_on" settings for the object
   */
  OutputOnWarehouse(const MultiMooseEnum & output_on, const InputParameters & params);
};

/**
 * A helper warehouse for storing OutputData objects for the various output types
 *
 * To keep syntax consistent and to provide the error checking for accessing invalid map keys
 * the OutputMapWrapper is used for accessing the OutputData classes as well.
 *
 * @see OutputOnWarehouse OutputMapWrapper OutputData
 */
class OutputDataWarehouse : public OutputMapWrapper<OutputData>
{
public:

  /**
   * Populate the OutputData structures for all output types that are 'variable' based
   */
  OutputDataWarehouse();
};


/**
 * Based class for output objects
 *
 * Each output class (e.g., Exodus) should inherit from this base class. At a minimum, the pure
 * virtual methods for the various types of output must be defined in the child class.
 *
 * @see Exodus Console CSV
 */
template<class OutputBase>
class AdvancedOutput : public OutputBase
{
public:

  /**
   * Class constructor
   *
   * The constructor performs all of the necessary initialization of the various
   * output lists required for the various output types.
   * @param name The name of the output object
   * @param parameters The InputParameters for the object
   */
  AdvancedOutput(const std::string & name, InputParameters & parameters);

  /**
   * Class destructor
   */
  virtual ~AdvancedOutput();

  /**
   * Returns true if any of the other has methods return true.
   */
  virtual bool hasOutput();

  /**
   * Tests that any output exist for the given output type
   *
   * @see Exodus
   */
  bool hasOutput(const OutputExecFlagType & type);

  /**
   * Returns true if there exists nodal nonlinear variables for output
   * @return True if nonlinear variable output exists
   * @see getNodalVariableOutput
   */
  bool hasNodalVariableOutput();

  /**
   * The list of nodal nonlinear variables names that are set for output
   * @return A vector of strings containing the names of the nodal nonlinear variables for output
   *
   * Note: The list returned by this will contain the names of both elemental and nodal variable
   * names if 'elemental_as_nodal = true' in the input file. The libMesh output system
   * (EquationSystems::build_solution_vector) performs the correct action within the solution vector
   * by setting the nodal values as the average of the values for each of the element that the node
   * shares.
   *
   * @see hasNodalVariableOutput
   */
  const std::vector<std::string> & getNodalVariableOutput();

  /**
   * Returns true if there exists elemental nonlinear variables for output
   * @return True if nonlinear variable output exists
   * @see getElementalVariableOutput
   */
  bool hasElementalVariableOutput();

  /**
   * The list of elemental nonlinear variables names that are set for output
   * @return A vector of strings containing the names of the nonlinear variables for output
   * @see hasElementalVariableOutput
   */
  const std::vector<std::string> & getElementalVariableOutput();

  /**
   * Returns true if there exists scalar variables for output
   * @return True if scalar variable output exists
   * @see getScalarVariableOutput
   */
  bool hasScalarOutput();

  /**
   * The list of scalar variables names that are set for output
   * @return A vector of strings containing the names of the scalar variables for output
   * @see hasScalarVariableOutput
   */
  const std::vector<std::string> & getScalarOutput();

  /**
   * Returns true if there exists postprocessors for output
   * @return True if postprocessor output exists
   * @see getPostprocessorOutput
   */
  bool hasPostprocessorOutput();

  /**
   * The list of postprocessor names that are set for output
   * @return A vector of strings containing the names of the postprocessor variables for output
   * @see hasPostprocessorOutput
   */
  const std::vector<std::string> & getPostprocessorOutput();

  /**
   * Returns true if there exists VectorPostprocessors for output
   * @return True if VectorPostprocessor output exists
   * @see getVectorPostprocessorOutput
   */
  bool hasVectorPostprocessorOutput();

  /**
   * The list of VectorPostprocessor names that are set for output
   * @return A vector of strings containing the names of the VectorPostprocessor variables for output
   * @see hasVectorPostprocessorOutput
   */
  const std::vector<std::string> & getVectorPostprocessorOutput();

  /**
   * A method for enabling individual output type control
   * @param names (optional) Space separated of output type names that are supported by this Output object,
   *              if this is omitted all outputs types will be supported. The list of available output
   *              types is given below.
   *
   * Output objects vary widely in what type of outputs they support (e.g., elemental variables,
   * or postprocessor data). This method provides the user a means for controlling the types of
   * outputs that are supported for the object being created. This is a static method that MUST
   * be used to append parameters inside the objects validParams function.
   *
   * List of Output Types and Method Names
   * The output system is designed around overloading virtual method calls to output the
   * various output types, the following list gives the name of the output type and the associated
   * virtual method that should be overloaded to perform the output in the object being created.
   *
   * Type                 virtual Method Name
   * -------------------- ----------------------------
   * nodal                outputNodalVariables()
   * elemental            outputElementalVariables()
   * scalar               outputScalarVariables()
   * postprocessor        outputPostprocessors()
   * vector_postprocessor outputVectorPostprocessors()
   * input                outputInput()
   * system_information   outputSystemInformation()
   *
   * @see CSV Exodus
   */
  static InputParameters enableOutputTypes(const std::string & names = std::string());


protected:

  /**
   * Calls the output() method if output should occur
   * @param type The type execution flag (see Moose.h)
   */
  void outputStep(const OutputExecFlagType & type);

  /**
   * A single call to this function should output all the necessary data for a single timestep. By
   * default this function performs calls each of the four virtual output methods: outputScalarVariables(),
   * outputPostprocessors(), outputElementalVariables(), and outputNodalVariables(). But, only if output exists
   * for each type.
   *
   * @see outputNodalVariables outputElementalVariables outputScalarVariables outputPostprocessors
   */
  virtual void output(const OutputExecFlagType & type);

  /**
   * Performs output of nodal nonlinear variables
   * The child class must define this method to output the nonlinear variables as desired
   * @see Exodus::outputNodalVariables
   */
  virtual void outputNodalVariables();

  /**
   * Performs output of elemental nonlinear variables
   * The child class must define this method to output the nonlinear variables as desired
   * @see Exodus::outputElementalVariables
   */
  virtual void outputElementalVariables();

  /**
   * Performs output of scalar variables
   * The child class must define this method to output the scalar variables as desired
   * @see Exodus::outputScalarVariables
   */
  virtual void outputScalarVariables();

  /**
   * Performs output of postprocessors
   * The child class must define this method to output the postprocessors as desired
   * @see Exodus::outputPostprocessors
   */
  virtual void outputPostprocessors();

  /**
   * Performs output of VectorPostprocessors
   * The child class must define this method to output the VectorPostprocessors as desired
   */
  virtual void outputVectorPostprocessors();

  /**
   * Performs the output of the input file
   * By default this method does nothing and is not called, the individual Output objects are responsible for calling it
   */
  virtual void outputInput();

  /**
   * \todo{Make this call automatic in similar fashion to outputInput}
   * Performs the output of system information
   * By default this method does nothing and is not called by output()
   */
  virtual void outputSystemInformation();

private:

  /**
   * Initialization method.
   * This populates the various data structures needed to control the output
   */
  virtual void init();

  /**
   * Initializes the available lists for each of the output types
   */
  void initAvailableLists();

  /**
   * Initialize the possible execution types
   * @param name The name of the supplied MultiMoose enum from the _output_on std::map (e.g., scalars)
   * @param input The MultiMooseEnum for output type flags to initialize
   */
  void initExecutionTypes(const std::string & name, MultiMooseEnum & input);

  /**
   * Parses the user-supplied input for hiding and showing variables and postprocessors into
   * a list for each type of output
   * @param show The vector of names that are to be output
   * @param hide The vector of names that are to be suppressed from the output
   */
  void initShowHideLists(const std::vector<VariableName> & show, const std::vector<VariableName> & hide);

  /**
   * Helper function for initAvailableLists, templated on warehouse type and postprocessor_type
   * @param output_data Reference to OutputData struct to initialize
   * @param warehouse Reference to the postprocessor or vector postprocessor warehouse
   */
  template <typename warehouse_type, typename postprocessor_type>
  bool
  initPostprocessorOrVectorPostprocessorLists(OutputData & output_data, warehouse_type & warehouse);

  /**
   * Initializes the list of items to be output using the available, show, and hide lists
   * @param data The OutputData to operate on
   */
  void initOutputList(OutputData & data);

  /**
   * Handles logic for determining if a step should be output
   * @return True if a call if output should be preformed
   */
  bool shouldOutput(const std::string & name, const OutputExecFlagType & type);

  /**
   * Method for defining the available parameters based on the types of outputs
   * @param params The InputParamters object to add parameters to
   * @param types The types of output this object should support (see Output::enableOutputTypes)
   *
   * Each output object may have a varying set of supported output types (e.g., elemental
   * variables may not be supported). This private, static method populates the InputParameters
   * object with the correct parameters based on the items contained in the MultiMooseEnum.
   *
   * This method is private, users should utlize the Output::enableOutputTypes method
   *
   * @see Output::enableOutputTypes
   */
  static void addValidParams(InputParameters & params, const MultiMooseEnum & types);

  /**
   * Helper method for checking if output types exists
   * @param name The name of the output type to test (e.g., postprocessors)
   */
  bool hasOutputHelper(const std::string & name);

  /**
   * Get the supported types of output (e.g., postprocessors, etc.)
   */
  static MultiMooseEnum getOutputTypes();

  /// Storage structures for the various output types
  OutputDataWarehouse _output_data;

  /// Storage for the individual component execute flags
  OutputOnWarehouse _advanced_output_on;

  /// Storage for the last output time for the various output types, this is used to avoid duplicate output when using OUTPUT_FINAL flag
  std::map<std::string, Real> _last_output_time;

  // Allow complete access
  friend class OutputWarehouse;
  friend class FileOutput;
  friend class OversampleOutput;
  friend class PetscOutput;
  friend class Console;
  friend class TransientMultiApp;
};

// Helper function for initAvailableLists, templated on warehouse type and postprocessor_type
template <class OutputBase>
template <typename warehouse_type, typename postprocessor_type>
bool
AdvancedOutput<OutputBase>::initPostprocessorOrVectorPostprocessorLists(OutputData & output_data, warehouse_type & warehouse)
{
  // Return value
  bool has_limited_pps = false;

  // Loop through each of the execution flags
  for (unsigned int i = 0; i < Moose::exec_types.size(); i++)
  {
    // Loop through each of the postprocessors
    for (typename std::vector<postprocessor_type *>::const_iterator postprocessor_it = warehouse(Moose::exec_types[i])[0].all().begin();
         postprocessor_it != warehouse(Moose::exec_types[i])[0].all().end();
         ++postprocessor_it)
    {
      // Store the name in the available postprocessors
      postprocessor_type *pps = *postprocessor_it;
      output_data.available.push_back(pps->PPName());

      // Extract the list of outputs
      std::set<OutputName> pps_outputs = pps->getOutputs();

      // Check that the outputs lists are valid
      OutputBase::_app.getOutputWarehouse().checkOutputs(pps_outputs);

      // Check that the output object allows postprocessor output,
      // account for "all" keyword (if it is present assume "all" was desired)
      if ( pps_outputs.find(OutputBase::_name) != pps_outputs.end() || pps_outputs.find("all") != pps_outputs.end() )
      {
        if (!hasPostprocessorOutput())
          mooseWarning("Postprocessor '" << pps->PPName()
                       << "' has requested to be output by the '" << OutputBase::_name
                       << "' output, but postprocessor output is not support by this type of output object.");
      }

      // Set the flag state for postprocessors that utilize 'outputs' parameter
      if (!pps_outputs.empty() && pps_outputs.find("all") == pps_outputs.end())
        has_limited_pps = true;
    }
  }

  return has_limited_pps;
}

#endif /* ADVANCEDOUTPUT_H */
