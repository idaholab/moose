#include "GenericExecutionerBlock.h"
//#include "AdaptivityBlock.h"
#include "Factory.h"
#include "PetscSupport.h"
#include "Conversion.h"
#include "Moose.h"
#include "Parser.h"
#include "Executioner.h"
#include "MProblem.h"
#include "VariablesBlock.h"
#include "AdaptivityBlock.h"

template<>
InputParameters validParams<GenericExecutionerBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  params.addRequiredParam<std::string> ("type", "Specifies the type of Executioner to be used");

  params.addParam<Real>        ("l_tol",           1.0e-5,   "Linear Tolerance");
  params.addParam<Real>        ("l_abs_step_tol",  -1,       "Linear Absolute Step Tolerance");
  params.addParam<unsigned int>("l_max_its",       10000,    "Max Linear Iterations");
  params.addParam<unsigned int>("nl_max_its",      50,       "Max Nonlinear Iterations");
  params.addParam<unsigned int>("nl_max_funcs",    10000,    "Max Nonlinear solver function evaluations");
  params.addParam<Real>        ("nl_abs_tol",      1.0e-50,  "Nonlinear Absolute Tolerance");
  params.addParam<Real>        ("nl_rel_tol",      1.0e-8,   "Nonlinear Relative Tolerance");
  params.addParam<Real>        ("nl_abs_step_tol", 1.0e-50,  "Nonlinear Absolute step Tolerance");
  params.addParam<Real>        ("nl_rel_step_tol", 1.0e-50,  "Nonlinear Relative step Tolerance");
  params.addParam<bool>        ("no_fe_reinit",    false,    "Specifies whether or not to reinitialize FEs");
  params.addParam<bool>        ("perf_log",        false,    "Specifies whether or not the Performance log should be printed");
  params.addParam<bool>        ("auto_scaling",    false,    "Turns on automatic variable scaling");
  params.addParam<std::string> ("scheme",          "backward-euler",  "Time integration scheme used.");

#ifdef LIBMESH_HAVE_PETSC
  params.addParam<std::vector<std::string> >("petsc_options", "Singleton Petsc options");
  params.addParam<std::vector<std::string> >("petsc_options_iname", "Names of Petsc name/value pairs");
  params.addParam<std::vector<std::string> >("petsc_options_value", "Values of Petsc name/value pairs (must correspond with \"petsc_options_iname\"");
#endif //LIBMESH_HAVE_PETSC

  return params;
}


GenericExecutionerBlock::GenericExecutionerBlock(const std::string & name, InputParameters params) :
    ParserBlock(name, params),
    _type(getType())
{
  addPrereq("Mesh");
#if 0
  // Register execution prereqs
  addPrereq("Mesh");
  addPrereq("Variables");
//  addPrereq("AuxVariables");
#endif
  setClassParams(Factory::instance()->getValidParams(_type));
}

void
GenericExecutionerBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the GenericExecutionerBlock Object\n";
#endif

  InputParameters class_params = getClassParams();

  // Steady and derived Executioners need to know the number of adaptivity steps to take.  This paramter
  // is held in the child block Adaptivity and needs to be pulled early
  ParserBlock *adaptivity_block = locateBlock("Executioner/Adaptivity");
  if (adaptivity_block != NULL)
  {
    AdaptivityBlock *a = dynamic_cast<AdaptivityBlock *>(adaptivity_block);
    class_params.set<unsigned int>("steps") = a->getSteps();
  }
  else
    class_params.set<unsigned int>("steps") = 0;
  
#ifdef LIBMESH_HAVE_PETSC
  std::vector<std::string> petsc_options,  petsc_inames, petsc_values;
  petsc_options = getParamValue<std::vector<std::string> >("petsc_options");
  petsc_inames = getParamValue<std::vector<std::string> >("petsc_options_iname");
  petsc_values = getParamValue<std::vector<std::string> >("petsc_options_value");

  if (petsc_inames.size() != petsc_values.size())
    mooseError("Petsc names and options from input file are not the same length");

  for (unsigned int i=0; i<petsc_options.size(); ++i)
    PetscOptionsSetValue(petsc_options[i].c_str(), PETSC_NULL);
  
  for (unsigned int i=0; i<petsc_inames.size(); ++i)
    PetscOptionsSetValue(petsc_inames[i].c_str(), petsc_values[i].c_str());
#endif //LIBMESH_HAVE_PETSC

  class_params.set<Moose::Mesh *>("_mesh") = _parser_handle._mesh;
  _parser_handle._executioner = static_cast<Executioner *>(Factory::instance()->create(_type, "Executioner", class_params));
  if (dynamic_cast<Moose::MProblem *>(&_parser_handle._executioner->problem()) != NULL)
  {
    Moose::MProblem *mproblem = dynamic_cast<Moose::MProblem *>(&_parser_handle._executioner->problem());
    _parser_handle._problem = mproblem;

    // FIXME: HACK! Can initialize displaced problem after we have instance of problem
    ParserBlock * mesh_blk = _parser_handle.root()->locateBlock("Mesh");
    if(mesh_blk->isParamValid("displacements"))
    {
      std::vector<std::string> displacements = mesh_blk->getParamValue<std::vector<std::string> >("displacements");
      _parser_handle._problem->initDisplacedProblem(displacements);
    }

    // handle functions
    ParserBlock * fns = locateBlock("Functions");
    if (fns)
      fns->execute();

    VariablesBlock * vars = dynamic_cast<VariablesBlock *>(_parser_handle.root()->locateBlock("Variables"));
    if (vars!= NULL)
      vars->execute();
    VariablesBlock * aux_vars = dynamic_cast<VariablesBlock *>(_parser_handle.root()->locateBlock("AuxVariables"));
    if (aux_vars!= NULL)
      aux_vars->execute();

    // handle periodic BCs
    ParserBlock * pb = locateBlock("BCs/Periodic");
    if (pb)
      pb->execute();

    mproblem->init();
    if (vars != NULL)
      vars->copyNodalValues(mproblem->getNonlinearSystem());
    if (aux_vars != NULL)
      aux_vars->copyNodalValues(mproblem->getAuxiliarySystem());

    // solver params
    EquationSystems & es = _parser_handle._problem->es();
    es.parameters.set<Real> ("linear solver tolerance")
      = getParamValue<Real>("l_tol");

    es.parameters.set<Real> ("linear solver absolute step tolerance")
      = getParamValue<Real>("l_abs_step_tol");

    es.parameters.set<unsigned int> ("linear solver maximum iterations")
      = getParamValue<unsigned int>("l_max_its");

    es.parameters.set<unsigned int> ("nonlinear solver maximum iterations")
      = getParamValue<unsigned int>("nl_max_its");

    es.parameters.set<unsigned int> ("nonlinear solver maximum function evaluations")
      = getParamValue<unsigned int>("nl_max_funcs");

    es.parameters.set<Real> ("nonlinear solver absolute residual tolerance")
      = getParamValue<Real>("nl_abs_tol");

    es.parameters.set<Real> ("nonlinear solver relative residual tolerance")
      = getParamValue<Real>("nl_rel_tol");

    es.parameters.set<Real> ("nonlinear solver absolute step tolerance")
      = getParamValue<Real>("nl_abs_step_tol");

    es.parameters.set<Real> ("nonlinear solver relative step tolerance")
      = getParamValue<Real>("nl_rel_step_tol");

#ifdef LIBMESH_HAVE_PETSC
    mproblem->getNonlinearSystem()._l_abs_step_tol = getParamValue<Real>("l_abs_step_tol");
#endif
//    _moose_system._no_fe_reinit = getParamValue<bool>("no_fe_reinit");

    if (!getParamValue<bool>("perf_log"))
      Moose::perf_log.disable_logging();

//    _moose_system._auto_scaling = getParamValue<bool>("auto_scaling");

    Moose::ImplicitSystem & nl = _parser_handle._problem->getNonlinearSystem();
    nl.timeSteppingScheme(Moose::stringToEnum<Moose::TimeSteppingScheme>(getParamValue<std::string>("scheme")));

    ParserBlock * blk;

    blk= locateBlock("GlobalParams");
    if (blk)
      blk->execute();

    blk = locateBlock("Materials");
    if (blk)
      blk->execute();

    blk = locateBlock("Kernels");
    if (blk)
      blk->execute();
    blk = locateBlock("BCs");
    if (blk)
      blk->execute();
    blk= locateBlock("DiracKernels");
    if (blk)
      blk->execute();
    blk= locateBlock("Dampers");
    if (blk)
      blk->execute();
    blk= locateBlock("Stabilizers");
    if (blk)
      blk->execute();

    blk= locateBlock("AuxKernels");
    if (blk)
      blk->execute();
    blk = locateBlock("AuxBCs");
    if (blk)
      blk->execute();

    blk = locateBlock("Postprocessors");
    if (blk)
      blk->execute();
  }

  visitChildren();
}

