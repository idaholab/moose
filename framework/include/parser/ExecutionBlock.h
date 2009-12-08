#ifndef EXECUTIONBLOCK_H
#define EXECUTIONBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class ExecutionBlock;

template<>
InputParameters validParams<ExecutionBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  params.addParam<std::string> ("type",            "",       "Specifies \"Steady\" or \"Transient\" Solver Strategy", true);
  params.addParam<Real>        ("l_tol",           1.0e-5,   "Linear Tolerance",                  false);
  params.addParam<Real>        ("l_abs_step_tol",  -1,       "Linear Absolute Step Tolerance",    false);
  params.addParam<unsigned int>("l_max_its",       10000,    "Max Linear Iterations",             false);
  params.addParam<unsigned int>("nl_max_its",      50,       "Max Nonlinear Iterations",          false);
  params.addParam<unsigned int>("nl_max_funcs",    10000,    "Max Nonlinear solver function evaluations", false);
  params.addParam<Real>        ("nl_abs_tol",      1.0e-50,  "Nonlinear Absolute Tolerance",      false);
  params.addParam<Real>        ("nl_rel_tol",      1.0e-8,  "Nonlinear Relative Tolerance",      false);
  params.addParam<Real>        ("nl_abs_step_tol", 1.0e-50,  "Nonlinear Absolute step Tolerance", false);
  params.addParam<Real>        ("nl_rel_step_tol", 1.0e-50,  "Nonlinear Relative step Tolerance", false);
  params.addParam<bool>        ("no_fe_reinit",    false,    "Specifies whether or not to reinitialize FEs", false);
  params.addParam<bool>        ("perf_log",        false,    "Specifies whether or not the Performance log should be printed", false);
  params.addParam<bool>        ("auto_scaling",    false,    "Turns on automatic variable scaling", false);

#ifdef LIBMESH_HAVE_PETSC
  params.addParam<std::vector<std::string> >("petsc_options", "Singleton Petsc options", false);
  params.addParam<std::vector<std::string> >("petsc_options_iname", "Names of Petsc name/value pairs", false);
  params.addParam<std::vector<std::string> >("petsc_options_value", "Values of Petsc name/value pairs (must correspond with \"petsc_options_iname\"", false);
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
