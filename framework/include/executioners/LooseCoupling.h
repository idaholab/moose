/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef LOOSECOUPLING_H
#define LOOSECOUPLING_H

#include "Executioner.h"
#include "MooseMesh.h"
#include "CoupledProblem.h"

class LooseCoupling;

template<>
InputParameters validParams<LooseCoupling>();


class LooseCoupling : public Executioner
{
public:
  LooseCoupling(const std::string & name, InputParameters parameters);
  virtual ~LooseCoupling();

  virtual void execute();

  virtual Problem & problem() { return _problem; }

  InputParameters setupProblemParams(std::string name, MooseMesh * mesh);

protected:
  bool _shared_mesh;
  std::vector<Parser *> _slave_parser;
  std::vector<std::string> _input_files;
  std::vector<std::string> _solve_order;

  CoupledProblem & _problem;

  int & _t_step;                        ///< Current timestep.
  Real & _time;                         ///< Current time
  Real _time_old;
  Real _input_dt;                       ///< The dt from the input file.
  Real & _dt;                           ///< Current delta t... or timestep size.
  Real & _dt_old;

  Real _prev_dt;
  bool _reset_dt;

  Real _end_time;
  Real _dtmin;
  Real _dtmax;
  Real _num_steps;
  int _n_startup_steps;

  ActionWarehouse & _act_wh;

  void executeBlocks(const std::string & name);
};

#endif /* LOOSECOUPLING_H */
