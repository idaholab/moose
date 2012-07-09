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

#include "Parser.h"
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
#include "Moose.h"
#include "MooseApp.h"

#include "GlobalParamsAction.h"

#include "SyntaxTree.h"
#include "InputFileFormatter.h"
#include "YAMLFormatter.h"

#include "FileName.h"

// libMesh
#include "getpot.h"


template<>
void Parser::setScalarParameter<MooseEnum>(const std::string & full_name, const std::string & short_name,
                                           InputParameters::Parameter<MooseEnum>* param, bool in_global, GlobalParamsAction *global_block);


Parser::Parser(ActionWarehouse & action_wh) :
    _action_wh(action_wh),
    _syntax(_action_wh.syntax()),
    _syntax_formatter(NULL),
    _getpot_initialized(false),
    _sort_alpha(false),
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

void
Parser::parse(const std::string &input_filename)
{
  std::string curr_identifier;
  std::map<std::string, std::vector<std::string> > active_lists;
  std::vector<std::string> section_names;
  InputParameters active_list_params = validParams<Action>();
  InputParameters params = validParams<EmptyAction>();

  // vector for initializing active blocks
  std::vector<std::string> all(1);
  all[0] = "__all__";

  checkFileReadable(input_filename, true);

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

    for (std::multimap<std::string, Syntax::ActionInfo>::iterator i = iters.first; i != iters.second; ++i)
    {
      if (!is_parent)
      {
        params = ActionFactory::instance()->getValidParams(i->second._action);

        // Before we build any objects we need to make sure that the section they are in is active
        if (isSectionActive(curr_identifier, active_lists))
        {
          params.set<ActionWarehouse *>("awh") = &_action_wh;
          params.set<Parser *>("parser") = this;

          extractParams(curr_identifier, params);

          // Check Action Parameters Now
          params.checkParams(curr_identifier);

          // Add the parsed syntax to the parameters object for consumption by the Action
          params.set<std::string>("name") = curr_identifier;
          params.set<std::string>("action") = i->second._action_name;

          // Create the Action
          Action * action = ActionFactory::instance()->create(i->second._action, params);
          mooseAssert (action != NULL, std::string("Action") + i->second._action + " not created");

          // extract the MooseObject params if necessary
          ObjectAction * object_action = dynamic_cast<ObjectAction *>(action);
          if (object_action)
            extractParams(curr_identifier, object_action->getObjectParams());

          // add it to the warehouse
          _action_wh.addActionBlock(action);
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

    oss << message_indicator << ": The following parameters were unused in your input file:\n";
    for (std::set<std::string>::iterator i=difference.begin(); i != difference.end(); ++i)
      oss << *i << "\n";
    oss << message_indicator << "\n\n";

    if (error_on_warn)
      mooseError(oss.str());
    else
      std::cout << oss.str();
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
      std::cout << oss.str();
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
    // If the action_name is NULL that means we need to figure out which action_name
    // goes with this syntax for the purpose of building the Moose Object part of the tree.
    // We will figure this out by asking the ActionFactory for the registration info.
    if (act_info._action_name == "")
      act_info._action_name = ActionFactory::instance()->getActionName(act_info._action);

    all_names.push_back(std::pair<std::string, Syntax::ActionInfo>(iter->first, act_info));
  }

  // Sort the Syntax
  std::sort(all_names.begin(), all_names.end(), InputFileSort(_sort_alpha));

  for (std::vector<std::pair<std::string, Syntax::ActionInfo> >::iterator act_names = all_names.begin(); act_names != all_names.end(); ++act_names)
  {
    InputParameters action_obj_params = ActionFactory::instance()->getValidParams(act_names->second._action);
    _syntax_formatter->insertNode(act_names->first, act_names->second._action, true, &action_obj_params);

    const std::string & action_name = act_names->second._action_name;
    std::string act_name = act_names->first;

    // We need to see if this action is inherited from MooseObjectAction
    // If it is, then we will loop over all the Objects in MOOSE's Factory object to print them out
    // if they have matching "built_by_action" tags.
    if (action_obj_params.have_parameter<bool>("isObjectAction") && action_obj_params.get<bool>("isObjectAction"))
    {
      for (registeredMooseObjectIterator moose_obj = Factory::instance()->registeredObjectsBegin();
           moose_obj != Factory::instance()->registeredObjectsEnd();
           ++moose_obj)
      {
        InputParameters moose_obj_params = (moose_obj->second)();

        if (moose_obj_params.have_parameter<std::string>("built_by_action") &&
            (moose_obj_params.get<std::string>("built_by_action") == action_name ||
             // Print out aux_bcs which are "built_by_action" add_aux_kernel
             (action_name == "add_aux_bc" &&
              moose_obj_params.get<std::string>("built_by_action") == "add_aux_kernel")))
        {

          std::string name;
          size_t pos = 0;
          if (act_name[act_name.size()-1] == '*')
          {
            pos = act_name.size()-1;
            name = act_name.substr(0, pos) + moose_obj->first;
          }
          else
          {
            name = act_name + "/<type>/" + moose_obj->first;
          }

          moose_obj_params.set<std::string>("type") = moose_obj->first;
          moose_obj_params.seenInInputFile("type");

          _syntax_formatter->insertNode(name, moose_obj->first, false, &moose_obj_params);
        }
      }
    }
  }

  std::cout << _syntax_formatter->print(search_string);
}

void
Parser::tokenize(const std::string &str, std::vector<std::string> &elements, unsigned int min_len, const std::string &delims)
{
  elements.clear();

  std::string::size_type last_pos = str.find_first_not_of(delims, 0);
  std::string::size_type pos = str.find_first_of(delims, std::min(last_pos + min_len, str.size()));

  while (pos != std::string::npos || last_pos != std::string::npos)
  {
    elements.push_back(str.substr(last_pos, pos - last_pos));
    // skip delims between tokens
    last_pos = str.find_first_not_of(delims, pos);
    if (last_pos == std::string::npos) break;
    pos = str.find_first_of(delims, std::min(last_pos + min_len, str.size()));
  }
}

void
Parser::escape(std::string &str)
{
  std::map<char, std::string> escapes;
  escapes['\a'] = "\\a";
  escapes['\b'] = "\\b";
  escapes['\f'] = "\\f";
  escapes['\n'] = "\\n";
  escapes['\t'] = "\\t";
  escapes['\v'] = "\\v";
  escapes['\r'] = "\\r";

  for (std::map<char, std::string>::iterator it = escapes.begin(); it != escapes.end(); ++it)
    for (size_t pos=0; (pos=str.find(it->first, pos)) != std::string::npos; pos+=it->second.size())
      str.replace(pos, 1, it->second);
}


std::string
Parser::trim(std::string str, const std::string &white_space)
{
  std::string r = str.erase(str.find_last_not_of(white_space)+1);
  return r.erase(0,r.find_first_not_of(white_space));
}

bool Parser::pathContains(const std::string &expression,
                          const std::string &string_to_find,
                          const std::string &delims)
{
  std::vector<std::string> elements;

  tokenize(expression, elements, 0, delims);

  std::vector<std::string>::iterator found_it = std::find(elements.begin(), elements.end(), string_to_find);
  if (found_it != elements.end())
    return true;
  else
    return false;
}

const GetPot *
Parser::getPotHandle() const
{
  return _getpot_initialized ? &_getpot_file : NULL;
}

void
Parser::checkFileReadable(const std::string & filename, bool check_line_endings)
{
  std::ifstream in(filename.c_str(), std::ifstream::in);
  if (in.fail())
    mooseError((std::string("Unable to open file \"") + filename
                + std::string("\". Check to make sure that it exists and that you have read permission.")).c_str());

  if (check_line_endings)
  {
    std::istream_iterator<char> iter(in);
    std::istream_iterator<char> eos;
    in >> std::noskipws;
    while (iter != eos)
      if (*iter++ == '\r')
        mooseError(filename + " contains Windows(DOS) line endings which are not supported.");
  }

  in.close();
}

void
Parser::checkFileWritable(const std::string & filename)
{
  std::ofstream out(filename.c_str(), std::ofstream::out);
  if (out.fail())
    mooseError((std::string("Unable to open file \"") + filename
                + std::string("\". Check to make sure that it exists and that you have write permission.")).c_str());

  out.close();
}


void
Parser::setSortAlpha(bool sort_alpha_flag)
{
  _sort_alpha = sort_alpha_flag;
}

bool
Parser::getSortFlag() const
{
  return _sort_alpha;
}

void
Parser::extractParams(const std::string & prefix, InputParameters &p)
{
  static const std::string global_params_block_name = "GlobalParams";

  static const std::string global_params_action_name = "set_global_params";
  ActionIterator act_iter = _action_wh.actionBlocksWithActionBegin(global_params_action_name);
  GlobalParamsAction *global_params_block = NULL;

  // We are grabbing only the first
  if (act_iter != _action_wh.actionBlocksWithActionEnd(global_params_action_name))
    global_params_block = dynamic_cast<GlobalParamsAction *>(*act_iter);

  for (InputParameters::iterator it = p.begin(); it != p.end(); ++it)
  {
    bool found = false;
    bool in_global = false;
    std::string orig_name = prefix + "/" + it->first;
    std::string full_name = orig_name;

    // Mark parameters appearing in the input file or command line
    if (_getpot_file.have_variable(full_name.c_str()) || Moose::app->commandLine().haveVariable(full_name.c_str()))
    {
      p.seenInInputFile(it->first);
      _extracted_vars.insert(full_name);  // Keep track of all variables extracted from the input file
      found = true;
    }
    // Wait! Check the GlobalParams section
    else if (global_params_block != NULL)
    {
      full_name = global_params_block_name + "/" + it->first;
      if (_getpot_file.have_variable(full_name.c_str()))
      {
        p.seenInInputFile(it->first);
        _extracted_vars.insert(full_name);  // Keep track of all variables extracted from the input file
        found = true;
        in_global = true;
      }
    }

    if (found)
    {
      InputParameters::Parameter<Real> * real_param = dynamic_cast<InputParameters::Parameter<Real>*>(it->second);
      InputParameters::Parameter<int>  * int_param  = dynamic_cast<InputParameters::Parameter<int>*>(it->second);
      InputParameters::Parameter<unsigned int>  * uint_param  = dynamic_cast<InputParameters::Parameter<unsigned int>*>(it->second);
      InputParameters::Parameter<SubdomainID>  * subdomain_id_param  = dynamic_cast<InputParameters::Parameter<SubdomainID>*>(it->second);
      InputParameters::Parameter<BoundaryID>  * boundary_id_param  = dynamic_cast<InputParameters::Parameter<BoundaryID>*>(it->second);
      InputParameters::Parameter<bool> * bool_param = dynamic_cast<InputParameters::Parameter<bool>*>(it->second);
      InputParameters::Parameter<RealVectorValue> * real_vec_val_param = dynamic_cast<InputParameters::Parameter<RealVectorValue>*>(it->second);
      InputParameters::Parameter<RealTensorValue> * real_tensor_val_param = dynamic_cast<InputParameters::Parameter<RealTensorValue>*>(it->second);
      InputParameters::Parameter<std::string> * string_param = dynamic_cast<InputParameters::Parameter<std::string>*>(it->second);
      InputParameters::Parameter<FileName> * file_name_param = dynamic_cast<InputParameters::Parameter<FileName>*>(it->second);
      InputParameters::Parameter<MooseEnum> * enum_param = dynamic_cast<InputParameters::Parameter<MooseEnum>*>(it->second);
      InputParameters::Parameter<std::vector<Real> > * vec_real_param = dynamic_cast<InputParameters::Parameter<std::vector<Real> >*>(it->second);
      InputParameters::Parameter<std::vector<int>  > * vec_int_param  = dynamic_cast<InputParameters::Parameter<std::vector<int> >*>(it->second);
      InputParameters::Parameter<std::vector<unsigned int>  > * vec_uint_param  = dynamic_cast<InputParameters::Parameter<std::vector<unsigned int> >*>(it->second);
      InputParameters::Parameter<std::vector<SubdomainID>  > * vec_subdomain_id_param  = dynamic_cast<InputParameters::Parameter<std::vector<SubdomainID> >*>(it->second);
      InputParameters::Parameter<std::vector<BoundaryID>  > * vec_boundary_id_param  = dynamic_cast<InputParameters::Parameter<std::vector<BoundaryID> >*>(it->second);
      InputParameters::Parameter<std::vector<bool>  > * vec_bool_param  = dynamic_cast<InputParameters::Parameter<std::vector<bool> >*>(it->second);
      InputParameters::Parameter<std::vector<std::string> > * vec_string_param = dynamic_cast<InputParameters::Parameter<std::vector<std::string> >*>(it->second);


      if (real_param)
        setScalarParameter<Real>(full_name, it->first, real_param, in_global, global_params_block);
      else if (int_param)
        setScalarParameter<int>(full_name, it->first, int_param, in_global, global_params_block);
      else if (subdomain_id_param)
        setScalarParameter<SubdomainID>(full_name, it->first, subdomain_id_param, in_global, global_params_block);
      else if (boundary_id_param)
        setScalarParameter<BoundaryID>(full_name, it->first, boundary_id_param, in_global, global_params_block);
      else if (uint_param)
        setScalarParameter<unsigned int>(full_name, it->first, uint_param, in_global, global_params_block);
      else if (bool_param)
        setScalarParameter<bool>(full_name, it->first, bool_param, in_global, global_params_block);
      // Real Vector Param
      else if (real_vec_val_param)
        setRealVectorValue(full_name, it->first, real_vec_val_param, in_global, global_params_block);
      else if (real_tensor_val_param)
        setRealTensorValue(full_name, it->first, real_tensor_val_param, in_global, global_params_block);
      else if (string_param)
        setScalarParameter<std::string>(full_name, it->first, string_param, in_global, global_params_block);
      else if (file_name_param)
        setScalarParameter<FileName>(full_name, it->first, file_name_param, in_global, global_params_block);
      else if (enum_param)
        setScalarParameter<MooseEnum>(full_name, it->first, enum_param, in_global, global_params_block);
      else if (vec_real_param)
        setVectorParameter<Real>(full_name, it->first, vec_real_param, in_global, global_params_block);
      else if (vec_int_param)
        setVectorParameter<int>(full_name, it->first, vec_int_param, in_global, global_params_block);
      else if (vec_uint_param)
        setVectorParameter<unsigned int>(full_name, it->first, vec_uint_param, in_global, global_params_block);
      else if (vec_subdomain_id_param)
        setVectorParameter<SubdomainID>(full_name, it->first, vec_subdomain_id_param, in_global, global_params_block);
      else if (vec_boundary_id_param)
        setVectorParameter<BoundaryID>(full_name, it->first, vec_boundary_id_param, in_global, global_params_block);
      else if (vec_bool_param)
        setVectorParameter<bool>(full_name, it->first, vec_bool_param, in_global, global_params_block);
      else if (vec_string_param)
        setVectorParameter<std::string>(full_name, it->first, vec_string_param, in_global, global_params_block);
    }
  }
}

void Parser::setRealVectorValue(const std::string & full_name, const std::string & short_name, InputParameters::Parameter<RealVectorValue> * param, bool in_global, GlobalParamsAction * global_block)
{
  GetPot *gp;

  // See if this variable was passed on the command line
  // if it was then we will retrieve the value from the command line instead of the file
  if (Moose::app->commandLine().isVariableOnCommandLine(full_name))
    gp = Moose::app->commandLine().getPot();
  else
    gp = &_getpot_file;

  int vec_size = gp->vector_variable_size(full_name.c_str());

  if (vec_size != LIBMESH_DIM)
    mooseError(std::string("Error in RealVectorValue parameter ") + full_name + ": size is " << vec_size
               << ", should be " << LIBMESH_DIM);

  RealVectorValue value;
  for (int i = 0; i < vec_size; ++i)
    value(i) = Real(gp->get_value_no_default(full_name.c_str(), (Real) 0.0, i));

  param->set() = value;
  if (in_global)
    global_block->setScalarParam<RealVectorValue>(short_name) = value;
}

template<typename T>
void Parser::setScalarParameter(const std::string & full_name, const std::string & short_name, InputParameters::Parameter<T> * param, bool in_global, GlobalParamsAction * global_block)
{
  GetPot *gp;

  // See if this variable was passed on the command line
  // if it was then we will retrieve the value from the command line instead of the file
  if (Moose::app->commandLine().isVariableOnCommandLine(full_name))
    gp = Moose::app->commandLine().getPot();
  else
    gp = &_getpot_file;

  T value = gp->get_value_no_default(full_name.c_str(), param->get());
  param->set() = value;
  if (in_global)
    global_block->setScalarParam<T>(short_name) = value;
}

template<typename T>
void Parser::setVectorParameter(const std::string & full_name, const std::string & short_name, InputParameters::Parameter<std::vector<T> >* param, bool in_global, GlobalParamsAction * global_block)
{
  GetPot *gp;

  // See if this variable was passed on the command line
  // if it was then we will retrieve the value from the command line instead of the file
  if (Moose::app->commandLine().isVariableOnCommandLine(full_name))
    gp = Moose::app->commandLine().getPot();
  else
    gp = &_getpot_file;

  int vec_size = gp->vector_variable_size(full_name.c_str());
  if (gp->have_variable(full_name.c_str()))
    param->set().resize(vec_size);

  for (int i = 0; i < vec_size; ++i)
    param->set()[i] = gp->get_value_no_default(full_name.c_str(), param->get()[i], i);

  if (in_global)
  {
    global_block->setVectorParam<T>(short_name).resize(vec_size);
    for (int i = 0; i < vec_size; ++i)
      global_block->setVectorParam<T>(short_name)[i] = param->get()[i];
  }
}

void Parser::setRealTensorValue(const std::string & full_name, const std::string & short_name, InputParameters::Parameter<RealTensorValue> * param, bool in_global, GlobalParamsAction * global_block)
{
  GetPot *gp;

  // See if this variable was passed on the command line
  // if it was then we will retrieve the value from the command line instead of the file
  if (Moose::app->commandLine().isVariableOnCommandLine(full_name))
    gp = Moose::app->commandLine().getPot();
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
    global_block->setScalarParam<RealTensorValue>(short_name) = value;
}

template<>
void Parser::setScalarParameter<MooseEnum>(const std::string & full_name, const std::string & short_name, InputParameters::Parameter<MooseEnum> * param, bool in_global, GlobalParamsAction * global_block)
{
  GetPot *gp;

  // See if this variable was passed on the command line
  // if it was then we will retrieve the value from the command line instead of the file
  if (Moose::app->commandLine().isVariableOnCommandLine(full_name))
    gp = Moose::app->commandLine().getPot();
  else
    gp = &_getpot_file;

  std::string current_name = param->get();

  std::string value = gp->get_value_no_default(full_name.c_str(), current_name);
  param->set() = value;
  if (in_global)
    global_block->setScalarParam<MooseEnum>(short_name) = value;
}

//--------------------------------------------------------------------------
// Input File Sorter Functor methods

Parser::InputFileSort::InputFileSort(bool sort_alpha):
    _sort_alpha(sort_alpha)
{
  _o.reserve(16);
  _o.push_back("GlobalParams");
  _o.push_back("Problem");
  _o.push_back("Mesh");
  _o.push_back("Functions");
  _o.push_back("Preconditioning");
  _o.push_back("Variables");
  _o.push_back("AuxVariables");
  _o.push_back("Kernels");
  _o.push_back("DGKernels");
  _o.push_back("DiracKernels");
  _o.push_back("AuxKernels");
  _o.push_back("Dampers");
  _o.push_back("BCs");
  _o.push_back("AuxBCs");
  _o.push_back("Materials");
  _o.push_back("Postprocessors");
  _o.push_back("Executioner");
  _o.push_back("Output");
}

bool
Parser::InputFileSort::operator() (Action *a, Action *b) const
{
  std::vector<std::string> elements;
  std::string short_a, short_b;
  Parser::tokenize(a->name(), elements);
  short_a = elements[0];
  elements.clear();
  Parser::tokenize(b->name(), elements);
  short_b = elements[0];

  return sorter(short_a, short_b) >= 0 ? false : true;
}

bool
Parser::InputFileSort::operator() (const std::pair<std::string, Syntax::ActionInfo> &a, const std::pair<std::string, Syntax::ActionInfo> &b) const
{
  std::vector<std::string> elements;
  std::string short_a, short_b;
  Parser::tokenize(a.first, elements);
  short_a = elements[0];
  elements.clear();
  Parser::tokenize(b.first, elements);
  short_b = elements[0];

  int ret = sorter(short_a, short_b);
  if (ret == 0)
    return a.first.size() > b.first.size() ? false : true;
  else
    return ret > 0 ? false : true;
}

int
Parser::InputFileSort::sorter(const std::string &a, const std::string &b) const
{
  if (_sort_alpha)
    return a < b ? -1 : 1;
  else
    return std::find(_o.begin(), _o.end(), a) - std::find(_o.begin(), _o.end(), b);
}
