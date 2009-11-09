#include <string>
#include <map>
#include <fstream>

#include "Parser.h"
#include "getpot.h"
#include "parameters.h"
#include "ParserBlockFactory.h"
#include "KernelFactory.h"
#include "BCFactory.h"
#include "MaterialFactory.h"

Parser::Parser(std::string input_filename)
  :_input_filename(input_filename),
   _input_tree(NULL),
   _getpot_initialized(false)
{
  // Build the Parse Tree from the input file
  parse();
}

Parser::Parser(bool build_full_tree)
  :_input_tree(NULL),
   _getpot_initialized(false)
{
  if (build_full_tree)
    // Build a full tree of all objects from Factory iterators
    buildFullTree();
  else
    mooseError("Parser should be contstructed with a filename or argument to build full tree");
}

Parser::~Parser() 
{
  if (_input_tree != NULL) 
  {
    delete (_input_tree);
  }
}

void
Parser::parse()
{
  std::vector<std::string> elements;
  std::string curr_value, curr_identifier;
  ParserBlock *curr_block, *t;

  checkInputFile();
  
  // GetPot object
  _getpot_file.parse_input_file(_input_filename);
  _getpot_initialized = true;
  _input_tree = new ParserBlock("/", "/", NULL, *this);
  _section_names = _getpot_file.get_section_names();

  // The variable_names function returns section names and variable names in
  // one large string so we can build the data structure in one pass
  for (std::vector<std::string>::iterator i=_section_names.begin(); i != _section_names.end(); ++i) 
  {
    elements.clear();
    tokenize(*i, elements);
    curr_block = _input_tree;
    curr_identifier = "";
    
    for (int j=0; j<elements.size(); ++j) 
    {
      // We don't want a leading slash
      if (j)
        curr_identifier += "/";
      curr_identifier += elements[j];

      std::vector<ParserBlock *>::reverse_iterator last = curr_block->_children.rbegin();
      if (last == curr_block->_children.rend() || (*last)->getID() != curr_identifier) 
      {
        // Add the new block to the parse tree
        std::string matched_identifier = ParserBlockFactory::instance()->isRegistered(curr_identifier);
      
        if (matched_identifier.length())
          curr_block->_children.push_back(ParserBlockFactory::instance()->add(matched_identifier, curr_identifier, curr_block, *this));
        else
          curr_block->_children.push_back(new ParserBlock(curr_identifier, curr_identifier, curr_block, *this));
      }

      curr_block = *(curr_block->_children.rbegin());
    }

    // Extract all the requested parameters from the input file
    extractParams(curr_block->getID(), curr_block->getBlockParams());
    extractParams(curr_block->getID(), curr_block->getClassParams());
  }

  fixupOptionalBlocks();

#ifdef DEBUG
  _input_tree->printBlockData();
#endif
}

void
Parser::buildFullTree()
{
  std::string prefix;
  ParserBlock *curr_block;
  
  // Build the root of the tree
  _input_tree = new ParserBlock("/", "/", NULL, *this);
  curr_block = _input_tree;


  // Build all the ParserBlock Types
  for (ParserBlockNamesIterator i = ParserBlockFactory::instance()->registeredParserBlocksBegin();
       i != ParserBlockFactory::instance()->registeredParserBlocksEnd(); ++i)
  {
    // skip the generic matches here because they don't correlate to a real instance
    if ((*i)[i->length()-1] == '*')
      continue;
    
    std::string matched_identifier = ParserBlockFactory::instance()->isRegistered(*i);
    curr_block->_children.push_back(ParserBlockFactory::instance()->add(matched_identifier, *i, curr_block, *this));
  }
  
  // Create the Kernels Block
  curr_block = ParserBlockFactory::instance()->add("Kernels", "Kernels", _input_tree, *this);
  _input_tree->_children.push_back(curr_block);
  prefix = "Kernels/";

  // Add all the Kernels
  for (KernelNamesIterator i = KernelFactory::instance()->registeredKernelsBegin();
       i != KernelFactory::instance()->registeredKernelsEnd(); ++i)
  {
    std::string curr_identifier(prefix + *i);
    std::string matched_identifier = ParserBlockFactory::instance()->isRegistered(curr_identifier);

    if (matched_identifier.length())
      curr_block->_children.push_back(ParserBlockFactory::instance()->add(matched_identifier, curr_identifier, curr_block, *this));
    else
      mooseError("An error that shouldn't have occured has occured and this isn't even Microsoft in Parser::BuildFullTree");
  }

  // Create the BCs Block
  curr_block = ParserBlockFactory::instance()->add("BCs", "BCs", _input_tree, *this);
  _input_tree->_children.push_back(curr_block);
  prefix = "BCs/";
  
  // Add all the BCs
  for (BCNamesIterator i = BCFactory::instance()->registeredBCsBegin();
       i != BCFactory::instance()->registeredBCsEnd(); ++i)
  {
    std::string curr_identifier(prefix + *i);
    std::string matched_identifier = ParserBlockFactory::instance()->isRegistered(curr_identifier);

    if (matched_identifier.length())
      curr_block->_children.push_back(ParserBlockFactory::instance()->add(matched_identifier, curr_identifier, curr_block, *this));
    else
      mooseError("An error that shouldn't have occured has occured and this isn't even Microsoft in Parser::BuildFullTree");
  }

  // Create the Materials Block
  curr_block = ParserBlockFactory::instance()->add("Materials", "Materials", _input_tree, *this);
  _input_tree->_children.push_back(curr_block);
  prefix = "Materials/";
  
  // Add all the Materials
  for (MaterialNamesIterator i = MaterialFactory::instance()->registeredMaterialsBegin();
       i != MaterialFactory::instance()->registeredMaterialsEnd(); ++i)
  {
    std::string curr_identifier(prefix + *i);
    std::string matched_identifier = ParserBlockFactory::instance()->isRegistered(curr_identifier);

    if (matched_identifier.length())
      curr_block->_children.push_back(ParserBlockFactory::instance()->add(matched_identifier, curr_identifier, curr_block, *this));
    else
      mooseError("An error that shouldn't have occured has occured and this isn't even Microsoft in Parser::BuildFullTree");
  }

  _input_tree->printBlockData();
}

void
Parser::extractParams(const std::string & prefix, Parameters &p)
{
  for (Parameters::iterator iter = p.begin(); iter != p.end(); ++iter)
  {
    setParameters(prefix + "/" + iter->first, iter);
  }
  
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
  /* Create a vector of Optional Blocks to fill in if they don't exist in the tree and where
   * they should fit (after the second id listed).  The vector->first is used as the type of block
   * to create and insert into the tree
   */
  std::vector<std::pair<std::string, std::string> > optionalBlocks;
  std::vector<std::pair<std::string, std::string> >::iterator i;
  ParserBlock *block_ptr;

  optionalBlocks.push_back(std::make_pair("Preconditioning", "Variables"));
  optionalBlocks.push_back(std::make_pair("AuxVariables", "Preconditioning"));
  optionalBlocks.push_back(std::make_pair("AuxKernels", "Kernels"));
  optionalBlocks.push_back(std::make_pair("BCs", "AuxKernels"));
  optionalBlocks.push_back(std::make_pair("AuxBCs", "BCs"));

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

      if (position == block_ptr->_parent->_children.end())
        mooseError(("Unable to find required block " + i->second + " for optional block insertion").c_str());

      // Increment one past this location so the new element be inserted afterwards      
      block_ptr->_parent->_children.insert(++position,
                                            ParserBlockFactory::instance()->add(i->first, i->first, block_ptr->_parent, *this));
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
Parser::checkInputFile()
{
  std::ifstream in(_input_filename.c_str(), std::ifstream::in);
  if (in.fail())
    mooseError((std::string("Unable to open file \"") + _input_filename
                + std::string("\". Check to make sure that it exists and that you have read permission.")).c_str());

  in.close();
}


// function to set parameters with arbitrary type
bool
Parser::setParameters(std::string name, Parameters::iterator &it)
{
  Parameters::Parameter<Real> * real_param = dynamic_cast<Parameters::Parameter<Real>*>(it->second);
  Parameters::Parameter<int>  * int_param  = dynamic_cast<Parameters::Parameter<int>*>(it->second);
  Parameters::Parameter<unsigned int>  * uint_param  = dynamic_cast<Parameters::Parameter<unsigned int>*>(it->second);
  Parameters::Parameter<bool> * bool_param = dynamic_cast<Parameters::Parameter<bool>*>(it->second);
  Parameters::Parameter<std::string> * string_param = dynamic_cast<Parameters::Parameter<std::string>*>(it->second);
  Parameters::Parameter<std::vector<Real> > * vec_real_param = dynamic_cast<Parameters::Parameter<std::vector<Real> >*>(it->second);
  Parameters::Parameter<std::vector<int>  > * vec_int_param  = dynamic_cast<Parameters::Parameter<std::vector<int> >*>(it->second);
  Parameters::Parameter<std::vector<bool>  > * vec_bool_param  = dynamic_cast<Parameters::Parameter<std::vector<bool> >*>(it->second);
  Parameters::Parameter<std::vector<std::string> > * vec_string_param = dynamic_cast<Parameters::Parameter<std::vector<std::string> >*>(it->second);
  Parameters::Parameter<std::vector<std::vector<Real> > > * tensor_real_param = dynamic_cast<Parameters::Parameter<std::vector<std::vector<Real> > >*>(it->second);
  Parameters::Parameter<std::vector<std::vector<int> > >  * tensor_int_param  = dynamic_cast<Parameters::Parameter<std::vector<std::vector<int> > >*>(it->second);
  Parameters::Parameter<std::vector<std::vector<bool> > > * tensor_bool_param = dynamic_cast<Parameters::Parameter<std::vector<std::vector<bool> > >*>(it->second);
  
  bool default_flag = false;
  
  if( real_param )
  {
    Real from_input;
    from_input = _getpot_file((name).c_str(), real_param->get());
    //check whether parameter exists in inputfile.
    if( real_param->get() == from_input )
      default_flag = true;
    real_param->set()= from_input;
    return default_flag;
  }
  else if( int_param )
  {
    int from_input;
    from_input = _getpot_file((name).c_str(), int_param->get());
    if( int_param->get() == from_input )
      default_flag = true;
    int_param->set() = from_input;
    return default_flag;
  }
  else if( uint_param )
  {
    unsigned int from_input;
    from_input = _getpot_file((name).c_str(), uint_param->get());
    if( uint_param->get() == from_input )
      default_flag = true;
    uint_param->set() = from_input;
    return default_flag;
  }
  else if( bool_param )
  {
    bool from_input;
    from_input = _getpot_file((name).c_str(), bool_param->get());
    if( bool_param->get() == from_input )
      default_flag = true;
    bool_param->set() = from_input;
    return default_flag;
  }
  else if( string_param )
  {
    std::string from_input;
    from_input = _getpot_file((name).c_str(), string_param->get());
    if( string_param->get() == from_input )
      default_flag = true;
    string_param->set() = from_input;
    return default_flag;
  }
  else if( vec_real_param )
  {
    int vec_size = _getpot_file.vector_variable_size((name).c_str());
    vec_real_param->set().resize(vec_size);

    if( vec_size == 0)
      default_flag = true;
    
    for(int j=0;j<vec_size;j++)
      vec_real_param->set()[j] = _getpot_file((name).c_str(), 0.0, j);

    return default_flag;
  }
  else if( vec_int_param )
  {
    int vec_size = _getpot_file.vector_variable_size((name).c_str());
    vec_int_param->set().resize(vec_size);

    if( vec_size == 0 )
      default_flag = true;
    
    for(int j=0;j<vec_size;j++)
      vec_int_param->set()[j] = _getpot_file((name).c_str(), vec_int_param->get()[j], j);

    return default_flag;
  }
  else if( vec_bool_param )
  {
    int vec_size = _getpot_file.vector_variable_size((name).c_str());
    vec_bool_param->set().resize(vec_size);
    
    if( vec_size == 0 )
      default_flag = true;
    
    for(int j=0;j<vec_size;j++)
      vec_bool_param->set()[j] = _getpot_file((name).c_str(), vec_bool_param->get()[j], j);

    return default_flag;
  }
  else if( vec_string_param )
  {
    int vec_size = _getpot_file.vector_variable_size((name).c_str());
    vec_string_param->set().resize(vec_size);

    if( vec_size == 0 )
      default_flag = true;
    
    for(int j=0;j<vec_size;j++)
      vec_string_param->set()[j] = _getpot_file((name).c_str(), vec_string_param->get()[j].c_str(), j);

    return default_flag;
  }
  else if( tensor_real_param)
  {
    int vec_size = _getpot_file.vector_variable_size((name).c_str());
    vec_size = pow(vec_size,0.5);
    if( vec_size == 0 )
      default_flag = true;

    tensor_real_param->set().resize(vec_size);
    for(int j=0;j<vec_size;j++)
      tensor_real_param->set()[j].resize(vec_size);
    
    unsigned int cntr = 0;
    for(int j=0;j<vec_size;j++)
    {
      for(int i=0;i<vec_size;i++)
      {
        tensor_real_param->set()[i][j] = _getpot_file((name).c_str(), tensor_real_param->get()[i][j], cntr);
        cntr++;
      }
    }
    return default_flag;
  }
  else if( tensor_int_param )
  {
    int vec_size = _getpot_file.vector_variable_size((name).c_str());
    vec_size = pow(vec_size,0.5);

    if( vec_size == 0 )
      default_flag = true;
    
    if( vec_size == 0 )
    {
      tensor_int_param->set().resize(1);
      tensor_int_param->set()[0].resize(1);
    }
    else
    {
      tensor_int_param->set().resize(vec_size);
      for(int j=0;j<vec_size;j++)
        tensor_int_param->set()[j].resize(vec_size);
    }
    
    unsigned int cntr = 0;
    for(int j=0;j<vec_size;j++)
    {
      for(int i=0;i<vec_size;i++)
      {
        tensor_int_param->set()[i][j] = _getpot_file((name).c_str(), tensor_int_param->get()[i][j], cntr);
        cntr++;
      }
    }
    return default_flag;
  }
  else if( tensor_bool_param)
  {
    int vec_size = _getpot_file.vector_variable_size((name).c_str());
    vec_size = pow(vec_size,0.5);

    if( vec_size == 0 )
      default_flag = true;
    
    if( vec_size == 0 )
    {
      tensor_bool_param->set().resize(1);
      tensor_bool_param->set()[0].resize(1);
    }
    else
    {
      tensor_bool_param->set().resize(vec_size);
      for(int j=0;j<vec_size;j++)
        tensor_bool_param->set()[j].resize(vec_size);
    }
    
    unsigned int cntr = 0;
    for(int j=0;j<vec_size;j++)
    {
      for(int i=0;i<vec_size;i++)
      {
        tensor_bool_param->set()[i][j] = _getpot_file((name).c_str(), tensor_bool_param->get()[i][j], cntr);
        cntr++;
      }
    }
    return default_flag;
  }
  else
    return true;
}
