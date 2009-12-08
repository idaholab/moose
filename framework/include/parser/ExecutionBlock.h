#ifndef EXECUTIONBLOCK_H
#define EXECUTIONBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class ExecutionBlock;

template<>
InputParameters validParams<ExecutionBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  params.addRequiredParam<std::string> ("type", "Specifies \"Steady\" or \"Transient\" Solver Strategy");
  
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

#ifdef LIBMESH_HAVE_PETSC
  params.addParam<std::vector<std::string> >("petsc_options", "Singleton Petsc options");
  params.addParam<std::vector<std::string> >("petsc_options_iname", "Names of Petsc name/value pairs");
  params.addParam<std::vector<std::string> >("petsc_options_value", "Values of Petsc name/value pairs (must correspond with \"petsc_options_iname\"");
#endif //LIBMESH_HAVE_PETSC

  return params;
}

class ExecutionBlock: public ParserBlock
{
public:
  ExecutionBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params);

  virtual void execute();
};

  

#endif //EXECUTIONBLOCK_H
