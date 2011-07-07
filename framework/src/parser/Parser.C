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

#include "InputFileFormatter.h"
#include "YAMLFormatter.h"

// libMesh
#include "getpot.h"

Parser::Parser():
  _mesh(NULL),
  _displaced_mesh(NULL),
  _problem(NULL),
  _executioner(NULL),
  _exreader(NULL),
  _loose(false),
  _syntax_formatter(NULL),
  _getpot_initialized(false)
{
  initOptions();

  // Need to make sure that the parser pointer is set in the warehouse for various functions
  // TODO: Rip the Parser Pointer out of the warehouse
  Moose::action_warehouse.setParserPointer(this);
  
  if (Moose::command_line != NULL)
  {
    if (searchCommandLine("Help"))
    {
      printUsage();
      exit(0);
    }
    if (searchCommandLine("Dump"))
    {
      initSyntaxFormatter(INPUT_FILE, true);
      
      buildFullTree( /*&Parser::printInputParameterData*/ );
      exit(0);
    }
    if (searchCommandLine("YAML"))
    {
      initSyntaxFormatter(YAML, true);
      
      buildFullTree( /*&Parser::printYAMLParameterData*/ );
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
  if (_syntax_formatter)
    delete _syntax_formatter;
}

void
Parser::registerActionSyntax(const std::string & action, const std::string & syntax, const std::string & action_name)
{

  ActionInfo action_info;
  action_info._action = action;
  action_info._action_name = action_name;
  
  _associated_actions.insert(std::make_pair(syntax, action_info));
}

std::string
Parser::isAssociated(const std::string & real_id, bool * is_parent)
{
  /**
   * This implementation assumes that wildcards can occur in the place of an entire token but not as part
   * of a token (i.e.  'Variables/ * /InitialConditions' is valid but not 'Variables/Partial* /InitialConditions'.
   * Since maps are ordered, a reverse traversal through the registered list will always select a more
   * specific match before a wildcard match ('*' == char(42))
   */
  bool local_is_parent;
  if (is_parent == NULL)
    is_parent = &local_is_parent;  // Just so we don't have to keep checking below when we want to set the value

  std::multimap<std::string, ActionInfo>::reverse_iterator it;
  std::vector<std::string> real_elements, reg_elements;
  std::string return_value;

  tokenize(real_id, real_elements);

  *is_parent = false;
  for (it=_associated_actions.rbegin(); it != _associated_actions.rend(); ++it)
  {
    std::string reg_id = it->first;
    if (reg_id == real_id)
    {
      *is_parent = false;
      return reg_id;
    }

    reg_elements.clear();
    tokenize(reg_id, reg_elements);
    if (real_elements.size() <= reg_elements.size())
    {
      bool keep_going = true;
      for (unsigned int j=0; keep_going && j<real_elements.size(); ++j)
      {
        if (real_elements[j] != reg_elements[j] && reg_elements[j] != std::string("*"))
          keep_going = false;
      }
      if (keep_going)
      {
        if (real_elements.size() < reg_elements.size() && !*is_parent)
        {
          // We found a parent, the longest parent in fact but we need to keep
          // looking to make sure that the real thing isn't registered
          *is_parent = true;
          return_value = reg_id;
        }
        else if (real_elements.size() == reg_elements.size())
        {
          *is_parent = false;
          return reg_id;
        }
      }
    }
  }

  if (*is_parent)
    return return_value;
  else
    return std::string("");
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

  syntax.clear();
  cli_opt.desc = "Runs the specified number of threads (Intel TBB) per process";
  syntax.push_back("--n_threads");
  cli_opt.cli_syntax = syntax;
  cli_opt.required = false;
  _cli_options["Threads"] = cli_opt;
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
    _inactive_strings.insert(s + "/");
  
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
  InputParameters params = validParams<EmptyAction>();

  // Build Command Line Vars Vector
  buildCommandLineVarsVector();
  
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
    // There may be more than one Action registered for a given section in which case we need to
    // build them all
    bool is_parent;
    std::string registered_identifier = isAssociated(*i, &is_parent);

    // We need to retrieve a list of Actions associated with the current identifier
    std::pair<std::multimap<std::string, ActionInfo>::iterator, std::multimap<std::string, ActionInfo>::iterator>
      iters = _associated_actions.equal_range(registered_identifier);

    if (iters.first == iters.second)
      mooseError(std::string("A '") + curr_identifier + "' is not an associated Action\n\n");
    
    for (std::multimap<std::string, ActionInfo>::iterator i = iters.first; i != iters.second; ++i)
    {
      if (!is_parent)
      {
        std::cerr << i->second._action << "\n";
        params = ActionFactory::instance()->getValidParams(i->second._action);

        // Before we build any objects we need to make sure that the section they are in is active
        if (isSectionActive(curr_identifier, active_lists))
        {
          params.set<Parser *>("parser_handle") = this;
    
          extractParams(curr_identifier, params);

          // Check Action Parameters Now
          checkParams(curr_identifier, params);

          // Add the parsed syntax to the parameters object for consumption by the Action
          params.set<std::string>("name") = curr_identifier;
          params.set<std::string>("build_by_action") = i->second._action_name;
          params.set<std::string>("action") = i->second._action_name;

          // Create the Action
          Action * action = ActionFactory::instance()->create(i->second._action, params);
          mooseAssert (action != NULL, std::string("Action") + i->second._action + " not created");

          // extract the MooseObject params if necessary
          MooseObjectAction * moose_object_action = dynamic_cast<MooseObjectAction *>(action);
          if (moose_object_action)
            extractParams(curr_identifier, moose_object_action->getMooseObjectParams());
    
          // add it to the warehouse
          Moose::action_warehouse.addActionBlock(action);
        }
      }
    }
    
    // Extract and save the current "active" list in the datastructure
    active_list_params.set<std::vector<std::string> >("active") = all;
    extractParams(curr_identifier, active_list_params);
    active_lists[curr_identifier] = active_list_params.get<std::vector<std::string> >("active");
  }
  
  // Check to make sure that all sections in the input file that are explicitly listed are actually present
  checkActiveUsed(section_names, active_lists);

  // Print the input file syntax if requested
  if (Moose::command_line && searchCommandLine("ShowTree"))
  {
    Moose::action_warehouse.printInputFile(std::cout);
    std::cout << std::endl << std::endl;
  }
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
Parser::initSyntaxFormatter(SyntaxFormatterType type, bool dump_mode, std::ostream & out)
{
  if (_syntax_formatter)
    delete _syntax_formatter;

  switch (type)
  {
  case INPUT_FILE:
    _syntax_formatter = new InputFileFormatter(out, dump_mode);
    break;
  case YAML:
    _syntax_formatter = new YAMLFormatter(out, dump_mode);
    break;
  default:
    mooseError("Unrecognized Syntax Formatter requested");
  } 
}

void
Parser::buildFullTree()
{
  std::string prev_name = "";
  std::vector<InputParameters *> params_ptrs(2);
  std::vector<std::pair<std::string, std::string> > all_names;

  _syntax_formatter->preamble();

  // Reserve a little space so we don't have to reallocate too many times
  all_names.reserve(100);
  for (ActionFactory::iterator act_obj = ActionFactory::instance()->begin();
       act_obj != ActionFactory::instance()->end();
       ++act_obj)
    all_names.push_back(std::pair<std::string, std::string>(act_obj->first, act_obj->second._action_name));  

  // Sort the Actions (well just the names and the action_name)
  std::sort(all_names.begin(), all_names.end(), InputFileSort());

  for (std::vector<std::pair<std::string, std::string> >::iterator act_names = all_names.begin(); act_names != all_names.end(); ++act_names)
  {
    std::string action;
    
    InputParameters action_obj_params = ActionFactory::instance()->getValidParams(act_names->first);

//    for (std::vector<InputParameters>::iterator it = all_action_params.begin(); it != all_action_params.end(); ++it)
//    {
//      InputParameters action_obj_params = *it;
      const std::string & action_name = act_names->second;
      std::string act_name = act_names->first;
      
      params_ptrs[0] = &action_obj_params;
      
      bool print_once = false;
      for (registeredMooseObjectIterator moose_obj = Factory::instance()->registeredObjectsBegin();
           moose_obj != Factory::instance()->registeredObjectsEnd();
           ++moose_obj)
      {
        InputParameters moose_obj_params = (moose_obj->second)();
      
        if (moose_obj_params.have_parameter<std::string>("built_by_action") &&
            moose_obj_params.get<std::string>("built_by_action") == action_name)
        {
          print_once = true;
          std::string name;
          size_t pos = 0;
          if (act_name[act_name.size()-1] == '*')
            pos = act_name.size()-1;
          else
            pos = act_name.size();

          // Executioner syntax is non standard - we'll hack it here
          if (act_name == "Executioner")
            name = "Executioner";
          else if (act_name.find("InitialCondition") != std::string::npos)
            name = act_name;
          else
            name = act_name.substr(0, pos) + moose_obj->first;
          
          moose_obj_params.set<std::string>("type") = moose_obj->first;
          moose_obj_params.seenInInputFile("type");          
          params_ptrs[1] = &moose_obj_params;
          
          _syntax_formatter->print(name, &prev_name, params_ptrs);
          
          prev_name = name;
        }
      }

      if (!print_once && action_obj_params.have_parameter<std::string>("built_by_action") &&
          action_obj_params.get<std::string>("built_by_action") == action_name &&
          ActionFactory::instance()->isParsed(act_name))
      {
        params_ptrs[1] = NULL;
        
        _syntax_formatter->print(act_name, prev_name == "" ? NULL : &prev_name, params_ptrs);

        prev_name = act_name;
      }

//    }
  }
  params_ptrs[0] = NULL;
  params_ptrs[1] = NULL;
  _syntax_formatter->print("", &prev_name, params_ptrs);

  _syntax_formatter->postscript();
}

void
Parser::tokenize(const std::string &str, std::vector<std::string> &elements, unsigned int min_len, const std::string &delims)
{
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

Executioner *
Parser::getExecutioner()
{
  mooseAssert(_executioner != NULL, "Executioner is NULL!");
  return _executioner;
}

void
Parser::execute()
{
  for (ActionWarehouse::iterator i = Moose::action_warehouse.begin();
       i != Moose::action_warehouse.end();
       ++i)
  {
    // Delay the InputParameters check of MOOSE based objects until just before "acting"
    // so that Meta-Actions can complete the build of parameters as necessary
    MooseObjectAction * moose_obj_action = dynamic_cast<MooseObjectAction *>(*i);
    if (moose_obj_action != NULL)
      checkParams(moose_obj_action->name(), moose_obj_action->getMooseObjectParams());

    // Act!
    (*i)->act();
  }
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
Parser::buildCommandLineVarsVector()
{
  if (Moose::command_line == NULL)
    return;
  
  for(const char* var; (var = Moose::command_line->next_nominus()) != NULL; )
  {
    std::vector<std::string> name_value_pairs;
    tokenize(var, name_value_pairs, 0, "=");
    _command_line_vars.insert(name_value_pairs[0]);
  }
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

    // Mark parameters appearing in the input file or command line
    if (_getpot_file.have_variable(full_name.c_str())
      || (Moose::command_line && Moose::command_line->have_variable(full_name.c_str())))
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
    std::string orig_name = prefix + "/" + it->first;
    std::string full_name = orig_name;

    if (!p.wasSeenInInput(it->first) && p.isParamRequired(it->first))
    {
      // The parameter is required but missing
      std::string doc = p.getDocString(it->first);
      mooseError("The required parameter '" + orig_name + "' is missing\nDoc String: \"" +
                 p.getDocString(it->first) + "\"");
    }
  }
}

template<typename T>
void Parser::setScalarParameter(const std::string & full_name, const std::string & short_name, InputParameters::Parameter<T>* param, bool in_global, GlobalParamsAction *global_block)
{
  GetPot *gp;
  
  // See if this variable was passed on the command line (and previously stored in the CLI vars vector)
  // if it was then we will retrieve the value from the command line instead of the file
  if (_command_line_vars.find(full_name) != _command_line_vars.end())
    gp = Moose::command_line;
  else
    gp = &_getpot_file;
  
  T value = (*gp)(full_name.c_str(), param->get());
  
  param->set() = value;
  if (in_global)
    global_block->setScalarParam<T>(short_name) = value;
}

template<typename T>
void Parser::setVectorParameter(const std::string & full_name, const std::string & short_name, InputParameters::Parameter<std::vector<T> >* param, bool in_global, GlobalParamsAction *global_block)
{
  GetPot *gp;
  
  // See if this variable was passed on the command line (and previously stored in the CLI vars vector)
  // if it was then we will retrieve the value from the command line instead of the file
  if (_command_line_vars.find(full_name) != _command_line_vars.end())
    gp = Moose::command_line;
  else
    gp = &_getpot_file;
  
  int vec_size = gp->vector_variable_size(full_name.c_str());

  if (gp->have_variable(full_name.c_str()))
    param->set().resize(vec_size);
    
  for (int i=0; i<vec_size; ++i)
    param->set()[i] = (*gp)(full_name.c_str(), param->get()[i], i);

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
  GetPot *gp;
  
  // See if this variable was passed on the command line (and previously stored in the CLI vars vector)
  // if it was then we will retrieve the value from the command line instead of the file
  if (_command_line_vars.find(full_name) != _command_line_vars.end())
    gp = Moose::command_line;
  else
    gp = &_getpot_file;
  
  int vec_size = gp->vector_variable_size(full_name.c_str());
  int one_dim = pow(vec_size, 0.5);

  param->set().resize(one_dim);
  for (int i=0; i<one_dim; ++i)
  {
    param->set()[i].resize(one_dim);
    for (int j=0; j<one_dim; ++j)
      param->set()[i][j] = (*gp)(full_name.c_str(), param->get()[i][j], i*one_dim+j);
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


//--------------------------------------------------------------------------
// Input File Sorter Functor methods

Parser::InputFileSort::InputFileSort() 
{ 
  _o.reserve(13);
  _o.push_back("Mesh");
  _o.push_back("Functions");
  _o.push_back("Preconditioning");
  _o.push_back("Variables"); 
  _o.push_back("AuxVariables");
  _o.push_back("Kernels");
  _o.push_back("DiracKernels");
  _o.push_back("AuxKernels");
  _o.push_back("Dampers");
  _o.push_back("Stabilizers");
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
Parser::InputFileSort::operator() (const std::pair<std::string, std::string> &a, const std::pair<std::string, std::string> &b) const
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
  return std::find(_o.begin(), _o.end(), a) - std::find(_o.begin(), _o.end(), b);
}
