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

#include <string>
#include <map>
#include <fstream>
#include <iomanip>
#include <algorithm>

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

#include "GlobalParamsAction.h"

#include "SyntaxTree.h"
#include "InputFileFormatter.h"
#include "YAMLFormatter.h"

#include "MooseTypes.h"

// libMesh
#include "libmesh/getpot.h"

Parser::Parser(MooseApp & app, ActionWarehouse & action_wh) :
    _app(app),
    _factory(app.getFactory()),
    _action_wh(action_wh),
    _action_factory(app.getActionFactory()),
    _syntax(_action_wh.syntax()),
    _syntax_formatter(NULL),
    _getpot_initialized(false),
    _sections_read(false)
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
  for (std::set<std::string>::iterator i = _inactive_strings.begin(); i != _inactive_strings.end(); ++i)
    if (s.find(*i) == 0)
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
  std::vector<std::string> all(1);
  all[0] = "__all__";

  MooseUtils::checkFileReadable(input_filename, true);

  // GetPot object
  _getpot_file.parse_input_file(input_filename);
  _getpot_initialized = true;
  _inactive_strings.clear();

  section_names = _getpot_file.get_section_names();
  // Set the class variable to indicate that sections names have been read, this is used later by the checkOverriddenParams function
  _sections_read = true;

  for (std::vector<std::string>::iterator i=section_names.begin(); i != section_names.end(); ++i)
  {
    curr_identifier = i->erase(i->size()-1);  // Chop off the last character (the trailing slash)

    // Before we retrieve any actions or build any objects, make sure that the section they are in is active
    if (isSectionActive(curr_identifier, active_lists))
    {
      // Extract the block parameters before constructing the action
      // There may be more than one Action registered for a given section in which case we need to
      // build them all
      bool is_parent;
      std::string registered_identifier = _syntax.isAssociated(*i, &is_parent);

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

          // Create the Action
          Action * action_obj = _action_factory.create(it->second._action, curr_identifier, params);
          mooseAssert (action_obj != NULL, std::string("Action") + it->second._action + " not created");

          // extract the MooseObject params if necessary
          MooseObjectAction * object_action = dynamic_cast<MooseObjectAction *>(action_obj);
          if (object_action)
            extractParams(curr_identifier, object_action->getObjectParams());

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

  for (std::map<std::string, std::vector<std::string> >::const_iterator i = active_lists.begin();
       i != active_lists.end(); ++i)
    for (std::vector<std::string>::const_iterator j = (i->second).begin(); j != (i->second).end(); ++j)
    {
      active_lists_set.insert(i->first);
      if (*j != "__all__")
        active_lists_set.insert(i->first + "/" + *j);
    }

  std::sort(sections.begin(), sections.end());

  std::set_difference(active_lists_set.begin(), active_lists_set.end(), sections.begin(), sections.end(),
                      std::inserter(difference, difference.end()));

  if (!difference.empty())
  {
    std::ostringstream oss;
    oss << "One or more active lists in the input file are missing a referenced section:\n";
    for (std::vector<std::string>::iterator i = difference.begin(); i != difference.end();  ++i)
      oss << *i << "\n";
    mooseError(oss.str());
  }
}

void
Parser::checkUnidentifiedParams(std::vector<std::string> & all_vars, bool error_on_warn)
{
  std::set<std::string> difference;
  std::string message_indicator(error_on_warn ? "*** ERROR" : "*** WARNING");

  std::sort(all_vars.begin(), all_vars.end());

  std::set_difference(all_vars.begin(), all_vars.end(), _extracted_vars.begin(), _extracted_vars.end(),
                      std::inserter(difference, difference.end()));

  // Remove unparsed parameters that were located in an inactive sections
  for (std::set<std::string>::iterator i=_inactive_strings.begin(); i != _inactive_strings.end(); ++i)
    for (std::set<std::string>::iterator j=difference.begin(); j != difference.end(); /*no increment*/)
    {
      std::set<std::string>::iterator curr = j++;
      if (curr->find(*i) != std::string::npos)
        difference.erase(curr);
    }

  if (!difference.empty())
  {
    std::ostringstream oss;

    oss << "\n" << message_indicator << ": The following parameters were unused in your input file:\n";
    for (std::set<std::string>::iterator i=difference.begin(); i != difference.end(); ++i)
      oss << *i << "\n";
    oss << message_indicator << "\n";

    if (error_on_warn)
      mooseError(oss.str());
    else
      Moose::out << oss.str();
  }
}

void
Parser::checkOverriddenParams(bool error_on_warn)
{
  if (!_sections_read && error_on_warn)
    // The user has requested errors but we haven't done any parsing yet so throw an error
    mooseError("No parsing has been done, so checking for overridden parameters is not possible");

  std::set<std::string> overridden_vars = _getpot_file.get_overridden_variables();
  std::string message_indicator(error_on_warn ? "*** ERROR" : "*** WARNING");

  if (!overridden_vars.empty())
  {
    std::ostringstream oss;

    oss << message_indicator << ": The following variables were overridden or supplied multiple times:\n";
    for (std::set<std::string>::const_iterator i=overridden_vars.begin();
         i != overridden_vars.end(); ++i)
      oss << *i << "\n";
    oss << message_indicator << "\n\n";

    if (error_on_warn)
      mooseError(oss.str());
    else
      Moose::out << oss.str();
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
Parser::buildFullTree(const std::string &search_string)
{
//  std::string prev_name = "";
//  std::vector<InputParameters *> params_ptrs(2);
  std::vector<std::pair<std::string, Syntax::ActionInfo> > all_names;

  for (std::multimap<std::string, Syntax::ActionInfo>::const_iterator iter = _syntax.getAssociatedActions().begin();
       iter != _syntax.getAssociatedActions().end(); ++iter)
  {
    Syntax::ActionInfo act_info = iter->second;
    // If the task is NULL that means we need to figure out which task
    // goes with this syntax for the purpose of building the Moose Object part of the tree.
    // We will figure this out by asking the ActionFactory for the registration info.
    if (act_info._task == "")
      act_info._task = _action_factory.getTaskName(act_info._action);

    all_names.push_back(std::pair<std::string, Syntax::ActionInfo>(iter->first, act_info));
  }

  for (std::vector<std::pair<std::string, Syntax::ActionInfo> >::iterator act_names = all_names.begin(); act_names != all_names.end(); ++act_names)
  {
    InputParameters action_obj_params = _action_factory.getValidParams(act_names->second._action);
    _syntax_formatter->insertNode(act_names->first, act_names->second._action, true, &action_obj_params);

    const std::string & task = act_names->second._task;
    std::string act_name = act_names->first;

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
            _syntax.verifyMooseObjectTask(moose_obj_params.get<std::string>("_moose_base"), task))                     // and that base is associated
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

  Moose::out << _syntax_formatter->print(search_string);
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
void Parser::setScalarParameter<MooseEnum>(const std::string & full_name, const std::string & short_name,
                                           InputParameters::Parameter<MooseEnum>* param, bool in_global, GlobalParamsAction *global_block);

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


// Macros for parameter extraction
#define dynamicCastAndExtractScalar(type, param, full_name, short_name, in_global, global_block)                                        \
  do                                                                                                                                    \
  {                                                                                                                                     \
    InputParameters::Parameter<type> * scalar_p = dynamic_cast<InputParameters::Parameter<type>*>(param);                               \
    if (scalar_p)                                                                                                                       \
      setScalarParameter<type>(full_name, short_name, scalar_p, in_global, global_block);                                               \
  } while(0)


#define dynamicCastAndExtractVector(type, param, full_name, short_name, in_global, global_block)                                        \
  do                                                                                                                                    \
  {                                                                                                                                     \
    InputParameters::Parameter<std::vector<type> > * vector_p = dynamic_cast<InputParameters::Parameter<std::vector<type> >*>(param);   \
    if (vector_p)                                                                                                                       \
      setVectorParameter<type>(full_name, short_name, vector_p, in_global, global_block);                                               \
  } while(0)

void
Parser::extractParams(const std::string & prefix, InputParameters &p)
{
  static const std::string global_params_block_name = "GlobalParams";

  static const std::string global_params_task = "set_global_params";
  ActionIterator act_iter = _action_wh.actionBlocksWithActionBegin(global_params_task);
  GlobalParamsAction *global_params_block = NULL;

  // We are grabbing only the first
  if (act_iter != _action_wh.actionBlocksWithActionEnd(global_params_task))
    global_params_block = dynamic_cast<GlobalParamsAction *>(*act_iter);

  for (InputParameters::iterator it = p.begin(); it != p.end(); ++it)
  {
    bool found = false;
    bool in_global = false;
    std::string orig_name = prefix + "/" + it->first;
    std::string full_name = orig_name;

    // Mark parameters appearing in the input file or command line
    if (_getpot_file.have_variable(full_name.c_str()) || (_app.commandLine() && _app.commandLine()->haveVariable(full_name.c_str())))
    {
      p.set_attributes(it->first, false);
      _extracted_vars.insert(full_name);  // Keep track of all variables extracted from the input file
      found = true;
    }
    // Wait! Check the GlobalParams section
    else if (global_params_block != NULL)
    {
      full_name = global_params_block_name + "/" + it->first;
      if (_getpot_file.have_variable(full_name.c_str()))
      {
        p.set_attributes(it->first, false);
        _extracted_vars.insert(full_name);  // Keep track of all variables extracted from the input file
        found = true;
        in_global = true;
      }
    }

    if (!found)
    {
      // In the case where we have OutFileName but it wasn't actually found in the input filename,
      // we will populate it with the actual parsed filename which is available here in the parser.

      InputParameters::Parameter<OutFileBase> * scalar_p = dynamic_cast<InputParameters::Parameter<OutFileBase>*>(it->second);
      if (scalar_p)
      {
        std::string input_file_name = getFileName();
        mooseAssert(input_file_name != "", "Input Filename is NULL");
        size_t pos = input_file_name.find_last_of('.');
        mooseAssert(pos != std::string::npos, "Unable to determine suffix of input file name");
        scalar_p->set() = input_file_name.substr(0,pos) + "_out";
        p.set_attributes(it->first, false);
      }
    }
    else
    {
      /**
       * Scalar types
       */
      // built-ins
      dynamicCastAndExtractScalar(Real                  , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractScalar(int                   , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractScalar(unsigned int          , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractScalar(bool                  , it->second, full_name, it->first, in_global, global_params_block);

      // Moose Scalars
      dynamicCastAndExtractScalar(SubdomainID           , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractScalar(BoundaryID            , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractScalar(RealVectorValue       , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractScalar(Point                 , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractScalar(MooseEnum             , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractScalar(RealTensorValue       , it->second, full_name, it->first, in_global, global_params_block);

      // Moose String-derived scalars
      dynamicCastAndExtractScalar(/*std::*/string       , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractScalar(SubdomainName         , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractScalar(BoundaryName          , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractScalar(FileName              , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractScalar(FileNameNoExtension   , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractScalar(MeshFileName          , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractScalar(OutFileBase           , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractScalar(VariableName          , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractScalar(NonlinearVariableName , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractScalar(AuxVariableName       , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractScalar(FunctionName          , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractScalar(UserObjectName        , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractScalar(PostprocessorName     , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractScalar(IndicatorName         , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractScalar(MarkerName            , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractScalar(MultiAppName          , it->second, full_name, it->first, in_global, global_params_block);

      /**
       * Vector types
       */
      // built-ins
      dynamicCastAndExtractVector(Real                  , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractVector(int                   , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractVector(unsigned int          , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractVector(bool                  , it->second, full_name, it->first, in_global, global_params_block);

      // Moose Vectors
      dynamicCastAndExtractVector(SubdomainID           , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractVector(BoundaryID            , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractVector(RealVectorValue       , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractVector(Point                 , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractVector(MooseEnum             , it->second, full_name, it->first, in_global, global_params_block);
      /* We won't try to do vectors of tensors ;) */

      // Moose String-derived vectors
      dynamicCastAndExtractVector(/*std::*/string       , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractVector(SubdomainName         , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractVector(BoundaryName          , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractVector(VariableName          , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractVector(NonlinearVariableName , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractVector(AuxVariableName       , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractVector(FunctionName          , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractVector(UserObjectName        , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractVector(IndicatorName         , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractVector(MarkerName            , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractVector(MultiAppName          , it->second, full_name, it->first, in_global, global_params_block);
      dynamicCastAndExtractVector(PostprocessorName     , it->second, full_name, it->first, in_global, global_params_block);
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
  param->set() = value;
  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setScalarParam<T>(short_name) = value;
  }
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
    value(i) = Real(gp->get_value_no_default(full_name.c_str(), (Real) 0.0, i));

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
      value(j) = Real(gp->get_value_no_default(full_name.c_str(), (Real) 0.0, i*LIBMESH_DIM+j));
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
      value(i, j) = Real(gp->get_value_no_default(full_name.c_str(), (Real) 0.0, i * LIBMESH_DIM + j));

  param->set() = value;
  if (in_global)
  {
    global_block->remove(short_name);
    global_block->setScalarParam<RealTensorValue>(short_name) = value;
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
    values[i] = (std::string)enum_values[i];

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
