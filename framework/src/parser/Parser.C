#include <string>
#include <map>
#include <fstream>
#include <iomanip>

#include "Parser.h"
#include "MooseInit.h"
#include "InputParameters.h"
#include "ParserBlockFactory.h"

#include "ActionFactory.h"
#include "Action.h"
#include "MooseObjectAction.h"
#include "ActionWarehouse.h"

#include "MProblem.h"
#include "MooseMesh.h"
#include "Executioner.h"
#include "Moose.h"

#include "MeshBlock.h"
#include "MeshGenerationBlock.h"
#include "GenericMeshModifierBlock.h"
#include "FunctionsBlock.h"
#include "GenericFunctionsBlock.h"
#include "VariablesBlock.h"
#include "GenericVariableBlock.h"
#include "KernelsBlock.h"
#include "GenericKernelBlock.h"
#include "BCsBlock.h"
#include "GenericBCBlock.h"
#include "GenericICBlock.h"
#include "AuxVariablesBlock.h"
#include "AuxKernelsBlock.h"
#include "GenericAuxKernelBlock.h"	
#include "MaterialsBlock.h"
#include "GenericMaterialBlock.h"
#include "OutputBlock.h"
#include "GenericExecutionerBlock.h"
#include "PreconditioningBlock.h"
#include "PBPBlock.h"
#include "PostprocessorsBlock.h"
#include "GenericPostprocessorBlock.h"
#include "PeriodicBlock.h"
#include "GenericPeriodicBlock.h"
#include "DampersBlock.h"
#include "GenericDamperBlock.h"
#include "DiracKernelsBlock.h"
#include "GenericDiracKernelBlock.h"
#include "StabilizersBlock.h"
#include "GenericStabilizerBlock.h"
#include "GlobalParamsBlock.h"
#include "AdaptivityBlock.h"

#include "GlobalParamsAction.h"

// libMesh
#include "getpot.h"


// Static Data initialization
const std::string Parser::_show_tree = "--show_tree";

bool Parser::registered = false;

void
Parser::registerObjects()
{
  registerNamedParserBlock(MeshBlock, "Mesh");
  registerNamedParserBlock(MeshGenerationBlock, "Mesh/Generation");
  registerNamedParserBlock(GenericMeshModifierBlock, "Mesh/*");
  registerNamedParserBlock(FunctionsBlock, "Functions");
  registerNamedParserBlock(GenericFunctionsBlock, "Functions/*");
  registerNamedParserBlock(VariablesBlock, "Variables");
  registerNamedParserBlock(GenericVariableBlock, "Variables/*");
  registerNamedParserBlock(GenericICBlock, "Variables/*/InitialCondition");
  registerNamedParserBlock(AuxVariablesBlock, "AuxVariables");
  registerNamedParserBlock(GenericICBlock, "AuxVariables/*/InitialCondition");
  // Reuse the GenericVariableBlock for AuxVariables/*
  registerNamedParserBlock(GenericVariableBlock, "AuxVariables/*");
  registerNamedParserBlock(KernelsBlock, "Kernels");
  registerNamedParserBlock(GenericKernelBlock, "Kernels/*");
//  registerNamedParserBlock(DGKernelsBlock, "DGKernels");
//  registerNamedParserBlock(GenericDGKernelBlock, "DGKernels/*");
  registerNamedParserBlock(AuxKernelsBlock, "AuxKernels");
  registerNamedParserBlock(GenericAuxKernelBlock, "AuxKernels/*");
  registerNamedParserBlock(BCsBlock, "BCs");
  registerNamedParserBlock(GenericBCBlock, "BCs/*");
  // Reuse the BCsBlock for AuxBCs
  registerNamedParserBlock(BCsBlock, "AuxBCs");
  // Reuse the GenericBCBlock for AuxBCs/*
  registerNamedParserBlock(GenericBCBlock, "AuxBCs/*");
  registerNamedParserBlock(StabilizersBlock, "Stabilizers");
  registerNamedParserBlock(GenericStabilizerBlock, "Stabilizers/*");
  registerNamedParserBlock(MaterialsBlock, "Materials");
  registerNamedParserBlock(GenericMaterialBlock, "Materials/*");
  registerNamedParserBlock(OutputBlock, "Output");
  registerNamedParserBlock(PreconditioningBlock, "Preconditioning");
  registerNamedParserBlock(PBPBlock, "Preconditioning/PBP");
  registerNamedParserBlock(PeriodicBlock, "BCs/Periodic");
  registerNamedParserBlock(GenericPeriodicBlock, "BCs/Periodic/*");
  registerNamedParserBlock(GenericExecutionerBlock, "Executioner");
  registerNamedParserBlock(AdaptivityBlock, "Executioner/Adaptivity");
  registerNamedParserBlock(PostprocessorsBlock, "Postprocessors");
  registerNamedParserBlock(GenericPostprocessorBlock, "Postprocessors/*");
  registerNamedParserBlock(PostprocessorsBlock, "Postprocessors/Residual");
  registerNamedParserBlock(GenericPostprocessorBlock, "Postprocessors/Residual/*");
  registerNamedParserBlock(PostprocessorsBlock, "Postprocessors/Jacobian");
  registerNamedParserBlock(GenericPostprocessorBlock, "Postprocessors/Jacobian/*");
  registerNamedParserBlock(PostprocessorsBlock, "Postprocessors/NewtonIter");
  registerNamedParserBlock(GenericPostprocessorBlock, "Postprocessors/NewtonIter/*");
  registerNamedParserBlock(DampersBlock, "Dampers");
  registerNamedParserBlock(GenericDamperBlock, "Dampers/*");
  registerNamedParserBlock(GlobalParamsBlock, "GlobalParams");
  registerNamedParserBlock(DiracKernelsBlock, "DiracKernels");
  registerNamedParserBlock(GenericDiracKernelBlock, "DiracKernels/*");

//   registerNamedAction(MeshBlock, "Mesh");
//   registerNamedAction(MeshGenerationBlock, "Mesh/Generation");
//   registerNamedAction(GenericMeshModifierBlock, "Mesh/*");
//   registerNamedAction(FunctionsBlock, "Functions");
//   registerNamedAction(GenericFunctionsBlock, "Functions/*");
//   registerNamedAction(VariablesBlock, "Variables");
//   registerNamedAction(GenericVariableBlock, "Variables/*");
//   registerNamedAction(GenericICBlock, "Variables/*/InitialCondition");
//   registerNamedAction(AuxVariablesBlock, "AuxVariables");
//   registerNamedAction(GenericICBlock, "AuxVariables/*/InitialCondition");
//   // Reuse the GenericVariableBlock for AuxVariables/*
//   registerNamedAction(GenericVariableBlock, "AuxVariables/*");
//   registerNamedAction(KernelsBlock, "Kernels");
//   registerNamedAction(GenericKernelBlock, "Kernels/*");
// //  registerNamedAction(DGKernelsBlock, "DGKernels");
// //  registerNamedAction(GenericDGKernelBlock, "DGKernels/*");
//   registerNamedAction(AuxKernelsBlock, "AuxKernels");
//   registerNamedAction(GenericAuxKernelBlock, "AuxKernels/*");
//   registerNamedAction(BCsBlock, "BCs");
//   registerNamedAction(GenericBCBlock, "BCs/*");
//   // Reuse the BCsBlock for AuxBCs
//   registerNamedAction(BCsBlock, "AuxBCs");
//   // Reuse the GenericBCBlock for AuxBCs/*
//   registerNamedAction(GenericBCBlock, "AuxBCs/*");
//   registerNamedAction(StabilizersBlock, "Stabilizers");
//   registerNamedAction(GenericStabilizerBlock, "Stabilizers/*");
//   registerNamedAction(MaterialsBlock, "Materials");
//   registerNamedAction(GenericMaterialBlock, "Materials/*");
//   registerNamedAction(OutputBlock, "Output");
//   registerNamedAction(PreconditioningBlock, "Preconditioning");
// //  registerNamedAction(PBPBlock, "Preconditioning/PBP");
//   registerNamedAction(PeriodicBlock, "BCs/Periodic");
//   registerNamedAction(GenericPeriodicBlock, "BCs/Periodic/*");
//   registerNamedAction(GenericExecutionerBlock, "Executioner");
//   registerNamedAction(AdaptivityBlock, "Executioner/Adaptivity");
//   registerNamedAction(PostprocessorsBlock, "Postprocessors");
//   registerNamedAction(GenericPostprocessorBlock, "Postprocessors/*");
//   registerNamedAction(PostprocessorsBlock, "Postprocessors/Residual");
//   registerNamedAction(GenericPostprocessorBlock, "Postprocessors/Residual/*");
//   registerNamedAction(PostprocessorsBlock, "Postprocessors/Jacobian");
//   registerNamedAction(GenericPostprocessorBlock, "Postprocessors/Jacobian/*");
//   registerNamedAction(PostprocessorsBlock, "Postprocessors/NewtonIter");
//   registerNamedAction(GenericPostprocessorBlock, "Postprocessors/NewtonIter/*");
//   registerNamedAction(DampersBlock, "Dampers");
//   registerNamedAction(GenericDamperBlock, "Dampers/*");
//   registerNamedAction(GlobalParamsBlock, "GlobalParams");
//   registerNamedAction(DiracKernelsBlock, "DiracKernels");
//   registerNamedAction(GenericDiracKernelBlock, "DiracKernels/*");

  
  registered = true;
}

///////////////

Parser::Parser(const std::string &dump_string) :
    _mesh(NULL),
    _problem(NULL),
    _executioner(NULL),
    _exreader(NULL),
    _loose(false),
    _input_filename(""),
    _dump_string(dump_string),
    _input_tree(NULL),
    _getpot_initialized(false),
    _tree_printed(false)
{
  if (!registered)
    registerObjects();

  if (Moose::command_line != NULL)
  {
    if (Moose::command_line->search("-h") || Moose::command_line->search("--help"))
    {
      printUsage();
      exit(0);
    }
    if (Moose::command_line->search(dump_string))
    {
      buildFullTree( "dump" );
      exit(0);
    }
    if (Moose::command_line->search( "--yaml" ))
    {
      //important: start and end yaml data delimiters used by python
      std::cout << "**START YAML DATA**\n";
      buildFullTree( "yaml" );
      std::cout << "**END YAML DATA**\n";
      exit(0);
    }
  }
  else
    printUsage();
}

Parser::~Parser()
{
  if (_input_tree != NULL)
    delete _input_tree;

  delete _exreader;
}

bool
Parser::isSectionActive(const std::string & s, const InputParameters & params) const
{
  bool retValue = false;

  // Base Level is always active
  if (s.find_last_of('/') == std::string::npos)
    return true;
  
  std::string short_name = s.substr(s.find_last_of('/') != std::string::npos ? s.find_last_of('/') + 1 : 0);
  
  std::vector<std::string> active = params.get<std::vector<std::string> >("active");
  try
  {
    if (active.at(0) == "__all__")
      retValue = true;
    else
      retValue = std::find(active.begin(), active.end(), short_name) != active.end();
  }
  catch (std::out_of_range)
  {}

  return retValue;
}


void
Parser::parse_new(const std::string &input_filename)
{
  std::string curr_identifier;
  
  _input_filename = input_filename;
  
  checkInputFile();
  
  // GetPot object
  _getpot_file.parse_input_file(_input_filename);
  _getpot_initialized = true;

  _section_names = _getpot_file.get_section_names();

  /**
   * This is our base InputParameters object which will be extracted for all
   * section names.  We use this to get the "active" blocks which determine which
   * child blocks (Actions) will be created.
   */
  InputParameters base_action_params;

  /**
   * The section_names function returns a list of section names including all
   * all parent names of any nested sections.  We will exploit the fact that they are ordered
   * i.e.
   * Variables/
   * Variables/u
   * to construct our active lists as we build our objects.  Base level sections are _always_
   * active.
   */
  for (std::vector<std::string>::iterator i=_section_names.begin(); i != _section_names.end(); ++i)
  {
    curr_identifier = i->erase(i->size()-1);  // Chop off the last character (the trailing slash)

    std::cout << *i << "\n";

    // Extract the block parameters before constructing the action
    InputParameters params = ActionFactory::instance()->getValidParams(*i);

    // If this is a base level block it is active and we need to reset the base_action_params
    if (curr_identifier.find_last_of('/') == std::string::npos)
      base_action_params = validParams<Action>();
    
    // Before we build any objects we need to make sure that the section they are in is active
    // and that they aren't an unregistered parent (signified by an empty params object)
    if (isSectionActive(curr_identifier, base_action_params) && params.n_parameters() != 0)
    {
      params.set<Parser *>("parser_handle") = this;
    
      extractParams(curr_identifier, params);
      std::cout << params << "\n";

      // Create the Action
      Action * action = ActionFactory::instance()->create(curr_identifier, params);

      // extract the MooseObject params if necessary
      MooseObjectAction * moose_object_action = dynamic_cast<MooseObjectAction *>(action);
      if (moose_object_action)
      {
        extractParams(curr_identifier, moose_object_action->getMooseObjectParams());
        std::cout << "Moose Object Params:\n" << moose_object_action->getMooseObjectParams();
      }
    
      // add it to the warehouse
      Moose::action_warehouse.addActionBlock(action);
    }

    // Last we need to grab our base_action_params, i.e. "active" for the next nested level
    base_action_params = validParams<Action>();
    extractParams(curr_identifier, base_action_params);
  }
}

void
Parser::parse(const std::string &input_filename)
{
  std::vector<std::string> elements;
  std::string curr_value, curr_identifier;
  ParserBlock *curr_block;

  _input_filename = input_filename;
  
  checkInputFile();
  
  // GetPot object
  _getpot_file.parse_input_file(_input_filename);
  _getpot_initialized = true;

  InputParameters root_params = validParams<ParserBlock>();
  root_params.set<ParserBlock *>("parent") = NULL;
  root_params.set<Parser *>("parser_handle") = this;
  _input_tree = new ParserBlock("/", root_params);
  
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

    for (unsigned int j=0; j<elements.size(); ++j)
    {
      if (! curr_block->notifyChildUsed(elements[j]) )
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
//        InputParameters params2 = ActionFactory::instance()->getValidParams(curr_identifier);
        params.set<ParserBlock *>("parent") = curr_block;
        params.set<Parser *>("parser_handle") = this;
   
//        params2.set<Parser *>("parser_handle") = this;
        
        curr_block->_children.push_back(ParserBlockFactory::instance()->add(curr_identifier, params));
//        Moose::action_warehouse.addActionBlock(ActionFactory::instance()->create(curr_identifier, params2));

        // TODO Fill in parameters of Action Blocks
//        extractParams(curr_identifier, params2);
//        std::cout << "Curr_identifier: " << curr_identifier << "\n" <<  params2;
        
      }

      curr_block = *(curr_block->_children.rbegin());
    }

    // Extract all the requested parameters from the input file
    extractParams(curr_block->getID(), curr_block->getBlockParams());
    extractParams(curr_block->getID(), curr_block->getClassParams());

  }

  /*
  // Extract params for Action Blocks
  for (ActionIterator a = Moose::action_warehouse.allActionsBegin(this);
       a != Moose::action_warehouse.allActionsEnd();
       ++a)
  {
    
    //std::cerr << "Extracting Params for: " << (*a)->name() << "\n";
    extractParams((*a)->name(), (*a)->getParams());
  }
  */
  
  

  // This will throw a mooseError if the active lists aren't all used up
  _input_tree->checkActiveUsed();

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
Parser::buildFullTree( const std::string format )
{
#if 0
  std::string prefix;
  ParserBlock *curr_block;

  // Build the root of the tree
  InputParameters params = validParams<ParserBlock>();
  params.set<ParserBlock *>("parent") = NULL;
  params.set<Parser *>("parser_handle") = this;
  _input_tree = new ParserBlock("/", _moose_system, params);
  
  curr_block = _input_tree;
  ParserBlockFactory * pb_factory = ParserBlockFactory::instance();

  // Build all the base level ParserBlock types that are not generic
  for (ParserBlockNamesIterator i = pb_factory->registeredParserBlocksBegin();
       i != pb_factory->registeredParserBlocksEnd(); ++i)
  {
    // skip the generic matches here because they don't correlate to a real instance and well pick them up manually below
    if (i->find('*') != std::string::npos || i->find("Executioner") != std::string::npos)
      continue;

    // std::string matched_identifier = pb_factory->isRegistered(*i);
    params = pb_factory->getValidParams(*i);
    params.set<ParserBlock *>("parent") = curr_block;
    params.set<Parser *>("parser_handle") = this;
    curr_block->_children.push_back(pb_factory->add(*i, _moose_system, params));
  }
  
  {
    // Manually add a sample variable
    prefix = "Variables/";
    std::string var_name("SampleVar");
    curr_block = curr_block->locateBlock("Variables");
    mooseAssert(curr_block != NULL, "A Variables ParserBlock does not appear to exist");
    
    params = pb_factory->getValidParams(prefix + var_name);
    params.set<ParserBlock *>("parent") = curr_block;
    params.set<Parser *>("parser_handle") = this;
    curr_block->_children.push_back(pb_factory->add(prefix + var_name, _moose_system, params));
    
    // Add all the IC Blocks
    curr_block = curr_block->locateBlock(prefix + var_name);
    params = pb_factory->getValidParams(prefix + var_name +  "/InitialCondition");
    params.set<ParserBlock *>("parent") = curr_block;
    params.set<Parser *>("parser_handle") = this;
    
    mooseAssert(curr_block != NULL, "The sample variable block appears to be missing");
    for (InitialConditionNamesIterator i = InitialConditionFactory::instance()->registeredInitialConditionsBegin();
         i != InitialConditionFactory::instance()->registeredInitialConditionsEnd(); ++i)
    {
      params.set<std::string>("type") = *i;
      curr_block->_children.push_back(pb_factory->add(prefix + var_name + "/InitialCondition", _moose_system, params));
    }
  }

  {
    // Manually add a sample variable
    prefix = "AuxVariables/";
    std::string var_name("SampleAuxVar");
    curr_block = curr_block->locateBlock("AuxVariables");
    mooseAssert(curr_block != NULL, "A AuxVariables ParserBlock does not appear to exist");
    
    params = pb_factory->getValidParams(prefix + var_name);
    params.set<ParserBlock *>("parent") = curr_block;
    params.set<Parser *>("parser_handle") = this;
    curr_block->_children.push_back(pb_factory->add(prefix + var_name, _moose_system, params));

    // Add all the IC Blocks
    curr_block = curr_block->locateBlock(prefix + var_name);
    params = pb_factory->getValidParams(prefix + var_name +  "/InitialCondition");
    params.set<ParserBlock *>("parent") = curr_block;
    params.set<Parser *>("parser_handle") = this;

    mooseAssert(curr_block != NULL, "The sample aux variable block appears to be missing");
    for (InitialConditionNamesIterator i = InitialConditionFactory::instance()->registeredInitialConditionsBegin();
         i != InitialConditionFactory::instance()->registeredInitialConditionsEnd(); ++i)
    {
      params.set<std::string>("type") = *i;
      curr_block->_children.push_back(pb_factory->add(prefix + var_name + "/InitialCondition", _moose_system, params));
    }
  }

  // Add all the Functions
  curr_block = curr_block->locateBlock("Functions");
  prefix = "Functions/";
  params = pb_factory->getValidParams(prefix + "foo");
  params.set<ParserBlock *>("parent") = curr_block;
  params.set<Parser *>("parser_handle") = this;
  for (FunctionNamesIterator i = FunctionFactory::instance()->registeredFunctionsBegin();
       i != FunctionFactory::instance()->registeredFunctionsEnd(); ++i)
  {
    params.set<std::string>("type") = *i;
    curr_block->_children.push_back(pb_factory->add(prefix + *i, _moose_system, params));
  }

  // Add all the Kernels
  curr_block = curr_block->locateBlock("Kernels");
  prefix = "Kernels/";
  params = pb_factory->getValidParams(prefix + "foo");
  params.set<ParserBlock *>("parent") = curr_block;
  params.set<Parser *>("parser_handle") = this;
  for (KernelNamesIterator i = KernelFactory::instance()->registeredKernelsBegin();
       i != KernelFactory::instance()->registeredKernelsEnd(); ++i)
  {
    params.set<std::string>("type") = *i;
    curr_block->_children.push_back(pb_factory->add(prefix + *i, _moose_system, params));
  }
  
  // Add all the DG Kernels
  curr_block = curr_block->locateBlock("DGKernels");
  prefix = "DGKernels/";
  params = pb_factory->getValidParams(prefix + "foo");
  params.set<ParserBlock *>("parent") = curr_block;
  params.set<Parser *>("parser_handle") = this;
  for (DGKernelNamesIterator i = DGKernelFactory::instance()->registeredDGKernelsBegin();
       i != DGKernelFactory::instance()->registeredDGKernelsEnd(); ++i)
  {
    params.set<std::string>("type") = *i;
    curr_block->_children.push_back(pb_factory->add(prefix + *i, _moose_system, params));
  }

  // Add all the Dampers Kernels
  curr_block = curr_block->locateBlock("Dampers");
  prefix = "Dampers/";
  params = pb_factory->getValidParams(prefix + "foo");
  params.set<ParserBlock *>("parent") = curr_block;
  params.set<Parser *>("parser_handle") = this;
  for (DamperNamesIterator i = DamperFactory::instance()->registeredDampersBegin();
       i != DamperFactory::instance()->registeredDampersEnd(); ++i)
  {
    params.set<std::string>("type") = *i;
    curr_block->_children.push_back(pb_factory->add(prefix + *i, _moose_system, params));
  }

  
  // Add all the AuxKernels
  curr_block = curr_block->locateBlock("AuxKernels");
  prefix = "AuxKernels/";
  params = pb_factory->getValidParams(prefix + "foo");
  params.set<ParserBlock *>("parent") = curr_block;
  params.set<Parser *>("parser_handle") = this;
  for (AuxKernelNamesIterator i = AuxFactory::instance()->registeredAuxKernelsBegin();
       i != AuxFactory::instance()->registeredAuxKernelsEnd(); ++i)
  {
    params.set<std::string>("type") = *i;
    curr_block->_children.push_back(pb_factory->add(prefix + *i, _moose_system, params));
  }
  
  // Add all the BCs
  curr_block = curr_block->locateBlock("BCs");
  prefix = "BCs/";
  params = pb_factory->getValidParams(prefix + "foo");
  params.set<ParserBlock *>("parent") = curr_block;
  params.set<Parser *>("parser_handle") = this;
  for (BCNamesIterator i = BCFactory::instance()->registeredBCsBegin();
       i != BCFactory::instance()->registeredBCsEnd(); ++i)
  {
    params.set<std::string>("type") = *i;
    curr_block->_children.push_back(pb_factory->add(prefix + *i, _moose_system, params));
  }
  
  // Add all the Materials
  curr_block = curr_block->locateBlock("Materials");
  prefix = "Materials/";
  params = pb_factory->getValidParams(prefix + "foo");
  params.set<ParserBlock *>("parent") = curr_block;
  params.set<Parser *>("parser_handle") = this;
  for (MaterialNamesIterator i = MaterialFactory::instance()->registeredMaterialsBegin();
       i != MaterialFactory::instance()->registeredMaterialsEnd(); ++i)
  {
    params.set<std::string>("type") = *i;
    curr_block->_children.push_back(pb_factory->add(prefix + *i, _moose_system, params));
  }
  
  // Add all the Executioners
  curr_block = _input_tree;
  prefix = "";
  params = pb_factory->getValidParams("Executioner");
  params.set<ParserBlock *>("parent") = curr_block;
  params.set<Parser *>("parser_handle") = this;
  for (ExecutionerNamesIterator i = ExecutionerFactory::instance()->registeredExecutionersBegin();
       i != ExecutionerFactory::instance()->registeredExecutionersEnd(); ++i)
  {
    params.set<std::string>("type") = *i;
    ParserBlock * exec = pb_factory->add("Executioner", _moose_system, params);
    curr_block->_children.push_back(exec);

    // Add the adaptivity block to each executioner
    InputParameters adapt_params = pb_factory->getValidParams("Executioner/Adaptivity");
    adapt_params.set<ParserBlock *>("parent") = exec;
    adapt_params.set<Parser *>("parser_handle") = this;
    exec->_children.push_back(pb_factory->add("Executioner/Adaptivity", _moose_system, adapt_params));
  }

  // Add all the PostProcessors
  curr_block = curr_block->locateBlock("Postprocessors");
  prefix = "Postprocessors/";
  params = pb_factory->getValidParams(prefix + "foo");
  params.set<ParserBlock *>("parent") = curr_block;
  params.set<Parser *>("parser_handle") = this;
  
  for (PostprocessorNamesIterator i = PostprocessorFactory::instance()->registeredPostprocessorsBegin();
       i != PostprocessorFactory::instance()->registeredPostprocessorsEnd(); ++i)
  {
    params.set<std::string>("type") = *i;
    curr_block->_children.push_back(pb_factory->add(prefix + *i, _moose_system, params));
  }

  // Add all the Stabilizers
  curr_block = curr_block->locateBlock("Stabilizers");
  prefix = "Stabilizers/";
  params = pb_factory->getValidParams(prefix + "foo");
  params.set<ParserBlock *>("parent") = curr_block;
  params.set<Parser *>("parser_handle") = this;
  for (StabilizerNamesIterator i = StabilizerFactory::instance()->registeredStabilizersBegin();
       i != StabilizerFactory::instance()->registeredStabilizersEnd(); ++i)
  {
    params.set<std::string>("type") = *i;
    curr_block->_children.push_back(pb_factory->add(prefix + *i, _moose_system, params));
  }
#endif

  if (format == "yaml")
    _input_tree->printBlockYAML();
  else // "dump" is all that's left
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
Parser::fixupOptionalBlocks()
{
#if 1
  /* Create a vector of Optional Blocks to fill in if they don't exist in the parsed tree.
   * The pairs should consist of the required block followed by the optional block.  The optional
   * block will be inserted into the tree immediately following the required block.
   */
  std::vector<std::pair<std::string, std::string> > optional_blocks;
  std::vector<std::pair<std::string, std::string> >::iterator i;
  ParserBlock *block_ptr;

//  optional_blocks.push_back(std::make_pair("Mesh", ""));
//  optional_blocks.push_back(std::make_pair("Variables", "Preconditioning"));
//  optional_blocks.push_back(std::make_pair("Preconditioning", "AuxVariables"));
//  optional_blocks.push_back(std::make_pair("Kernels", "AuxKernels"));
//  optional_blocks.push_back(std::make_pair("AuxKernels", "BCs"));
//  optional_blocks.push_back(std::make_pair("BCs", "AuxBCs"));
//  optional_blocks.push_back(std::make_pair("Executioner", "Postprocessors"));
//  optional_blocks.push_back(std::make_pair("Variables", "AuxVariables"));

  // First see if the Optional Block exists
  for (i = optional_blocks.begin(); i != optional_blocks.end(); ++i)
  {
    if (_input_tree->locateBlock(i->second) == NULL)
    {
      // Get a pointer to the required block to prepare for insertion
      // The newly constructed block will be the sibling before this block
      // which means it better exist and it better not be the root
      block_ptr = _input_tree->locateBlock(i->first);
      if (block_ptr == NULL || block_ptr->_parent == NULL)
        mooseError("Major Error in ParserBlock Structure!\nPlease make sure that your input file does not contain ""Windows"" line endings");

      ParserBlock::PBChildIterator position =
        find(block_ptr->_parent->_children.begin(), block_ptr->_parent->_children.end(), block_ptr);

      if (position == block_ptr->_parent->_children.end())
        mooseError(("Unable to find required block " + i->first + " for optional block insertion").c_str());


      InputParameters params = ParserBlockFactory::instance()->getValidParams(i->second);
      params.set<ParserBlock *>("parent") = block_ptr->_parent;
      params.set<Parser *>("parser_handle") = this;

      // Increment one past this location so the new element be inserted afterwards
      block_ptr->_parent->_children.insert(++position, ParserBlockFactory::instance()->add(i->second, params));
    }
  }
#endif
}

void
Parser::execute_new()
{
  /// Iterate over the actions inside of the ActionWarehouse
  /// TODO: Make this work instead of the parser block actions below
  
  for (ActionIterator a = Moose::action_warehouse.allActionsBegin(this);
       a != Moose::action_warehouse.allActionsEnd();
       ++a)
    (*a)->act();
}


void
Parser::execute()
{
  _executed_blocks.clear();
//  _input_tree->execute();

  std::string nm[] = { "Mesh", "Executioner", "Output" };
  std::vector<std::string> block_names(nm, nm + sizeof(nm) / sizeof(std::string));
  for (std::vector<std::string>::iterator it = block_names.begin(); it != block_names.end(); ++it)
  {
    ParserBlock * blk = _input_tree->locateBlock(*it);
    if (blk)
      blk->execute();
  }

  
//  ParserBlock * meshb = _input_tree->locateBlock("Mesh");
//  if (meshb)
//    meshb->execute();
//
//  ParserBlock * exeb = _input_tree->locateBlock("Executioner");
//  if (exeb)
//    exeb->execute();
//
//  ParserBlock * outb = _input_tree->locateBlock("Output");
//  if (outb)
//    outb->execute();

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
  exit(1);
}

void
Parser::extractParams(const std::string & prefix, InputParameters &p)
{
  static const std::string global_params_block_name = "GlobalParams";
  static const std::string global_params_action_name = "set_global_params";
//  ActionIterator act_iter = Moose::action_warehouse.actionBlocksWithActionBegin(global_params_action_name);
  GlobalParamsAction *global_params_block = NULL;

  // We are grabbing only the first 
//  if (act_iter != Moose::action_warehouse.actionBlocksWithActionEnd())
//    global_params_block = dynamic_cast<GlobalParamsAction *>(*act_iter);
  
  //ParserBlock *parser_block = _input_tree != NULL ? _input_tree->locateBlock(global_params_block_name) : NULL;
  //GlobalParamsBlock *global_params_block = parser_block != NULL ? dynamic_cast<GlobalParamsBlock *>(parser_block) : NULL;

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
    
    if (!found && p.isParamRequired(it->first))
      // The parameter is required but missing
      mooseError("The required parameter '" + orig_name + "' is missing\n");
    
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

