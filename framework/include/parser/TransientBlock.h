#ifndef TRANSIENTBLOCK_H
#define TRANSIENTBLOCK_H

#include "ParserBlock.h"
#include "InputParameters.h"

//Forward Declarations
class TransientBlock;

template<>
InputParameters validParams<TransientBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  params.addParam<Real>        ("start_time",                         0.0,    "The start time of the simulation");
  params.addParam<Real>        ("end_time",                           1.0e30, "The end time of the simulation");
  params.addParam<Real>        ("dt",                                 -1.0,    "The timestep size between solves");
  params.addParam<Real>        ("dtmin",                              0.0, "The minimum timestep size in an adaptive run");
  params.addParam<Real>        ("dtmax",                              1.0e30, "The maximum timestep size in an adaptive run");
  params.addParam<int>         ("num_steps",                          -1,     "The number of timesteps in a transient run");
  params.addParam<int>         ("n_startup_steps",                    0,      "The number of timesteps during startup");
  params.addParam<bool>        ("adaptive_time_stepping",             false,  "Specifies whether adaptive time stepping is turned on or off (default: off)");
  params.addParam<bool>        ("sol_time_adaptive_time_stepping",    false,  "Specifies that an algorithm that minimizes solution time (wall time) is used");
  params.addParam<bool>        ("exponential_time_stepping",          false,  "Specifies that an algorithm that employees exponetial time stepping is used");
  params.addParam<bool>        ("trans_ss_check",                     false,  "TODO: doc string");
  params.addParam<Real>        ("ss_check_tol",                       1.0e-08,"TODO: doc string");
  params.addParam<Real>        ("ss_tmin",                            0.0,    "TODO: doc string");
  params.addParam<Real>        ("reject_step_error",                  -1.0,   "TODO: doc string");
  return params;
}

class TransientBlock: public ParserBlock
{
public:
  TransientBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params);

  virtual void execute();

  void setOutOfOrderTransientParams(Parameters & params) const;
};

  

#endif //TRANSIENTBLOCK_H
