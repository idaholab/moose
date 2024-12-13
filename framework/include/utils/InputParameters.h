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
#include "MooseUtils.h"
#include "MooseError.h"
#include "MooseTypes.h"
#include "MultiMooseEnum.h"
#include "ExecFlagEnum.h"
#include "Conversion.h"
#include "DataFileUtils.h"

#include "libmesh/parameters.h"

#ifdef LIBMESH_HAVE_FPARSER
#include "libmesh/fparser.hh"
#else
template <typename T>
class FunctionParserBase
{
}
#endif

#include <tuple>
#include <unordered_map>
#include <mutex>
#include <optional>
#include <filesystem>
#include <regex>

#include <gtest/gtest.h>

// Forward declarations
class Action;
class ActionFactory;
class Factory;
class FEProblemBase;
class InputParameters;
class MooseEnum;
class MooseObject;
class MooseBase;
class MultiMooseEnum;
class Problem;
namespace hit
{
class Node;
}
namespace Moose
{
class Builder;
}
class CommandLine;

/**
 * The main MOOSE class responsible for handling user-defined
 * parameters in almost every MOOSE system.
 */
class InputParameters : public libMesh::Parameters
{
public:
  InputParameters(const InputParameters & rhs);
  InputParameters(const Parameters & rhs);

  virtual ~InputParameters() = default;

  virtual void clear() override;

  /**
   * Structure for storing information about a command line parameter
   */
  struct CommandLineMetadata
  {
    enum ArgumentType
    {
      NONE,
      OPTIONAL,
      REQUIRED
    };

    /// The syntax for the parameter
    std::string syntax;
    /// The switches for the parameter (i.e., [-t, --timing])
    std::vector<std::string> switches;
    /// The type of argument
    ArgumentType argument_type;
    /// Whether or not the argument is required
    bool required;
    /// Whether or not the parameter was set by the CommandLine
    bool set_by_command_line = false;
    /// Whether or not the parameter is global (passed to MultiApps)
    bool global = false;
  };

  /**
   * Class that is used as a parameter to setHitNode() that allows only
   * relevant classes to set the hit node
   */
  class SetHitNodeKey
  {
    friend class Action;
    friend class ActionFactory;
    friend class Moose::Builder;
    friend class Factory;
    friend class FEProblemBase;
    friend class InputParameters;
    FRIEND_TEST(InputParametersTest, fileNames);
    SetHitNodeKey() {}
    SetHitNodeKey(const SetHitNodeKey &) {}
  };

  /**
   * Class that is used as a parameter to setHitNode(param) that allows only
   * relevant classes to set the hit node
   */
  class SetParamHitNodeKey
  {
    friend class Moose::Builder;
    FRIEND_TEST(InputParametersTest, fileNames);
    SetParamHitNodeKey() {}
    SetParamHitNodeKey(const SetParamHitNodeKey &) {}
  };

  /**
   * Determines whether or not the given type is a type that is supported for
   * a command line parameter.
   *
   * In particular, whether or not CommandLine::populateCommandLineParams
   * supports extracting these types.
   */
  template <typename T>
  struct isValidCommandLineType
  {
    static constexpr bool value =
        std::is_same_v<T, std::string> || std::is_same_v<T, std::vector<std::string>> ||
        std::is_same_v<T, Real> || std::is_same_v<T, unsigned int> || std::is_same_v<T, int> ||
        std::is_same_v<T, bool> || std::is_same_v<T, MooseEnum>;
  };

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

  /// Prints the deprecated parameter message, assuming we have the right flags set
  bool attemptPrintDeprecated(const std::string & name);

  /// This functions is called in set as a 'callback' to avoid code duplication
  template <typename T>
  void setHelper(const std::string & name);

  /**
   * Returns a writable reference to the named parameters.  Note: This is not a virtual
   * function! Use caution when comparing to the parent class implementation
   * @param name The name of the parameter to set
   * @param quiet_mode When true the parameter is kept with set_by_add_param=true,
   * this is generally not needed.
   *
   * "quite_mode" returns a writable reference to the named parameter, without setting
   * set_by_add_param to false. Using this method of set will make the parameter to continue to
   * behave if its value where set ONLY by addParam and not by any other method.
   *
   * This was added for handling parameters in the Output objects that have behavior dependent
   * on whether the user modified the parameters.
   *
   */
  template <typename T>
  T & set(const std::string & name, bool quiet_mode = false);

  /**
   * Given a series of parameters names and values, sets each name to
   * the corresponding value.  Any number of name, value pairs can be
   * supplied.
   *
   * Note that each \p value must be of the correct type for the
   * parameter of that name, not merely of a type convertible to the
   * correct type.
   *
   * @param name The name of the first parameter to set
   */
  template <typename T, typename... Ts>
  void setParameters(const std::string & name, const T & value, Ts... extra_input_parameters);

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
   * missing in the input file, an error will be thrown
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

  ///@{
  /**
   * These methods add an optional parameter and a documentation string to the InputParameters
   * object. The first version of this function takes a default value which is used if the parameter
   * is not found in the input file. The second method will leave the parameter uninitialized but
   * can be checked with "isParamValid" before use.
   */
  template <typename T, typename S>
  void addParam(const std::string & name, const S & value, const std::string & doc_string);
  template <typename T>
  void addParam(const std::string & name, const std::string & doc_string);
  ///@}

  /**
   * Enable support for initializer lists as default arguments for container type.
   */
  template <typename T>
  void addParam(const std::string & name,
                const std::initializer_list<typename T::value_type> & value,
                const std::string & doc_string)
  {
    addParam<T>(name, T{value}, doc_string);
  }

  ///@{
  // BEGIN RANGE CHECKED PARAMETER METHODS
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
  // END RANGE CHECKED PARAMETER METHODS
  ///@}

  /**
   * These methods add an option parameter and with a customer type to the InputParameters object.
   * The custom type will be output in YAML dumps and can be used within the GUI application.
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
  template <typename T>
  void addDeprecatedCustomTypeParam(const std::string & name,
                                    const std::string & custom_type,
                                    const std::string & doc_string,
                                    const std::string & deprecation_msg);

  /**
   * These method add a parameter to the InputParameters object which can be retrieved like any
   * other parameter. This parameter however is not printed in the Input file syntax dump or web
   * page dump so does not take a documentation string.  The first version of this function takes an
   * optional default value.
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
  template <typename T>
  void addCommandLineParam(const std::string & name,
                           const std::string & syntax,
                           const std::initializer_list<typename T::value_type> & value,
                           const std::string & doc_string)
  {
    addCommandLineParam<T>(name, syntax, T{value}, doc_string);
  }

  /**
   * Add a command line parameter with an optional value.
   *
   * This is a deprecated option and only remains for two parameters:
   * "mesh_only" and "recover". There are issues with command line
   * parameters with optional values because if a value following
   * one of these is a hit cli parameter, we don't know if we should
   * apply it to the optional option or as a hit parameter.
   *
   * It is also allowed for "run" as we take all arguments past
   * --run and pass to python.
   *
   * @param name The name of the parameer
   * @param syntax Space separated list of command-line switch syntax that can set this option
   * @param value The default value to assign
   * @param doc_string Documentation.  This will be shown for --help
   */
  template <typename T>
  void addOptionalValuedCommandLineParam(const std::string & name,
                                         const std::string & syntax,
                                         const T & value,
                                         const std::string & doc_string);

  /**
   * Sets the command line parameter with \p name as global.
   *
   * Global here means that it will be passed to all child MultiApps.
   */
  void setGlobalCommandLineParam(const std::string & name);

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
   * @return Whether or not the parameter \p name is a command line parameter
   */
  bool isCommandLineParameter(const std::string & name) const;

  /**
   * @return Queries for the command line metadata for the parameter \p name
   *
   * Will return an empty optional if the parameter is not a command line param.
   */
  std::optional<InputParameters::CommandLineMetadata>
  queryCommandLineMetadata(const std::string & name) const;

  /**
   * @return The command line metadata for the parameter \p name.
   */
  const InputParameters::CommandLineMetadata &
  getCommandLineMetadata(const std::string & name) const;

  /**
   * Class that is used as a parameter to commandLineParamSet() that allows only
   * the CommandLine to set that a parmeter is set by the command line
   */
  class CommandLineParamSetKey
  {
    friend class CommandLine;
    FRIEND_TEST(InputParametersTest, commandLineParamSetNotCLParam);
    CommandLineParamSetKey() {}
    CommandLineParamSetKey(const CommandLineParamSetKey &) {}
  };
  /**
   * Marks the command line parameter \p name as set by the CommandLine.
   *
   * Protected by the CommandLineParamSetKey so that only the CommandLine can call this.
   */
  void commandLineParamSet(const std::string & name, const CommandLineParamSetKey);

  /**
   * Get the documentation string for a parameter
   */
  const std::string & getDescription(const std::string & name) const;

  /**
   * This method takes a space delimited list of parameter names and adds them to the specified
   * group name.
   * This information is used in the GUI to group parameters into logical sections.
   */
  void addParamNamesToGroup(const std::string & space_delim_names, const std::string group_name);

  /**
   * This method renames a parameter group
   * @param old_name previous name of the parameter group
   * @param new_name new name of the parameter group
   */
  void renameParameterGroup(const std::string & old_name, const std::string & new_name);

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
   * Changes the parameter to not be required.
   * @param name The parameter name
   */
  template <typename T>
  void makeParamNotRequired(const std::string & name);

  /**
   * This method adds a coupled variable name pair.  The parser will look for variable
   * name pair in the input file and can return a reference to the storage location
   * for the coupled variable if found
   */
  void addCoupledVar(const std::string & name, const std::string & doc_string);

  /**
   * This method adds a deprecated coupled variable name pair.  The parser will look for variable
   * name pair in the input file and can return a reference to the storage location
   * for the coupled variable if found. The doc string for the deprecated variable will be
   * constructed from the doc string for the new variable. A deprecation message will also be
   * automatically generated
   */
  void addDeprecatedCoupledVar(const std::string & old_name,
                               const std::string & new_name,
                               const std::string & removal_date = "");

  /**
   * This method adds a coupled variable name pair.  The parser will look for variable
   * name pair in the input file and can return a reference to the storage location
   * for the coupled variable if found
   *
   * Also - you can provide a default value for this variable in the case that an actual variable is
   * not provided.
   */
  void addCoupledVar(const std::string & name, const Real value, const std::string & doc_string);

  /**
   * This method adds a coupled variable name pair.  The parser will look for variable
   * name pair in the input file and can return a reference to the storage location
   * for the coupled variable if found
   *
   * Also - you can provide a vector of values for this variable in the case that an actual variable
   * is not provided.
   */
  void addCoupledVar(const std::string & name,
                     const std::vector<Real> & value,
                     const std::string & doc_string);

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
   * Returns the documentation unit string for the specified parameter name
   */
  std::string getDocUnit(const std::string & name) const;

  /**
   * Set the unit string of a parameter.
   *
   * This method is only used within MooseDocs and the input syntax dump in order to provide a
   * developer-expected unit for software quality assurance purposes.
   */
  void setDocUnit(const std::string & name, const std::string & doc_unit);

  /**
   * Returns a boolean indicating whether the specified parameter is required or not
   */
  bool isParamRequired(const std::string & name) const;

  /**
   * Forces parameter of given name to be not required regardless of type
   */
  void makeParamNotRequired(const std::string & name);

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
   * Returns True if the parameters is deprecated.
   */
  bool isParamDeprecated(const std::string & name) const;

  /**
   * This method returns true if all of the parameters in this object are valid
   * (i.e. isParamValid(name) == true - for all parameters)
   */
  bool areAllRequiredParamsValid() const;

  /**
   * Prints the type of the requested parameter by name
   */
  std::string type(const std::string & name) const;

  /**
   * Returns a Boolean indicating whether the specified parameter is private or not
   */
  bool isPrivate(const std::string & name) const;

  /**
   * Declare the given parameters as controllable
   */
  void declareControllable(const std::string & name, std::set<ExecFlagType> execute_flags = {});

  /**
   * Marker a parameter that has been changed by the Control system (this is for output purposes)
   */
  void markControlled(const std::string & name);

  /**
   * Returns a Boolean indicating whether the specified parameter is controllable
   */
  bool isControllable(const std::string & name) const;

  /**
   * Return the allowed execute flags for a controllable parameter
   */
  const std::set<ExecFlagType> & getControllableExecuteOnTypes(const std::string & name) const;

  /**
   * This method must be called from every base "Moose System" to create linkage with the Action
   * System.
   * See "Moose.C" for the registerMooseObjectTask() calls.
   */
  void registerBase(const std::string & value);

  /**
   * @return The base system of the object these parameters are for, if any
   */
  std::optional<std::string> getBase() const;

  /**
   * This method is used to define the MOOSE system name that is used by the TheWarehouse object
   * for storing objects to be retrieved for execution. The base class of every object class
   * that will be called for execution (e.g., UserObject objects) should call this method.
   *
   * This is different from registerBase because the name supplied to registerBase is used to
   * associate syntax, but the objects created often go to the same objects for execution, as is
   * the case for Postprocessor object which are executed with UserObjects.
   *
   * See the AttribSystem object for use Attribute.h/C.
   */
  void registerSystemAttributeName(const std::string & value);

  /**
   * Get the system attribute name if it was registered. Otherwise throw an error.
   * See the AttribSystem object for use Attribute.h/C.
   */
  const std::string & getSystemAttributeName() const;

  /**
   * This method is here to indicate which Moose types a particular Action may build. It takes a
   * space delimited list of registered MooseObjects.  TODO: For now we aren't actually checking
   * this list when we build objects. Since individual actions can do whatever they want it's not
   * exactly trivial to check this without changing the user API.  This function properly restricts
   * the syntax and YAML dumps.
   */
  void registerBuildableTypes(const std::string & names);

  /**
   * Tells MOOSE about a RelationshipManager that this object needs.  RelationshipManagers
   * handle element "ghosting", "non-local DOF access" and "sparsity pattern" relationships.
   *
   * Basically: if this object needs non-local (ie non-current-element) data access then you
   * probably need a relationship manager
   *
   * @param name The name of the RelationshipManager type
   * @param rm_type The type (GEOMETRIC/ALGEBRAIC) of the RelationshipManger.  Note: You can use
   * boolean logic to to "or" RelationshipManagerTypes together to make a RelationshipManager that
   * is multi-typed.
   * @param input_parameter_callback This is a function pointer that will get called to fill in the
   * RelationShipManager's InputParameters.  See MooseTypes.h for the signature of this function.
   */
  void addRelationshipManager(
      const std::string & name,
      Moose::RelationshipManagerType rm_type,
      Moose::RelationshipManagerInputParameterCallback input_parameter_callback = nullptr);

  /**
   * Clears all currently registered RelationshipManagers
   */
  void clearRelationshipManagers() { _buildable_rm_types.clear(); }

  /**
   * Returns the list of buildable types as a std::vector<std::string>
   */
  const std::vector<std::string> & getBuildableTypes() const;

  /**
   * Returns the list of buildable (or required) RelationshipManager object types for this object.
   */
  const std::vector<std::tuple<std::string,
                               Moose::RelationshipManagerType,
                               Moose::RelationshipManagerInputParameterCallback>> &
  getBuildableRelationshipManagerTypes() const;

  ///@{
  /**
   * Mutators for controlling whether or not the outermost level of syntax will be collapsed when
   * printed.
   */
  void collapseSyntaxNesting(bool collapse);
  bool collapseSyntaxNesting() const;
  ///@}

  ///@{
  /**
   * Mutators for controlling whether or not the outermost level of syntax will be collapsed when
   * printed.
   */
  void mooseObjectSyntaxVisibility(bool visibility);
  bool mooseObjectSyntaxVisibility() const;
  ///@}

  ///@{
  /**
   * Copy and Copy/Add operators for the InputParameters object
   */
  using Parameters::operator=;
  using Parameters::operator+=;
  InputParameters & operator=(const InputParameters & rhs);
  InputParameters & operator+=(const InputParameters & rhs);
  ///@}

  /**
   * This function checks parameters stored in the object to make sure they are in the correct
   * state as the user expects:
   *   Required parameters are verified as valid meaning that they were either initialized when
   *   they were created, or were read from an input file or some other valid source
   */
  void checkParams(const std::string & parsing_syntax);

  /**
   * Finalizes the parameters, which must be done before constructing any objects
   * with these parameters (to be called in the corresponding factories).
   * typed parameters.
   *
   * This calls checkParams() and sets up the absolute paths for all file name.
   */
  void finalize(const std::string & parsing_syntax);

  /**
   * @return A file base to associate with these parameters.
   *
   * Optionally, an input parameter can be provided via \p param_name.
   *
   * If the parameter is provided, we have the following options:
   * - The parameter itself has a hit node set (context for that parameter)
   * - The InputParameters object has a hit node set (context for all parameters)
   * - Neither of the above and we die
   *
   * In the event that a the parameter is set via command line, this will
   * attempt to look at the parameter's parents to find a suitable context.
   */
  std::filesystem::path
  getFileBase(const std::optional<std::string> & param_name = std::optional<std::string>()) const;

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
   * Return the coupled variable parameter names.
   */
  const std::set<std::string> & getCoupledVariableParamNames() const { return _coupled_vars; }

  /**
   * Return the new to deprecated variable name map
   */
  const std::unordered_map<std::string, std::string> & getNewToDeprecatedVarMap() const
  {
    return _new_to_deprecated_coupled_vars;
  }

  /// Return whether a parameter has a range check
  bool isRangeChecked(const std::string & param_name) const;

  /// Return the range check function for any parameter (empty string if it is not range checked)
  std::string rangeCheckedFunction(const std::string & name) const;

  /// Return whether a parameter has a default
  bool hasDefault(const std::string & param_name) const;

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
   * @param i By default 0, in general the index of the requested coupled default value.
   */
  Real defaultCoupledValue(const std::string & coupling_name, unsigned int i = 0) const;

  /**
   * Get the number of defaulted coupled value entries
   *
   * @param coupling_name The name of the coupling parameter to get the default value for.
   */
  unsigned int numberDefaultCoupledValues(const std::string & coupling_name) const;

  /**
   * Set the default value for an optionally coupled variable (called by the Parser).
   *
   * @param coupling_name The name of the coupling parameter to get the default value for.
   * @param value Default value to set.
   * @param i By default 0, in general the index of the requested coupled default value.
   */
  void defaultCoupledValue(const std::string & coupling_name, Real value, unsigned int i = 0);

  /**
   * Returns the auto build vectors for all parameters.
   */
  std::map<std::string, std::pair<std::string, std::string>> getAutoBuildVectors() const;

  // BEGIN APPLY PARAMETER METHODS
  /**
   * Method for applying common parameters
   * @param common The set of parameters to apply to the parameters stored in this object
   * @param exclude A vector of parameters to exclude
   *
   * In order to apply common parameter 4 statements must be satisfied
   *   (1) A local parameter must exist with the same name as common parameter
   *   (2) Common parameter must be valid
   *   (3) Local parameter must be invalid OR not have been set from its default
   *   (4) Both cannot be private (unless \p allow_private = true)
   *
   * Output objects have a set of common parameters that are passed
   * down to each of the output objects created. This method is used for
   * applying those common parameters.
   *
   * @see CommonOutputAction AddOutputAction
   */
  void applyParameters(const InputParameters & common,
                       const std::vector<std::string> & exclude = {},
                       const bool allow_private = false);

  /**
   * Method for applying common parameters
   * @param common The set of parameters to apply to the parameters stored in this object
   * @param include A vector of parameters to apply
   *
   * In order to apply common parameter 4 statements must be satisfied
   *   (1) A local parameter must exist with the same name as common parameter
   *   (2) Common parameter must valid
   *   (3) Local parameter must be invalid OR not have been set from its default
   *   (4) Both cannot be private
   *
   * Output objects have a set of common parameters that are passed
   * down to each of the output objects created. This method is used for
   * applying those common parameters.
   *
   * @see CommonOutputAction AddOutputAction
   */
  void applySpecificParameters(const InputParameters & common,
                               const std::vector<std::string> & include,
                               bool allow_private = false);

  /**
   * Apply values from a single parameter in common, to a single parameter stored in this object
   * @param common The set of InputParameters from which to extract parameters from
   * @param common_name The name within common from which to get the parameter values
   *
   * In order to apply common parameter 4 statements must be satisfied
   *   (1) A local parameter must exist with the same name as common parameter
   *   (2) Common parameter must valid
   *   (3) Local parameter must be invalid OR not have been set from its default
   *   (except if override_default is set)
   *   (4) Both cannot be private
   */
  void applyParameter(const InputParameters & common,
                      const std::string & common_name,
                      bool allow_private = false,
                      bool override_default = false);
  // END APPLY PARAMETER METHODS

  /**
   * Apply properties of a single coupled variable in common, to a single coupled variable stored in
   * this object
   * @param common The set of InputParameters from which to extract the coupled variable's
   * properties
   * @param var_name The name of the coupled variable whose properties are to be applied
   *
   * In order to apply the properties, both the local parameters and the common parameters must
   * have a coupled variable with name var_name
   */
  void applyCoupledVar(const InputParameters & common, const std::string & var_name);

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
   * when returning most scalar and vector types.
   */
  template <typename T>
  static const T & getParamHelper(const std::string & name,
                                  const InputParameters & pars,
                                  const T * the_type,
                                  const MooseBase * moose_base = nullptr);
  ///@}

  using Parameters::get;

  /// Combine two vector parameters into a single vector of pairs
  template <typename R1,
            typename R2,
            typename V1 = typename std::conditional<std::is_same<R1, MooseEnumItem>::value,
                                                    MultiMooseEnum,
                                                    std::vector<R1>>::type,
            typename V2 = typename std::conditional<std::is_same<R2, MooseEnumItem>::value,
                                                    MultiMooseEnum,
                                                    std::vector<R2>>::type>
  std::vector<std::pair<R1, R2>> get(const std::string & param1, const std::string & param2) const;

  /**
   * @returns list of all parameters
   */
  std::set<std::string> getParametersList() const;

  /**
   * Return list of controllable parameters
   */
  std::set<std::string> getControllableParameters() const;

  /**
   * Return names of parameters within a group.
   */
  std::set<std::string> getGroupParameters(const std::string & group) const;

  /**
   * Provide a set of reserved values for a parameter. These are values that are in addition
   * to the normal set of values the parameter can take.
   */
  void setReservedValues(const std::string & name, const std::set<std::string> & reserved);

  /**
   * Get a set of reserved parameter values.
   * Returns a set by value since we can return an empty set.
   */
  std::set<std::string> reservedValues(const std::string & name) const;

  /**
   * @return A string representing the location (i.e. filename,linenum) in the input text for the
   * block containing parameters for this object.
   */
  std::string blockLocation() const;

  /**
   * @return A string representing the full HIT parameter path from the input file (e.g.
   * "Mesh/foo") for the block containing parameters for this object.
   */
  std::string blockFullpath() const;

  /**
   * @return The hit node associated with setting the parameter \p param, if any
   */
  const hit::Node * getHitNode(const std::string & param) const;
  /**
   * Sets the hit node associated with the parameter \p param to \p node
   *
   * Is protected to be called by only the Builder via the SetParamHitNodeKey.
   */
  void setHitNode(const std::string & param, const hit::Node & node, const SetParamHitNodeKey);

  /**
   * @return A string representing the location in the input text the parameter originated from
   * (i.e. filename,linenum) for the given param
   */
  std::string inputLocation(const std::string & param) const;

  /**
   * @return A string representing the full HIT parameter path from the input file (e.g.
   * "Mesh/foo/bar" for param "bar") for the given param.
   */
  std::string paramFullpath(const std::string & param) const;

  /// generate error message prefix with parameter name and location (if available)
  std::string errorPrefix(const std::string & param) const;

  /**
   * @return A string representing the raw, unmodified token text for the given param.
   * This is only set if this parameter is parsed from hit
   */
  std::string rawParamVal(const std::string & param) const;

  /**
   * Informs this object that values for this parameter set from the input file or from the command
   * line should be ignored
   */
  template <typename T>
  void ignoreParameter(const std::string & name);

  /**
   * Whether to ignore the value of an input parameter set in the input file or from the command
   * line.
   */
  bool shouldIgnore(const std::string & name);

  /**
   * @returns True if the parameter with name \p name is of type T.
   */
  template <typename T>
  bool isType(const std::string & name) const;

  /**
   * Determine the actual variable name from the given variable \emph parameter name
   * @param var_param_name the name of the variable parameter, e.g. 'variable'
   * @param moose_object_with_var_param_name the name of the moose object holding the variable
   * parameter. Used for potential error messaging
   */
  std::string varName(const std::string & var_param_name,
                      const std::string & moose_object_with_var_param_name) const;

  /**
   * Rename a parameter and provide a new documentation string
   * @param old_name The old name of the parameter
   * @param new_name The new name of the parameter
   * @param new_docstring The new documentation string for the parameter
   *                      If left empty, uses the old docstring for the renamed parameter
   */
  void renameParam(const std::string & old_name,
                   const std::string & new_name,
                   const std::string & new_docstring);

  /**
   * Rename a coupled variable and provide a new documentation string
   * @param old_name The old name of the coupled variable
   * @param new_name The new name of the coupled variable
   * @param new_docstring The new documentation string for the coupled variable
   */
  void renameCoupledVar(const std::string & old_name,
                        const std::string & new_name,
                        const std::string & new_docstring);

  void deprecateParam(const std::string & old_name,
                      const std::string & new_name,
                      const std::string & removal_date);

  void deprecateCoupledVar(const std::string & old_name,
                           const std::string & new_name,
                           const std::string & removal_date);

  /**
   * Checks whether the provided name is a renamed parameter name. If so we return the 'new' name.
   * If not we return the incoming name
   * @param name The name to check for whether it is a renamed name
   * @return The new name if the incoming \p name is a renamed name, else \p name
   */
  std::string checkForRename(const std::string & name) const;

  /**
   * A wrapper around the \p Parameters base class method. Checks for parameter rename before
   * calling the base class method
   * @param name The name to query the parameter values map with
   * @return The parameter value corresponding to the (possibly renamed) name
   */
  template <typename T>
  const T & get(std::string_view name) const;

  /**
   * A wrapper around the \p Parameters base class method. Checks for parameter rename before
   * calling the base class method
   * @param name The name to query the parameter values map with
   * @return Whether there is a key in the parameter values map corresponding to the (possibly
   * renamed) name
   */
  template <typename T>
  bool have_parameter(std::string_view name) const;

  /**
   * A routine to transfer a parameter from one class' validParams to another
   * @param source_param The parameters list holding the param we would like to transfer
   * @param name The name of the parameter to transfer
   * @param new_description A new description of the parameter. If unspecified, uses the
   * source_params'
   */
  template <typename T>
  void transferParam(const InputParameters & source_param,
                     const std::string & name,
                     const std::string & new_name = "",
                     const std::string & new_description = "");

  /**
   * Return all the aliased names associated with \p param_name. The returned container will always
   * contain \p param_name itself. Other aliases in addition to \p param_name will include the base
   * class parameter name if \p param_name is the derived class parameter name, or deprecated names
   * that \p param_name is meant to replace.
   * @param param_name The name of the parameter that we want to lookup aliases for. This parameter
   * name must exist in our metadata and parameter names to values map, e.g. this parameter must
   * represent the derived class parameter name if a base class parameter has been renamed or the
   * blessed parameter name in situations where associated parameter names have been deprecated
   * @return All aliases which logically resolve-to/are-associated-with \p param_name, including \p
   * param_name itself
   */
  std::vector<std::string> paramAliases(const std::string & param_name) const;

  /**
   * @return The hit node that represents the syntax responsible for creating
   * these parameters, if any
   */
  const hit::Node * getHitNode() const { return _hit_node; }
  /**
   * Sets the hit node that represents the syntax responsible for creating
   * these parameters
   *
   * Is protected to be called by only the ActionFactory, Builder, and Factory
   * via the SetHitNodeKey.
   */
  void setHitNode(const hit::Node & node, const SetHitNodeKey) { _hit_node = &node; }

  /**
   * @return Whether or not finalize() has been called
   */
  bool isFinalized() const { return _finalized; }

  /**
   * @return The DataFileName path for the parameter \p name (if any).
   */
  std::optional<Moose::DataFileUtils::Path> queryDataFileNamePath(const std::string & name) const;

private:
  // Private constructor so that InputParameters can only be created in certain places.
  InputParameters();

  /**
   * Method to terminate the recursive setParameters definition
   */
  void setParameters() {}

  /**
   * Appends description of what a functor is to a doc string.
   */
  std::string appendFunctorDescription(const std::string & doc_string) const;

  /**
   * Private method for setting deprecated coupled variable documentation strings
   */
  void setDeprecatedVarDocString(const std::string & new_name, const std::string & doc_string);

  void renameParamInternal(const std::string & old_name,
                           const std::string & new_name,
                           const std::string & docstring,
                           const std::string & removal_date);

  void renameCoupledVarInternal(const std::string & old_name,
                                const std::string & new_name,
                                const std::string & docstring,
                                const std::string & removal_date);

  static void callMooseErrorHelper(const MooseBase & moose_base, const std::string & error);

  struct Metadata
  {
    std::string _doc_string;
    /// The developer-designated unit of the parameter for use in documentation
    std::string _doc_unit;
    /// The custom type that will be printed in the YAML dump for a parameter if supplied
    std::string _custom_type;
    /// The data pertaining to a command line parameter (empty if not a command line param)
    std::optional<CommandLineMetadata> _cl_data;
    /// The searched path information pertaining to a DataFileName parameter
    std::optional<Moose::DataFileUtils::Path> _data_file_name_path;
    /// The names of the parameters organized into groups
    std::string _group;
    /// The map of functions used for range checked parameters
    std::string _range_function;
    /// directions for auto build vectors (base_, 5) -> "base_0 base_1 base_2 base_3 base_4")
    std::pair<std::string, std::string> _autobuild_vecs;
    /// True for parameters that are required (i.e. will cause an abort if not supplied)
    bool _required = false;
    /**
     * Whether the parameter is either explicitly set or provided a default value when added
     * Note: We do not store MooseEnum names in valid params, instead we ask MooseEnums whether
     *       they are valid or not.
     */
    bool _valid = false;
    /// The set of parameters that will NOT appear in the the dump of the parser tree
    bool _is_private = false;
    bool _have_coupled_default = false;
    /// The default value for optionally coupled variables
    std::vector<Real> _coupled_default = {0};
    /// True if a parameters value was set by addParam, and not set again.
    bool _set_by_add_param = false;
    /// The reserved option names for a parameter
    std::set<std::string> _reserved_values;
    /// If non-empty, this parameter is deprecated.
    std::string _deprecation_message;
    /// Original location of parameter node; used for error messages
    const hit::Node * _hit_node;
    /// True if the parameters is controllable
    bool _controllable = false;
    /// Controllable execute flag restriction
    std::set<ExecFlagType> _controllable_flags;
    /// whether user setting of this parameter should be ignored
    bool _ignore = false;
  };

  Metadata & at(const std::string & param_name)
  {
    const auto param = checkForRename(param_name);
    if (_params.count(param) == 0)
      mooseError("param '", param, "' not present in InputParams");
    return _params[param];
  }
  const Metadata & at(const std::string & param_name) const
  {
    const auto param = checkForRename(param_name);
    if (_params.count(param) == 0)
      mooseError("param '", param, "' not present in InputParams");
    return _params.at(param);
  }

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

  /**
   * Make sure the parameter name doesn't have any invalid characters.
   */
  void checkParamName(const std::string & name) const;

  /**
   * This method is called when adding a Parameter with a default value, can be specialized for
   * non-matching types.
   */
  template <typename T, typename S>
  void setParamHelper(const std::string & name, T & l_value, const S & r_value);

  /**
   * Helper for all of the addCommandLineParam() calls, which sets up _cl_data in the metadata
   *
   * @param name The parameter name
   * @param syntax The parameter syntax
   * @param required Whether or not the parameter is required
   * @param value_required Whethre or not the parameter requires a value
   */
  template <typename T>
  void addCommandLineParamHelper(const std::string & name,
                                 const std::string & syntax,
                                 const bool required,
                                 const bool value_required);

  /// original location of input block (i.e. filename,linenum) - used for nice error messages.
  std::string _block_location;

  /// full HIT path of the block from the input file - used for nice error messages.
  std::string _block_fullpath;

  /// The actual parameter data. Each Metadata object contains attributes for the corresponding
  /// parameter.
  std::map<std::string, Metadata> _params;

  /// The coupled variables set
  std::set<std::string> _coupled_vars;

  /// The class description for the owning object. This string is used in many places including
  /// mouse-over events, and external documentation produced from the source code.
  std::string _class_description;

  /// The parameter is used to restrict types that can be built.  Typically this is used for
  /// MooseObjectAction derived Actions.
  std::vector<std::string> _buildable_types;

  /// The RelationshipManagers that this object may either build or require.
  /// The optional second argument may be supplied to "downgrade" the functionality of the corresponding
  /// relationship manager (e.g. An AlgebraicRelationshipManager could be only used as a
  /// GeometricRelationshipManager for a given simulation).
  std::vector<std::tuple<std::string,
                         Moose::RelationshipManagerType,
                         Moose::RelationshipManagerInputParameterCallback>>
      _buildable_rm_types;

  /// This parameter collapses one level of nesting in the syntax blocks.  It is used
  /// in conjunction with MooseObjectAction derived Actions.
  bool _collapse_nesting;

  /// This parameter hides derived MOOSE object types from appearing in syntax dumps
  bool _moose_object_syntax_visibility;

  /// Flag for disabling deprecated parameters message, this is used by applyParameters to avoid
  /// dumping messages.
  bool _show_deprecated_message;

  /// A flag for toggling the error message in the copy constructor.
  bool _allow_copy;

  /// A map from deprecated coupled variable names to the new blessed name
  std::unordered_map<std::string, std::string> _new_to_deprecated_coupled_vars;

  /// A map from base-class/deprecated parameter names to derived-class/blessed parameter names and
  /// the deprecation messages in the case that the "old" parameter name is a deprecated parameter
  /// name. The deprecation message will be empty if the "old" parameter name represents a base
  /// class parameter name
  std::map<std::string, std::pair<std::string, std::string>> _old_to_new_name_and_dep;

  /// A map from derived-class/blessed parameter names to associated base-class/deprecated parameter
  /// names
  std::multimap<std::string, std::string> _new_to_old_names;

  /// The hit node representing the syntax that created these parameters, if any
  const hit::Node * _hit_node;

  /// Whether or not we've called finalize() on these parameters yet
  bool _finalized;

  // These are the only objects allowed to _create_ InputParameters
  friend InputParameters emptyInputParameters();
  friend class InputParameterWarehouse;
  friend class Parser;
  // for the printInputFile function in the action warehouse
  friend class ActionWarehouse;
};

template <typename T>
void
InputParameters::setHelper(const std::string & /*name*/)
{
}

// Template and inline function implementations
template <typename T>
T &
InputParameters::set(const std::string & name_in, bool quiet_mode)
{
  const auto name = checkForRename(name_in);

  checkParamName(name);
  checkConsistentType<T>(name);

  T & result = this->Parameters::set<T>(name);

  if (quiet_mode)
    _params[name]._set_by_add_param = true;

  setHelper<T>(name);

  return result;
}

template <typename T, typename... Ts>
void
InputParameters::setParameters(const std::string & name,
                               const T & value,
                               Ts... extra_input_parameters)
{
  this->set<T>(name) = value;
  this->setParameters(extra_input_parameters...);
}

template <typename T, typename UP_T>
void
InputParameters::rangeCheck(const std::string & full_name,
                            const std::string & short_name,
                            InputParameters::Parameter<std::vector<T>> * param,
                            std::ostream & oss)
{
  mooseAssert(param, "Parameter is NULL");

  if (!isParamValid(short_name) || _params[short_name]._range_function.empty())
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
  if (fp.ParseAndDeduceVariables(_params[short_name]._range_function, vars) != -1) // -1 for success
  {
    oss << "Error parsing expression: " << _params[short_name]._range_function << '\n';
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
          oss << "Range checking empty vector: " << _params[short_name]._range_function << '\n';
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
          oss << "Error parsing expression: " << _params[short_name]._range_function << '\n';
          return;
        }
        std::istringstream iss(vars[j]);
        iss.seekg(short_name.size() + 1);

        size_t index;
        if (iss >> index && iss.eof())
        {
          if (index >= value.size())
          {
            oss << "Error parsing expression: " << _params[short_name]._range_function
                << "\nOut of range variable " << vars[j] << '\n';
            return;
          }
          parbuf[j] = value[index];
        }
        else
        {
          oss << "Error parsing expression: " << _params[short_name]._range_function
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
      oss << "Error evaluating expression: " << _params[short_name]._range_function << '\n';
      return;
    }

    if (!result)
    {
      oss << "Range check failed for parameter " << full_name
          << "\n\tExpression: " << _params[short_name]._range_function << "\n";
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

  if (!isParamValid(short_name) || _params[short_name]._range_function.empty())
    return;

  // Parse the expression
  FunctionParserBase<UP_T> fp;
  if (fp.Parse(_params[short_name]._range_function, short_name) != -1) // -1 for success
  {
    oss << "Error parsing expression: " << _params[short_name]._range_function << '\n';
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
    oss << "Error evaluating expression: " << _params[short_name]._range_function
        << "\nPerhaps you used the wrong variable name?\n";
    return;
  }

  if (!result)
    oss << "Range check failed for parameter " << full_name
        << "\n\tExpression: " << _params[short_name]._range_function << "\n\tValue: " << value[0]
        << '\n';
}

template <typename T>
T
InputParameters::getCheckedPointerParam(const std::string & name_in,
                                        const std::string & error_string) const
{
  const auto name = checkForRename(name_in);

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
  checkParamName(name);
  checkConsistentType<T>(name);

  InputParameters::insert<T>(name);
  auto & metadata = _params[name];
  metadata._required = true;
  if (std::is_same_v<T, MooseFunctorName>)
    metadata._doc_string = appendFunctorDescription(doc_string);
  else
    metadata._doc_string = doc_string;
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
  checkParamName(name);
  checkConsistentType<T>(name);

  T & l_value = InputParameters::set<T>(name);
  auto & metadata = _params[name];
  if (std::is_same_v<T, MooseFunctorName>)
    metadata._doc_string = appendFunctorDescription(doc_string);
  else
    metadata._doc_string = doc_string;

  // Set the parameter now
  setParamHelper(name, l_value, value);

  /* Indicate the default value, as set via addParam, is being used. The parameter is removed from
     the list whenever
     it changes, see set_attributes */
  metadata._set_by_add_param = true;
}

template <typename T>
void
InputParameters::addParam(const std::string & name, const std::string & doc_string)
{
  checkParamName(name);
  checkConsistentType<T>(name);

  InputParameters::insert<T>(name);
  if (std::is_same_v<T, MooseFunctorName>)
    _params[name]._doc_string = appendFunctorDescription(doc_string);
  else
    _params[name]._doc_string = doc_string;
}

template <typename T, typename S>
void
InputParameters::setParamHelper(const std::string & /*name*/, T & l_value, const S & r_value)
{
  l_value = r_value;
}

template <typename T>
void
InputParameters::addCommandLineParamHelper(const std::string & name,
                                           const std::string & syntax,
                                           const bool required,
                                           const bool value_required)
{
  static_assert(isValidCommandLineType<T>::value,
                "This type is not a supported command line parameter type. See "
                "CommandLine::populateCommandLineParams to add it as a supported type.");

  auto & cl_data = at(name)._cl_data;
  cl_data = CommandLineMetadata();

  // Split up the syntax by whitespace
  std::vector<std::string> syntax_split;
  MooseUtils::tokenize(syntax, syntax_split, 1, " \t\n\v\f\r");

  // Set the single syntax string as the combined syntax with removed whitespace
  cl_data->syntax = MooseUtils::stringJoin(syntax_split);
  mooseAssert(cl_data->syntax.size(), "Empty token");

  // Set the switches; only parse those that begin with "-" as we also
  // provide examples within the syntax
  for (const auto & val : syntax_split)
    if (val.rfind("-", 0) == 0)
    {
      if (!std::regex_search(val, std::regex("^\\-+[a-zA-Z]")))
        mooseError("The switch '",
                   val,
                   "' for the command line parameter '",
                   name,
                   "' is invalid. It must begin with an alphabetical character.");

      cl_data->switches.push_back(val);
      libMesh::add_command_line_name(val);
    }

  cl_data->required = required;
  cl_data->global = false;

  // No arguments needed for a boolean parameter
  if constexpr (std::is_same_v<T, bool>)
  {
    (void)value_required; // purposely unused; doesn't take a value
    cl_data->argument_type = CommandLineMetadata::ArgumentType::NONE;
  }
  // MooseEnums require a value
  else if constexpr (std::is_same_v<T, MooseEnum>)
  {
    (void)value_required; // purposely unused; always required
    cl_data->argument_type = CommandLineMetadata::ArgumentType::REQUIRED;
  }
  // The user didn't specify a default, so a value is required
  else if (value_required)
    cl_data->argument_type = CommandLineMetadata::ArgumentType::REQUIRED;
  // Otherwise, it's optional (user specified a default)
  else
    cl_data->argument_type = CommandLineMetadata::ArgumentType::OPTIONAL;
}

template <typename T>
void
InputParameters::addRequiredRangeCheckedParam(const std::string & name,
                                              const std::string & parsed_function,
                                              const std::string & doc_string)
{
  addRequiredParam<T>(name, doc_string);
  _params[name]._range_function = parsed_function;
}

template <typename T>
void
InputParameters::addRangeCheckedParam(const std::string & name,
                                      const T & value,
                                      const std::string & parsed_function,
                                      const std::string & doc_string)
{
  addParam<T>(name, value, doc_string);
  _params[name]._range_function = parsed_function;
}

template <typename T>
void
InputParameters::addRangeCheckedParam(const std::string & name,
                                      const std::string & parsed_function,
                                      const std::string & doc_string)
{
  addParam<T>(name, doc_string);
  _params[name]._range_function = parsed_function;
}

template <typename T>
void
InputParameters::addRequiredCustomTypeParam(const std::string & name,
                                            const std::string & custom_type,
                                            const std::string & doc_string)
{
  addRequiredParam<T>(name, doc_string);
  _params[name]._custom_type = custom_type;
}

template <typename T>
void
InputParameters::addCustomTypeParam(const std::string & name,
                                    const T & value,
                                    const std::string & custom_type,
                                    const std::string & doc_string)
{
  addParam<T>(name, value, doc_string);
  _params[name]._custom_type = custom_type;
}

template <typename T>
void
InputParameters::addCustomTypeParam(const std::string & name,
                                    const std::string & custom_type,
                                    const std::string & doc_string)
{
  addParam<T>(name, doc_string);
  _params[name]._custom_type = custom_type;
}

template <typename T>
void
InputParameters::addDeprecatedCustomTypeParam(const std::string & name,
                                              const std::string & custom_type,
                                              const std::string & doc_string,
                                              const std::string & deprecation_message)
{
  _show_deprecated_message = false;
  addParam<T>(name, doc_string);
  auto & metadata = _params[name];
  metadata._custom_type = custom_type;

  metadata._deprecation_message = deprecation_message;
  _show_deprecated_message = true;
}

template <typename T>
void
InputParameters::addPrivateParam(const std::string & name)
{
  checkParamName(name);
  checkConsistentType<T>(name);

  InputParameters::insert<T>(name);
  _params[name]._is_private = true;
}

template <typename T>
void
InputParameters::addPrivateParam(const std::string & name, const T & value)
{
  checkParamName(name);
  checkConsistentType<T>(name);

  InputParameters::set<T>(name) = value;
  auto & metadata = _params[name];
  metadata._is_private = true;
  metadata._set_by_add_param = true;
}

template <typename T>
void
InputParameters::addRequiredCommandLineParam(const std::string & name,
                                             const std::string & syntax,
                                             const std::string & doc_string)
{
  static_assert(!std::is_same_v<T, bool>, "Cannot be used for a bool");

  addRequiredParam<T>(name, doc_string);
  addCommandLineParamHelper<T>(name, syntax, /* required = */ true, /* value_required = */ true);
}

template <typename T>
void
InputParameters::addCommandLineParam(const std::string & name,
                                     const std::string & syntax,
                                     const std::string & doc_string)
{
  static_assert(!std::is_same_v<T, MooseEnum>,
                "addCommandLineParam() without a value cannot be used with a MooseEnum because a "
                "MooseEnum requires initialization");

  auto constexpr is_bool = std::is_same_v<T, bool>;
  if constexpr (is_bool)
  {
    addParam<T>(name, false, doc_string);
  }
  else
  {
    addParam<T>(name, doc_string);
  }

  addCommandLineParamHelper<T>(
      name, syntax, /* required = */ false, /* value_required = */ !is_bool);
}

template <typename T>
void
InputParameters::addCommandLineParam(const std::string & name,
                                     const std::string & syntax,
                                     const T & value,
                                     const std::string & doc_string)
{
  if constexpr (std::is_same_v<T, bool>)
    mooseAssert(!value, "Default for bool must be false");

  addParam<T>(name, value, doc_string);
  addCommandLineParamHelper<T>(name, syntax, /* required = */ false, /* value_required = */ true);
}

template <typename T>
void
InputParameters::addOptionalValuedCommandLineParam(const std::string & name,
                                                   const std::string & syntax,
                                                   const T & value,
                                                   const std::string & doc_string)
{
  mooseAssert(name == "mesh_only" || name == "recover" || name == "run",
              "Not supported for new parameters");
  static_assert(!std::is_same_v<T, bool>, "Cannot be used for a bool (does not take a value)");
  addParam<T>(name, value, doc_string);
  addCommandLineParamHelper<T>(name, syntax, /* required = */ false, /* value_required = */ false);
}

template <typename T>
void
InputParameters::checkConsistentType(const std::string & name_in) const
{
  const auto name = checkForRename(name_in);

  // If we don't currently have the Parameter, can't be any inconsistency
  InputParameters::const_iterator it = _values.find(name);
  if (it == _values.end())
    return;

  // Now, if we already have the Parameter, but it doesn't have the
  // right type, throw an error.
  if (!this->Parameters::have_parameter<T>(name))
    mooseError("Attempting to set parameter \"",
               name,
               "\" with type (",
               libMesh::demangle(typeid(T).name()),
               ")\nbut the parameter already exists as type (",
               it->second->type(),
               ")");
}

template <typename T>
void
InputParameters::suppressParameter(const std::string & name_in)
{
  const auto name = checkForRename(name_in);
  if (!this->have_parameter<T>(name))
    mooseError("Unable to suppress nonexistent parameter: ", name);

  auto & metadata = _params[name];
  metadata._required = false;
  metadata._is_private = true;
  metadata._controllable = false;
}

template <typename T>
void
InputParameters::ignoreParameter(const std::string & name_in)
{
  const auto name = checkForRename(name_in);
  suppressParameter<T>(name);
  _params[name]._ignore = true;
}

template <typename T>
void
InputParameters::makeParamRequired(const std::string & name_in)
{
  const auto name = checkForRename(name_in);

  if (!this->have_parameter<T>(name))
    mooseError("Unable to require nonexistent parameter: ", name);

  _params[name]._required = true;
}

template <typename T>
void
InputParameters::makeParamNotRequired(const std::string & name_in)
{
  const auto name = checkForRename(name_in);

  if (!this->have_parameter<T>(name))
    mooseError("Unable to un-require nonexistent parameter: ", name);

  _params[name]._required = false;
}

template <typename T>
void
InputParameters::addDeprecatedParam(const std::string & name,
                                    const T & value,
                                    const std::string & doc_string,
                                    const std::string & deprecation_message)
{
  _show_deprecated_message = false;
  mooseAssert(!_old_to_new_name_and_dep.count(name),
              "Attempting to deprecate via addDeprecatedParam the parameter, '"
                  << name << "', already deprecated via deprecateParam or renamed via renameParam");
  addParam<T>(name, value, doc_string);

  _params[name]._deprecation_message = deprecation_message;
  _show_deprecated_message = true;
}

template <typename T>
void
InputParameters::addDeprecatedParam(const std::string & name,
                                    const std::string & doc_string,
                                    const std::string & deprecation_message)
{
  _show_deprecated_message = false;
  mooseAssert(!_old_to_new_name_and_dep.count(name),
              "Attempting to deprecate via addDeprecatedParam the parameter, '"
                  << name << "', already deprecated via deprecateParam or renamed via renameParam");
  addParam<T>(name, doc_string);

  _params[name]._deprecation_message = deprecation_message;
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
void InputParameters::addRequiredParam<std::vector<MultiMooseEnum>>(
    const std::string & name,
    const std::vector<MultiMooseEnum> & moose_enums,
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
void InputParameters::addParam<std::vector<MultiMooseEnum>>(const std::string & /*name*/,
                                                            const std::string & /*doc_string*/);

template <>
void
InputParameters::addRequiredParam<std::vector<MultiMooseEnum>>(const std::string & /*name*/,
                                                               const std::string & /*doc_string*/);

template <>
void InputParameters::addPrivateParam<MooseEnum>(const std::string & /*name*/);

template <>
void InputParameters::addPrivateParam<MultiMooseEnum>(const std::string & /*name*/);

template <>
void InputParameters::addDeprecatedParam<MooseEnum>(const std::string & /*name*/,
                                                    const std::string & /*doc_string*/,
                                                    const std::string & /*deprecation_message*/);

template <>
void
InputParameters::addDeprecatedParam<MultiMooseEnum>(const std::string & /*name*/,
                                                    const std::string & /*doc_string*/,
                                                    const std::string & /*deprecation_message*/);

template <>
void InputParameters::addDeprecatedParam<std::vector<MooseEnum>>(
    const std::string & /*name*/,
    const std::string & /*doc_string*/,
    const std::string & /*deprecation_message*/);

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

template <>
void InputParameters::setParamHelper<MooseFunctorName, Real>(const std::string & /*name*/,
                                                             MooseFunctorName & l_value,
                                                             const Real & r_value);

template <>
void InputParameters::setParamHelper<MooseFunctorName, int>(const std::string & /*name*/,
                                                            MooseFunctorName & l_value,
                                                            const int & r_value);

// TODO: pass MooseBase here instead and use it in objects
template <typename T>
const T &
InputParameters::getParamHelper(const std::string & name_in,
                                const InputParameters & pars,
                                const T *,
                                const MooseBase * moose_base /* = nullptr */)
{
  const auto name = pars.checkForRename(name_in);

  if (!pars.isParamValid(name))
  {
    std::stringstream err;
    err << "The parameter \"" << name << "\" is being retrieved before being set.";
    if (moose_base)
      callMooseErrorHelper(*moose_base, err.str());
    else
      mooseError(err.str());
  }

  return pars.get<T>(name);
}

// Declare specializations so we don't fall back on the generic
// implementation, but the definition will be in InputParameters.C so
// we won't need to bring in *MooseEnum header files here.
template <>
const MooseEnum &
InputParameters::getParamHelper<MooseEnum>(const std::string & name,
                                           const InputParameters & pars,
                                           const MooseEnum *,
                                           const MooseBase * moose_base /* = nullptr */);

template <>
const MultiMooseEnum &
InputParameters::getParamHelper<MultiMooseEnum>(const std::string & name,
                                                const InputParameters & pars,
                                                const MultiMooseEnum *,
                                                const MooseBase * moose_base /* = nullptr */);

template <typename R1, typename R2, typename V1, typename V2>
std::vector<std::pair<R1, R2>>
InputParameters::get(const std::string & param1_in, const std::string & param2_in) const
{
  const auto param1 = checkForRename(param1_in);
  const auto param2 = checkForRename(param2_in);

  const auto & v1 = get<V1>(param1);
  const auto & v2 = get<V2>(param2);

  auto controllable = getControllableParameters();
  if (controllable.count(param1) || controllable.count(param2))
    mooseError(errorPrefix(param1),
               " and/or ",
               errorPrefix(param2) +
                   " are controllable parameters and cannot be retireved using "
                   "the MooseObject::getParam/InputParameters::get methods for pairs");

  if (v1.size() != v2.size())
    mooseError("Vector parameters ",
               errorPrefix(param1),
               "(size: ",
               v1.size(),
               ") and " + errorPrefix(param2),
               "(size: ",
               v2.size(),
               ") are of different lengths \n");

  std::vector<std::pair<R1, R2>> parameter_pairs;
  auto i1 = v1.begin();
  auto i2 = v2.begin();
  for (; i1 != v1.end() && i2 != v2.end(); ++i1, ++i2)
    parameter_pairs.emplace_back(std::make_pair(*i1, *i2));
  return parameter_pairs;
}

InputParameters emptyInputParameters();

template <typename T>
bool
InputParameters::isType(const std::string & name_in) const
{
  const auto name = checkForRename(name_in);

  if (!_params.count(name))
    mooseError("Parameter \"", name, "\" is not valid.");
  return have_parameter<T>(name);
}

template <typename T>
const T &
InputParameters::get(std::string_view name_in) const
{
  const auto name = checkForRename(std::string(name_in));

  return Parameters::get<T>(name);
}

template <typename T>
bool
InputParameters::have_parameter(std::string_view name_in) const
{
  const auto name = checkForRename(std::string(name_in));

  return Parameters::have_parameter<T>(name);
}

template <typename T>
void
InputParameters::transferParam(const InputParameters & source_params,
                               const std::string & name_in,
                               const std::string & new_name,
                               const std::string & new_description)
{
  const auto name = source_params.checkForRename(std::string(name_in));
  const auto p_name = new_name.empty() ? name_in : new_name;
  if (!source_params.have_parameter<T>(name) && !source_params.hasCoupledValue(name))
    mooseError("The '",
               name_in,
               "' parameter could not be transferred because it does not exist with type '",
               MooseUtils::prettyCppType<T>(),
               "' in the source parameters");
  if (name != name_in)
    mooseWarning("The transferred parameter " + name_in + " is deprecated in favor of " + name +
                 " in the source parameters. The new name should likely be used for the parameter "
                 "transfer instead.");
  const std::string description =
      new_description.empty() ? source_params.getDescription(name) : new_description;

  if (source_params.isParamRequired(name))
  {
    // Check for a variable parameter
    if (source_params.hasCoupledValue(name))
      addRequiredCoupledVar(p_name, description);
    // Enums parameters have a default list of options
    else if constexpr (std::is_same_v<MooseEnum, T> || std::is_same_v<MultiMooseEnum, T>)
      addRequiredParam<T>(p_name, source_params.get<T>(name), description);
    else if (source_params.isRangeChecked(name))
      addRequiredRangeCheckedParam<T>(
          p_name, source_params.rangeCheckedFunction(name), description);
    else
      addRequiredParam<T>(p_name, description);
  }
  else
  {
    // Check for a variable parameter
    if (source_params.hasCoupledValue(name))
    {
      if (!source_params.hasDefaultCoupledValue(name))
        addCoupledVar(p_name, description);
      else if (source_params.numberDefaultCoupledValues(name) == 1)
        addCoupledVar(p_name, source_params.defaultCoupledValue(name), description);
      else
      {
        std::vector<Real> coupled_values;
        for (const auto i : libMesh::make_range(source_params.numberDefaultCoupledValues(name)))
          coupled_values.push_back(source_params.defaultCoupledValue(name, i));
        addCoupledVar(p_name, coupled_values, description);
      }
    }
    else if (source_params.isRangeChecked(name))
    {
      if (source_params.hasDefault(name))
        addRangeCheckedParam<T>(p_name,
                                source_params.get<T>(name),
                                source_params.rangeCheckedFunction(name),
                                description);
      else
        addRangeCheckedParam<T>(p_name, source_params.rangeCheckedFunction(name), description);
    }
    else if constexpr (std::is_same_v<MooseEnum, T> || std::is_same_v<MultiMooseEnum, T>)
      addParam<T>(p_name, source_params.get<T>(name), description);
    else
    {
      if (source_params.hasDefault(name))
        addParam<T>(p_name, source_params.get<T>(name), description);
      else
        addParam<T>(p_name, description);
    }
  }

  // Copy other attributes
  if (source_params.isPrivate(name))
    _params[p_name]._is_private = true;
  if (source_params.isControllable(name))
    _params[p_name]._controllable = true;
}
