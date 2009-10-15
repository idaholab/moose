#include "TransientBlock.h"

TransientBlock::TransientBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, const GetPot & input_file)
  :ParserBlock(reg_id, real_id, parent, input_file)
{
  _block_params.set<Real>        ("start_time")                         = 0.0;
  _block_params.set<Real>        ("end_time")                           = 1.0e30;
  _block_params.set<Real>        ("dt")                                 = -1.0;
  _block_params.set<Real>        ("dtmin")                              = 0.0;
  _block_params.set<Real>        ("dtmax")                              = 1.0e30;
  _block_params.set<int>         ("num_steps")                          = -1;
  _block_params.set<int>         ("n_startup_steps")                    = 0;
  _block_params.set<bool>        ("adaptive_time_stepping")             = false;
  _block_params.set<bool>        ("sol_time_adaptive_time_stepping")    = false;
  _block_params.set<bool>        ("exponential_time_stepping")          = false;
  _block_params.set<bool>        ("trans_ss_check")                     = false;
  _block_params.set<Real>        ("ss_check_tol")                       = 1.0e-08;
  _block_params.set<Real>        ("ss_tmin")                            = 0.0;
  _block_params.set<Real>        ("reject_step_error")                  = -1.0;
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
  params.set<Real> ("time")                            = _block_params.get<Real> ("start_time");
  params.set<Real> ("end_time")                        = _block_params.get<Real> ("end_time");
  params.set<Real> ("dt")                              = _block_params.get<Real> ("dt");
  params.set<Real> ("dtmin")                           = _block_params.get<Real> ("dtmin");
  params.set<Real> ("dtmax")                           = _block_params.get<Real> ("dtmax");
  params.set<int>  ("num_steps")                       = _block_params.get<int>  ("num_steps");    
  params.set<int>  ("n_startup_steps")                 = _block_params.get<int>  ("n_startup_steps");
  params.set<bool> ("adaptive_time_stepping")          = _block_params.get<bool> ("adaptive_time_stepping");
  params.set<bool> ("sol_time_adaptive_time_stepping") = _block_params.get<bool> ("sol_time_adaptive_time_stepping");
  params.set<bool> ("exponential_time_stepping")       = _block_params.get<bool> ("exponential_time_stepping");
  params.set<bool> ("trans_ss_check")                  = _block_params.get<bool> ("trans_ss_check");
  params.set<Real> ("ss_check_tol")                    = _block_params.get<Real> ("ss_check_tol");
  params.set<Real> ("ss_tmin")                         = _block_params.get<Real> ("ss_tmin");
  params.set<Real> ("reject_step_error")               = _block_params.get<Real> ("reject_step_error");
}
