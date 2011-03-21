#ifndef LOOSECOUPLING_H
#define LOOSECOUPLING_H

#include "Executioner.h"
#include "MooseMesh.h"
#include "CoupledProblem.h"

class LooseCoupling : public Executioner
{
public:
  LooseCoupling(const std::string & name, InputParameters parameters);
  virtual ~LooseCoupling();

  virtual void execute();

  virtual Problem & problem() { return _problem; }

protected:
  bool _shared_mesh;
  std::vector<Parser *> _slave_parser;
  std::vector<std::string> _input_files;
  std::vector<std::string> _solve_order;

  CoupledProblem _problem;

  int & _t_step;                        /// Current timestep.
  Real & _time;                         /// Current time
  Real _time_old;
  Real _input_dt;                       /// The dt from the input file.
  Real & _dt;                           /// Current delta t... or timestep size.
  Real & _dt_old;

  Real _prev_dt;
  bool _reset_dt;

  Real _end_time;
  Real _dtmin;
  Real _dtmax;
  Real _num_steps;
  int _n_startup_steps;



  void executeBlocks(const std::string & name);
};

template<>
InputParameters validParams<LooseCoupling>();

#endif /* LOOSECOUPLING_H_ */
