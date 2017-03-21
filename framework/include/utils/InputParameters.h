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

#ifndef INPUTPARAMETERS_H
#define INPUTPARAMETERS_H

// MOOSE includes
#include "MooseError.h"
#include "MooseTypes.h"
#include "MooseEnum.h"
#include "MultiMooseEnum.h"
#include "MooseUtils.h"

// libMesh includes
#include "libmesh/parameters.h"
#include "libmesh/parsed_function.h"

#ifdef LIBMESH_HAVE_FPARSER
#include "libmesh/fparser.hh"
#else
template <typename T>
class FunctionParserBase
{
};
#endif

// Forward declarations
class MooseObject;
class Action;
class Problem;
class MooseApp;
class InputParameters;

/**
 * This is the templated validParams() function that every
 * MooseObject-derived class is required to specialize.
 */
template <class T>
InputParameters validParams();

/**
 * The main MOOSE class responsible for handling user-defined
 * parameters in almost every MOOSE system.
 */
class InputParameters : public Parameters
{
public:
  InputParameters(const InputParameters & rhs);
  InputParameters(const Parameters & rhs);

  virtual ~InputParameters() = default;

  virtual void clear() override;

  /**
   * This method adds a description of the class that will be displayed
   * in the input file syntax dump
   */
  void addClassDescription(const std::string & doc_string);

  /**
   * Returns the class description
   */
  std::string getClassDescription() const;

  /**
   * Override from libMesh to set user-defined attributes on our parameter
   */
  virtual void set_attributes(const std::string & name, bool inserted_only) override;

  /**
   * Returns a writable reference to the named parameters.  Note: This is not a virtual
   * function! Use caution when comparing to the parent class implementation
   * @param name The name of the parameter to set
   * @param quiet_mode When true the parameter is not removed from the _set_by_add_param list,
   * this is generally not needed.
   *
   * "quite_mode" returns a writable reference to the named parameter, without removing it from the
   * _set_by_add_param list. Using this method of set will make the parameter to continue to
   * behave if its value where set ONLY by addParam and not by any other method.
   *
   * This was added for handling parameters in the Output objects that have behavior dependent
   * on whether the user modified the parameters.
   *
   */
  template <typename T>
  T & set(const std::string & name, bool quiet_mode = false);

  /**
   * Runs a range on the supplied parameter if it exists and throws an error if that check fails.
   * @returns Boolean indicating whether range check exists
   */
  template <typename T, typename UP_T>
  void rangeCheck(const std::string & full_name,
                  const std::string & short_name,
                  InputParameters::Parameter<T> * param,
                  std::ostream & oss = Moose::out);
  template <typename T, typename UP_T>
  void rangeCheck(const std::string & full_name,
                  const std::string & short_name,
                  InputParameters::Parameter<std::vector<T>> * param,
                  std::ostream & oss = Moose::out);

  /**
   * Verifies that the requested parameter exists and is not NULL and returns it to the caller.
   * The template parameter must be a pointer or an error will be thrown.
   */
  template <typename T>
  T getCheckedPointerParam(const std::string & name, const std::string & error_string = "") const;

  /**
   * This method adds a parameter and documentation string to the InputParameters
   * object that will be extracted from the input file.  If the parameter is
   * missing in the input file, and error will be thrown
   */
  template <typename T>
  void addRequiredParam(const std::string & name, const std::string & doc_string);

  /**
   * This version of addRequiredParam is here for a consistent use with MooseEnums.  Use of
   * this function for any other type will throw an error.
   */
  template <typename T>
  void
  addRequiredParam(const std::string & name, const T & moose_enum, const std::string & doc_string);

  /**
   * These methods add an option parameter and a documentation string to the InputParameters object.
   * The first version of this function takes a default value which is used if the parameter
   * is not found in the input file.  The second method will leave the parameter uninitialized
   * but can be checked with "isParamValid" before use
   */
  template <typename T, typename S>
  void addParam(const std::string & name, const S & value, const std::string & doc_string);
  template <typename T>
  void addParam(const std::string & name, const std::string & doc_string);

  /**
   * These methods add an range checked parameters. A lower and upper bound can be supplied and the
   * supplied parameter will be checked to fall within that range.
   */
  template <typename T>
  void addRequiredRangeCheckedParam(const std::string & name,
                                    const std::string & parsed_function,
                                    const std::string & doc_string);
  template <typename T>
  void addRangeCheckedParam(const std::string & name,
                            const T & value,
                            const std::string & parsed_function,
                            const std::string & doc_string);
  template <typename T>
  void addRangeCheckedParam(const std::string & name,
                            const std::string & parsed_function,
                            const std::string & doc_string);

  /**
   * These methods add an option parameter and with a customer type to the InputParameters object.
   * The custom
   * type will be output in YAML dumps and can be used within the GUI application.
   */
  template <typename T>
  void addRequiredCustomTypeParam(const std::string & name,
                                  const std::string & custom_type,
                                  const std::string & doc_string);
  template <typename T>
  void addCustomTypeParam(const std::string & name,
                          const T & value,
                          const std::string & custom_type,
                          const std::string & doc_string);
  template <typename T>
  void addCustomTypeParam(const std::string & name,
                          const std::string & custom_type,
                          const std::string & doc_string);

  /**
   * These method add a parameter to the InputParameters object which can be retrieved
   * like any other parameter.  This parameter however is not printed in the Input file syntax
   * dump or web page dump so does not take a documentation string.  The first version
   * of this function takes an optional default value.
   */
  template <typename T>
  void addPrivateParam(const std::string & name, const T & value);
  template <typename T>
  void addPrivateParam(const std::string & name);

  /**
   * Add parameters for retrieval from the command line.
   *
   * NOTE: This ONLY works for App objects!  This is not valid for normal MOOSE objects!
   *
   * @param name The name of the parameter
   * @param syntax Space separated list of command-line switch syntax that can set this option
   * @param doc_string Documentation.  This will be shown for --help
   */
  template <typename T>
  void addRequiredCommandLineParam(const std::string & name,
                                   const std::string & syntax,
                                   const std::string & doc_string);
  template <typename T>
  void addCommandLineParam(const std::string & name,
                           const std::string & syntax,
                           const std::string & doc_string);
  template <typename T>
  void addCommandLineParam(const std::string & name,
                           const std::string & syntax,
                           const T & value,
                           const std::string & doc_string);

  /**
   * Add and mark a parameter for deprecation.  This will allow us to assist users as API changes
   * are made.  If the user
   * supplies a value or even uses the default, a warning will be printed.
   *
   * @param name The name of the parameter
   * @param doc_string Documentation.  This will be shown for --help
   * @param deprecation_message The message that will will print about why this param was
   * deprecated.  It might mention the "new way".
   */
  template <typename T>
  void addRequiredDeprecatedParam(const std::string & name,
                                  const std::string & doc_string,
                                  const std::string & deprecation_message);

  /**
   * @param name The name of the parameter
   * @param value The default value of this parameter if it requires one
   * @param doc_string Documentation.  This will be shown for --help
   * @param deprecation_message The message that will will print about why this param was
   * deprecated.  It might mention the "new way".
   */
  template <typename T>
  void addDeprecatedParam(const std::string & name,
                          const T & value,
                          const std::string & doc_string,
                          const std::string & deprecation_message);

  template <typename T>
  void addDeprecatedParam(const std::string & name,
                          const std::string & doc_string,
                          const std::string & deprecation_message);

  /**
   * This method checks to make sure that we aren't adding a parameter with the same name but a
   * different type.  It
   * throws a MooseError if an inconsistent type is detected. While this state is supported by
   * libMesh it brings
   * nothing but blood and tears for those who try ;)
   *
   * @param name the name of the parameter
   */
  template <typename T>
  void checkConsistentType(const std::string & name) const;

  /**
   * Get the syntax for a command-line parameter
   */
  std::vector<std::string> getSyntax(const std::string & name);

  /**
   * Get the documentation string for a parameter
   */
  const std::string & getDescription(const std::string & name);

  /**
   * This method takes a space delimited list of parameter names and adds them to the specified
   * group name.
   * This information is used in the GUI to group parameters into logical sections.
   */
  void addParamNamesToGroup(const std::string & space_delim_names, const std::string group_name);

  /**
   * This method retrieves the group name for the passed parameter name if one exists.  Otherwise an
   * empty string is returned.
   */
  std::string getGroupName(const std::string & param_name) const;

  /**
   * This method suppresses an inherited parameter so that it isn't required or valid
   * in the derived class. The parameter is added to the private parameter list.
   * Suppressing a parameter can have dire consequences.
   * Use at your own risk!
   */
  template <typename T>
  void suppressParameter(const std::string & name);

  /**
   * Changes the parameter to be required.
   * @param name The parameter name
   */
  template <typename T>
  void makeParamRequired(const std::string & name);

  /**
   * This method adds a coupled variable name pair.  The parser will look for variable
   * name pair in the input file and can return a reference to the storage location
   * for the coupled variable if found
   */
  void addCoupledVar(const std::string & name, const std::string & doc_string);

  /**
   * This method adds a coupled variable name pair.  The parser will look for variable
   * name pair in the input file and can return a reference to the storage location
   * for the coupled variable if found
   *
   * Also - you can provide a default value for this variable in the case that an actual variable is
   * not provided.
   */
  void addCoupledVar(const std::string & name, const Real value, const std::string & doc_string);

  ///@{
  /**
   * These methods add a coupled variable name pair. The parser will look for variable
   * name pair in the input file and can return a reference to the storage location
   * for the coupled variable if found.
   *
   * This version of the method will build a vector if the given the base_name and num_name
   * parameters exist
   * in the input file:
   *   e.g.
   *   [./foo]
   *     ...
   *     some_base = base_
   *     some_num  = 5
   *   [../]
   *
   *   # The coupling parameter will be passed this vector: "base_0 base_1 base_2 base_3 base_4"
   */
  void addCoupledVarWithAutoBuild(const std::string & name,
                                  const std::string & base_name,
                                  const std::string & num_name,
                                  const std::string & doc_string);
  void addRequiredCoupledVarWithAutoBuild(const std::string & name,
                                          const std::string & base_name,
                                          const std::string & num_name,
                                          const std::string & doc_string);
  ///@}

  /**
   * Utility functions for retrieving one of the MooseTypes variables into the common "string" base
   * class.
   * Scalar and Vector versions are supplied
   */
  std::string getMooseType(const std::string & name) const;
  std::vector<std::string> getVecMooseType(const std::string & name) const;

  /**
   * This method adds a coupled variable name pair.  The parser will look for variable
   * name pair in the input file and can return a reference to the storage location
   * for the coupled variable.  If the coupled variable is not supplied in the input
   * file, and error is thrown.
   *
   * Version 2: An auto built vector will be built from the base_name and num_name param. See
   * addCoupledVar for an example
   */
  void addRequiredCoupledVar(const std::string & name, const std::string & doc_string);

  /**
   * Returns the documentation string for the specified parameter name
   */
  std::string getDocString(const std::string & name) const;

  /**
   * Set the doc string of a parameter.
   *
   * This method is generally used from within the validParams function to modify the documentation
   * for an
   * existing parameter, such as a parameter that is supplied from an interface class.
   */
  void setDocString(const std::string & name, const std::string & doc);

  /**
   * Returns a boolean indicating whether the specified parameter is required or not
   */
  bool isParamRequired(const std::string & name) const;

  /**
   * This method returns parameters that have been initialized in one fashion or another,
   * i.e. The value was supplied as a default argument or read and properly converted from
   * the input file
   */
  bool isParamValid(const std::string & name) const;

  /**
   * Returns whether or not the parameter was set due to addParam. If not then it was either set
   * programmatically
   * or was read through the input file.
   */
  bool isParamSetByAddParam(const std::string & name) const;

  /**
   * This method returns true if all of the parameters in this object are valid
   * (i.e. isParamValid(name) == true - for all parameters)
   */
  bool areAllRequiredParamsValid() const;

  /**
   * Prints the type of the requested parameter by name
   */
  std::string type(const std::string & name);

  /**
   * Returns a Boolean indicating whether the specified parameter is private or not
   */
  bool isPrivate(const std::string & name) const;

  /**
   * Declare the given parameters as controllable
   */
  void declareControllable(const std::string & name);

  /**
   * Marker a parameter that has been changed by the Control system (this is for output purposes)
   */
  void markControlled(const std::string & name);

  /**
   * Returns a Boolean indicating whether the specified parameter is controllable
   */
  bool isControllable(const std::string & name);

  /**
   * This method must be called from every base "Moose System" to create linkage with the Action
   * System.
   * See "Moose.C" for the registerMooseObjectTask() calls.
   */
  void registerBase(const std::string & value);

  /**
   * This method is here to indicate which Moose types a particular Action may build. It takes a
   * space
   * delimited list of registered MooseObjects.  TODO: For now we aren't actually checking this list
   * when we build objects. Since individual actions can do whatever they want it's not exactly
   * trivial
   * to check this without changing the user API.  This function properly restricts the syntax and
   * yaml dumps.
   */
  void registerBuildableTypes(const std::string & names);

  /**
   * Returns the list of buildable types as a std::vector<std::string>
   */
  const std::vector<std::string> & getBuildableTypes() const;

  /**
   * Mutators for controlling whether or not the outermost level of syntax will be collapsed when
   * printed.
   */
  void collapseSyntaxNesting(bool collapse);
  bool collapseSyntaxNesting() const;

  /**
   * Mutators for controlling whether or not the outermost level of syntax will be collapsed when
   * printed.
   */
  void mooseObjectSyntaxVisibility(bool visibility);
  bool mooseObjectSyntaxVisibility() const;

  /**
   * Copy and Copy/Add operators for the InputParameters object
   */
  using Parameters::operator=;
  using Parameters::operator+=;
  InputParameters & operator=(const InputParameters & rhs);
  InputParameters & operator+=(const InputParameters & rhs);

  /**
   * This function checks parameters stored in the object to make sure they are in the correct
   * state as the user expects:
   *   Required parameters are verified as valid meaning that they were either initialized when
   *   they were created, or were read from an input file or some other valid source
   */
  void checkParams(const std::string & parsing_syntax);

  /**
   * Methods returning iterators to the coupled variables names stored in this
   * InputParameters object
   */
  inline std::set<std::string>::const_iterator coupledVarsBegin() const
  {
    return _coupled_vars.begin();
  }
  inline std::set<std::string>::const_iterator coupledVarsEnd() const
  {
    return _coupled_vars.end();
  }

  /**
   * Return whether or not the coupled variable exists
   * @param coupling_name The name of the coupled variable to test for
   * @return True if the variable exists in the coupled variables for this InputParameters object
   */
  bool hasCoupledValue(const std::string & coupling_name) const;

  /**
   * Return whether or not the requested parameter has a default coupled value.
   *
   * @param coupling_name The name of the coupling parameter to get the default value for.
   */
  bool hasDefaultCoupledValue(const std::string & coupling_name) const;

  /**
   * Get the default value for an optionally coupled variable.
   *
   * @param coupling_name The name of the coupling parameter to get the default value for.
   */
  Real defaultCoupledValue(const std::string & coupling_name) const;

  /**
   * Set the default value for an optionally coupled variable (called by the Parser).
   *
   * @param coupling_name The name of the coupling parameter to get the default value for.
   * @param value Default value to set.
   */
  void defaultCoupledValue(const std::string & coupling_name, Real value);

  /**
  * Returns the auto build vectors for all parameters.
  */
  const std::map<std::string, std::pair<std::string, std::string>> & getAutoBuildVectors() const;

  /**
   * Get the default value for a postprocessor added with addPostprocessor
   * @param name The name of the postprocessor
   * @param suppress_error If true, the error check is suppressed
   * @return The default value for the postprocessor
   */
  const PostprocessorValue & getDefaultPostprocessorValue(const std::string & name,
                                                          bool suppress_error = false) const;

  /**
   * Set the default value for a postprocessor added with addPostprocessor
   * @param name The name of the postprocessor
   * @value value The value of the postprocessor default to set
   */
  void setDefaultPostprocessorValue(const std::string & name, const PostprocessorValue & value);

  /**
   * Returns true if a default PostprocessorValue is defined
   * @param name The name of the postprocessor
   * @return True if a default value exists
   */
  bool hasDefaultPostprocessorValue(const std::string & name) const;

  /**
   * Method for applying common parameters
   * @param common The set of parameters to apply to the parameters stored in this object
   * @param exclude A vector of parameters to exclude
   *
   * In order to apply common parameter 4 statements must be satisfied
   *   (1) A local parameter must exist with the same name as common parameter
   *   (2) Common parameter must valid
   *   (3) Local parameter must be invalid OR not have been set from its default
   *   (4) Neither may be private
   *
   * Output objects have a set of common parameters that are passed
   * down to each of the output objects created. This method is used for
   * applying those common parameters.
   *
   * @see CommonOutputAction AddOutputAction
   */
  void applyParameters(const InputParameters & common,
                       std::vector<std::string> exclude = std::vector<std::string>());

  /**
   * Deprecated method.  Use isParamSetByUser() instead.
   */
  bool paramSetByUser(const std::string & name) const;

  /**
   * Method returns true if the parameter was by the user
   * @param name The parameter name
   */
  bool isParamSetByUser(const std::string & name) const;

  ///@{
  /*
   * These methods are here to retrieve parameters for scalar and vector types respectively. We will
   * throw errors
   * when returning most scalar types, but will allow retrieving empty vectors.
   */
  template <typename T>
  static const T &
  getParamHelper(const std::string & name, const InputParameters & pars, const T * the_type);

  template <typename T>
  static const std::vector<T> & getParamHelper(const std::string & name,
                                               const InputParameters & pars,
                                               const std::vector<T> * the_type);
  ///@}

  /**
   * Return list of controllable parameters
   */
  const std::set<std::string> & getControllableParameters() const { return _controllable_params; }

private:
  // Private constructor so that InputParameters can only be created in certain places.
  InputParameters();

  /**
   * Toggle the availability of the copy constructor
   *
   * When MooseObject is created via the Factory this flag is set to false, so when a MooseObject is
   * created if
   * the constructor is not a const reference an error is produced. This method allows the
   * InputParameterWarehouse
   * to disable copying.
   */
  void allowCopy(bool status) { _allow_copy = status; }

  /// This method is called when adding a Parameter with a default value, can be specialized for non-matching types
  template <typename T, typename S>
  void setParamHelper(const std::string & name, T & l_value, const S & r_value);

  /// The documentation strings for each parameter
  std::map<std::string, std::string> _doc_string;

  /// The custom type that will be printed in the YAML dump for a parameter if supplied
  std::map<std::string, std::string> _custom_type;

  /// Syntax for command-line parameters
  std::map<std::string, std::vector<std::string>> _syntax;

  /// The names of the parameters organized into groups
  std::map<std::string, std::string> _group;

  /// The map of functions used for range checked parameters
  std::map<std::string, std::string> _range_functions;

  /// The map of auto build vectors (base_, 5 -> "base_0 base_1 base_2 base_3 base_4")
  std::map<std::string, std::pair<std::string, std::string>> _auto_build_vectors;

  /// The parameter is used to restrict types that can be built.  Typically this
  /// is used for MooseObjectAction derived Actions.
  std::vector<std::string> _buildable_types;

  /// This parameter collapses one level of nesting in the syntax blocks.  It is used
  /// in conjunction with MooseObjectAction derived Actions.
  bool _collapse_nesting;

  /// This parameter hides derived MOOSE object types from appearing in syntax dumps
  bool _moose_object_syntax_visibility;

  /// The set of parameters that are required (i.e. will cause an abort if not supplied)
  std::set<std::string> _required_params;

  /**
   * The set of parameters either explicitly set or provided a default value when added
   * Note: We do not store MooseEnum names in valid params, instead we ask MooseEnums whether
   *       they are valid or not.
   */
  std::set<std::string> _valid_params;

  /// The set of parameters that will NOT appear in the the dump of the parser tree
  std::set<std::string> _private_params;

  /// The coupled variables set
  std::set<std::string> _coupled_vars;

  /// The list of deprecated params
  std::map<std::string, std::string> _deprecated_params;

  /// The default value for optionally coupled variables
  std::map<std::string, Real> _default_coupled_value;

  /// The default value for postprocessors
  std::map<std::string, PostprocessorValue> _default_postprocessor_value;

  /// If a parameters value was set by addParam, and not set again, it will appear in this list (see applyParameters)
  std::set<std::string> _set_by_add_param;

  /// A list of parameters declared as controllable
  std::set<std::string> _controllable_params;

  /// Flag for disabling deprecated parameters message, this is used by applyParameters to avoid dumping messages
  bool _show_deprecated_message;

  /// A flag for toggling the error message in the copy constructor
  bool _allow_copy;

  // These are the only objects allowed to _create_ InputParameters
  friend InputParameters validParams<MooseObject>();
  friend InputParameters validParams<Action>();
  friend InputParameters validParams<Problem>();
  friend InputParameters emptyInputParameters();
  friend InputParameters validParams<MooseApp>();
  friend class InputParameterWarehouse;
};

// Template and inline function implementations
template <typename T>
T &
InputParameters::set(const std::string & name, bool quiet_mode)
{
  checkConsistentType<T>(name);

  if (!this->have_parameter<T>(name))
    _values[name] = new Parameter<T>;

  set_attributes(name, false);

  if (quiet_mode)
    _set_by_add_param.insert(name);

  return cast_ptr<Parameter<T> *>(_values[name])->set();
}

template <typename T, typename UP_T>
void
InputParameters::rangeCheck(const std::string & full_name,
                            const std::string & short_name,
                            InputParameters::Parameter<std::vector<T>> * param,
                            std::ostream & oss)
{
  mooseAssert(param, "Parameter is NULL");

  if (_range_functions.count(short_name) == 0 || !isParamValid(short_name))
    return;

  /**
   * Automatically detect the variables used in the range checking expression.
   * We allow the following variables (where snam is the short_name of the parameter)
   *
   * snam       : tests every component in the vector
   *              'snam > 0'
   * snam_size  : the size of the vector
   *              'snam_size = 5'
   * snam_i     : where i is a number from 0 to sname_size-1 tests a specific component
   *              'snam_0 > snam_1'
   */
  FunctionParserBase<UP_T> fp;
  std::vector<std::string> vars;
  if (fp.ParseAndDeduceVariables(_range_functions[short_name], vars) != -1) // -1 for success
  {
    oss << "Error parsing expression: " << _range_functions[short_name] << '\n';
    return;
  }

  // Fparser parameter buffer
  std::vector<UP_T> parbuf(vars.size());

  // parameter vector
  const std::vector<T> & value = param->set();

  // iterate over all vector values (maybe ;)
  bool need_to_iterate = false;
  unsigned int i = 0;
  do
  {
    // set parameters
    for (unsigned int j = 0; j < vars.size(); j++)
    {
      if (vars[j] == short_name)
      {
        if (value.size() == 0)
        {
          oss << "Range checking empty vector: " << _range_functions[short_name] << '\n';
          return;
        }

        parbuf[j] = value[i];
        need_to_iterate = true;
      }
      else if (vars[j] == short_name + "_size")
        parbuf[j] = value.size();
      else
      {
        if (vars[j].substr(0, short_name.size() + 1) != short_name + "_")
        {
          oss << "Error parsing expression: " << _range_functions[short_name] << '\n';
          return;
        }
        std::istringstream iss(vars[j]);
        iss.seekg(short_name.size() + 1);

        size_t index;
        if (iss >> index)
        {
          if (index >= value.size())
          {
            oss << "Error parsing expression: " << _range_functions[short_name]
                << "\nOut of range variable " << vars[j] << '\n';
            return;
          }
          parbuf[j] = value[index];
        }
        else
        {
          oss << "Error parsing expression: " << _range_functions[short_name]
              << "\nInvalid variable " << vars[j] << '\n';
          return;
        }
      }
    }

    // ensure range-checked input file parameter comparison functions
    // do absolute floating point comparisons instead of using a default epsilon.
    auto tmp_eps = fp.epsilon();
    fp.setEpsilon(0);
    UP_T result = fp.Eval(&parbuf[0]);
    fp.setEpsilon(tmp_eps);

    // test function using the parameters determined above
    if (fp.EvalError())
    {
      oss << "Error evaluating expression: " << _range_functions[short_name] << '\n';
      return;
    }

    if (!result)
    {
      oss << "Range check failed for parameter " << full_name
          << "\n\tExpression: " << _range_functions[short_name] << "\n";
      if (need_to_iterate)
        oss << "\t Component: " << i << '\n';
    }

  } while (need_to_iterate && ++i < value.size());
}

template <typename T, typename UP_T>
void
InputParameters::rangeCheck(const std::string & full_name,
                            const std::string & short_name,
                            InputParameters::Parameter<T> * param,
                            std::ostream & oss)
{
  mooseAssert(param, "Parameter is NULL");

  if (_range_functions.find(short_name) == _range_functions.end() || !isParamValid(short_name))
    return;

  // Parse the expression
  FunctionParserBase<UP_T> fp;
  if (fp.Parse(_range_functions[short_name], short_name) != -1) // -1 for success
  {
    oss << "Error parsing expression: " << _range_functions[short_name] << '\n';
    return;
  }

  // ensure range-checked input file parameter comparison functions
  // do absolute floating point comparisons instead of using a default epsilon.
  auto tmp_eps = fp.epsilon();
  fp.setEpsilon(0);
  // We require a non-const value for the implicit upscaling of the parameter type
  std::vector<UP_T> value(1, param->set());
  UP_T result = fp.Eval(&value[0]);
  fp.setEpsilon(tmp_eps);

  if (fp.EvalError())
  {
    oss << "Error evaluating expression: " << _range_functions[short_name]
        << "\nPerhaps you used the wrong variable name?\n";
    return;
  }

  if (!result)
    oss << "Range check failed for parameter " << full_name
        << "\n\tExpression: " << _range_functions[short_name] << "\n\tValue: " << value[0] << '\n';
}

template <typename T>
T
InputParameters::getCheckedPointerParam(const std::string & name,
                                        const std::string & error_string) const
{
  T param = this->get<T>(name);

  // Note: You will receive a compile error on this line if you attempt to pass a non-pointer
  // template type to this method
  if (param == NULL)
    mooseError("Parameter ", name, " is NULL.\n", error_string);
  return this->get<T>(name);
}

template <typename T>
void
InputParameters::addRequiredParam(const std::string & name, const std::string & doc_string)
{
  checkConsistentType<T>(name);

  InputParameters::insert<T>(name);
  _required_params.insert(name);
  _doc_string[name] = doc_string;
}

template <typename T>
void
InputParameters::addRequiredParam(const std::string & /*name*/,
                                  const T & /*value*/,
                                  const std::string & /*doc_string*/)
{
  mooseError("You cannot call addRequiredParam and supply a default value for this type, please "
             "use addParam instead");
}

template <typename T, typename S>
void
InputParameters::addParam(const std::string & name, const S & value, const std::string & doc_string)
{
  checkConsistentType<T>(name);

  T & l_value = InputParameters::set<T>(name);
  _doc_string[name] = doc_string;

  // Set the parameter now
  setParamHelper(name, l_value, value);

  /* Indicate the default value, as set via addParam, is being used. The parameter is removed from
     the list whenever
     it changes, see set_attributes */
  _set_by_add_param.insert(name);
}

template <typename T>
void
InputParameters::addParam(const std::string & name, const std::string & doc_string)
{
  checkConsistentType<T>(name);

  InputParameters::insert<T>(name);
  _doc_string[name] = doc_string;
}

template <typename T, typename S>
void
InputParameters::setParamHelper(const std::string & /*name*/, T & l_value, const S & r_value)
{
  l_value = r_value;
}

template <typename T>
void
InputParameters::addRequiredRangeCheckedParam(const std::string & name,
                                              const std::string & parsed_function,
                                              const std::string & doc_string)
{
  addRequiredParam<T>(name, doc_string);
  _range_functions[name] = parsed_function;
}

template <typename T>
void
InputParameters::addRangeCheckedParam(const std::string & name,
                                      const T & value,
                                      const std::string & parsed_function,
                                      const std::string & doc_string)
{
  addParam<T>(name, value, doc_string);
  _range_functions[name] = parsed_function;
}

template <typename T>
void
InputParameters::addRangeCheckedParam(const std::string & name,
                                      const std::string & parsed_function,
                                      const std::string & doc_string)
{
  addParam<T>(name, doc_string);
  _range_functions[name] = parsed_function;
}

template <typename T>
void
InputParameters::addRequiredCustomTypeParam(const std::string & name,
                                            const std::string & custom_type,
                                            const std::string & doc_string)
{
  addRequiredParam<T>(name, doc_string);
  _custom_type[name] = custom_type;
}

template <typename T>
void
InputParameters::addCustomTypeParam(const std::string & name,
                                    const T & value,
                                    const std::string & custom_type,
                                    const std::string & doc_string)
{
  addParam<T>(name, value, doc_string);
  _custom_type[name] = custom_type;
}

template <typename T>
void
InputParameters::addCustomTypeParam(const std::string & name,
                                    const std::string & custom_type,
                                    const std::string & doc_string)
{
  addParam<T>(name, doc_string);
  _custom_type[name] = custom_type;
}

template <typename T>
void
InputParameters::addPrivateParam(const std::string & name)
{
  checkConsistentType<T>(name);

  InputParameters::insert<T>(name);
  _private_params.insert(name);
}

template <typename T>
void
InputParameters::addPrivateParam(const std::string & name, const T & value)
{
  checkConsistentType<T>(name);

  InputParameters::set<T>(name) = value;
  _private_params.insert(name);
  _set_by_add_param.insert(name);
}

template <typename T>
void
InputParameters::addRequiredCommandLineParam(const std::string & name,
                                             const std::string & syntax,
                                             const std::string & doc_string)
{
  addRequiredParam<T>(name, doc_string);
  MooseUtils::tokenize(syntax, _syntax[name], 1, " \t\n\v\f\r");
}

template <typename T>
void
InputParameters::addCommandLineParam(const std::string & name,
                                     const std::string & syntax,
                                     const std::string & doc_string)
{
  addParam<T>(name, doc_string);
  MooseUtils::tokenize(syntax, _syntax[name], 1, " \t\n\v\f\r");
}

template <typename T>
void
InputParameters::addCommandLineParam(const std::string & name,
                                     const std::string & syntax,
                                     const T & value,
                                     const std::string & doc_string)
{
  addParam<T>(name, value, doc_string);
  MooseUtils::tokenize(syntax, _syntax[name], 1, " \t\n\v\f\r");
}

template <typename T>
void
InputParameters::checkConsistentType(const std::string & name) const
{
  // Do we have a paremeter with the same name but a different type?
  InputParameters::const_iterator it = _values.find(name);
  if (it != _values.end() && dynamic_cast<const Parameter<T> *>(it->second) == NULL)
    mooseError("Attempting to set parameter \"",
               name,
               "\" with type (",
               demangle(typeid(T).name()),
               ")\nbut the parameter already exists as type (",
               it->second->type(),
               ")");
}

template <typename T>
void
InputParameters::suppressParameter(const std::string & name)
{
  if (!this->have_parameter<T>(name))
    mooseError("Unable to suppress nonexistent parameter: ", name);

  _required_params.erase(name);
  _private_params.insert(name);
}

template <typename T>
void
InputParameters::addRequiredDeprecatedParam(const std::string & name,
                                            const std::string & doc_string,
                                            const std::string & deprecation_message)
{
  _show_deprecated_message = false;
  addRequiredParam<T>(name, doc_string);

  _deprecated_params.insert(std::make_pair(name, deprecation_message));
  _show_deprecated_message = true;
}

template <typename T>
void
InputParameters::addDeprecatedParam(const std::string & name,
                                    const T & value,
                                    const std::string & doc_string,
                                    const std::string & deprecation_message)
{
  _show_deprecated_message = false;
  addParam<T>(name, value, doc_string);

  _deprecated_params.insert(std::make_pair(name, deprecation_message));
  _show_deprecated_message = true;
}

template <typename T>
void
InputParameters::addDeprecatedParam(const std::string & name,
                                    const std::string & doc_string,
                                    const std::string & deprecation_message)
{
  _show_deprecated_message = false;
  addParam<T>(name, doc_string);

  _deprecated_params.insert(std::make_pair(name, deprecation_message));
  _show_deprecated_message = true;
}

// Forward declare MooseEnum specializations for add*Param
template <>
void InputParameters::addRequiredParam<MooseEnum>(const std::string & name,
                                                  const MooseEnum & moose_enum,
                                                  const std::string & doc_string);

template <>
void InputParameters::addRequiredParam<MultiMooseEnum>(const std::string & name,
                                                       const MultiMooseEnum & moose_enum,
                                                       const std::string & doc_string);

template <>
void InputParameters::addRequiredParam<std::vector<MooseEnum>>(
    const std::string & name,
    const std::vector<MooseEnum> & moose_enums,
    const std::string & doc_string);

template <>
void InputParameters::addParam<MooseEnum>(const std::string & /*name*/,
                                          const std::string & /*doc_string*/);

template <>
void InputParameters::addParam<MultiMooseEnum>(const std::string & /*name*/,
                                               const std::string & /*doc_string*/);

template <>
void InputParameters::addParam<std::vector<MooseEnum>>(const std::string & /*name*/,
                                                       const std::string & /*doc_string*/);

template <>
void InputParameters::addDeprecatedParam<MooseEnum>(const std::string & name,
                                                    const std::string & doc_string,
                                                    const std::string & deprecation_message);

template <>
void InputParameters::addDeprecatedParam<MultiMooseEnum>(const std::string & name,
                                                         const std::string & doc_string,
                                                         const std::string & deprecation_message);

template <>
void InputParameters::addDeprecatedParam<std::vector<MooseEnum>>(
    const std::string & name,
    const std::string & doc_string,
    const std::string & deprecation_message);

// Forward declare specializations for setParamHelper
template <>
void InputParameters::setParamHelper<PostprocessorName, Real>(const std::string & name,
                                                              PostprocessorName & l_value,
                                                              const Real & r_value);

template <>
void InputParameters::setParamHelper<PostprocessorName, int>(const std::string & name,
                                                             PostprocessorName & l_value,
                                                             const int & r_value);

template <>
void InputParameters::setParamHelper<FunctionName, Real>(const std::string & /*name*/,
                                                         FunctionName & l_value,
                                                         const Real & r_value);

template <>
void InputParameters::setParamHelper<FunctionName, int>(const std::string & /*name*/,
                                                        FunctionName & l_value,
                                                        const int & r_value);

template <>
void InputParameters::setParamHelper<MaterialPropertyName, Real>(const std::string & /*name*/,
                                                                 MaterialPropertyName & l_value,
                                                                 const Real & r_value);

template <>
void InputParameters::setParamHelper<MaterialPropertyName, int>(const std::string & /*name*/,
                                                                MaterialPropertyName & l_value,
                                                                const int & r_value);

template <typename T>
const T &
InputParameters::getParamHelper(const std::string & name, const InputParameters & pars, const T *)
{
  if (!pars.isParamValid(name))
    mooseError("The parameter \"", name, "\" is being retrieved before being set.\n");
  return pars.get<T>(name);
}

template <typename T>
const std::vector<T> &
InputParameters::getParamHelper(const std::string & name,
                                const InputParameters & pars,
                                const std::vector<T> *)
{
  return pars.get<std::vector<T>>(name);
}

template <>
inline const MooseEnum &
InputParameters::getParamHelper<MooseEnum>(const std::string & name,
                                           const InputParameters & pars,
                                           const MooseEnum *)
{
  return pars.get<MooseEnum>(name);
}

template <>
inline const MultiMooseEnum &
InputParameters::getParamHelper<MultiMooseEnum>(const std::string & name,
                                                const InputParameters & pars,
                                                const MultiMooseEnum *)
{
  return pars.get<MultiMooseEnum>(name);
}

InputParameters emptyInputParameters();

#endif /* INPUTPARAMETERS_H */
