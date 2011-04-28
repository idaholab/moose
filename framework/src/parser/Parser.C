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

#include "MProblem.h"
#include "MooseMesh.h"
#include "Executioner.h"
#include "Moose.h"

#include "GlobalParamsAction.h"

// libMesh
#include "getpot.h"

Parser::Parser():
  _mesh(NULL),
  _displaced_mesh(NULL),
  _problem(NULL),
  _executioner(NULL),
  _exreader(NULL),
  _loose(false),
  _getpot_initialized(false)
{
  initOptions();
  
  if (Moose::command_line != NULL)
  {
    if (searchCommandLine("Help"))
    {
      printUsage();
      exit(0);
    }
    if (searchCommandLine("Dump"))
    {
      buildFullTree( &Parser::printInputParameterData );
      exit(0);
    }
    if (searchCommandLine("YAML"))
    {
      //important: start and end yaml data delimiters used by python
      std::cout << "**START YAML DATA**\n";
      std::cout << "  - name: TODO\n";
      std::cout << "    desc:\n";
      std::cout << "    type:\n";
      std::cout << "    parameters:\n";
      std::cout << "    subblocks:\n";
      buildFullTree( &Parser::printYAMLParameterData );
      std::cout << "**END YAML DATA**\n";
      exit(0);
    }
  }
  else
    printUsage();

  Moose::action_warehouse.clear();                      // new parser run, get rid of old actions
}

Parser::~Parser()
{
  delete _exreader;
}

void
Parser::initOptions()
{
  // Static Data Initialization
  CLIOption cli_opt;
  std::vector<std::string> syntax;

  syntax.clear();
  cli_opt.desc = "Shows the parsed input file before running the simulation";
  syntax.push_back("--show_tree");
  cli_opt.cli_syntax = syntax;
  cli_opt.required = false;
  _cli_options["ShowTree"] = cli_opt;

  syntax.clear();
  cli_opt.desc = "Displays CLI usage statement";
  syntax.push_back("-h");
  syntax.push_back("--help");
  cli_opt.cli_syntax = syntax;
  cli_opt.required = false;
  _cli_options["Help"] = cli_opt;

  syntax.clear();
  cli_opt.desc = "Shows a full dump of available input file syntax";
  syntax.push_back("--dump");
  cli_opt.cli_syntax = syntax;
  cli_opt.required = false;
  _cli_options["Dump"] = cli_opt;
  
  syntax.clear();
  cli_opt.desc = "Dumps input file syntax in YAML format";
  syntax.push_back("--yaml");
  cli_opt.cli_syntax = syntax;
  cli_opt.required = false;
  _cli_options["YAML"] = cli_opt;

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

  // Finally see if any of the deactive strings are partially contained in this path
  for (std::set<std::string>::iterator i = _inactive_strings.begin(); i != _inactive_strings.end(); ++i)
    if (s.find(*i) != std::string::npos)
      retValue = false;
  
  // If this section is not active - then keep track of it for future checks
  if (!retValue)
    _inactive_strings.insert(s);

  return retValue;
}

bool
Parser::searchCommandLine(const std::string &option_name)
{
  std::map<std::string, CLIOption>::iterator pos;

  pos = _cli_options.find(option_name);
  if (pos != _cli_options.end())
    for (unsigned int i=0; i<pos->second.cli_syntax.size(); ++i)
      if (Moose::command_line->search(pos->second.cli_syntax[i]))
        return true;

  return false;  
}

void
Parser::parse(const std::string &input_filename)
{
  std::string curr_identifier;
  std::map<std::string, std::vector<std::string> > active_lists;
  std::vector<std::string> section_names;
  InputParameters active_list_params = validParams<Action>();
  
  // vector for initializing active blocks
  std::vector<std::string> all(1);
  all[0] = "__all__";
  
  checkFileReadable(input_filename, true);
  
  // GetPot object
  _getpot_file.parse_input_file(input_filename);
  _getpot_initialized = true;
  _inactive_strings.clear();

  section_names = _getpot_file.get_section_names();

  for (std::vector<std::string>::iterator i=section_names.begin(); i != section_names.end(); ++i)
  {
    curr_identifier = i->erase(i->size()-1);  // Chop off the last character (the trailing slash)
    
    // Extract the block parameters before constructing the action
    InputParameters params = ActionFactory::instance()->getValidParams(*i);

    // Before we build any objects we need to make sure that the section they are in is active
    // and that they aren't an unregistered parent (signified by an empty params object)
    if (isSectionActive(curr_identifier, active_lists) && params.n_parameters() != 0)
    {
      params.set<Parser *>("parser_handle") = this;
    
      extractParams(curr_identifier, params);

      // Create the Action
      Action * action = ActionFactory::instance()->create(curr_identifier, params);

      // extract the MooseObject params if necessary
      MooseObjectAction * moose_object_action = dynamic_cast<MooseObjectAction *>(action);
      if (moose_object_action)
        extractParams(curr_identifier, moose_object_action->getMooseObjectParams());
    
      // add it to the warehouse
      Moose::action_warehouse.addActionBlock(action);
    }

    // Extract and save the current "active" list in the datastructure
    active_list_params.set<std::vector<std::string> >("active") = all;
    extractParams(curr_identifier, active_list_params);
    active_lists[curr_identifier] = active_list_params.get<std::vector<std::string> >("active");
  }

  // TODO: Check active used?
  // This will throw a mooseError if the active lists aren't all used up
  // _input_tree->checkActiveUsed();

  // Print the input file syntax if requested
  if (Moose::command_line && searchCommandLine("ShowTree"))
  {
    const std::string * prev_name = NULL;
    for (ActionIterator a = Moose::action_warehouse.inputFileActionsBegin();
         a != Moose::action_warehouse.inputFileActionsEnd();
         ++a)
    {
      (*a)->printInputFile(prev_name);
      prev_name = &((*a)->name());
    }
  }
}


void
Parser::buildFullTree( void (Parser::*data_printer)(const std::string &name, const std::string *type, std::vector<InputParameters *> & param_ptrs) )
{
  for (registeredActionIterator act_obj = ActionFactory::instance()->registeredActionsBegin();
       act_obj != ActionFactory::instance()->registeredActionsEnd();
       ++act_obj)
  {
    InputParameters action_obj_params = ActionFactory::instance()->getValidParams(act_obj->first);
    const std::string & action_name = act_obj->second;

    std::vector<InputParameters *> params_ptrs(2);
    params_ptrs[0] = &action_obj_params;

    if (action_obj_params.have_parameter<std::string>("built_by_action") &&
        action_obj_params.get<std::string>("built_by_action") == action_name &&
        ActionFactory::instance()->isParsed(act_obj->first))
    {
      params_ptrs[1] = NULL;
      (this->*data_printer)(act_obj->first, NULL, params_ptrs);
    }
    
    
    for (registeredMooseObjectIterator moose_obj = Factory::instance()->registeredObjectsBegin();
         moose_obj != Factory::instance()->registeredObjectsEnd();
         ++moose_obj)
    {
      InputParameters moose_obj_params = (moose_obj->second)();
      
      if (moose_obj_params.have_parameter<std::string>("built_by_action") &&
          moose_obj_params.get<std::string>("built_by_action") == action_name)
      { 
        params_ptrs[1] = &moose_obj_params;
        (this->*data_printer)(act_obj->first, &(moose_obj->first), params_ptrs);
      }
    }
  }
}


void
Parser::printInputParameterData(const std::string & name, const std::string * type, std::vector<InputParameters *> & param_ptrs)
{
  std::ostream & out = std::cout;
  std::vector<std::string> elements;
  Parser::tokenize(name, elements);

  std::string spacing = "";
  for (unsigned int i=0; i<elements.size(); ++i)
    spacing += "  ";

  
  out << "\n" << spacing << "block name: " << name << "\n";
  
  if (type)
  {
    out << spacing << "type: " << *type << "\n";
    std::string class_desc = param_ptrs[1]->getClassDescription();
    if (class_desc != "")
      out << spacing << "description: " << class_desc << "\n";
  }
  
  
  out << spacing << "{\n";
  
  out << spacing << "  Valid Parameters:\n";

  for (unsigned int i=0; i<param_ptrs.size() && param_ptrs[i]; ++i)
  {
    for (InputParameters::iterator iter = param_ptrs[i]->begin(); iter != param_ptrs[i]->end(); ++iter) 
    {
      // First make sure we want to see this parameter
      if (param_ptrs[i]->isPrivate(iter->first)) 
        continue;

      // Block params may be required and will have a doc string
      std::string required = param_ptrs[i]->isParamRequired(iter->first) || iter->first == "type" ? "*" : " ";
      std::string valid = param_ptrs[i]->isParamValid(iter->first) ? " (valid)" : " ";

      out << spacing << "    " << std::left << std::setw(30) << required + iter->first << ": ";
    
      iter->second->print(out);

      out << valid << "\n" << spacing << "    " << std::setw(30) << " " << "    " << param_ptrs[i]->getDocString(iter->first) << "\n";
    }
  }
  
//  visitChildren(&ParserBlock::printBlockData, true, false);

  out << spacing << "}\n";
}


void
Parser::printYAMLParameterData(const std::string & name, const std::string * type, std::vector<InputParameters *> & param_ptrs)
{
  std::ostream & out = std::cout;
  std::vector<std::string> elements;
  Parser::tokenize(name, elements);

  std::string spacing = "";
  for (unsigned int i=0; i<elements.size(); ++i)
    spacing += "  ";

  out << spacing << "- name: " << name << "\n";
  spacing += "  ";

  std::string class_desc, type_str;
  if (type)
  {
    class_desc = param_ptrs[1]->getClassDescription();
    type_str = *type;
  }
  
  //will print "" if there is no type or desc, which translates to None in python
  out << spacing << "desc: !!str " << class_desc << "\n";
  out << spacing << "type: " << type_str << "\n";
  
  out << spacing << "parameters:\n";
  std::string subblocks = spacing + "subblocks: \n";
  spacing += "  ";

  for (unsigned int i=0; i<param_ptrs.size() && param_ptrs[i]; ++i)
  {
    for (InputParameters::iterator iter = param_ptrs[i]->begin(); iter != param_ptrs[i]->end(); ++iter) 
    {
      std::string name = iter->first;
      // First make sure we want to see this parameter, also block active and type
      if (param_ptrs[i]->isPrivate(iter->first) || name == "active" || name == "type") 
        continue;

      // Block params may be required and will have a doc string
      std::string required = param_ptrs[i]->isParamRequired(iter->first) ? "Yes" : "No";

      out << spacing << "- name: " << name << "\n";
      out << spacing << "  required: " << required << "\n";
      out << spacing << "  default: !!str ";

      //prints the value, which is the default value when dumping the tree
      //because it hasn't been changed
      iter->second->print(out);

      out << "\n" << spacing << "  description: |\n    " << spacing
                << param_ptrs[i]->getDocString(iter->first) << "\n";
    }
  }

  //if there aren't any sub blocks it will just parse as None in python
  out << subblocks;
}

void
Parser::tokenize(const std::string &str, std::vector<std::string> &elements, const std::string &delims)
{
  std::string::size_type last_pos = str.find_first_not_of(delims, 0);
  std::string::size_type pos = str.find_first_of(delims, last_pos);

  while (pos != std::string::npos || last_pos != std::string::npos)
  {
    elements.push_back(str.substr(last_pos, pos - last_pos));
    // skip delims between tokens
    last_pos = str.find_first_not_of(delims, pos);
    pos = str.find_first_of(delims, last_pos);
  }
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
  
  tokenize(expression, elements, delims);

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

Executioner *
Parser::getExecutioner()
{
  mooseAssert(_executioner != NULL, "Executioner is NULL!");
  return _executioner;
}

void
Parser::execute()
{
  Action *action = Moose::action_warehouse.allActionsBegin(this);
  do
  {
    // if the action is MOOSE object-based, check tht params are valid
    MooseObjectAction * mo_action = dynamic_cast<MooseObjectAction *>(action);
    if (mo_action != NULL)
      checkParams(mo_action->name(), mo_action->getMooseObjectParams());
    action->act();
  } while ( (action = Moose::action_warehouse.allActionsNext(this)) );

         /*
  for (ActionIterator a = Moose::action_warehouse.allActionsBegin(this);
       a != Moose::action_warehouse.allActionsEnd();
       ++a)
    (*a)->act();
         */
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
Parser::printUsage() const
{
  // Grab the first item out of argv
  std::string command((*Moose::command_line)[0]);
  command.substr(command.find_last_of("/\\")+1);

  std::cout << "\nUsage: " << command << " [-i <input file> --show_tree | <Option>]\n\n"
            << "Options:\n" << std::left;
  
  for (std::map<std::string, CLIOption>::const_iterator i=_cli_options.begin(); i != _cli_options.end(); ++i)
  {
    std::stringstream oss;
    for (unsigned int j=0; j<i->second.cli_syntax.size(); ++j)
    {
      if (j) oss << " | ";
      oss << i->second.cli_syntax[j];
    }
    std::cout << "  " << std::setw(50) << oss.str() << i->second.desc << "\n";
  }
  
  std::cout << "\nSolver Options:\n"
            << "  See solver manual for details (Petsc or Trilinos)\n";
  exit(0);
}

void
Parser::extractParams(const std::string & prefix, InputParameters &p)
{
  static const std::string global_params_block_name = "GlobalParams";

  static const std::string global_params_action_name = "set_global_params";
  ActionIterator act_iter = Moose::action_warehouse.actionBlocksWithActionBegin(global_params_action_name);
  GlobalParamsAction *global_params_block = NULL;

  // We are grabbing only the first 
  if (act_iter != Moose::action_warehouse.actionBlocksWithActionEnd(global_params_action_name))
    global_params_block = dynamic_cast<GlobalParamsAction *>(*act_iter);

  for (InputParameters::iterator it = p.begin(); it != p.end(); ++it)
  {
    bool found = false;
    bool in_global = false;
    std::string orig_name = prefix + "/" + it->first;
    std::string full_name = orig_name;

    // Mark parameters appearing in the input file
    if (_getpot_file.have_variable(full_name.c_str()))
    {
      p.seenInInputFile(it->first);
      found = true; 
    }
    // Wait! Check the GlobalParams section
    else if (global_params_block != NULL)
    {
      full_name = global_params_block_name + "/" + it->first;
      if (_getpot_file.have_variable(full_name.c_str()))
      {
        p.seenInInputFile(it->first);
        found = true;
        in_global = true;
      }
    }

    if (found)
    {
      InputParameters::Parameter<Real> * real_param = dynamic_cast<InputParameters::Parameter<Real>*>(it->second);
      InputParameters::Parameter<int>  * int_param  = dynamic_cast<InputParameters::Parameter<int>*>(it->second);
      InputParameters::Parameter<unsigned int>  * uint_param  = dynamic_cast<InputParameters::Parameter<unsigned int>*>(it->second);
      InputParameters::Parameter<bool> * bool_param = dynamic_cast<InputParameters::Parameter<bool>*>(it->second);
      InputParameters::Parameter<std::string> * string_param = dynamic_cast<InputParameters::Parameter<std::string>*>(it->second);
      InputParameters::Parameter<std::vector<Real> > * vec_real_param = dynamic_cast<InputParameters::Parameter<std::vector<Real> >*>(it->second);
      InputParameters::Parameter<std::vector<int>  > * vec_int_param  = dynamic_cast<InputParameters::Parameter<std::vector<int> >*>(it->second);
      InputParameters::Parameter<std::vector<unsigned int>  > * vec_uint_param  = dynamic_cast<InputParameters::Parameter<std::vector<unsigned int> >*>(it->second);
      InputParameters::Parameter<std::vector<bool>  > * vec_bool_param  = dynamic_cast<InputParameters::Parameter<std::vector<bool> >*>(it->second);
      InputParameters::Parameter<std::vector<std::string> > * vec_string_param = dynamic_cast<InputParameters::Parameter<std::vector<std::string> >*>(it->second);
      InputParameters::Parameter<std::vector<std::vector<Real> > > * tensor_real_param = dynamic_cast<InputParameters::Parameter<std::vector<std::vector<Real> > >*>(it->second);
      InputParameters::Parameter<std::vector<std::vector<int> > >  * tensor_int_param  = dynamic_cast<InputParameters::Parameter<std::vector<std::vector<int> > >*>(it->second);
      InputParameters::Parameter<std::vector<std::vector<bool> > > * tensor_bool_param = dynamic_cast<InputParameters::Parameter<std::vector<std::vector<bool> > >*>(it->second);

      if (real_param)
        setScalarParameter<Real>(full_name, it->first, real_param, in_global, global_params_block);
      else if (int_param)
        setScalarParameter<int>(full_name, it->first, int_param, in_global, global_params_block);
      else if (uint_param)
        setScalarParameter<unsigned int>(full_name, it->first, uint_param, in_global, global_params_block);
      else if (bool_param)
        setScalarParameter<bool>(full_name, it->first, bool_param, in_global, global_params_block);
      else if (string_param)
        setScalarParameter<std::string>(full_name, it->first, string_param, in_global, global_params_block);
      else if (vec_real_param)
        setVectorParameter<Real>(full_name, it->first, vec_real_param, in_global, global_params_block);
      else if (vec_int_param)
        setVectorParameter<int>(full_name, it->first, vec_int_param, in_global, global_params_block);
      else if (vec_uint_param)
        setVectorParameter<unsigned int>(full_name, it->first, vec_uint_param, in_global, global_params_block);
      else if (vec_bool_param)
        setVectorParameter<bool>(full_name, it->first, vec_bool_param, in_global, global_params_block);
      else if (vec_string_param)
        setVectorParameter<std::string>(full_name, it->first, vec_string_param, in_global, global_params_block);
      else if (tensor_real_param)
        setTensorParameter<Real>(full_name, it->first, tensor_real_param, in_global, global_params_block);
      else if (tensor_int_param)
        setTensorParameter<int>(full_name, it->first, tensor_int_param, in_global, global_params_block);
      else if (tensor_bool_param)
        setTensorParameter<bool>(full_name, it->first, tensor_bool_param, in_global, global_params_block);
    }
  }
}

void
Parser::checkParams(const std::string & prefix, InputParameters &p)
{
  static const std::string global_params_block_name = "GlobalParams";

  static const std::string global_params_action_name = "set_global_params";
  ActionIterator act_iter = Moose::action_warehouse.actionBlocksWithActionBegin(global_params_action_name);
  GlobalParamsAction *global_params_block = NULL;

  // We are grabbing only the first
  if (act_iter != Moose::action_warehouse.actionBlocksWithActionEnd(global_params_action_name))
    global_params_block = dynamic_cast<GlobalParamsAction *>(*act_iter);

  for (InputParameters::iterator it = p.begin(); it != p.end(); ++it)
  {
    // bool found = false; // This variable is unused?!
    // bool in_global = false; // This variable is unused?!
    std::string orig_name = prefix + "/" + it->first;
    std::string full_name = orig_name;

    if (!p.wasSeenInInput(it->first) && p.isParamRequired(it->first))
    {
      // The parameter is required but missing
      mooseError("The required parameter '" + orig_name + "' is missing\n");
    }
  }
}

template<typename T>
void Parser::setScalarParameter(const std::string & full_name, const std::string & short_name, InputParameters::Parameter<T>* param, bool in_global, GlobalParamsAction *global_block)
{
  T value = _getpot_file(full_name.c_str(), param->get());
  
  param->set() = value;
  if (in_global)
    global_block->setScalarParam<T>(short_name) = value;
}

template<typename T>
void Parser::setVectorParameter(const std::string & full_name, const std::string & short_name, InputParameters::Parameter<std::vector<T> >* param, bool in_global, GlobalParamsAction *global_block)
{
  int vec_size = _getpot_file.vector_variable_size(full_name.c_str());

  if (_getpot_file.have_variable(full_name.c_str()))
    param->set().resize(vec_size);
    
  for (int i=0; i<vec_size; ++i)
    param->set()[i] = _getpot_file(full_name.c_str(), param->get()[i], i);

  if (in_global)
  {
    global_block->setVectorParam<T>(short_name).resize(vec_size);
    for (int i=0; i<vec_size; ++i)
      global_block->setVectorParam<T>(short_name)[i] = param->get()[i];
  }
}

template<typename T>
void Parser::setTensorParameter(const std::string & full_name, const std::string & short_name, InputParameters::Parameter<std::vector<std::vector<T> > >* param, bool in_global, GlobalParamsAction *global_block)
{
  int vec_size = _getpot_file.vector_variable_size(full_name.c_str());
  int one_dim = pow(vec_size, 0.5);

  param->set().resize(one_dim);
  for (int i=0; i<one_dim; ++i)
  {
    param->set()[i].resize(one_dim);
    for (int j=0; j<one_dim; ++j)
      param->set()[i][j] = _getpot_file(full_name.c_str(), param->get()[i][j], i*one_dim+j);
  }

  if (in_global)
  {
    global_block->setTensorParam<T>(short_name).resize(one_dim);
    for (int i=0; i<one_dim; ++i)
    {
      global_block->setTensorParam<T>(short_name)[i].resize(one_dim);
      for (int j=0; j<one_dim; ++j)
        global_block->setTensorParam<T>(short_name)[i][j] = param->get()[i][j];
    }
  }
}

