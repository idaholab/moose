#include "ParserBlock.h"

#include <vector>
#include <string>
#include <set>

//MOOSE includes
#include "Parser.h"

//libMesh includes
#include "parameters.h"
#include "getpot.h"

ParserBlock::ParserBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle)
  :_reg_id(reg_id),
   _real_id(real_id),
   _parser_handle(parser_handle),
   _parent(parent)
{
  // Add the "active" parameter to all blocks to support selective child visitation (turn blocks on and off without comments)
  addParam<std::vector<std::string> >("active", "If specified only the blocks named will be visited and made active", false);

  // "names" in the input file is now deprecated
  addParam<std::vector<std::string> >("names", "Deprecated DO NOT USE!", false);
}

ParserBlock::~ParserBlock() 
{
  for (std::vector<ParserBlock *>::iterator i = _children.begin(); i != _children.end(); ++i) 
    delete (*i);
}

std::string
ParserBlock::getShortName() const
{
  return _real_id.substr(_real_id.find_last_of('/')+1);
}

std::string
ParserBlock::getType() const
{
  const GetPot *getpot_handle = _parser_handle.getPotHandle();
  return getpot_handle == NULL ? getShortName() : (*getpot_handle)((_real_id + "/type").c_str(), "");
}

void
ParserBlock::execute()
{
  visitChildren();
}

unsigned int
ParserBlock::n_activeChildren() const
{
  std::vector<std::string> active_children = getParamValue<std::vector<std::string> >("active");
  std::vector<std::string> named_children = getParamValue<std::vector<std::string> >("names");

  if (named_children.size())
    mooseError(std::string("Error in: ") + _real_id + ". The use of 'names' is deprecated.");
    
  // if there is no parameter named "active" then assume that all children are active
  if (active_children.size() == 0) 
    return _children.size();
  
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
ParserBlock::visitChildren(void (ParserBlock::*action)(), bool visit_active_only)
{
  std::vector<std::string> active_children = getParamValue<std::vector<std::string> >("active");
  std::vector<std::string> named_children = getParamValue<std::vector<std::string> >("names");

  if (named_children.size())
    mooseError(std::string("Error in: ") + _real_id + ". The use of 'names' is deprecated.");
  
  // if there is no parameter named "active" then assume that all children are to be visited
  if (active_children.size() == 0)
    visit_active_only = false;

  // Load the children names into a set for faster locating
  std::set<std::string> child_set(active_children.begin(), active_children.end());

  std::vector<ParserBlock *>::iterator i;
  for (i=_children.begin(); i!=_children.end(); ++i)
    if (!visit_active_only || child_set.find((*i)->getShortName()) != child_set.end())
      ((*i)->*action)();  // Call the method through the function pointer
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
  Parser::tokenize(_real_id, elements);

  std::string spacing = "";
  for (unsigned int i=0; i<elements.size(); ++i)
    spacing += "\t";

  
  std::cout << "\n"
            << spacing << "name: " <<  getShortName() << "\n"
            << spacing << "type: " <<  typeid(*this).name() << "\n"
            << spacing << "  params={\n";

  for (Parameters::iterator iter = _block_params.begin(); iter != _block_params.end(); ++iter) 
  {
    // Block params may be required and will have a doc string
    std::string required = _required_params.find(iter->first) != _required_params.end() ? "*" : " ";

    std::cout << spacing << "    " << std::left << std::setw(30) << required + iter->first << ": ";
    std::cout.width(10);
    iter->second->print(std::cout);
    std::cout << _doc_string[iter->first] << "\n";
  }
  
  for (Parameters::iterator iter = _class_params.begin(); iter != _class_params.end(); ++iter)
  {
    std::cout << spacing << "    " << std::setw(30) << iter->first << ": ";
    std::cout.width(10);
    iter->second->print(std::cout);
    std::cout << "\n";
  }
  
  std::cout << spacing << "  }\n";

  visitChildren(&ParserBlock::printBlockData);
}


    
      
      
