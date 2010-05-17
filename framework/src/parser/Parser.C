#include <string>
#include <map>
#include <fstream>
#include <iomanip>

#include "MooseSystem.h"
#include "Parser.h"
#include "getpot.h"
#include "InputParameters.h"
#include "ParserBlockFactory.h"
#include "KernelFactory.h"
#include "AuxFactory.h"
#include "BCFactory.h"
#include "MaterialFactory.h"
#include "InitialConditionFactory.h"
#include "ExecutionerFactory.h"

// Static Data initialization
const std::string Parser::_show_tree = "--show_tree";

 
Parser::Parser(MooseSystem & moose_system, const std::string &dump_string)
  :_moose_system(moose_system),
   _input_filename(""),
   _input_tree(NULL),
   _getpot_initialized(false),
   _tree_printed(false),
   _dump_string(dump_string)
{
  if (Moose::command_line != NULL)
  {
    if (Moose::command_line->search("-h") || Moose::command_line->search("--help")) 
    {
      printUsage();
      exit(0);
    }
    if (Moose::command_line->search(dump_string))
    {
      buildFullTree();
      exit(0);
    }
  }
  else
    printUsage();
}

Parser::~Parser() 
{
  if (_input_tree != NULL) 
  {
    delete (_input_tree);
  }
}

void
Parser::parse(const std::string &input_filename)
{
  std::vector<std::string> elements;
  std::string curr_value, curr_identifier;
  ParserBlock *curr_block, *t;

  _input_filename = input_filename;
  
  checkInputFile();
  
  // GetPot object
  _getpot_file.parse_input_file(_input_filename);
  _getpot_initialized = true;
  
  InputParameters root_params = validParams<ParserBlock>();
  root_params.set<ParserBlock *>("parent") = NULL;
  root_params.set<Parser *>("parser_handle") = this;
  _input_tree = new ParserBlock("/", _moose_system, root_params);
  
  _section_names = _getpot_file.get_section_names();

  /**
   * The variable_names function returns section names and variable names in
   * one large string so we can build the data structure in one pass.
   * (.i.e) /Variables/temp/InitialCondition will be split on slashes and
   * a tree of three levels will be build for each remaining piece of text
   */
  for (std::vector<std::string>::iterator i=_section_names.begin(); i != _section_names.end(); ++i) 
  {
    elements.clear();
    tokenize(*i, elements);
    curr_block = _input_tree;
    curr_identifier = "";

    for (int j=0; j<elements.size(); ++j) 
    {
      if (! curr_block->checkActive(elements[j]) )
        continue;
      
      // Add slashes as seperators back into our text strings as we build them up but don't
      // start with an initial slash
      if (j)
        curr_identifier += "/";
      curr_identifier += elements[j];

      std::vector<ParserBlock *>::reverse_iterator last = curr_block->_children.rbegin();
      // See if this block is already in the tree from a previous section string iteration
      if (last == curr_block->_children.rend() || (*last)->getID() != curr_identifier) 
      {
        InputParameters params = ParserBlockFactory::instance()->getValidParams(curr_identifier);
        params.set<ParserBlock *>("parent") = curr_block;
        params.set<Parser *>("parser_handle") = this;

        curr_block->_children.push_back(ParserBlockFactory::instance()->add(curr_identifier, _moose_system, params));          
      }

      curr_block = *(curr_block->_children.rbegin());
    }

    // Extract all the requested parameters from the input file
    extractParams(curr_block->getID(), curr_block->getBlockParams());
    extractParams(curr_block->getID(), curr_block->getClassParams());
  }

  fixupOptionalBlocks();

  if (!_tree_printed) 
  {
#ifdef DEBUG
    _input_tree->printBlockData();
    _tree_printed = true;
#else
    if (Moose::command_line && Moose::command_line->search("--show-tree")) 
    {
      _input_tree->printBlockData();
      _tree_printed = true;
    }
#endif
  }
}

void
Parser::buildFullTree()
{
  std::string prefix;
  ParserBlock *curr_block;

  // Build the root of the tree
  InputParameters params = validParams<ParserBlock>();
  params.set<ParserBlock *>("parent") = NULL;
  params.set<Parser *>("parser_handle") = this;
  _input_tree = new ParserBlock("/", _moose_system, params);
  
  curr_block = _input_tree;

  // Build all the base level ParserBlock types that are not generic
  for (ParserBlockNamesIterator i = ParserBlockFactory::instance()->registeredParserBlocksBegin();
       i != ParserBlockFactory::instance()->registeredParserBlocksEnd(); ++i)
  {
    // skip the generic matches here because they don't correlate to a real instance and well pick them up manually below
    if (i->find('*') != std::string::npos || i->find("Executioner") != std::string::npos)
      continue;

    // std::string matched_identifier = ParserBlockFactory::instance()->isRegistered(*i);
    params = ParserBlockFactory::instance()->getValidParams(*i);
    params.set<ParserBlock *>("parent") = curr_block;
    params.set<Parser *>("parser_handle") = this;
    curr_block->_children.push_back(ParserBlockFactory::instance()->add(*i, _moose_system, params));
  }
  
  {
    // Manually add a sample variable
    prefix = "Variables/";
    std::string var_name("SampleVar");
    curr_block = curr_block->locateBlock("Variables");
    mooseAssert(curr_block != NULL, "A Variables ParserBlock does not appear to exist");
    
    params = ParserBlockFactory::instance()->getValidParams(prefix + var_name);
    params.set<ParserBlock *>("parent") = curr_block;
    params.set<Parser *>("parser_handle") = this;
    curr_block->_children.push_back(ParserBlockFactory::instance()->add(prefix + var_name, _moose_system, params));
    
    // Add all the IC Blocks
    curr_block = curr_block->locateBlock(prefix + var_name);
    params = ParserBlockFactory::instance()->getValidParams(prefix + var_name +  "/InitialCondition");
    params.set<ParserBlock *>("parent") = curr_block;
    params.set<Parser *>("parser_handle") = this;
    
    mooseAssert(curr_block != NULL, "The sample variable block appears to be missing");
    for (InitialConditionNamesIterator i = InitialConditionFactory::instance()->registeredInitialConditionsBegin();
         i != InitialConditionFactory::instance()->registeredInitialConditionsEnd(); ++i)
    {
      params.set<std::string>("type") = *i;
      curr_block->_children.push_back(ParserBlockFactory::instance()->add(prefix + var_name + "/InitialCondition", _moose_system, params));
    }
  }

  {
    // Manually add a sample variable
    prefix = "AuxVariables/";
    std::string var_name("SampleAuxVar");
    curr_block = curr_block->locateBlock("AuxVariables");
    mooseAssert(curr_block != NULL, "A AuxVariables ParserBlock does not appear to exist");
    
    params = ParserBlockFactory::instance()->getValidParams(prefix + var_name);
    params.set<ParserBlock *>("parent") = curr_block;
    params.set<Parser *>("parser_handle") = this;
    curr_block->_children.push_back(ParserBlockFactory::instance()->add(prefix + var_name, _moose_system, params));

    // Add all the IC Blocks
    curr_block = curr_block->locateBlock(prefix + var_name);
    params = ParserBlockFactory::instance()->getValidParams(prefix + var_name +  "/InitialCondition");
    params.set<ParserBlock *>("parent") = curr_block;
    params.set<Parser *>("parser_handle") = this;

    mooseAssert(curr_block != NULL, "The sample aux variable block appears to be missing");
    for (InitialConditionNamesIterator i = InitialConditionFactory::instance()->registeredInitialConditionsBegin();
         i != InitialConditionFactory::instance()->registeredInitialConditionsEnd(); ++i)
    {
      params.set<std::string>("type") = *i;
      curr_block->_children.push_back(ParserBlockFactory::instance()->add(prefix + var_name + "/InitialCondition", _moose_system, params));
    }
  }

  // Add all the Kernels
  curr_block = curr_block->locateBlock("Kernels");
  prefix = "Kernels/";
  params = ParserBlockFactory::instance()->getValidParams(prefix + "foo");
  params.set<ParserBlock *>("parent") = curr_block;
  params.set<Parser *>("parser_handle") = this;
  for (KernelNamesIterator i = KernelFactory::instance()->registeredKernelsBegin();
       i != KernelFactory::instance()->registeredKernelsEnd(); ++i)
  {
    params.set<std::string>("type") = *i;
    curr_block->_children.push_back(ParserBlockFactory::instance()->add(prefix + *i, _moose_system, params));
  }
  
  // Add all the AuxKernels
  curr_block = curr_block->locateBlock("AuxKernels");
  prefix = "AuxKernels/";
  params = ParserBlockFactory::instance()->getValidParams(prefix + "foo");
  params.set<ParserBlock *>("parent") = curr_block;
  params.set<Parser *>("parser_handle") = this;
  for (AuxKernelNamesIterator i = AuxFactory::instance()->registeredAuxKernelsBegin();
       i != AuxFactory::instance()->registeredAuxKernelsEnd(); ++i)
  {
    params.set<std::string>("type") = *i;
    curr_block->_children.push_back(ParserBlockFactory::instance()->add(prefix + *i, _moose_system, params));
  }
  
  // Add all the BCs
  curr_block = curr_block->locateBlock("BCs");
  prefix = "BCs/";
  params = ParserBlockFactory::instance()->getValidParams(prefix + "foo");
  params.set<ParserBlock *>("parent") = curr_block;
  params.set<Parser *>("parser_handle") = this;
  for (BCNamesIterator i = BCFactory::instance()->registeredBCsBegin();
       i != BCFactory::instance()->registeredBCsEnd(); ++i)
  {
    params.set<std::string>("type") = *i;
    curr_block->_children.push_back(ParserBlockFactory::instance()->add(prefix + *i, _moose_system, params));
  }

  // Add all the Materials
  curr_block = curr_block->locateBlock("Materials");
  prefix = "Materials/";
  params = ParserBlockFactory::instance()->getValidParams(prefix + "foo");
  params.set<ParserBlock *>("parent") = curr_block;
  params.set<Parser *>("parser_handle") = this;
  for (MaterialNamesIterator i = MaterialFactory::instance()->registeredMaterialsBegin();
       i != MaterialFactory::instance()->registeredMaterialsEnd(); ++i)
  {
    params.set<std::string>("type") = *i;
    curr_block->_children.push_back(ParserBlockFactory::instance()->add(prefix + *i, _moose_system, params));
  }
  
  // Add all the Executioners
  curr_block = _input_tree;
  prefix = "";
  params = ParserBlockFactory::instance()->getValidParams("Executioner");
  params.set<ParserBlock *>("parent") = curr_block;
  params.set<Parser *>("parser_handle") = this;
  for (ExecutionerNamesIterator i = ExecutionerFactory::instance()->registeredExecutionersBegin();
       i != ExecutionerFactory::instance()->registeredExecutionersEnd(); ++i)
  {
    params.set<std::string>("type") = *i;
    curr_block->_children.push_back(ParserBlockFactory::instance()->add("Executioner", _moose_system, params));
  }
  
  _input_tree->printBlockData();
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

MooseSystem &
Parser::getMooseSystem()
{
  return _moose_system;
}

const GetPot *
Parser::getPotHandle() const
{
  return _getpot_initialized ? &_getpot_file : NULL;
}

/*
void
Parser::fixupOptionalBlocks()
{
  // Create a vector of Optional Blocks to stick in the tree if they don't exist
  std::vector<std::string> optionalBlocks;
  std::vector<std::string>::iterator i;

  optionalBlocks.push_back("AuxVariables");
  optionalBlocks.push_back("AuxKernels");
  optionalBlocks.push_back("BCs");
  optionalBlocks.push_back("AuxBCs");

  for (i = optionalBlocks.begin(); i != optionalBlocks.end(); ++i)
    if (_input_tree->locateBlock(*i) == NULL)
      _input_tree->_children.push_back(ParserBlockFactory::instance()->add(*i, *i, _input_tree, *this));
}
*/


/*
void
Parser::fixupOptionalBlocks()
{
  /* Create a map of Optional Blocks to fill in if they don't exist in the tree and where
   * they should fit (before the second id listed) /
  std::map<std::string, std::string> optionalBlocks;
  std::map<std::string, std::string>::iterator i;
  ParserBlock *block_ptr;

  optionalBlocks["AuxVariables"] = "Kernels";
  optionalBlocks["AuxKernels"] = "BCs";
	
  // First see if the Optional Block exists
  for (i = optionalBlocks.begin(); i != optionalBlocks.end(); ++i)
  {
    if (_input_tree->locateBlock(i->first) == NULL)
    {
      // Get a pointer to the required block to prepare for insertion
      // The newly constructed block will be the sibling before this block
      // which means it better exist and it better not be the root
      block_ptr = _input_tree->locateBlock(i->second);
      if (block_ptr == NULL || block_ptr->_parent == NULL)
	mooseError("");

      ParserBlock::PBChildIterator position =
	find(block_ptr->_parent->_children.begin(), block_ptr->_parent->_children.end(), block_ptr);
     
      block_ptr->_parent->_children.insert(position,
                                           ParserBlockFactory::instance()->add(i->first, i->first, block_ptr->_parent, *this));
    }
  }
}
*/


void
Parser::fixupOptionalBlocks()
{
  /* Create a vector of Optional Blocks to fill in if they don't exist in the parsed tree.
   * The pairs should consist of the required block followed by the optional block.  The optional
   * block will be inserted into the tree immediatly following the required block.
   */
  std::vector<std::pair<std::string, std::string> > optionalBlocks;
  std::vector<std::pair<std::string, std::string> >::iterator i;
  ParserBlock *block_ptr;

  optionalBlocks.push_back(std::make_pair("Variables", "Preconditioning"));
  optionalBlocks.push_back(std::make_pair("Preconditioning", "AuxVariables"));
  optionalBlocks.push_back(std::make_pair("Kernels", "AuxKernels"));
  optionalBlocks.push_back(std::make_pair("AuxKernels", "BCs"));
  optionalBlocks.push_back(std::make_pair("BCs", "AuxBCs"));
  
  // First see if the Optional Block exists
  for (i = optionalBlocks.begin(); i != optionalBlocks.end(); ++i) 
  {
    if (_input_tree->locateBlock(i->second) == NULL) 
    {
      // Get a pointer to the required block to prepare for insertion
      // The newly constructed block will be the sibling before this block
      // which means it better exist and it better not be the root
      block_ptr = _input_tree->locateBlock(i->first);
      if (block_ptr == NULL || block_ptr->_parent == NULL)
        mooseError("Major Error in ParserBlock Structure");

      ParserBlock::PBChildIterator position =
        find(block_ptr->_parent->_children.begin(), block_ptr->_parent->_children.end(), block_ptr);

      if (position == block_ptr->_parent->_children.end())
        mooseError(("Unable to find required block " + i->first + " for optional block insertion").c_str());


      InputParameters params = ParserBlockFactory::instance()->getValidParams(i->second);
      params.set<ParserBlock *>("parent") = block_ptr->_parent;
      params.set<Parser *>("parser_handle") = this;

      // Increment one past this location so the new element be inserted afterwards
      block_ptr->_parent->_children.insert(++position, ParserBlockFactory::instance()->add(i->second, _moose_system, params));
    }
  }
}


void
Parser::execute()
{
  _executed_blocks.clear();
  _input_tree->execute();
}

void
Parser::printTree()
{
  if (_tree_printed)
    return;  
  _input_tree->printBlockData();
  _tree_printed = true;
}

void
Parser::checkInputFile()
{
  std::ifstream in(_input_filename.c_str(), std::ifstream::in);
  if (in.fail())
    mooseError((std::string("Unable to open file \"") + _input_filename
                + std::string("\". Check to make sure that it exists and that you have read permission.")).c_str());

  in.close();
}

void
Parser::printUsage() const
{
  // Grab the first item out of argv
  std::string str((*Moose::command_line)[0]);
  str.substr(str.find_last_of("/\\")+1);
  std::cout << "\nUsage: " << str << " <command>" << "\n\n"
            << "Available Commands:\n" << std::left
            << std::setw(50) << "  -i <filename> [" + _show_tree + "] [solver options]"
            << "standard application execution with various options\n"
            << std::setw(50) << "  -h | --help"
            << "display usage statement\n"
            << std::setw(50) << "  --dump"
            << "show a full dump of available input file syntax\n\n"
            << "Solver Options:\n"
            << "  See solver manual for details (Petsc or Trilinos)\n";
}

void
Parser::extractParams(const std::string & prefix, InputParameters &p)
{
  for (InputParameters::iterator it = p.begin(); it != p.end(); ++it)
  {
    std::string full_name = prefix + "/" + it->first;

    // Mark parameters appearing in the input file
    if (_getpot_file.have_variable(full_name.c_str()))
      p.seenInInputFile(it->first);
    // The parameter is required but missing
    else if (p.isParamRequired(it->first))
      mooseError("The required parameter '" + full_name + "' is missing\n");
      
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
      setScalarParameter<Real>(full_name, real_param);
    else if (int_param)
      setScalarParameter<int>(full_name, int_param);
    else if (uint_param)
      setScalarParameter<unsigned int>(full_name, uint_param);
    else if (bool_param)
      setScalarParameter<bool>(full_name, bool_param);
    else if (string_param)
      setScalarParameter<std::string>(full_name, string_param);
    else if (vec_real_param)
      setVectorParameter<Real>(full_name, vec_real_param);
    else if (vec_int_param)
      setVectorParameter<int>(full_name, vec_int_param);
    else if (vec_uint_param)
      setVectorParameter<unsigned int>(full_name, vec_uint_param);
    else if (vec_bool_param)
      setVectorParameter<bool>(full_name, vec_bool_param);
    else if (vec_string_param)
      setVectorParameter<std::string>(full_name, vec_string_param);
    else if (tensor_real_param)
      setTensorParameter<Real>(full_name, tensor_real_param);
    else if (tensor_int_param)
      setTensorParameter<int>(full_name, tensor_int_param);
    else if (tensor_bool_param)
      setTensorParameter<bool>(full_name, tensor_bool_param);
  }
}

template<typename T>
void Parser::setScalarParameter(const std::string & name, InputParameters::Parameter<T>* param)
{
  param->set() = _getpot_file(name.c_str(), param->get());
}

template<typename T>
void Parser::setVectorParameter(const std::string & name, InputParameters::Parameter<std::vector<T> >* param) 
{
  int vec_size = _getpot_file.vector_variable_size(name.c_str());

  if (_getpot_file.have_variable(name.c_str())) 
    param->set().resize(vec_size);
    
  for (unsigned int i=0; i<vec_size; ++i) 
    param->set()[i] = _getpot_file(name.c_str(), param->get()[i], i);
}

template<typename T>
void Parser::setTensorParameter(const std::string & name, InputParameters::Parameter<std::vector<std::vector<T> > >* param) 
{
  int vec_size = _getpot_file.vector_variable_size(name.c_str());
  int one_dim = pow(vec_size, 0.5);

  param->set().resize(one_dim);
  for (unsigned int i=0; i<one_dim; ++i) 
  {
    param->set()[i].resize(one_dim);
    for (unsigned int j=0; j<one_dim; ++j)
      param->set()[i][j] = _getpot_file(name.c_str(), param->get()[i][j], i*one_dim+j);
  }
}

