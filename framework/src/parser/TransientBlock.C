#include "TransientBlock.h"

TransientBlock::TransientBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, const GetPot & input_file)
  :ParserBlock(reg_id, real_id, parent, input_file)
{
  addParam<Real>        ("start_time",                         0.0,    "The start time of the simulation", false);
  addParam<Real>        ("end_time",                           1.0e30, "The end time of the simulation",   false);
  addParam<Real>        ("dt",                                 -1.0,    "The timestep size between solves", false);
  addParam<Real>        ("dtmin",                              0.0, "The minimum timestep size in an adaptive run", false);
  addParam<Real>        ("dtmax",                              1.0e30, "The maximum timestep size in an adaptive run", false);
  addParam<int>         ("num_steps",                          -1,     "The number of timesteps in a transient run", false);
  addParam<int>         ("n_startup_steps",                    0,      "The number of timesteps during startup", false);
  addParam<bool>        ("adaptive_time_stepping",             false,  "Specifies whether adaptive time stepping is turned on or off (default: off)", false);
  addParam<bool>        ("sol_time_adaptive_time_stepping",    false,  "Specifies that an algorithm that minimizes solution time (wall time) is used", false);
  addParam<bool>        ("exponential_time_stepping",          false,  "Specifies that an algorithm that employees exponetial time stepping is used", false);
  addParam<bool>        ("trans_ss_check",                     false,  "TODO: doc string", false);
  addParam<Real>        ("ss_check_tol",                       1.0e-08,"TODO: doc string", false);
  addParam<Real>        ("ss_tmin",                            0.0,    "TODO: doc string", false);
  addParam<Real>        ("reject_step_error",                  -1.0,   "TODO: doc string", false);
}

void
TransientBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the TransientBlock Object\n";
#endif

  visitChildren();
}

void
TransientBlock::setOutOfOrderTransientParams(Parameters & params) const
{
  std::cerr << "Setting Time Param in Moose equation_system" << std::endl;
  params.set<Real> ("time")                            = getParamValue<Real> ("start_time");
  params.set<Real> ("end_time")                        = getParamValue<Real> ("end_time");
  params.set<Real> ("dt")                              = getParamValue<Real> ("dt");
  params.set<Real> ("dtmin")                           = getParamValue<Real> ("dtmin");
  params.set<Real> ("dtmax")                           = getParamValue<Real> ("dtmax");
  params.set<int>  ("num_steps")                       = getParamValue<int>  ("num_steps");    
  params.set<int>  ("n_startup_steps")                 = getParamValue<int>  ("n_startup_steps");
  params.set<bool> ("adaptive_time_stepping")          = getParamValue<bool> ("adaptive_time_stepping");
  params.set<bool> ("sol_time_adaptive_time_stepping") = getParamValue<bool> ("sol_time_adaptive_time_stepping");
  params.set<bool> ("exponential_time_stepping")       = getParamValue<bool> ("exponential_time_stepping");
  params.set<bool> ("trans_ss_check")                  = getParamValue<bool> ("trans_ss_check");
  params.set<Real> ("ss_check_tol")                    = getParamValue<Real> ("ss_check_tol");
  params.set<Real> ("ss_tmin")                         = getParamValue<Real> ("ss_tmin");
  params.set<Real> ("reject_step_error")               = getParamValue<Real> ("reject_step_error");
}
