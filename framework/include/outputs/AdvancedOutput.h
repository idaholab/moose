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
#include "AdvancedOutputUtils.h" // OutputDataWarehouse
#include "MooseTypes.h"
#include "UserObject.h"
#include "ExecuteMooseObjectWarehouse.h"
#include "FileOutput.h"

// Forward declarations
class OutputWarehouse;
class FileOutput;
class PetscOutput;
class Console;
class TransientMultiApp;

/**
 * Based class for output objects
 *
 * Each output class (e.g., Exodus) should inherit from this base class. At a minimum, the pure
 * virtual methods for the various types of output must be defined in the child class.
 *
 * @see Exodus Console CSV
 */
class AdvancedOutput : public FileOutput
{
public:
  static InputParameters validParams();

  /**
   * Class constructor
   *
   * The constructor performs all of the necessary initialization of the various
   * output lists required for the various output types.
   * @param parameters The InputParameters for the object
   */
  AdvancedOutput(const InputParameters & parameters);

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
  bool hasOutput(const ExecFlagType & type);

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
  const std::set<std::string> & getNodalVariableOutput();

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
  const std::set<std::string> & getElementalVariableOutput();

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
  const std::set<std::string> & getScalarOutput();

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
  const std::set<std::string> & getPostprocessorOutput();

  /**
   * Returns true if there exists VectorPostprocessors for output
   * @return True if VectorPostprocessor output exists
   * @see getVectorPostprocessorOutput
   */
  bool hasVectorPostprocessorOutput();

  /**
   * The list of VectorPostprocessor names that are set for output
   * @return A vector of strings containing the names of the VectorPostprocessor variables for
   * output
   * @see hasVectorPostprocessorOutput
   */
  const std::set<std::string> & getVectorPostprocessorOutput();

  /**
   * Returns true if there exists Reporter for output
   * @return True if Reporter output exists
   * @see getReporterOutput
   */
  bool hasReporterOutput();

  /**
   * The list of Reporter names that are set for output
   * @return A vector  containing the names of the Reporter names for output
   * @see hasReporterOutput
   */
  const std::set<std::string> & getReporterOutput();

  /**
   * A method for enabling individual output type control
   * @param names (optional) Space separated of output type names that are supported by this Output
   * object,
   *              if this is omitted all outputs types will be supported. The list of available
   * output
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

  /**
   * Get the current advanced 'execute_on' selections for display
   */
  const OutputOnWarehouse & advancedExecuteOn() const;

protected:
  /// Call init() method on setup
  virtual void initialSetup();

  /**
   * Populates the various data structures needed to control the output
   */
  virtual void init();

  /**
   * Handles logic for determining if a step should be output
   * @return True if a call if output should be performed
   */
  virtual bool shouldOutput();

  /**
   * A single call to this function should output all the necessary data for a single timestep. By
   * default this function performs calls each of the four virtual output methods:
   * outputScalarVariables(),
   * outputPostprocessors(), outputElementalVariables(), and outputNodalVariables(). But, only if
   * output exists
   * for each type.
   *
   * @see outputNodalVariables outputElementalVariables outputScalarVariables outputPostprocessors
   */
  virtual void output();

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
   * By default this method does nothing and is not called, the individual Output objects are
   * responsible for calling it
   */
  virtual void outputInput();

  /**
   * \todo{Make this call automatic in similar fashion to outputInput}
   * Performs the output of system information
   * By default this method does nothing and is not called by output()
   */
  virtual void outputSystemInformation();

  /**
   * Output Reporter values.
   * The child class must define this method to output the Reporter values as desired
   * @see CSV::outputReporters
   */
  virtual void outputReporters();

  /**
   * Flags to control nodal output
   */
  bool _elemental_as_nodal, _scalar_as_nodal;

  /// Storage for Reporter values
  const ReporterData & _reporter_data;

private:
  /**
   * Initializes the available lists for each of the output types
   */
  void initAvailableLists();

  /**
   * Initialize the possible execution types
   * @param name The name of the supplied MultiMoose enum from the _execute_on std::map (e.g.,
   * scalars)
   * @param input The ExecFlagEnum for output type flags to initialize
   */
  void initExecutionTypes(const std::string & name, ExecFlagEnum & input);

  /**
   * Parses the user-supplied input for hiding and showing variables and postprocessors into
   * a list for each type of output
   * @param show The vector of names that are to be output
   * @param hide The vector of names that are to be suppressed from the output
   */
  void initShowHideLists(const std::vector<VariableName> & show,
                         const std::vector<VariableName> & hide);

  /**
   * Helper function for initAvailableLists, templated on warehouse type and postprocessor_type
   * @param execute_data_name Name of the OutputData struct to initialize
   * @param warehouse Reference to the postprocessor or vector postprocessor warehouse
   */
  template <typename postprocessor_type>
  void initPostprocessorOrVectorPostprocessorLists(const std::string & execute_data_name);

  /**
   * Initializes the list of items to be output using the available, show, and hide lists
   * @param data The OutputData to operate on
   */
  void initOutputList(OutputData & data);

  /**
   * Handles logic for determining if a step should be output
   * @return True if a call if output should be performed
   */
  bool wantOutput(const std::string & name, const ExecFlagType & type);

  /**
   * Method for defining the available parameters based on the types of outputs
   * @param params The InputParamters object to add parameters to
   * @param types The types of output this object should support (see Output::enableOutputTypes)
   *
   * Each output object may have a varying set of supported output types (e.g., elemental
   * variables may not be supported). This private, static method populates the InputParameters
   * object with the correct parameters based on the items contained in the ExecFlagEnum.
   *
   * This method is private, users should utilize the Output::enableOutputTypes method
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
  OutputDataWarehouse _execute_data;

  /// Storage for the last output time for the various output types, this is used to avoid duplicate output when using OUTPUT_FINAL flag
  std::map<std::string, Real> _last_execute_time;

  /// Flags for outputting PP/VPP data as a reporter
  const bool _postprocessors_as_reporters, _vectorpostprocessors_as_reporters;

  // Allow complete access
  friend class OutputWarehouse;
  friend class Console;
  friend class TransientMultiApp;
};

// Helper function for initAvailableLists, templated on warehouse type and postprocessor_type
template <typename postprocessor_type>
void
AdvancedOutput::initPostprocessorOrVectorPostprocessorLists(const std::string & execute_data_name)
{
  // Convenience reference to the OutputData being operated on (should used "postprocessors" or
  // "vector_postprocessors")
  OutputData & execute_data = _execute_data[execute_data_name];

  // Build the input file parameter name (i.e. "output_postprocessors_on" or
  // "output_vector_postprocessors_on")
  std::ostringstream oss;
  oss << "execute_" << execute_data_name << "_on";
  std::string execute_on_name = oss.str();

  std::vector<UserObject *> objs;
  _problem_ptr->theWarehouse()
      .query()
      .condition<AttribSystem>("UserObject")
      .condition<AttribThread>(0)
      .queryIntoUnsorted(objs);

  for (const auto & obj : objs)
  {
    auto pps = dynamic_cast<postprocessor_type *>(obj);
    if (!pps)
      continue;

    execute_data.available.insert(pps->PPName());

    // Extract the list of outputs
    const auto & pps_outputs = pps->getOutputs();

    // Check that the outputs lists are valid
    _app.getOutputWarehouse().checkOutputs(pps_outputs);

    // Check that the output object allows postprocessor output,
    // account for "all" keyword (if it is present assume "all" was desired)
    if (pps_outputs.find(name()) != pps_outputs.end() ||
        pps_outputs.find("all") != pps_outputs.end())
    {
      if (!_advanced_execute_on.contains(execute_data_name) ||
          (_advanced_execute_on[execute_data_name].isValid() &&
           _advanced_execute_on[execute_data_name].isValueSet("none")))
      {
        const bool is_pp_type = (execute_data_name == "postprocessors");
        const std::string pp_type_str = is_pp_type ? "post-processor" : "vector post-processor";
        mooseWarning("The ",
                     pp_type_str,
                     " '",
                     pps->PPName(),
                     "' has requested to be output by the '",
                     name(),
                     "' output, but ",
                     pp_type_str,
                     " output is disabled for that output object.");
      }
    }
  }
}
