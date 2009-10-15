#include "ExecutionBlock.h"

ExecutionBlock::ExecutionBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, const GetPot & input_file)
  :ParserBlock(reg_id, real_id, parent, input_file)
{
  _block_params.set<Real>        ("l_tol")               = 1.0e-5;
  _block_params.set<Real>        ("l_abs_step_tol")      = -1;
  _block_params.set<unsigned int>("l_max_its")           = 10000;
  _block_params.set<unsigned int>("nl_max_its")          = 50;
  _block_params.set<unsigned int>("nl_max_funcs")        = 10000;
  _block_params.set<Real>        ("nl_abs_tol")          = 1.0e-50;
  _block_params.set<Real>        ("nl_rel_tol")          = 1.0e-8;
  _block_params.set<Real>        ("nl_abs_step_tol")     = 1.0e-50;
  _block_params.set<Real>        ("nl_rel_step_tol")     = 1.0e-50;
  _block_params.set<bool>        ("no_fe_reinit")        = false;
  _block_params.set<std::string> ("type");
  _block_params.set<bool>        ("perf_log")            = false;
  _block_params.set<bool>        ("auto_scaling")        = false;
}

void
ExecutionBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the ExecutionBlock Object\n";
#endif

  Moose::equation_system->parameters.set<Real> ("linear solver tolerance")
    = _block_params.get<Real>("l_tol");

  Moose::equation_system->parameters.set<Real> ("linear solver absolute step tolerance")
    = _block_params.get<Real>("l_abs_step_tol");

  Moose::equation_system->parameters.set<unsigned int> ("linear solver maximum iterations")
    = _block_params.get<unsigned int>("l_max_its");    
  
  Moose::equation_system->parameters.set<unsigned int> ("nonlinear solver maximum iterations")
    = _block_params.get<unsigned int>("nl_max_its");
    
  Moose::equation_system->parameters.set<unsigned int> ("nonlinear solver maximum function evaluations")
    = _block_params.get<unsigned int>("nl_max_funcs");
    
  Moose::equation_system->parameters.set<Real> ("nonlinear solver absolute residual tolerance")
    = _block_params.get<Real>("nl_abs_tol");
  
  Moose::equation_system->parameters.set<Real> ("nonlinear solver relative residual tolerance")
    = _block_params.get<Real>("nl_rel_tol");
  
  Moose::equation_system->parameters.set<Real> ("nonlinear solver absolute step tolerance")
    = _block_params.get<Real>("nl_abs_step_tol");
  
  Moose::equation_system->parameters.set<Real> ("nonlinear solver relative step tolerance")
    = _block_params.get<Real>("nl_rel_step_tol");

  Moose::no_fe_reinit = _block_params.get<bool>("no_fe_reinit");

  if (!_block_params.get<bool>("perf_log"))
    Moose::perf_log.disable_logging();

  Moose::execution_type = _block_params.get<std::string>("type");

  Moose::auto_scaling = _block_params.get<bool>("auto_scaling");

  visitChildren();
}
