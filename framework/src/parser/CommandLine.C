//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CommandLine.h"

// C++ includes
#include <iomanip>
#include <optional>

#include "pcrecpp.h"

#include "hit.h"

#include "libmesh/libmesh.h"
#include "libmesh/simple_range.h"
#include "libmesh/parallel_algebra.h"
#include "libmesh/parallel_sync.h"

#include "PetscSupport.h"

CommandLine::CommandLine() {}
CommandLine::CommandLine(int argc, char * argv[]) { addArguments(argc, argv); }
CommandLine::CommandLine(const std::vector<std::string> & args) { addArguments(args); }

void
CommandLine::addArguments(int argc, char * argv[])
{
  for (int i = 0; i < argc; i++)
    addArgument(argv[i]);
}

void
CommandLine::addArgument(const std::string & arg)
{
  mooseAssert(!hasParsed(), "Has already parsed");
  _argv.push_back(arg);
}

void
CommandLine::addArguments(const std::vector<std::string> & args)
{
  for (const auto & arg : args)
    addArgument(arg);
}

bool
CommandLine::hasArgument(const std::string & arg) const
{
  return std::find(_argv.begin(), _argv.end(), arg) != _argv.end();
}

void
CommandLine::removeArgument(const std::string & arg)
{
  mooseAssert(!hasParsed(), "Has already parsed");
  auto it = std::find(_argv.begin(), _argv.end(), arg);
  if (it == _argv.end())
    mooseError("CommandLine::removeArgument(): The argument '", arg, "' does not exist");
  _argv.erase(it);
}

void
CommandLine::parse()
{
  mooseAssert(!hasParsed(), "Has already parsed");
  mooseAssert(_entries.empty(), "Should be empty");

  // Whether or not we have a entry that accepts values
  bool has_value_accepting_entry = false;

  // Helper for adding an entry
  auto add_entry = [this](const auto & name) -> Entry &
  {
    auto & entry = _entries.emplace_back();
    entry.name = name;
    return entry;
  };

  // Work through each argument
  for (const auto i : index_range(_argv))
  {
    const auto & arg = _argv[i];
    const auto begins_with_dash = MooseUtils::beginsWith(arg, "-");
    std::string subapp_prefix, subapp_name, hit_path, hit_value;

    // MultiApp syntax with a non-hit option
    if (!begins_with_dash && std::regex_search(arg, std::regex("^(([^\\s\n\t[\\]\\/=#&:]+):)[-]")))
    {
      mooseError("The MultiApp command line argument '",
                 arg,
                 "' sets a command line option.\nMultiApp command line arguments can only be "
                 "used for setting HIT parameters.");
    }
    // Match HIT CLI syntax (including for multiapps with the prefix)
    // For subapp hit cli syntax (i.e, <subname>:<value)), we will store the
    // subapp names to be stripped away in initSubAppCommandLine() as they
    // are passed down
    else if (!begins_with_dash &&
             pcrecpp::RE("(([^\\s\n\t[\\]\\/=#&:]+)?:)?((?:[^\\s\n\t[\\]=#&]+\\/"
                         ")?(?:[^\\s\n\t=#$'\"]+))=([^#]+)?")
                 .FullMatch(arg, &subapp_prefix, &subapp_name, &hit_path, &hit_value))
    {
      auto & entry = add_entry(hit_path);
      if (subapp_prefix.size())
      {
        if (subapp_name.empty()) // :param=value; means apply to all
          entry.global = true;
        else
          entry.subapp_name = subapp_name;
      }
      entry.value = MooseUtils::removeExtraWhitespace(hit_value);
      entry.value_separator = "=";
      entry.raw_args.push_back(arg);
      entry.hit_param = true;
      has_value_accepting_entry = false;
    }
    // Has an = sign in it, so we have a name=value (non-HIT)
    else if (const auto find_equals = arg.find("="); find_equals != std::string::npos)
    {
      const auto begin = arg.substr(0, find_equals);
      const auto end = arg.substr(find_equals + 1);
      auto & entry = add_entry(begin);
      entry.value = MooseUtils::removeExtraWhitespace(end);
      entry.value_separator = "=";
      entry.raw_args.push_back(arg);
      has_value_accepting_entry = false;
    }
    // Begins with dash(es) and a character, so a new argument. We pass on
    // everything else that starts with a dash as a value, so the error
    // will be associated with the value before it
    else if (std::regex_search(arg, std::regex("^\\-+[a-zA-Z]")))
    {
      auto & entry = add_entry(arg);
      entry.raw_args.push_back(arg);
      has_value_accepting_entry = true;
    }
    // Should be tagging on a value to the previous argument
    else
    {
      // First one is the executable
      if (i == 0)
      {
        auto & entry = add_entry(arg);
        entry.raw_args.push_back(arg);
        continue;
      }

      // Throw an error if this a value and we don't have anything to apply it to
      if (!has_value_accepting_entry)
      {
        std::stringstream err;
        err << "The command line argument '" << arg
            << "' is not applied to an option and is not a HIT parameter.";
        // Maybe they meant to apply it to the previous thing
        // Example: "-i foo.i bar.i" would suggest "-i 'foo.i bar.i'"
        if (i > 0 && _entries.back().value)
        {
          err << "\n\nDid you mean to combine this argument with the previous argument, such "
                 "as:\n\n  "
              << _entries.back().name << *_entries.back().value_separator << "'"
              << *_entries.back().value << " " << arg << "'\n";
        }
        mooseError(err.str());
      }

      auto & entry = _entries.back();
      if (entry.value)
        *entry.value += " " + MooseUtils::removeExtraWhitespace(arg);
      else
        entry.value = MooseUtils::removeExtraWhitespace(arg);
      entry.value_separator = " ";
      entry.raw_args.push_back(arg);
    }
  }

  _has_parsed = true;
}

const std::list<CommandLine::Entry> &
CommandLine::getEntries() const
{
  mooseAssert(hasParsed(), "Has not parsed");
  return _entries;
}

std::list<CommandLine::Entry> &
CommandLine::getEntries()
{
  mooseAssert(hasParsed(), "Has not parsed");
  return _entries;
}

CommandLine::~CommandLine() {}

std::unique_ptr<CommandLine>
CommandLine::initSubAppCommandLine(const std::string & multiapp_name,
                                   const std::string & subapp_name,
                                   const std::vector<std::string> & input_cli_args)
{
  mooseAssert(MooseUtils::beginsWith(subapp_name, multiapp_name),
              "Name for the subapp should begin with the multiapp");

  std::vector<std::string> subapp_args;

  // Start with the arguments from the input file; we want these to take the
  // lowest priority so we put them first. Also trim extra whitespace
  for (const auto & arg : input_cli_args)
    subapp_args.push_back(MooseUtils::removeExtraWhitespace(arg));

  // Pull out all of the arguments that are relevant to this multiapp from the parent
  // Note that the 0th argument of the main app, i.e., the name used to invoke the program,
  // is neither a global entry nor a subapp entry and, as such, won't be passed on
  for (auto & entry : as_range(getEntries().begin(), getEntries().end()))
    if (entry.global || (entry.subapp_name && (*entry.subapp_name == multiapp_name ||
                                               *entry.subapp_name == subapp_name)))
    {
      if (entry.hit_param)
      {
        // Append : to the beginning if this is global and should be passed to all
        const std::string prefix = entry.global ? ":" : "";
        // Apply the param, but without the subapp name
        subapp_args.push_back(prefix + entry.name + *entry.value_separator + *entry.value);
        // Mark this entry as used as a child has consumed it
        entry.used = true;
      }
      else
        subapp_args.insert(subapp_args.end(), entry.raw_args.begin(), entry.raw_args.end());
    }

  return std::make_unique<CommandLine>(subapp_args);
}

std::vector<std::string>
CommandLine::buildHitParams()
{
  mooseAssert(!_hit_params_built, "Already built");
  _hit_params_built = true;

  std::vector<std::string> params;

  // Collect all hit parameters that aren't for subapps
  for (auto & entry : getEntries())
    if (entry.hit_param && !entry.subapp_name)
    {
      mooseAssert(entry.value, "Should have a value");
      mooseAssert(entry.value_separator, "Should have value separator");
      mooseAssert(*entry.value_separator == "=", "Should be an equals");

      const std::string name_and_equals = entry.name + *entry.value_separator;
      std::string arg = name_and_equals;
      // In the case of empty values, we want them to be empty to hit
      if (entry.value->empty())
        arg += "''";
      else
        arg += *entry.value;

      // We could have issues with bash eating strings, so the first try
      // gives us a chance to wrap the value in quotes
      try
      {
        hit::check("CLI_ARG", arg);
      }
      catch (hit::Error & err)
      {
        // bash might have eaten quotes around a hit string value or vector
        // so try quoting after the "=" and reparse
        arg = name_and_equals + "'" + *entry.value + "'";
        try
        {
          hit::check("CLI_ARG", arg);
        }
        // At this point, we've failed to fix it
        catch (hit::Error & err)
        {
          mooseError("Failed to parse HIT in command line argument '", arg, "'\n\n", err.what());
        }
      }

      // Append to the total output
      params.push_back(arg);
      // Consider this parameter used
      entry.used = true;
    }

  return params;
}

void
CommandLine::populateCommandLineParams(
    InputParameters & params,
    const hit::Node * const command_line_root /* = nullptr */,
    const std::optional<std::set<std::string>> & filter_names /* = {} */)
{
  mooseAssert(!_command_line_params_populated, "Already populated");

  // Set the metadata for each command line parameter
  // We set this separately so that it can be used later to print usage
  std::map<std::string, std::string> switch_param_map;
  for (const auto & name_value_pair : params)
  {
    const auto & name = name_value_pair.first;

    // Filter enabled and not in filter
    if (filter_names && !filter_names->count(name))
      continue;
    // Param is private, don't parse
    if (params.isPrivate(name))
      continue;

    if (const auto metadata = params.queryCommandLineMetadata(name))
    {
      auto it_inserted_pair = _command_line_params.emplace(name, CommandLineParam());
      auto & option = it_inserted_pair.first->second;

      option.description = params.getDocString(name);
      if (const auto group_ptr = params.queryParameterGroup(name))
        option.group = *group_ptr;
      option.metadata = *metadata;

      // Make sure that one switch isn't specified for multiple parameters
      for (const auto & cli_switch : option.metadata.switches)
        if (const auto it_inserted_pair = switch_param_map.emplace(cli_switch, name);
            !it_inserted_pair.second)
          mooseError("The command line options '",
                     it_inserted_pair.first->second,
                     "' and '",
                     name,
                     "' both declare the command line switch '",
                     cli_switch,
                     "'");
    }
  }

  // Set each paramter that we have a value for
  for (const auto & [name, param] : _command_line_params)
  {
    auto entry_it = findCommandLineParam(name);
    if (entry_it != _entries.end())
    {
      auto & entry = *entry_it;
      mooseAssert(!entry.subapp_name, "Should not be for a subapp");

      bool found = false;
#define trySetParameter(type)                                                                      \
  if (!found && params.have_parameter<type>(name))                                                 \
  {                                                                                                \
    static_assert(InputParameters::isValidCommandLineType<type>::value, "Not a supported value");  \
    auto & value = params.set<type>(name);                                                         \
    setCommandLineParam(entry_it, param, entry.name, value);                                       \
    found = true;                                                                                  \
  }

      trySetParameter(std::string);
      trySetParameter(std::vector<std::string>);
      trySetParameter(Real);
      trySetParameter(unsigned int);
      trySetParameter(int);
      trySetParameter(bool);
      trySetParameter(MooseEnum);
#undef trySetParameter

      mooseAssert(found, "Should have been found");

      // If we found this parameter, that means we set it and we should mark in the
      // InputParameters that it is set so that isParamSetByUser() returns true for this param
      params.commandLineParamSet(name, entry.name, command_line_root, {});

      // If this parameter is global, mark its entry as global
      if (param.metadata.global)
        entry.global = true;

      // If the arg is of the form "--key=value", PETSc will recognize it as an unused
      // argument. That is, setting "--key" as a known command line argument is not
      // sufficient for PETSc to consider "--key=value" as known. Thus, we explicitly
      // add "--key=value" args as known when we come across them.
      if (entry.value_separator && *entry.value_separator == "=")
      {
        mooseAssert(entry.raw_args.size() == 1, "Should have one value");
        libMesh::add_command_line_name(entry.raw_args[0]);
      }
    }
    // If we didn't find it and it is required, we need to error
    else if (param.metadata.required)
      mooseError(
          "Missing required command-line parameter: ", name, "\nDoc string: ", param.description);
  }

  _command_line_params_populated = true;
}

std::string
CommandLine::getExecutableName() const
{
  mooseAssert(_entries.size() > 0, "Does not have any entries");

  // Grab the first item out of argv
  const auto & command = _entries.begin()->name;
  return command.substr(command.find_last_of("/\\") + 1);
}

std::string
CommandLine::getExecutableNameBase() const
{
  auto name = getExecutableName();
  name = name.substr(0, name.find_last_of("-"));
  if (name.find_first_of("/") != std::string::npos)
    name = name.substr(name.find_first_of("/") + 1, std::string::npos);
  return name;
}

void
CommandLine::printUsage() const
{
  std::ostringstream oss;

  // Header
  oss << "Usage: " << getExecutableName() << " [<options>]\n\n";
  oss << "(G) denotes global parameter (applied to subapps)\n\n";

  // Collect all of the command line parameters so that we can sort them
  std::size_t max_syntax_len = 0;                                 // max length of all switches
  std::vector<std::pair<std::string, std::string>> sorted_params; // name -> switch without dashes
  for (const auto & [name, option] : _command_line_params)
    if (option.metadata.switches.size())
    {
      const auto & metadata = option.metadata;
      max_syntax_len = std::max(max_syntax_len, metadata.syntax.size());
      std::smatch match;
      if (std::regex_match(metadata.switches[0], match, std::regex("^-{1,2}(.+)$")))
        sorted_params.emplace_back(name, match[1]);
      else
        mooseAssert(false, "Parameter switch does not start with -");
    }

  // Sort all command line parameter names by their switch (after the dashes)
  std::sort(sorted_params.begin(),
            sorted_params.end(),
            [](const auto & lhs, const auto & rhs) { return lhs.second < rhs.second; });

  // Helper for ouputting a single parameter
  const auto output_param = [&oss, &max_syntax_len](const auto & option)
  {
    const std::string global_prefix = option.metadata.global ? "(G) " : "";
    oss << std::left << "  " << std::setw(max_syntax_len + 2) << option.metadata.syntax
        << global_prefix + option.description << "\n";
  };

  // Collect parameters by group, printing the non-grouped ones first
  std::map<std::string, std::vector<std::string>> group_params;
  std::vector<std::string> non_group_params;
  for (const auto & name_switch_pair : sorted_params)
  {
    const auto & name = name_switch_pair.first;
    const auto & option = _command_line_params.at(name);
    if (option.group)
      group_params[*option.group].push_back(name);
    else
      output_param(option);
  }
  oss << "\n";

  // Print the grouped parameters, per group
  for (const auto & [group, names] : group_params)
  {
    oss << group << ":\n";
    for (const auto & name : names)
      output_param(_command_line_params.at(name));
    oss << "\n";
  }

  oss << "Solver Options:\n  See PETSc manual for details\n";

  Moose::out << oss.str();

  // If we get here, we are not running a simulation and should silence petsc's unused options
  // warning
  Moose::PetscSupport::setSinglePetscOption("-options_left", "0");
}

std::vector<std::string>
CommandLine::unusedHitParams(const Parallel::Communicator & comm) const
{
  libmesh_parallel_only(comm);

  std::vector<const Entry *> hit_params;
  std::vector<std::size_t> use_count;
  for (const auto & entry : getEntries())
    if (entry.hit_param)
    {
      hit_params.push_back(&entry);
      use_count.push_back(entry.used ? 1 : 0);
    }

  mooseAssert(comm.verify(use_count.size()), "Inconsistent HIT params across procs");
  comm.sum(use_count);

  std::vector<std::string> unused;
  for (const auto i : index_range(use_count))
    if (use_count[i] == 0)
      unused.push_back(hit_params[i]->raw_args[0]);
  return unused;
}

std::list<CommandLine::Entry>::const_iterator
CommandLine::findCommandLineParam(const std::string & name) const
{
  const auto find_param = _command_line_params.find(name);
  if (find_param == _command_line_params.end())
    mooseError("CommandLine::findCommandLineParam(): The parameter '",
               name,
               "' is not a command line parameter");

  // Search for the last thing that matches a switch from this param
  const auto & param = find_param->second;
  for (auto rit = _entries.rbegin(); rit != _entries.rend(); ++rit)
    for (const auto & search_switch : param.metadata.switches)
      if (rit->name == search_switch)
        return --(rit.base());

  return getEntries().end();
}

std::list<CommandLine::Entry>::iterator
CommandLine::findCommandLineParam(const std::string & name)
{
  const auto it = std::as_const(*this).findCommandLineParam(name);
  // Easy way to go from a const iterator -> non-const iterator
  return _entries.erase(it, it);
}

std::string
CommandLine::formatEntry(const CommandLine::Entry & entry) const
{
  std::stringstream oss;
  oss << entry.name;
  if (entry.value)
  {
    const auto q = (*entry.value).find(" ") != std::string::npos ? "'" : "";
    oss << *entry.value_separator << q << *entry.value << q;
  }
  return oss.str();
}
