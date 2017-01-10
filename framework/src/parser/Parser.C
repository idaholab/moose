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

// MOOSE includes
#include "MooseUtils.h"
#include "MooseInit.h"
#include "InputParameters.h"
#include "ActionFactory.h"
#include "Action.h"
#include "Factory.h"
#include "MooseObjectAction.h"
#include "ActionWarehouse.h"
#include "EmptyAction.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "Executioner.h"
#include "MooseApp.h"
#include "MooseEnum.h"
#include "MultiMooseEnum.h"
#include "MultiApp.h"
#include "GlobalParamsAction.h"
#include "SyntaxTree.h"
#include "InputFileFormatter.h"
#include "YAMLFormatter.h"
#include "MooseTypes.h"
#include "CommandLine.h"

// libMesh includes
#include "libmesh/getpot.h"

// Regular expression includes
#include "pcrecpp.h"

// C++ includes
#include <string>
#include <map>
#include <fstream>
#include <iomanip>
#include <algorithm>

// Free function for removing cli flags
bool isFlag(const std::string s)
{
  return s.length() && s[0] == '-';
}


Parser::Parser(MooseApp & app, ActionWarehouse & action_wh) :
    ConsoleStreamInterface(app),
    _app(app),
    _factory(app.getFactory()),
    _action_wh(action_wh),
    _action_factory(app.getActionFactory()),
    _syntax(_action_wh.syntax()),
    _syntax_formatter(NULL),
    _getpot_initialized(false),
    _sections_read(false),
    _current_params(NULL),
    _current_error_stream(NULL)
{
}

Parser::~Parser()
{
  delete _syntax_formatter;
}

bool
Parser::isSectionActive(const std::string & s,
                        const std::map<std::string, std::vector<std::string> > & active_lists)
{
  bool retValue = false;
  size_t found = s.find_last_of('/');

  // Base Level is always active
  if (found == std::string::npos)
    return true;

  std::string parent = s.substr(0, found);
  std::string short_name = s.substr(found+1);

  std::map<std::string, std::vector<std::string> >::const_iterator pos = active_lists.find(parent);
  if (pos == active_lists.end())  // If value is missing them the current block is active
    return true;

  std::vector<std::string> active = pos->second;

  if (!active.empty())
  {
    if (active[0] == "__all__")
      retValue = true;
    else
      retValue = std::find(active.begin(), active.end(), short_name) != active.end();
  }

  // Finally see if any of the inactive strings are partially contained in this path (matching from the beginning)
  for (const auto & search_string : _inactive_strings)
    if (s.find(search_string) == 0)
      retValue = false;

  // If this section is not active - then keep track of it for future checks
  if (!retValue)
    _inactive_strings.insert(s + "/");

  return retValue;
}


std::string
Parser::getFileName(bool stripLeadingPath) const
{
  if (!stripLeadingPath)
    return _input_filename;

  std::string filename;
  size_t pos = _input_filename.find_last_of('/');
  if (pos != std::string::npos)
    filename = _input_filename.substr(pos + 1);
  else
    filename = _input_filename;

  return filename;
}

void
Parser::parse(const std::string &input_filename)
{
  std::string curr_identifier;
  std::map<std::string, std::vector<std::string> > active_lists;
  std::vector<std::string> section_names;
  InputParameters active_list_params = validParams<Action>();
  InputParameters params = validParams<EmptyAction>();

  // Save the filename
  _input_filename = input_filename;

  // vector for initializing active blocks
  std::vector<std::string> all = {"__all__"};

  MooseUtils::checkFileReadable(input_filename, true);

  /**
   * Only allow the main application to "absorb" it's command line parameters into the input file object.
   * This allows DBEs with substitutions on the CLI to work for the master application but not sub apps.
   * If we did allow this, it would remove the ability to only set CLI overrides for the main app only.
   */
  if (_app.name() == "main")
    _getpot_file.absorb(*_app.commandLine()->getPot());

  // GetPot object
  _getpot_file.enable_request_recording();
  _getpot_file.parse_input_file(input_filename);

  /**
   * We re-parse the exact same file for error checking purposes. We don't want all of the CLI variables
   * involved in error checks.
   */
  _getpot_file_error_checking.parse_input_file(input_filename);

  _getpot_initialized = true;
  _inactive_strings.clear();

  /**
   * If this is a Multiapp or wrapper make sure we set a prefix on the CommandLine object for CLI overrides.
   */
  if (_app.name() != "main")
  {
    std::string name;
    std::string num;
    if (pcrecpp::RE("(.*?)"                       // Match the multiapp name
                    "(\\d+)"                      // math the multiapp number
          ).FullMatch(_app.name(), &name, &num))
      _app.commandLine()->setPrefix(name, num);
    else
      // Wrapper case
      _app.commandLine()->setPrefix(_app.name(), "0");
  }

  // Check for "unidentified nominuses".  These can indicate a vector
  // input which the user failed to wrap in quotes e.g.: v = 1 2
  {
    std::set<std::string> knowns;
    std::vector<std::string> ufos = _getpot_file_error_checking.unidentified_nominuses();
    if (!ufos.empty())
    {
      Moose::err << "Error: the following unidentified entries were found in your input file:" << std::endl;
      for (const auto & ufo : ufos)
        Moose::err << ufo << std::endl;
      mooseError("Your input file may have a syntax error, or you may have forgotten to put quotes around a vector, ie. v='1 2'.");
    }
  }

  section_names = _getpot_file.get_section_names();
  appendAndReorderSectionNames(section_names);

  // Set the class variable to indicate that sections names have been read, this is used later by the checkOverriddenParams function
  _sections_read = true;

  for (auto & section_name : section_names)
  {
    curr_identifier = section_name.erase(section_name.size()-1);  // Chop off the last character (the trailing slash)

    // Before we retrieve any actions or build any objects, make sure that the section they are in is active
    if (isSectionActive(curr_identifier, active_lists))
    {
      // Extract the block parameters before constructing the action
      // There may be more than one Action registered for a given section in which case we need to
      // build them all
      bool is_parent;
      std::string registered_identifier = _syntax.isAssociated(section_name, &is_parent);

      // We need to retrieve a list of Actions associated with the current identifier
      std::pair<std::multimap<std::string, Syntax::ActionInfo>::iterator,
        std::multimap<std::string, Syntax::ActionInfo>::iterator> iters = _syntax.getActions(registered_identifier);

      if (iters.first == iters.second)
        mooseError(std::string("A '") + curr_identifier + "' does not have an associated \"Action\".\nDid you leave off a leading \"./\" in one of your nested blocks?\n");

      for (std::multimap<std::string, Syntax::ActionInfo>::iterator it = iters.first; it != iters.second; ++it)
      {
        if (!is_parent)
        {
          params = _action_factory.getValidParams(it->second._action);

          params.set<ActionWarehouse *>("awh") = &_action_wh;

          extractParams(curr_identifier, params);

          // Add the parsed syntax to the parameters object for consumption by the Action
          params.set<std::string>("task") = it->second._task;
          params.set<std::string>("registered_identifier") = registered_identifier;
          params.addPrivateParam<std::string>("parser_syntax", curr_identifier);

          // Create the Action
          MooseSharedPointer<Action> action_obj = _action_factory.create(it->second._action, MooseUtils::shortName(curr_identifier), params);

          // extract the MooseObject params if necessary
          MooseSharedPointer<MooseObjectAction> object_action = MooseSharedNamespace::dynamic_pointer_cast<MooseObjectAction>(action_obj);
          if (object_action)
          {
            extractParams(curr_identifier, object_action->getObjectParams());
            object_action->getObjectParams().set<std::vector<std::string> >("control_tags").push_back(MooseUtils::baseName(curr_identifier));
          }

          // add it to the warehouse
          _action_wh.addActionBlock(action_obj);
        }
      }
    }

    // Extract and save the current "active" list in the data structure
    active_list_params.set<std::vector<std::string> >("active") = all;
    extractParams(curr_identifier, active_list_params);
    active_lists[curr_identifier] = active_list_params.get<std::vector<std::string> >("active");
  }

  // Check to make sure that all sections in the input file that are explicitly listed are actually present
  checkActiveUsed(section_names, active_lists);
}

void
Parser::checkActiveUsed(std::vector<std::string > & sections,
                        const std::map<std::string, std::vector<std::string> > & active_lists)
{
  std::set<std::string> active_lists_set;
  std::vector<std::string> difference;

  for (const auto & i : active_lists)
    for (const auto & j : i.second)
    {
      active_lists_set.insert(i.first);
      if (j != "__all__")
        active_lists_set.insert(i.first + "/" + j);
    }

  std::sort(sections.begin(), sections.end());

  std::set_difference(active_lists_set.begin(), active_lists_set.end(), sections.begin(), sections.end(),
                      std::inserter(difference, difference.end()));

  if (!difference.empty())
  {
    std::ostringstream oss;
    oss << "One or more active lists in the input file are missing a referenced section:\n";
    for (const auto & name : difference)
      oss << name << "\n";
    mooseError(oss.str());
  }
}

void
Parser::checkUnidentifiedParams(std::vector<std::string> & all_vars, bool error_on_warn, bool in_input_file, MooseSharedPointer<FEProblemBase> fe_problem) const
{
  // Make sure that multiapp overrides were processed properly
  int last = all_vars.size() - 1;                      // last is allowed to go negative
  for (int i = 0; i <= last; /* no increment */)       // i is an int because last is an int
  {
    std::string multi_app, variable;
    int app_num;

    /**
     * Command line parameters that contain a colon are assumed to apply to MultiApps
     * (e.g.  MultiApp_name[num]:fully_qualified_parameter)
     *
     * Note: Two separate regexs are used since the digit part is optional. Attempting
     * to have an optional capture into a non-string type will cause pcrecpp to report
     * false. Capturing into a string an converting is more work than just using two
     * regexs to begin with.
     */
    if (pcrecpp::RE("(.*?)"                                             // Match the MultiApp name
                    "(\\d+)"                                            // MultiApp number (leave off to apply to all MultiApps with this name)
                    ":"                                                 // the colon delimiter
                    "(.*)"                                              // the variable override that applies to the MultiApp
          ).FullMatch(all_vars[i], &multi_app, &app_num, &variable) &&
        fe_problem->hasMultiApp(multi_app) &&                           // Make sure the MultiApp exists
        // Finally make sure the number is in range (if provided)
        static_cast<unsigned int>(app_num) < fe_problem->getMultiApp(multi_app)->numGlobalApps())


      // delete the current item by copying the last item to this position and decrementing the vector end position
      all_vars[i] = all_vars[last--];

    else if (pcrecpp::RE("(.*?)"                                        // Same as above without the MultiApp number
                         ":"
                         "(.*)"
               ).FullMatch(all_vars[i], &multi_app, &variable) &&
             fe_problem->hasMultiApp(multi_app))                        // Make sure the MultiApp exists but no need to check numbers

      // delete (see comment above)
      all_vars[i] = all_vars[last--];


    // TODO: check to see if globals are unused
    else if (all_vars[i].find(":") == 0)
      all_vars[i] = all_vars[last--];

    else
      // only increment if we didn't "delete", otherwise we'll need to revisit the current index due to copy
      ++i;
  }

  mooseAssert(last + 1 >= 0, "index \"last\" is negative");

  // Remove the deleted items
  all_vars.resize(last+1);

  std::set<std::string> difference;

  std::sort(all_vars.begin(), all_vars.end());

  // Remove flags, they aren't "input" parameters
  all_vars.erase( std::remove_if(all_vars.begin(), all_vars.end(), isFlag), all_vars.end() );

  std::set_difference(all_vars.begin(), all_vars.end(), _extracted_vars.begin(), _extracted_vars.end(),
                      std::inserter(difference, difference.end()));

  // Remove un-parsed parameters that were located in an inactive sections
  for (const auto & inactive_string : _inactive_strings)
    for (std::set<std::string>::iterator j=difference.begin(); j != difference.end(); /*no increment*/)
    {
      std::set<std::string>::iterator curr = j++;
      if (curr->find(inactive_string) != std::string::npos)
        difference.erase(curr);
    }

  std::set<std::string> requested_vars = _getpot_file.get_requested_variables();

  std::set<std::string> no_overrides;
  std::set_difference(difference.begin(), difference.end(), requested_vars.begin(), requested_vars.end(),
                      std::inserter(no_overrides, no_overrides.end()));

  if (!no_overrides.empty())
  {
    std::ostringstream oss;

    oss << "The following parameters were unused " << (in_input_file ? "in your input file:\n" : "on the command line:\n");
    for (const auto & name : no_overrides)
      oss << name << "\n";

    if (error_on_warn)
      mooseError(oss.str());
    else
      mooseWarning(oss.str());
  }
}

void
Parser::checkOverriddenParams(bool error_on_warn) const
{
  if (!_sections_read && error_on_warn)
    // The user has requested errors but we haven't done any parsing yet so throw an error
    mooseError("No parsing has been done, so checking for overridden parameters is not possible");

  std::set<std::string> overridden_vars = _getpot_file_error_checking.get_overridden_variables();

  if (!overridden_vars.empty())
  {
    std::ostringstream oss;

    oss << "The following variables were overridden or supplied multiple times:\n";
    for (const auto & name : overridden_vars)
      oss << name << "\n";

    if (error_on_warn)
      mooseError(oss.str());
    else
      mooseWarning(oss.str());
  }
}

void
Parser::appendAndReorderSectionNames(std::vector<std::string> & section_names)
{
  /**
   * We only want to retrieve non-prefixed CLI overrides for the main application. We'll check the
   * name of the controlling application to determine whether to use the command line
   * here or not.
   */
  MooseSharedPointer<CommandLine> cmd_line;
//  if (_app.name() == "main") // See AppFactory::createApp
    cmd_line = _app.commandLine();

  if (cmd_line.get())
  {
    GetPot *get_pot = cmd_line->getPot();
    mooseAssert(get_pot, "GetPot object is NULL");

    std::vector<std::string> cli_variables = get_pot->get_variable_names();
    for (const auto & cli_var : cli_variables)
    {
      std::string::size_type colon_pos = cli_var.find(':');
      std::string::size_type last_slash_pos = cli_var.find_last_of('/');

      // Make sure that the variable does not contain a colon. This indicates that the override is for
      // a Multiapp parameter which we won't handle here.
      if (colon_pos == std::string::npos && last_slash_pos != std::string::npos)
      {
        // If the user supplies a CLI argument whose section doesn't exist in the input file, we'll append it here
        std::string section = cli_var.substr(0, last_slash_pos+1);
        if (std::find(section_names.begin(), section_names.end(), section) == section_names.end())
          section_names.push_back(section);
      }
    }
  }

  /**
   * There are a few order dependent actions that have to be built first in
   * order for the parser and application to function properly:
   *
   * SetupDebugAction: This action can contain an option for monitoring the parser progress. It must be parsed first
   *                   to capture all of the parsing output.
   *
   * GlobalParamsAction: This action is checked during the parameter extraction routines of all subsequent blocks.
   *                     It must be parsed early since it must exist during subsequent parameter extraction.
   *
   * DynamicObjectRegistration: This action must be built before any MooseObjectActions are built. This is because
   *                            we retrieve valid parameters from the Factory during parse time. Objects must
   *                            be registered before validParameters can be retrieved.
   */

  // Reverse order here since each call to reoderHelper moves the requested Action to the front
  reorderHelper(section_names, "DynamicObjectRegistrationAction", "dynamic_object_registration");
  reorderHelper(section_names, "GlobalParamsAction", "set_global_params");
  reorderHelper(section_names, "SetupDebugAction", "setup_debug");
}

void
Parser::reorderHelper(std::vector<std::string> & section_names, const std::string & action, const std::string & task) const
{
  /**
   * Note: I realize that doing inserts and deletes in a vector are "slow".  Swapping is not an option due to the
   *       way that active_lists are constructed.  These are small vectors ;)
   */
  std::string syntax = _syntax.getSyntaxByAction(action, task);
  syntax += '/';   // section names *always* have trailing slashes

  std::vector<std::string>::iterator pos = std::find(section_names.begin(), section_names.end(), syntax);
  if (pos != section_names.end())
  {
    section_names.erase(pos);
    section_names.insert(section_names.begin(), syntax);
  }
}



void
Parser::initSyntaxFormatter(SyntaxFormatterType type, bool dump_mode)
{
  if (_syntax_formatter)
    delete _syntax_formatter;

  switch (type)
  {
  case INPUT_FILE:
    _syntax_formatter = new InputFileFormatter(dump_mode);
    break;
  case YAML:
    _syntax_formatter = new YAMLFormatter(dump_mode);
    break;
  default:
    mooseError("Unrecognized Syntax Formatter requested");
    break;
  }
}

void
Parser::buildFullTree(const std::string & search_string)
{
//  std::string prev_name = "";
//  std::vector<InputParameters *> params_ptrs(2);
  std::vector<std::pair<std::string, Syntax::ActionInfo> > all_names;

  for (const auto & iter : _syntax.getAssociatedActions())
  {
    Syntax::ActionInfo act_info = iter.second;
    // If the task is NULL that means we need to figure out which task
    // goes with this syntax for the purpose of building the Moose Object part of the tree.
    // We will figure this out by asking the ActionFactory for the registration info.
    if (act_info._task == "")
      act_info._task = _action_factory.getTaskName(act_info._action);

    all_names.push_back(std::pair<std::string, Syntax::ActionInfo>(iter.first, act_info));
  }

  for (const auto & act_names : all_names)
  {
    InputParameters action_obj_params = _action_factory.getValidParams(act_names.second._action);
    _syntax_formatter->insertNode(act_names.first, act_names.second._action, true, &action_obj_params);

    const std::string & task = act_names.second._task;
    std::string act_name = act_names.first;

    // We need to see if this action is inherited from MooseObjectAction
    // If it is, then we will loop over all the Objects in MOOSE's Factory object to print them out
    // if they have associated bases matching the current task.
    if (action_obj_params.have_parameter<bool>("isObjectAction") && action_obj_params.get<bool>("isObjectAction"))
    {
      for (registeredMooseObjectIterator moose_obj = _factory.registeredObjectsBegin();
           moose_obj != _factory.registeredObjectsEnd();
           ++moose_obj)
      {
        InputParameters moose_obj_params = (moose_obj->second)();
        // Now that we know that this is a MooseObjectAction we need to see if it has been restricted
        // in any way by the user.
        const std::vector<std::string> & buildable_types = action_obj_params.getBuildableTypes();

        // See if the current Moose Object syntax belongs under this Action's block
        if ((buildable_types.empty() ||                                                                                // Not restricted
             std::find(buildable_types.begin(), buildable_types.end(), moose_obj->first) != buildable_types.end()) &&  // Restricted but found
            moose_obj_params.have_parameter<std::string>("_moose_base") &&                                             // Has a registered base
            _syntax.verifyMooseObjectTask(moose_obj_params.get<std::string>("_moose_base"), task) &&                   // and that base is associated
            action_obj_params.mooseObjectSyntaxVisibility())                                                           // and the Action says it's visible
        {
          std::string name;
          size_t pos = 0;
          bool is_action_params = false;;
          if (act_name[act_name.size()-1] == '*')
          {
            pos = act_name.size();

            if (!action_obj_params.collapseSyntaxNesting())
              name = act_name.substr(0, pos-1) + moose_obj->first;
            else
            {
              name = act_name.substr(0, pos-1) + "/<type>/" + moose_obj->first;
              is_action_params = true;
            }
          }
          else
          {
            name = act_name + "/<type>/" + moose_obj->first;
          }

          moose_obj_params.set<std::string>("type") = moose_obj->first;

          _syntax_formatter->insertNode(name, moose_obj->first, is_action_params, &moose_obj_params);
        }
      }
    }
  }

  // Do not change to _console, we need this printed to the stdout in all cases
  Moose::out << _syntax_formatter->print(search_string) << std::flush;
}


const GetPot *
Parser::getPotHandle() const
{
  return _getpot_initialized ? &_getpot_file : NULL;
}

/**************************************************************************************************************************
 **************************************************************************************************************************
 *                                            Parameter Extraction Routines                                               *
 **************************************************************************************************************************
 **************************************************************************************************************************/
using std::string;

// Template Specializations for retrieving special types from the input file
template<>
void Parser::setScalarParameter<RealVectorValue>(const std::string & full_name, const std::string & short_name,
                                                 InputParameters::Parameter<RealVectorValue> * param, bool in_global, GlobalParamsAction * global_block);

template<>
void Parser::setScalarParameter<Point>(const std::string & full_name, const std::string & short_name,
                                       InputParameters::Parameter<Point> * param, bool in_global, GlobalParamsAction * global_block);

template<>
void Parser::setScalarParameter<PostprocessorName>(const std::string & full_name, const std::string & short_name,
                                                   InputParameters::Parameter<PostprocessorName>* param, bool in_global, GlobalParamsAction *global_block);

template<>
void Parser::setScalarParameter<MooseEnum>(const std::string & full_name, const std::string & short_name,
                                           InputParameters::Parameter<MooseEnum>* param, bool in_global, GlobalParamsAction *global_block);

template<>
void Parser::setScalarParameter<MultiMooseEnum>(const std::string & full_name, const std::string & short_name,
                                                InputParameters::Parameter<MultiMooseEnum>* param, bool in_global, GlobalParamsAction *global_block);

template<>
void Parser::setScalarParameter<RealTensorValue>(const std::string & full_name, const std::string & short_name,
                                                 InputParameters::Parameter<RealTensorValue> * param, bool in_global, GlobalParamsAction * global_block);

// Vectors
template<>
void Parser::setVectorParameter<RealVectorValue>(const std::string & full_name, const std::string & short_name,
                                                 InputParameters::Parameter<std::vector<RealVectorValue> > * param, bool in_global, GlobalParamsAction * global_block);

template<>
void Parser::setVectorParameter<Point>(const std::string & full_name, const std::string & short_name,
                                       InputParameters::Parameter<std::vector<Point> > * param, bool in_global, GlobalParamsAction * global_block);

template<>
void Parser::setVectorParameter<MooseEnum>(const std::string & full_name, const std::string & short_name,
                                           InputParameters::Parameter<std::vector<MooseEnum> > * param, bool in_global, GlobalParamsAction * global_block);

template<>
void Parser::setVectorParameter<VariableName>(const std::string & full_name, const std::string & short_name,
                                              InputParameters::Parameter<std::vector<VariableName> > * param, bool in_global, GlobalParamsAction * global_block);

template<>
void Parser::setDoubleIndexParameter<VariableName>(const std::string & full_name, const std::string & short_name,
                                                   InputParameters::Parameter<std::vector<std::vector<VariableName> > >* param, bool /*in_global*/, GlobalParamsAction * /*global_block*/);

// Macros for parameter extraction
#define dynamicCastAndExtractScalar(type, param, full_name, short_name, in_global, global_block)                                        \
  do                                                                                                                                    \
  {                                                                                                                                     \
    InputParameters::Parameter<type> * scalar_p = dynamic_cast<InputParameters::Parameter<type>*>(param);                               \
    if (scalar_p)                                                                                                                       \
      setScalarParameter<type>(full_name, short_name, scalar_p, in_global, global_block);                                               \
  } while (0)

#define dynamicCastAndExtractScalarValueType(type, up_type, param, full_name, short_name, in_global, global_block)                      \
  do                                                                                                                                    \
  {                                                                                                                                     \
    InputParameters::Parameter<type> * scalar_p = dynamic_cast<InputParameters::Parameter<type>*>(param);                               \
    if (scalar_p)                                                                                                                       \
      setScalarValueTypeParameter<type, up_type>(full_name, short_name, scalar_p, in_global, global_block);                             \
  } while (0)

#define dynamicCastAndExtractVector(type, param, full_name, short_name, in_global, global_block)                                        \
  do                                                                                                                                    \
  {                                                                                                                                     \
    InputParameters::Parameter<std::vector<type> > * vector_p = dynamic_cast<InputParameters::Parameter<std::vector<type> >*>(param);   \
    if (vector_p)                                                                                                                       \
      setVectorParameter<type>(full_name, short_name, vector_p, in_global, global_block);                                               \
  } while (0)

#define dynamicCastAndExtractDoubleIndex(type, param, full_name, short_name, in_global, global_block)                                                                    \
  do                                                                                                                                                                     \
  {                                                                                                                                                                      \
    InputParameters::Parameter<std::vector<std::vector<type> > > * double_index_p = dynamic_cast<InputParameters::Parameter<std::vector<std::vector<type> > > *>(param); \
    if (double_index_p)                                                                                                                                                  \
      setDoubleIndexParameter<type>(full_name, short_name, double_index_p, in_global, global_block);                                                                     \
  } while (0)

void
Parser::extractParams(const std::string & prefix, InputParameters & p)
{
  std::ostringstream error_stream;
  static const std::string global_params_task = "set_global_params";
  static const std::string global_params_block_name = _syntax.getSyntaxByAction("GlobalParamsAction", global_params_task);

  ActionIterator act_iter = _action_wh.actionBlocksWithActionBegin(global_params_task);
  GlobalParamsAction *global_params_block = NULL;

  // We are grabbing only the first
  if (act_iter != _action_wh.actionBlocksWithActionEnd(global_params_task))
    global_params_block = dynamic_cast<GlobalParamsAction *>(*act_iter);

  // Set a pointer to the current InputParameters object being parsed so that it can be referred to in the extraction routines
  _current_params = &p;
  _current_error_stream = &error_stream;
  for (const auto & it : p)
  {
    bool found = false;
    bool in_global = false;
    std::string orig_name = prefix + "/" + it.first;
    std::string full_name = orig_name;

    // Mark parameters appearing in the input file or command line
    if (_getpot_file.have_variable(full_name.c_str()) || (_app.commandLine() && _app.commandLine()->haveVariable(full_name.c_str())))
    {
      p.set_attributes(it.first, false);
      _extracted_vars.insert(full_name);  // Keep track of all variables extracted from the input file
      found = true;
    }
    // Wait! Check the GlobalParams section
    else if (global_params_block != NULL)
    {
      full_name = global_params_block_name + "/" + it.first;
      if (_getpot_file.have_variable(full_name.c_str()))
      {
        p.set_attributes(it.first, false);
        _extracted_vars.insert(full_name);  // Keep track of all variables extracted from the input file
        found = true;
        in_global = true;
      }
    }

    if (!found)
    {
      /**
       * Special case handling
       *   if the parameter wasn't found in the input file or the cli object the logic in this branch will execute
       */

      // In the case where we have OutFileName but it wasn't actually found in the input filename,
      // we will populate it with the actual parsed filename which is available here in the parser.

      InputParameters::Parameter<OutFileBase> * scalar_p = dynamic_cast<InputParameters::Parameter<OutFileBase>*>(it.second);
      if (scalar_p)
      {
        std::string input_file_name = getFileName();
        mooseAssert(input_file_name != "", "Input Filename is NULL");
        size_t pos = input_file_name.find_last_of('.');
        mooseAssert(pos != std::string::npos, "Unable to determine suffix of input file name");
        scalar_p->set() = input_file_name.substr(0,pos) + "_out";
        p.set_attributes(it.first, false);
      }
    }
    else
    {
      if (p.isPrivate(it.first))
        mooseError("The parameter '" << full_name << "' is a private parameter and should not be used in an input file.");

      /**
       * Scalar types
       */
      // built-ins
      // NOTE: Similar dynamic casting is done in InputParameters.C, please update appropriately
      dynamicCastAndExtractScalarValueType(Real, Real         , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractScalarValueType(int,  long         , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractScalarValueType(long, long         , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractScalarValueType(unsigned int, long , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractScalar(bool                        , it.second, full_name, it.first, in_global, global_params_block);

      // Moose Scalars
      dynamicCastAndExtractScalar(SubdomainID           , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractScalar(BoundaryID            , it.second, full_name, it.first, in_global, global_params_block);

      // Moose Compound Scalars
      dynamicCastAndExtractScalar(RealVectorValue       , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractScalar(Point                 , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractScalar(MooseEnum             , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractScalar(MultiMooseEnum        , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractScalar(RealTensorValue       , it.second, full_name, it.first, in_global, global_params_block);

      // Moose String-derived scalars
      dynamicCastAndExtractScalar(/*std::*/string       , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractScalar(SubdomainName         , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractScalar(BoundaryName          , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractScalar(FileName              , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractScalar(FileNameNoExtension   , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractScalar(MeshFileName          , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractScalar(OutFileBase           , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractScalar(VariableName          , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractScalar(NonlinearVariableName , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractScalar(AuxVariableName       , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractScalar(FunctionName          , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractScalar(UserObjectName        , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractScalar(PostprocessorName     , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractScalar(VectorPostprocessorName, it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractScalar(IndicatorName         , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractScalar(MarkerName            , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractScalar(MultiAppName          , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractScalar(OutputName            , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractScalar(MaterialPropertyName  , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractScalar(MaterialName          , it.second, full_name, it.first, in_global, global_params_block);


      /**
       * Vector types
       */
      // built-ins
      dynamicCastAndExtractVector(Real                  , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractVector(int                   , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractVector(long                  , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractVector(unsigned int          , it.second, full_name, it.first, in_global, global_params_block);

      // We need to be able to parse 8-byte unsigned types when
      // libmesh is configured --with-dof-id-bytes=8.  Officially,
      // libmesh uses uint64_t in that scenario, which is usually
      // equivalent to 'unsigned long long'.  Note that 'long long'
      // has been around since C99 so most C++ compilers support it,
      // but presumably uint64_t is the "most standard" way to get a
      // 64-bit unsigned type, so we'll stick with that here.
#if LIBMESH_DOF_ID_BYTES == 8
      dynamicCastAndExtractVector(uint64_t              , it.second, full_name, it.first, in_global, global_params_block);
#endif

      // Moose Vectors
      dynamicCastAndExtractVector(SubdomainID           , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractVector(BoundaryID            , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractVector(RealVectorValue       , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractVector(Point                 , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractVector(MooseEnum             , it.second, full_name, it.first, in_global, global_params_block);
      /* We won't try to do vectors of tensors ;) */

      // Moose String-derived vectors
      dynamicCastAndExtractVector(/*std::*/string       , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractVector(FileName              , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractVector(FileNameNoExtension   , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractVector(MeshFileName          , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractVector(SubdomainName         , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractVector(BoundaryName          , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractVector(VariableName          , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractVector(NonlinearVariableName , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractVector(AuxVariableName       , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractVector(FunctionName          , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractVector(UserObjectName        , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractVector(IndicatorName         , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractVector(MarkerName            , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractVector(MultiAppName          , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractVector(PostprocessorName     , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractVector(VectorPostprocessorName, it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractVector(OutputName            , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractVector(MaterialPropertyName  , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractVector(MaterialName          , it.second, full_name, it.first, in_global, global_params_block);

      /**
       * Double indexed types
       */
       // built-ins
       dynamicCastAndExtractDoubleIndex(Real                  , it.second, full_name, it.first, in_global, global_params_block);
       dynamicCastAndExtractDoubleIndex(int                   , it.second, full_name, it.first, in_global, global_params_block);
       dynamicCastAndExtractDoubleIndex(long                  , it.second, full_name, it.first, in_global, global_params_block);
       dynamicCastAndExtractDoubleIndex(unsigned int          , it.second, full_name, it.first, in_global, global_params_block);
       // See vector type explanation
 #if LIBMESH_DOF_ID_BYTES == 8
       dynamicCastAndExtractDoubleIndex(uint64_t              , it.second, full_name, it.first, in_global, global_params_block);
 #endif

      dynamicCastAndExtractDoubleIndex(SubdomainID           , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractDoubleIndex(BoundaryID            , it.second, full_name, it.first, in_global, global_params_block);

      // Moose String-derived vectors
      dynamicCastAndExtractDoubleIndex(/*std::*/string       , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractDoubleIndex(FileName              , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractDoubleIndex(FileNameNoExtension   , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractDoubleIndex(MeshFileName          , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractDoubleIndex(SubdomainName         , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractDoubleIndex(BoundaryName          , it.second, full_name, it.first, in_global, global_params_block);
      // reading double indexed Variable name is problematic because Coupleable assumes they come as vectors
      // therefore they not included in this list
      dynamicCastAndExtractDoubleIndex(FunctionName          , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractDoubleIndex(UserObjectName        , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractDoubleIndex(IndicatorName         , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractDoubleIndex(MarkerName            , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractDoubleIndex(MultiAppName          , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractDoubleIndex(PostprocessorName     , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractDoubleIndex(VectorPostprocessorName, it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractDoubleIndex(OutputName            , it.second, full_name, it.first, in_global, global_params_block);
      dynamicCastAndExtractDoubleIndex(MaterialPropertyName  , it.second, full_name, it.first, in_global, global_params_block);
    }
  }

  // All of the parameters for this object have been extracted.  See if there are any errors
  if (!error_stream.str().empty())
    mooseError(error_stream.str());

  // Here we will see if there are any auto build vectors that need to be created
  const std::map<std::string, std::pair<std::string, std::string> > & auto_build_vectors = p.getAutoBuildVectors();
  for (const auto & it : auto_build_vectors)
  {
    // We'll autogenerate values iff the requested vector is not valid but both the base and number are valid
    const std::string & base_name = it.second.first;
    const std::string & num_repeat = it.second.second;

    if (!p.isParamValid(it.first) && p.isParamValid(base_name) && p.isParamValid(num_repeat))
    {
      unsigned int vec_size = p.get<unsigned int>(num_repeat);
      const std::string & name = p.get<std::string>(base_name);

      std::vector<VariableName> variable_names(vec_size);
      for (unsigned int i = 0; i < vec_size; ++i)
      {
        std::ostringstream oss;
        oss << name << i;
        variable_names[i] = oss.str();
      }

      // Finally set the autogenerated vector into the InputParameters object
      p.set<std::vector<VariableName> >(it.first) = variable_names;
    }
  }
}

template<typename T>
void Parser::setScalarParameter(const std::string & full_name, const std::string & short_name, InputParameters::Parameter<T> * param, bool in_global, GlobalParamsAction * global_block)
{
  GetPot *gp;

  // See if this variable was passed on the command line
  // if it was then we will retrieve the value from the command line instead of the file
  if (_app.commandLine() && _app.commandLine()->haveVariable(full_name.c_str()))
    gp = _app.commandLine()->getPot();
  else
    gp = &_getpot_file;

  T value = gp->get_value_no_default(full_name.c_str(), param->get());

  // Set the value here
  param->set() = value;

  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setScalarParam<T>(short_name) = value;
  }
}

template<typename T, typename UP_T>
void Parser::setScalarValueTypeParameter(const std::string & full_name, const std::string & short_name, InputParameters::Parameter<T> * param, bool in_global, GlobalParamsAction * global_block)
{
  setScalarParameter<T>(full_name, short_name, param, in_global, global_block);

  // If this is a range checked param, we need to make sure that the value falls within the requested range
  mooseAssert(_current_params, "Current params is NULL");

  _current_params->rangeCheck<T, UP_T>(full_name, short_name, param, *_current_error_stream);
}

template<typename T>
void Parser::setVectorParameter(const std::string & full_name, const std::string & short_name, InputParameters::Parameter<std::vector<T> >* param, bool in_global, GlobalParamsAction * global_block)
{
  GetPot *gp;

  // See if this variable was passed on the command line
  // if it was then we will retrieve the value from the command line instead of the file
  if (_app.commandLine() && _app.commandLine()->haveVariable(full_name.c_str()))
    gp = _app.commandLine()->getPot();
  else
    gp = &_getpot_file;

  int vec_size = gp->vector_variable_size(full_name.c_str());
  if (gp->have_variable(full_name.c_str()))
    param->set().resize(vec_size);

  for (int i = 0; i < vec_size; ++i)
    param->set()[i] = gp->get_value_no_default(full_name.c_str(), param->get()[i], i);

  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setVectorParam<T>(short_name).resize(vec_size);
    for (int i = 0; i < vec_size; ++i)
      global_block->setVectorParam<T>(short_name)[i] = param->get()[i];
  }
}


template<typename T>
void Parser::setDoubleIndexParameter(const std::string & full_name, const std::string & short_name, InputParameters::Parameter<std::vector<std::vector<T> > >* param, bool in_global, GlobalParamsAction * global_block)
{
  GetPot *gp;

  // See if this variable was passed on the command line
  // if it was then we will retrieve the value from the command line instead of the file
  if (_app.commandLine() && _app.commandLine()->haveVariable(full_name.c_str()))
    gp = _app.commandLine()->getPot();
  else
    gp = &_getpot_file;

  // Get the full string assigned to the variable full_name
  std::string buffer = gp->get_value_no_default(full_name, "");

  // split vector at delim ;
  // NOTE: the substrings are _not_ of type T yet
  std::vector<std::string> first_tokenized_vector;
  MooseUtils::tokenize(buffer, first_tokenized_vector, 1, ";");
  param->set().resize(first_tokenized_vector.size());

  for (unsigned j = 0; j < first_tokenized_vector.size(); ++j)
    if (!MooseUtils::tokenizeAndConvert<T>(first_tokenized_vector[j], param->set()[j]))
      mooseError("Reading parameter " << short_name << " failed.");

  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setDoubleIndexParam<T>(short_name).resize(first_tokenized_vector.size());
    for (unsigned j = 0; j < first_tokenized_vector.size(); ++j)
    {
      global_block->setDoubleIndexParam<T>(short_name)[j].resize(param->get()[j].size());
      for (unsigned int i = 0; i < param->get()[j].size(); ++i)
        global_block->setDoubleIndexParam<T>(short_name)[j][i] = param->get()[j][i];
    }
  }
}

template<typename T>
void Parser::setScalarComponentParameter(const std::string & full_name, const std::string & short_name, InputParameters::Parameter<T> * param, bool in_global, GlobalParamsAction * global_block)
{
  GetPot *gp;

  // See if this variable was passed on the command line
  // if it was then we will retrieve the value from the command line instead of the file
  if (_app.commandLine() && _app.commandLine()->haveVariable(full_name.c_str()))
    gp = _app.commandLine()->getPot();
  else
    gp = &_getpot_file;

  int vec_size = gp->vector_variable_size(full_name.c_str());

  if (vec_size != LIBMESH_DIM)
    mooseError(std::string("Error in Scalar Component parameter ") + full_name + ": size is " << vec_size
               << ", should be " << LIBMESH_DIM);

  T value;
  for (int i = 0; i < vec_size; ++i)
    value(i) = Real(gp->get_value_no_default(full_name.c_str(), static_cast<Real>(0.0), i));

  param->set() = value;
  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setScalarParam<T>(short_name) = value;
  }
}

template<typename T>
void Parser::setVectorComponentParameter(const std::string & full_name, const std::string & short_name, InputParameters::Parameter<std::vector<T> > * param, bool in_global, GlobalParamsAction * global_block)
{
  GetPot *gp;

  // See if this variable was passed on the command line
  // if it was then we will retrieve the value from the command line instead of the file
  if (_app.commandLine() && _app.commandLine()->haveVariable(full_name.c_str()))
    gp = _app.commandLine()->getPot();
  else
    gp = &_getpot_file;

  int vec_size = gp->vector_variable_size(full_name.c_str());

  if (vec_size % LIBMESH_DIM)
    mooseError(std::string("Error in Vector Component parameter ") + full_name + ": size is " << vec_size
               << ", should be a multiple of " << LIBMESH_DIM);

  std::vector<T> values;
  for (int i = 0; i < vec_size / LIBMESH_DIM; ++i)
  {
    T value;
    for (int j=0; j < LIBMESH_DIM; ++j)
      value(j) = Real(gp->get_value_no_default(full_name.c_str(), static_cast<Real>(0.0), i*LIBMESH_DIM+j));
    values.push_back(value);
  }

  param->set() = values;

  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setVectorParam<T>(short_name).resize(vec_size, values[0]);
    for (int i = 0; i < vec_size / LIBMESH_DIM; ++i)
      global_block->setVectorParam<T>(short_name)[i] = values[0];
  }
}

template<>
void Parser::setScalarParameter<RealVectorValue>(const std::string & full_name, const std::string & short_name, InputParameters::Parameter<RealVectorValue> * param, bool in_global, GlobalParamsAction * global_block)
{
  setScalarComponentParameter(full_name, short_name, param, in_global, global_block);
}

template<>
void Parser::setScalarParameter<Point>(const std::string & full_name, const std::string & short_name, InputParameters::Parameter<Point> * param, bool in_global, GlobalParamsAction * global_block)
{
  setScalarComponentParameter(full_name, short_name, param, in_global, global_block);
}

template<>
void Parser::setScalarParameter<MooseEnum>(const std::string & full_name, const std::string & short_name, InputParameters::Parameter<MooseEnum> * param, bool in_global, GlobalParamsAction * global_block)
{
  GetPot *gp;

  // See if this variable was passed on the command line
  // if it was then we will retrieve the value from the command line instead of the file
  if (_app.commandLine() && _app.commandLine()->haveVariable(full_name.c_str()))
    gp = _app.commandLine()->getPot();
  else
    gp = &_getpot_file;

  MooseEnum current_param = param->get();
  std::string current_name = current_param;

  std::string value = gp->get_value_no_default(full_name.c_str(), current_name);

  param->set() = value;
  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setScalarParam<MooseEnum>(short_name) = current_param;
  }
}

template<>
void Parser::setScalarParameter<MultiMooseEnum>(const std::string & full_name, const std::string & short_name, InputParameters::Parameter<MultiMooseEnum> * param, bool in_global, GlobalParamsAction * global_block)
{
  GetPot *gp;

  // See if this variable was passed on the command line
  // if it was then we will retrieve the value from the command line instead of the file
  if (_app.commandLine() && _app.commandLine()->haveVariable(full_name.c_str()))
    gp = _app.commandLine()->getPot();
  else
    gp = &_getpot_file;

  MultiMooseEnum current_param = param->get();

  int vec_size = gp->vector_variable_size(full_name.c_str());

  std::string raw_values;
  for (int i = 0; i < vec_size; ++i)
  {
    std::string single_value = gp->get_value_no_default(full_name.c_str(), "", i);
    raw_values += ' ' + single_value;
  }

  param->set() = raw_values;

  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setScalarParam<MultiMooseEnum>(short_name) = current_param;
  }
}

template<>
void Parser::setScalarParameter<RealTensorValue>(const std::string & full_name, const std::string & short_name, InputParameters::Parameter<RealTensorValue> * param, bool in_global, GlobalParamsAction * global_block)
{
  GetPot *gp;

  // See if this variable was passed on the command line
  // if it was then we will retrieve the value from the command line instead of the file
  if (_app.commandLine() && _app.commandLine()->haveVariable(full_name.c_str()))
    gp = _app.commandLine()->getPot();
  else
    gp = &_getpot_file;

  int vec_size = gp->vector_variable_size(full_name.c_str());
  if (vec_size != LIBMESH_DIM * LIBMESH_DIM)
    mooseError(std::string("Error in RealTensorValue parameter ") + full_name + ": size is " << vec_size
               << ", should be " << LIBMESH_DIM * LIBMESH_DIM);

  RealTensorValue value;
  for (int i = 0; i < LIBMESH_DIM; ++i)
    for (int j = 0; j < LIBMESH_DIM; ++j)
      value(i, j) = Real(gp->get_value_no_default(full_name.c_str(), static_cast<Real>(0.0), i * LIBMESH_DIM + j));

  param->set() = value;
  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setScalarParam<RealTensorValue>(short_name) = value;
  }
}

// Specialization for coupling a Real value where a postprocessor would be needed in MOOSE
template<>
void Parser::setScalarParameter<PostprocessorName>(const std::string & full_name, const std::string & short_name, InputParameters::Parameter<PostprocessorName> * param, bool in_global, GlobalParamsAction * global_block)
{
  GetPot *gp;

  // See if this variable was passed on the command line
  // if it was then we will retrieve the value from the command line instead of the file
  if (_app.commandLine() && _app.commandLine()->haveVariable(full_name.c_str()))
    gp = _app.commandLine()->getPot();
  else
    gp = &_getpot_file;

  PostprocessorName pps_name = gp->get_value_no_default(full_name.c_str(), param->get());

  // Set the value here
  param->set() = pps_name;

  Real real_value = -std::numeric_limits<Real>::max();
  std::istringstream ss(pps_name);

  if (ss >> real_value && ss.eof())
    _current_params->setDefaultPostprocessorValue(short_name, real_value);

  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setScalarParam<PostprocessorName>(short_name) = pps_name;
  }
}

template<>
void Parser::setVectorParameter<RealVectorValue>(const std::string & full_name, const std::string & short_name, InputParameters::Parameter<std::vector<RealVectorValue> > * param, bool in_global, GlobalParamsAction * global_block)
{
  setVectorComponentParameter(full_name, short_name, param, in_global, global_block);
}

template<>
void Parser::setVectorParameter<Point>(const std::string & full_name, const std::string & short_name, InputParameters::Parameter<std::vector<Point> > * param, bool in_global, GlobalParamsAction * global_block)
{
  setVectorComponentParameter(full_name, short_name, param, in_global, global_block);
}

template<>
void Parser::setVectorParameter<MooseEnum>(const std::string & full_name, const std::string & short_name, InputParameters::Parameter<std::vector<MooseEnum> > * param, bool in_global, GlobalParamsAction * global_block)
{
  GetPot *gp;

  // See if this variable was passed on the command line
  // if it was then we will retrieve the value from the command line instead of the file
  if (_app.commandLine() && _app.commandLine()->haveVariable(full_name.c_str()))
    gp = _app.commandLine()->getPot();
  else
    gp = &_getpot_file;

  std::vector<MooseEnum> enum_values = param->get();
  std::vector<std::string> values(enum_values.size());
  for (unsigned int i=0; i<values.size(); ++i)
    values[i] = static_cast<std::string>(enum_values[i]);

  /**
   * With MOOSE Enums we need a default object so it should have been passed in the param
   * pointer.  We are only going to use the first item in the vector (values[0]) and ignore the rest.
   */
  int vec_size = gp->vector_variable_size(full_name.c_str());
  if (gp->have_variable(full_name.c_str()))
    param->set().resize(vec_size, enum_values[0]);

  for (int i = 0; i < vec_size; ++i)
    param->set()[i] = gp->get_value_no_default(full_name.c_str(), values[0], i);

  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setVectorParam<MooseEnum>(short_name).resize(vec_size, enum_values[0]);
    for (int i = 0; i < vec_size; ++i)
      global_block->setVectorParam<MooseEnum>(short_name)[i] = values[0];
  }
}

// Specialization for coupling vectors. This routine handles default values and auto generated VariableValue vectors
template<>
void Parser::setVectorParameter<VariableName>(const std::string & full_name, const std::string & short_name, InputParameters::Parameter<std::vector<VariableName> > * param, bool /*in_global*/, GlobalParamsAction * /*global_block*/)
{
  GetPot *gp;

  // See if this variable was passed on the command line
  // if it was then we will retrieve the value from the command line instead of the file
  if (_app.commandLine() && _app.commandLine()->haveVariable(full_name.c_str()))
    gp = _app.commandLine()->getPot();
  else
    gp = &_getpot_file;

  int vec_size = gp->vector_variable_size(full_name.c_str());
  std::vector<VariableName> var_names(vec_size);

  bool has_var_names = false;
  for (int i = 0; i < vec_size; ++i)
  {
    VariableName var_name = gp->get_value_no_default(full_name.c_str(), "", i);

    Real real_value;
    std::istringstream ss(var_name);

    // If we are able to convert this value into a Real, then set a default coupled value
    if (ss >> real_value && ss.eof())
      // FIXME: the real_value is assigned to defaultCoupledValue overriding the value assigned before.
      // Currently there is no functionality to separately assign the correct "real_value[i]" in InputParameters
      _current_params->defaultCoupledValue(short_name, real_value);
    else
    {
      var_names[i] = var_name;
      has_var_names = true;
    }
  }

  if (has_var_names)
  {
    param->set().resize(vec_size);

    for (int i = 0; i < vec_size; ++i)
      if (var_names[i] == "")
        mooseError("MOOSE does not currently support a coupled vector where some parameters are reals and others are variables");
      else
        param->set()[i] = var_names[i];
  }
}
