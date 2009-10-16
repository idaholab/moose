#include "ParserBlock.h"

#include <vector>
#include <string>
#include <set>

//MOOSE includes
#include "Parser.h"

//libMesh includes
#include "parameters.h"
#include "getpot.h"

ParserBlock::ParserBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, const GetPot & input_file)
  :_reg_id(reg_id),
   _real_id(real_id),
   _input_file(input_file),
   _parent(parent)
{
  // Add the "names" parameter to the list so that all blocks can support selective child visitation
  _block_params.set<std::vector<std::string> >("names");
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
  return _input_file((_real_id + "/type").c_str(), "");
}

void
ParserBlock::execute()
{
  visitChildren();
}

unsigned int
ParserBlock::n_activeChildren() const
{
  std::vector<std::string> named_children = _block_params.get<std::vector<std::string> >("names");

  // if there is no parameter named "names" then assume that all children are active
  if (named_children.size() == 0) 
    return _children.size();
  
  // Make sure that all named children are actually in the _children list  
  // Load the children names into a set for faster locating
  std::set<std::string> child_set(named_children.begin(), named_children.end());
  unsigned int count = 0;

  std::vector<ParserBlock *>::const_iterator i;
  for (i=_children.begin(); i!=_children.end(); ++i)
    if (child_set.find((*i)->getShortName()) != child_set.end())
      ++count;
  return count;
}

void
ParserBlock::visitChildren(void (ParserBlock::*action)(), bool visit_named_only)
{
  std::vector<std::string> named_children = _block_params.get<std::vector<std::string> >("names");

  // if there is no parameter named "names" then assume that all children are to be visited
  if (named_children.size() == 0)
    visit_named_only = false;

  // Load the children names into a set for faster locating
  std::set<std::string> child_set(named_children.begin(), named_children.end());

  std::vector<ParserBlock *>::iterator i;
  for (i=_children.begin(); i!=_children.end(); ++i)
    if (!visit_named_only || child_set.find((*i)->getShortName()) != child_set.end())
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
            << spacing << "  block_params={\n";

  for (Parameters::iterator iter = _block_params.begin(); iter != _block_params.end(); ++iter) 
  {
    std::cout << spacing << "    " << iter->first << ": ";
    iter->second->print(std::cout);
    std::cout << "\n";
  }
  
  std::cout << spacing << "  }\n"
            << spacing << "  class_params={\n";
  
  for (Parameters::iterator iter = _class_params.begin(); iter != _class_params.end(); ++iter)
  {
    std::cout << spacing << "    " << iter->first << ": ";
    iter->second->print(std::cout);
    std::cout << "\n";
  }
  
  std::cout << spacing << "  }\n";

  visitChildren(&ParserBlock::printBlockData);
}


    
      
      
