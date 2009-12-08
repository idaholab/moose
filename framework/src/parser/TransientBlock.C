#include "TransientBlock.h"
#include "Moose.h"

TransientBlock::TransientBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params)
  :ParserBlock(reg_id, real_id, parent, parser_handle, params)
{}

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
