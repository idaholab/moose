//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseError.h"
#include "Conversion.h"
#include "MooseEnum.h"
#include "InputParameters.h"
#include "MooseUtils.h"

#include <list>
#include <string>
#include <map>
#include <memory>
#include <set>
#include <optional>
#include <regex>

/**
 * This class wraps provides and tracks access to command line parameters.
 */
class CommandLine
{
public:
  using ArgumentType = InputParameters::CommandLineMetadata::ArgumentType;
  /**
   * Stores name/value pairs for each command line argument
   */
  struct Entry
  {
    /// The name, i.e, ["-foo=bar"] -> "-foo" or ["--foo", "bar"] -> "--foo"
    std::string name;
    /// The name of the subapp, if any (with subapp:something=value syntax)
    std::optional<std::string> subapp_name;
    /// The value, i.e. ["-foo=bar"] -> "-foo" or ["-foo"] -> empty, if any
    std::optional<std::string> value;
    /// The string that separates the value, if a value exists (space or =)
    std::optional<std::string> value_separator;
    /// The raw arguments that represent these split values
    std::vector<std::string> raw_args;
    /// Whether or not this set of arguments was used
    bool used = false;
    /// Whether or not this parameter is global (passed to MultiApps)
    bool global = false;
    /// Whether or not this parameter is recognized as a HIT parameter
    bool hit_param = false;
  };

  /**
   * Stores information pertaining to a command line InputParameter
   */
  struct CommandLineParam
  {
    /// The description (doc string) for the parameter
    std::string description;
    /// The command line metadata for the parameter
    InputParameters::CommandLineMetadata metadata;
  };

  CommandLine();
  CommandLine(int argc, char * argv[]);
  CommandLine(const std::vector<std::string> & args);
  virtual ~CommandLine();

  /**
   * Adds arguments from raw argc and argv
   */
  void addArguments(int argc, char * argv[]);
  /**
   * Adds a single argument
   */
  void addArgument(const std::string & arg);
  /**
   * Adds arguments from a vector
   */
  void addArguments(const std::vector<std::string> & args);

  /**
   * @return Whether or not the raw argument \p arg is contained
   */
  bool hasArgument(const std::string & arg) const;

  /**
   * Removes an argument that must exist
   */
  void removeArgument(const std::string & arg);

  /**
   * Performs the parsing, which is the combining of arguments into [name, value] pairs.
   *
   * Must be called before extracing anything from the CommandLine.
   */
  void parse();

  /**
   * @return Whether or not the arguments have been parsed
   */
  bool hasParsed() const { return _has_parsed; }

  /**
   * Initializes a new CommandLine for a subapp with a MultiApp named \p multiapp_name
   * and a subapp named \p subapp_name.
   *
   * The arguments \p input_cli_args are the additional arguments to apply to the subapp,
   * such as those that have been specified in the MultiApp params (cli_args param).
   *
   * This will apply all global parameters from this parent application and all HIT CLI
   * parameters that have the same.
   */
  std::unique_ptr<CommandLine>
  initSubAppCommandLine(const std::string & multiapp_name,
                        const std::string & subapp_name,
                        const std::vector<std::string> & input_cli_args);

  /**
   * @return The parsed HIT command line parameters per the command line arguments.
   *
   * This will also mark all found HIT parameters as used.
   */
  std::string buildHitParams();

  /**
   * @return The raw argv arguments as a vector
   */
  const std::vector<std::string> & getArguments() { return _argv; }

  /**
   * Populates the command line input parameters from \p params.
   *
   * Will throw errors when conversions fail and may combine entires in
   * _entries if some are found that can be combined.
   */
  void populateCommandLineParams(InputParameters & params);

  /**
   * @return An iterator to the beginning of the options
   */
  auto begin() const { return _entries.begin(); }
  /**
   * @return An iterator to the end of the options
   */
  auto end() const { return _entries.end(); }

  /**
   * @return The combined argument entries
   */
  const std::list<Entry> & getEntries() const;

  /**
   * @return The executable name.
   */
  std::string getExecutableName() const;

  /**
   * @return The exectuable name base (the name without the -[opt,oprof,devel,dbg])
   */
  std::string getExecutableNameBase() const;

  /**
   * Print the usage info for this command line
   */
  void printUsage() const;

  /**
   * Returns the HIT command line arguments that are not used.
   *
   * The HIT command line arguments are considered used when they are accumulated
   * in buildHitParams().
   *
   * The commmunicator is needed because we need to sync this in parallel due to
   * the fact that sub apps could only be created on a subset of processors.
   */
  std::vector<std::string> unusedHitParams(const Parallel::Communicator & comm) const;

  /**
   * @return The entry iterator for the command line arguments for the command line input
   * parameter with name \p name, if any.
   */
  std::list<Entry>::const_iterator findCommandLineParam(const std::string & name) const;

  /**
   * @return A formatted representation of the given command line entry
   */
  std::string formatEntry(const Entry & entry) const;

private:
  /**
   * @return The combined argument entries
   */
  std::list<Entry> & getEntries();

  /**
   * Sets an InputParameters command line option at \p value.
   *
   * Will report an error if string -> value conversions fail or if the
   * parameter requires a value and one was not found.
   *
   * @param entry_it Iterator to the Entry object that we're extracting from
   * @param param The internal metadata for the command line parameter
   * @param cli_switch The command line switch for the parameter (-t, --timing, etc)
   * @param value The value that we want to fill into
   */
  template <typename T>
  void setCommandLineParam(std::list<Entry>::iterator entry_it,
                           const CommandLineParam & param,
                           const std::string & cli_switch,
                           T & value);

  /**
   * @return The entry iterator for the command line arguments for the command line input
   * parameter with name \p name, if any.
   */
  std::list<Entry>::iterator findCommandLineParam(const std::string & name);

  /// Storage for the raw argv
  std::vector<std::string> _argv;

  /// The parsed command line entries (arguments split into name value pairs)
  /// This is a list because it is necessary to combine Entry objects later on
  std::list<Entry> _entries;

  /// The command line parameters, added by populateCommandLineParams()
  std::map<std::string, CommandLineParam> _command_line_params;

  /// Whether or not the Parser has parsed yet
  bool _has_parsed = false;
  /// Whether or not command line parameters have been populated
  bool _command_line_params_populated = false;
};

template <typename T>
void
CommandLine::setCommandLineParam(std::list<CommandLine::Entry>::iterator entry_it,
                                 const CommandLineParam & param,
                                 const std::string & cli_switch,
                                 T & value)
{
  auto & entry = *entry_it;
  const auto required = param.metadata.argument_type == ArgumentType::REQUIRED;

  // Mark this entry as used
  entry.used = true;

  // Option doesn't have any arguments (boolean)
  if constexpr (std::is_same_v<bool, T>)
  {
    mooseAssert(param.metadata.argument_type == ArgumentType::NONE, "Incorrect argument type");

    if (entry.value)
      mooseError("The command line option '",
                 cli_switch,
                 "' is a boolean and does not support a value but the value '",
                 *entry.value,
                 "' was provided.\nDoc string: ",
                 param.description);
    value = true;
  }
  // Option has arguments
  else
  {
    mooseAssert(param.metadata.argument_type != ArgumentType::NONE, "Incorrect argument type");

    // Helper for setting a value depending on its type and also throwing a useful
    // error when the conversion fails
    const auto set_value = [&cli_switch](const std::string & from, auto & value)
    {
      using type = typename std::remove_reference<decltype(value)>::type;

      // Keep track of and change the throw on error characteristics so that
      // we can catch parsing errors for the argument
      const auto throw_on_error_orig = Moose::_throw_on_error;
      Moose::_throw_on_error = true;

      try
      {
        if constexpr (std::is_same_v<type, std::string> || std::is_same_v<type, MooseEnum>)
          value = from;
        else
          value = MooseUtils::convert<type>(from, true);
      }
      catch (std::exception & e)
      {
        Moose::_throw_on_error = throw_on_error_orig;
        mooseError("While parsing command line option '",
                   cli_switch,
                   "' with value '",
                   from,
                   "':\n\n",
                   e.what());
      }

      Moose::_throw_on_error = throw_on_error_orig;
    };

    // If a value doesn't exist, check the next argument to see if it
    // would work. This is needed for when we have argument values that
    // have = signs that get split. For example:
    // "--required-capabilities 'petsc>=3.11'" would get split into:
    //   - "--required-capabilities" with no value
    //   - "petsc>" with value "3.11"
    // which we want to re-combine into
    //   - "--required-capabilities" with value "petsc>=3.11"
    if (!entry.value)
    {
      auto next_entry_it = std::next(entry_it);
      if (next_entry_it != _entries.end())
      {
        const auto & next_entry = *next_entry_it;
        if (!next_entry.used &&                    // isn't already used
            (required || !next_entry.hit_param) && // if required, get the last value. if not, get
                                                   // it if it's not a hit param
            !MooseUtils::beginsWith(next_entry.name, "-") && // doesn't start with a -
            next_entry.value_separator &&                    // has a separator
            *next_entry.value_separator == "=" &&            // that separator is =
            next_entry.value)                                // and has a value after the =
        {
          const auto & next_entry = *next_entry_it;
          // Merge with the next Entry object and remove said next object
          entry.value = next_entry.name + *next_entry.value_separator + *next_entry.value;
          entry.raw_args.insert(
              entry.raw_args.end(), next_entry.raw_args.begin(), next_entry.raw_args.end());
          _entries.erase(next_entry_it);
        }
      }
    }

    // If we have a value, set the parameter to it
    if (entry.value)
    {
      // For vector<string>, we need to unpack the values
      if constexpr (std::is_same_v<T, std::vector<std::string>>)
      {
        std::vector<std::string> split_values;
        MooseUtils::tokenize(*entry.value, split_values, 1, " ");
        value.resize(split_values.size());
        for (const auto i : index_range(split_values))
          set_value(split_values[i], value[i]);
      }
      // For everything else, we can set them directly
      else
        set_value(*entry.value, value);
    }
    // No value, but one is required
    else if (param.metadata.argument_type == ArgumentType::REQUIRED)
      mooseError("The command line option '",
                 cli_switch,
                 "' requires a value and one was not provided.\nDoc string: ",
                 param.description);
  }
}
