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

#include "ParserBlock.h"

#include <vector>
#include <string>
#include <set>

//MOOSE includes
#include "Parser.h"
#include "Moose.h"

//libMesh includes
#include "InputParameters.h"
#include "getpot.h"

template<>
InputParameters validParams<ParserBlock>()
{
  InputParameters params;
  std::vector<std::string> blocks(1);
  blocks[0] = "__all__";

  // Add the "active" parameter to all blocks to support selective child visitation (turn blocks on and off without comments)
  params.addParam<std::vector<std::string> >("active", blocks, "If specified only the blocks named will be visited and made active");
  params.addParam<std::string>("type", "A string representing the object type that this ParserBlock will hold if applicable");
  params.addPrivateParam<ParserBlock *>("parent");
  params.addPrivateParam<Parser *>("parser_handle");
 
  return params;
}


bool ParserBlock::_deferred_execution = false;
std::ostream * ParserBlock::_out = &std::cout;

ParserBlock::ParserBlock(const std::string & name, InputParameters params)
  :_parent(params.get<ParserBlock*>("parent")),
   _name(name),
   _parser_handle(*params.get<Parser *>("parser_handle")),
   _getpot_handle(_parser_handle.getPotHandle()),
   _moose_system(_parser_handle._moose_system),
   _block_params(params)
{
  if (_getpot_handle) 
  {
    // Immediately extract params from the input file instead of when the tree is completely constructed
    _parser_handle.extractParams(name, _block_params);
    _active = getParamValue<std::vector<std::string> >("active");

    //make a copy so we can delete the names when they get used
    _used_children = _active;
  }
}

ParserBlock::~ParserBlock() 
{
  for (std::vector<ParserBlock *>::iterator i = _children.begin(); i != _children.end(); ++i) 
    delete (*i);
}

std::string
ParserBlock::getShortName() const
{
  return _name.substr(_name.find_last_of('/') != std::string::npos ? _name.find_last_of('/') + 1 : 0);
}

std::string
ParserBlock::getType() const
{
  std::string type;
  /**
   * The "type" should be in the input file for normal runs, however for syntax dumps the type will
   * be passed as a parameter.  
   */
  if (_getpot_handle)
    type = (*_getpot_handle)((_name + "/type").c_str(), "");
  else
    type = _block_params.get<std::string>("type");

  return type;
}

bool
ParserBlock::notifyChildUsed(const std::string &name)
{
  std::vector<std::string>::iterator loc = std::find(_used_children.begin(), _used_children.end(), name);
  if (loc != _used_children.end())
    _used_children.erase(loc);
  
  return checkActive(name);
}

bool
ParserBlock::checkActive(const std::string &name) const
{
  bool retValue = false;
  try 
  {
    if (_active.at(0) == "__all__")
      retValue = true;
    else
      retValue = std::find(_active.begin(), _active.end(), name) != _active.end();
  }
  catch (std::out_of_range)
  {}
    
  return retValue;
}

bool
ParserBlock::amIActive() const
{
  if (_parent == NULL)
    return true;
  else
    return _parent->checkActive(getShortName());
}

void
ParserBlock::checkActiveUsed()
{
  if (_used_children.size() != 0)
    if (_used_children[0] != "__all__")
      mooseError("Extra items listed in the active list in block \"" + getID() + "\"");

  visitChildren(&ParserBlock::checkActiveUsed, true, false);
}

void
ParserBlock::execute()
{
  visitChildren(&ParserBlock::execute);

  executeDeferred(&ParserBlock::execute);
  
  // See if all the deferred blocks have been executed
  std::list<ParserBlock *> & unexecuted = _parser_handle.getDeferredList();
  if (!unexecuted.empty())
  {
    std::cerr << "Unexecuted ParserBlocks remain, consider checking prereqs for the following blocks:\n";
    for (std::list<ParserBlock *>::iterator i=unexecuted.begin(); i!=unexecuted.end(); ++i)
      std::cout << (*i)->getID() << "\n";
    mooseError("*******************************************************************************");
  }
}

unsigned int
ParserBlock::n_activeChildren() const
{
  std::vector<std::string> active_children = getParamValue<std::vector<std::string> >("active");
    
  // if there is no parameter named "active" then assume that all children are active
  try 
  {
    if (active_children.at(0) == "__all__") 
      return _children.size();
  }
  catch (std::out_of_range) 
  {}

  // Make sure that all named children are actually in the _children list  
  // Load the children names into a set for faster locating
  std::set<std::string> child_set(active_children.begin(), active_children.end());
  unsigned int count = 0;

  std::vector<ParserBlock *>::const_iterator i;
  for (i=_children.begin(); i!=_children.end(); ++i)
    if (child_set.find((*i)->getShortName()) != child_set.end())
      ++count;
  return count;
}

void
ParserBlock::visitChildren(void (ParserBlock::*action)(), bool visit_active_only, bool check_prereqs)
{
  std::vector<std::string> active_children = getParamValue<std::vector<std::string> >("active");

  // if there is no parameter named "active" then assume that all children are to be visited
  try 
  {
    if (active_children.at(0) == "__all__") 
      visit_active_only = false;
  }
  catch (std::out_of_range) 
  {}

  // Load the children names into a set for faster locating
  std::set<std::string> child_set(active_children.begin(), active_children.end());

  std::vector<ParserBlock *>::iterator i;
  for (i=_children.begin(); i!=_children.end(); ++i)
  { 
    if (!visit_active_only || child_set.find((*i)->getShortName()) != child_set.end())
    {
      // Simple traversals (check prereqs = off) don't require any special logic - just run!
      if (!check_prereqs)
         ((*i)->*action)();
      else
      {
        if (checkPrereqs(*i))                                // Check Prereqs and defer if not met
        {
          if (!_parser_handle.isExecuted((*i)->getID()))     // Just ignore this block if already executed
          {
            ((*i)->*action)();                               // Call the method through the function pointer
            _parser_handle.markExecuted((*i)->getID());      // Add the current block to the executed set

            if (!_deferred_execution)                        // Execute deferred blocks unless we are
              executeDeferred(action);                       // already executing deferred blocks
          }
        }
        else
          _parser_handle.deferExecution(*i);                 // Add to the deferred list of prereqs not met
      }
    }
  }
  executeDeferred(action);                                         // execute deferred blocks before going on
}

bool
ParserBlock::checkPrereqs(ParserBlock *pb_ptr)
{
  std::set<std::string> result;
  
  std::set_difference(pb_ptr->_execute_prereqs.begin(), pb_ptr->_execute_prereqs.end(),
                      _parser_handle.getExecutedSetBegin(), _parser_handle.getExecutedSetEnd(),
                      std::insert_iterator<std::set<std::string> >(result, result.end()));

#ifdef DEBUG
  if (!result.empty())
  {
    std::cerr << "Can't execute " << pb_ptr->getShortName() << " due to the following unsatisfied prereqs:\n";
    for (std::set<std::string>::iterator i = result.begin(); i != result.end(); ++i)
      std::cerr << "\t" << *i << "\n";
    std::cerr << "\n";
  }
#endif 

  return result.empty() ? true : false;
}

void
ParserBlock::executeDeferred(void (ParserBlock::*action)())
{
  // Don't go off into recursion no-mans land (base case)
  if (_deferred_execution)
    return;
  
  _deferred_execution = true;  // Switch modes to "deferred execution" for the duration of this subroutine

  // Keep looping over the deferred blocks as long as the list isn't empty but is still shrinking
  unsigned int last_size, curr_size;
  do 
  {
    std::list<ParserBlock *> & deferred = _parser_handle.getDeferredList();
    last_size = deferred.size();

    for (std::list<ParserBlock *>::iterator i = deferred.begin(); i != deferred.end(); ) 
    {
      if (checkPrereqs(*i) && !_parser_handle.isExecuted((*i)->getID()))
      {
        /**
         * Because we are going to recurse down the children of this block and those children
         * will themselves call "executeDeferred" we need to mark the current block as executed
         * before the recursive call.  This will allow us to catch the base case at the top of this
         * function during the subsequent recursive calls and properly unwind the stack back to the
         * point where we originally entered the deferred execution block in "visitChildren"
         */
        _parser_handle.markExecuted((*i)->getID());
        ((*i)->*action)();

        deferred.erase(i++);    // VERY IMPORTANT: Erase with a postfix increment
      }
      else
        ++i;
    }

    curr_size = deferred.size();
  } while (last_size - curr_size);
  
  _deferred_execution = false;  // Switch modes back to "normal execution" as we leave this subroutine
}

ParserBlock *
ParserBlock::locateBlock(const std::string & id)
{
  std::vector<std::string> elements;
  std::vector<std::string>::iterator i;
  std::vector<ParserBlock *>::iterator j;
  Parser::tokenize(id, elements);
  bool found_it = true;

  // First we need to get to the top of the ParserBlock tree
  ParserBlock *curr_block = this;
  while(curr_block->_parent != NULL)
    curr_block = curr_block->_parent;

  for (i = elements.begin(); found_it &&  i != elements.end(); ++i)
  {
    j = curr_block->_children.begin();
    found_it = false;
    for (; !found_it && j != curr_block->_children.end(); ++j)
      if (*i == (*j)->getShortName()) 
      {
        curr_block = *j;
        found_it = true;
      }
  }
  
  if (found_it)
    return curr_block;
  else
    return NULL;
}

void
ParserBlock::printBlockData()
{
  std::ostream & out = *_out;
  std::vector<std::string> elements;
  Parser::tokenize(_name, elements);

  std::string spacing = "";
  for (unsigned int i=0; i<elements.size(); ++i)
    spacing += "  ";

  
  out << "\n"
            << spacing << "block name: " << _name << "\n";
  
  if (getType() != "")
    out << spacing << "type: " << getType() << "\n";
  std::string class_desc = _class_params.getClassDescription();
  if (class_desc != "")
    out << spacing << "description: " << class_desc << "\n";
  
  out << spacing << "{\n";
  
  out << spacing << "  Valid Parameters:\n";
  
  std::vector<InputParameters *> param_ptrs;
  param_ptrs.push_back(&_block_params);
  param_ptrs.push_back(&_class_params);

  for (unsigned int i=0; i<param_ptrs.size(); ++i)
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
  
  visitChildren(&ParserBlock::printBlockData, true, false);

  out << spacing << "}\n";
}

void
ParserBlock::printInputFile()
{
  std::ostream & out = *_out;
  std::vector<std::string> elements;
  Parser::tokenize(_name, elements);
  std::stringstream ss;

  std::string quotes   = "";
  std::string spacing  = "";
  std::string forward  = "";
  std::string backdots = "";
  int         offset   = 30;
  for (unsigned int i=1; i<elements.size(); ++i)
  {
    spacing += "  ";
    forward = ".";
    backdots += "../";
    offset -= 2;
  }

  int index = _name.find_last_of("/");
  if (index == (int)_name.npos)
    index = 0;
  std::string block_name = _name.substr(index);
  out << "\n" << spacing << "[" << forward << block_name << "]\n";
  
  std::vector<InputParameters *> param_ptrs;
  param_ptrs.push_back(&_block_params);
  param_ptrs.push_back(&_class_params);

  for (unsigned int i=0; i<param_ptrs.size(); ++i)
  {
    for (InputParameters::iterator iter = param_ptrs[i]->begin(); iter != param_ptrs[i]->end(); ++iter) 
    {
      // We only want non-private valid params
      if (param_ptrs[i]->isPrivate(iter->first) || !param_ptrs[i]->isParamValid(iter->first)) 
        continue;

      // Don't print active if it is the default all, that means it's not in the input file
      if (iter->first == "active")
      {
        libMesh::Parameters::Parameter<std::vector<std::string> > * val = dynamic_cast<libMesh::Parameters::Parameter<std::vector<std::string> >*>(iter->second);
        const std::vector<std::string> & active = val->get();
        if (val != NULL && active.size() == 1 && active[0] == "__all__")
          continue;
      }

      // Don't print type if it is blank
      if (iter->first == "type")
      {
        libMesh::Parameters::Parameter<std::string> * val = dynamic_cast<libMesh::Parameters::Parameter<std::string>*>(iter->second);
        const std::string & active = val->get();
        if (val != NULL && active == "")
          continue;
      }

      out << spacing << "  " << std::left << std::setw(offset) << iter->first << " = ";
    
      // Print the parameter's value to a stringstream.
      ss.str("");
      iter->second->print(ss);
      // If the value has spaces, surround it with quotes, otherwise no quotes
      std::string value = ss.str();
      if (value.find(' ') != std::string::npos)
        quotes = "'";
      else
        quotes = "";
      out << quotes << value << quotes << "\n";
    }
  }
  
  visitChildren(&ParserBlock::printInputFile, true, false);
  out << spacing << "[" << backdots << "]\n";
}

void
ParserBlock::printBlockYAML()
{
  std::ostream & out = *_out;
  std::vector<std::string> elements;
  Parser::tokenize(_name, elements);

  std::string spacing = "";
  for (unsigned int i=0; i<elements.size(); ++i)
    spacing += "  ";

  out << spacing << "- name: " << _name << "\n";
  spacing += "  ";
  
  //will print "" if there is no type or desc, which translates to None in python
  out << spacing << "desc: !!str " << _class_params.getClassDescription() << "\n";
  out << spacing << "type: " << getType() << "\n";
  
  out << spacing << "parameters:\n";
  std::string subblocks = spacing + "subblocks: \n";
  spacing += "  ";
  
  std::vector<InputParameters *> param_ptrs;
  param_ptrs.push_back(&_block_params);
  param_ptrs.push_back(&_class_params);

  for (unsigned int i=0; i<param_ptrs.size(); ++i)
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
  
  visitChildren(&ParserBlock::printBlockYAML, true, false);
}

void
ParserBlock::getInputFileLines(std::vector<std::string> & result)
{
  std::ostream * temp = _out;

  std::stringstream ss;
  set_ostream(ss);

  // print the input file into the stringstream
  for (unsigned int i = 0; i < _children.size(); i++)
    _children[i]->printInputFile();

  std::string line;
  while (std::getline(ss, line))
    result.push_back(line);

  //restore the previous value of _out
  set_ostream(*temp);
}

void
ParserBlock::set_ostream(std::ostream & out)
{
  _out = &out;
}
