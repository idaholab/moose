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

ParserBlock::ParserBlock(std::string name, MooseSystem & moose_system, InputParameters params)
  :_parent(params.get<ParserBlock*>("parent")),
   _moose_system(moose_system),
   _name(name),
   _parser_handle(*params.get<Parser *>("parser_handle")),
   _getpot_handle(_parser_handle.getPotHandle()),
   _block_params(params)
{
  if (_getpot_handle) 
  {
    // Immediately extract params from the input file instead of when the tree is completely constructed
    _parser_handle.extractParams(name, _block_params);
    _active = getParamValue<std::vector<std::string> >("active");
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

      // Simple traversals (check prereqs = off) don't require any special logic - just run!
      if (!check_prereqs)
         ((*i)->*action)();
      else
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
  executeDeferred(action);                                         // execute deferred blocks before going on
}

bool
ParserBlock::checkPrereqs(ParserBlock *pb_ptr)
{
  std::set<std::string> result;
  
  std::set_difference(pb_ptr->_execute_prereqs.begin(), pb_ptr->_execute_prereqs.end(),
                      _parser_handle.getExecutedSetBegin(), _parser_handle.getExecutedSetEnd(),
                      std::insert_iterator<std::set<std::string> >(result, result.end()));

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
  std::vector<std::string> elements;
  Parser::tokenize(_name, elements);

  std::string spacing = "";
  for (unsigned int i=0; i<elements.size(); ++i)
    spacing += "  ";

  
  std::cout << "\n"
            << spacing << "block name: " << _name << "\n";
  
  if (getType() != "")
    std::cout << spacing << "type: " << getType() << "\n";
  std::cout << spacing << "{\n";
  
  std::cout << spacing << "  Valid Parameters:\n";
  
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
      std::string required = param_ptrs[i]->isParamRequired(iter->first) ? "*" : " ";
      std::string valid = param_ptrs[i]->isParamValid(iter->first) ? " (valid)" : " ";

      std::cout << spacing << "    " << std::left << std::setw(30) << required + iter->first << ": ";
    
      iter->second->print(std::cout);

      std::cout << valid << "\n" << spacing << "    " << std::setw(30) << " " << "    " << param_ptrs[i]->getDocString(iter->first) << "\n";
    }
  }
  
  visitChildren(&ParserBlock::printBlockData, true, false);

  std::cout << spacing << "}\n";
}

void
ParserBlock::printBlockYAML()
{
  std::vector<std::string> elements;
  Parser::tokenize(_name, elements);

  std::string spacing = "";
  for (unsigned int i=0; i<elements.size(); ++i)
    spacing += "  ";

  std::cout << spacing << "- name: " << _name << "\n";
  spacing += "  ";
  
  //will print "" if there is no type, which translates to None in python
  std::cout << spacing << "type: " << getType() << "\n";
  
  std::cout << spacing << "parameters:\n";
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

      std::cout << spacing << "- name: " << name << "\n";
      std::cout << spacing << "  required: " << required << "\n";
      std::cout << spacing << "  default: !!str ";

      //prints the value, which is the default value when dumping the tree
      //because it hasn't been changed
      iter->second->print(std::cout);

      std::cout << "\n" << spacing << "  description: |\n    " << spacing
                << param_ptrs[i]->getDocString(iter->first) << "\n";
    }
  }

  //if there aren't any sub blocks it will just parse as None in python
  std::cout << subblocks;
  
  visitChildren(&ParserBlock::printBlockYAML, true, false);
}

