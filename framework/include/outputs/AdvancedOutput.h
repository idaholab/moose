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
#include "AdvancedOutputUtils.h" // OutputDataWarehouse
#include "MooseTypes.h"
#include "UserObject.h"
#include "ExecuteMooseObjectWarehouse.h"

// Forward declarations
class OutputWarehouse;
class FileOutput;
class OversampleOutput;
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
template <class T>
class AdvancedOutput : public T
{
public:
  // A typedef
  typedef const T OutputBase;

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
  /**
   * Initialization method.
   * This populates the various data structures needed to control the output
   */
  virtual void initialSetup();

  /**
   * Calls the output() method if output should occur
   * @param type The type execution flag (see Moose.h)
   */
  void outputStep(const ExecFlagType & type);

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
  virtual void output(const ExecFlagType & type);

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

private:
  /**
   * Initializes the available lists for each of the output types
   */
  void initAvailableLists();

  /**
   * Initialize the possible execution types
   * @param name The name of the supplied MultiMoose enum from the _execute_on std::map (e.g.,
   * scalars)
   * @param input The MultiMooseEnum for output type flags to initialize
   */
  void initExecutionTypes(const std::string & name, MultiMooseEnum & input);

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
   * @return True if a call if output should be preformed
   */
  bool shouldOutput(const std::string & name, const ExecFlagType & type);

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
  OutputDataWarehouse _execute_data;

  /// Storage for the last output time for the various output types, this is used to avoid duplicate output when using OUTPUT_FINAL flag
  std::map<std::string, Real> _last_execute_time;

  // Allow complete access
  friend class OutputWarehouse;
  friend class FileOutput;
  friend class OversampleOutput;
  friend class PetscOutput;
  friend class Console;
  friend class TransientMultiApp;
};

// Helper function for initAvailableLists, templated on warehouse type and postprocessor_type
template <class T>
template <typename postprocessor_type>
void
AdvancedOutput<T>::initPostprocessorOrVectorPostprocessorLists(
    const std::string & execute_data_name)
{

  // Get the UserObjectWarhouse
  const ExecuteMooseObjectWarehouse<UserObject> & warehouse = T::_problem_ptr->getUserObjects();

  // Convenience reference to the OutputData being operated on (should used "postprocessors" or
  // "vector_postprocessors")
  OutputData & execute_data = _execute_data[execute_data_name];

  // Build the input file parameter name (i.e. "output_postprocessors_on" or
  // "output_vector_postprocessors_on")
  std::ostringstream oss;
  oss << "execute_" << execute_data_name << "_on";
  std::string execute_on_name = oss.str();

  // True if the postprocessors has been limited using 'outputs' parameter
  bool has_limited_pps = false;

  // Loop through each of the execution flags
  const auto & objects = warehouse.getActiveObjects();
  for (const auto & object : objects)
  {
    // Store the name in the available postprocessors, if it does not already exist in the list
    std::shared_ptr<postprocessor_type> pps = std::dynamic_pointer_cast<postprocessor_type>(object);
    if (!pps)
      continue;

    execute_data.available.insert(pps->PPName());

    // Extract the list of outputs
    const auto & pps_outputs = pps->getOutputs();

    // Check that the outputs lists are valid
    T::_app.getOutputWarehouse().checkOutputs(pps_outputs);

    // Check that the output object allows postprocessor output,
    // account for "all" keyword (if it is present assume "all" was desired)
    if (pps_outputs.find(T::name()) != pps_outputs.end() ||
        pps_outputs.find("all") != pps_outputs.end())
    {
      if (!T::_advanced_execute_on.contains(execute_data_name) ||
          (T::_advanced_execute_on[execute_data_name].isValid() &&
           T::_advanced_execute_on[execute_data_name].contains("none")))
        mooseWarning(
            "Postprocessor '",
            pps->PPName(),
            "' has requested to be output by the '",
            T::name(),
            "' output, but postprocessor output is not support by this type of output object.");
    }

    // Set the flag state for postprocessors that utilize 'outputs' parameter
    if (!pps_outputs.empty() && pps_outputs.find("all") == pps_outputs.end())
      has_limited_pps = true;
  }

  // Produce the warning when 'outputs' is used, but postprocessor output is disabled
  if (has_limited_pps && T::isParamValid(execute_on_name))
  {
    const MultiMooseEnum & pp_on = T::template getParam<MultiMooseEnum>(execute_on_name);
    if (pp_on.contains("none"))
    {
      if (execute_on_name == "execute_postprocessors_on")
        mooseWarning("A Postprocessor utilizes the 'outputs' parameter; however, postprocessor "
                     "output is disabled for the '",
                     T::name(),
                     "' output object.");
      else if (execute_on_name == "execute_vectorpostprocessors_on")
        mooseWarning("A VectorPostprocessor utilizes the 'outputs' parameter; however, vector "
                     "postprocessor output is disabled for the '",
                     T::name(),
                     "' output object.");
    }
  }
}

#endif /* ADVANCEDOUTPUT_H */
