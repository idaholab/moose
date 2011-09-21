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

#include "LooseCoupling.h"
#include "Parser.h"
#include "MooseSyntax.h"
#include "CoupledProblem.h"
#include "SubProblem.h"
#include "FEProblem.h"
#include "ProblemFactory.h"
#include "ActionWarehouse.h"

template<>
InputParameters validParams<LooseCoupling>()
{
  InputParameters params = validParams<Executioner>();

//  params.addParam<std::string>("mesh_file", "Mesh file to read");
  params.addRequiredParam<std::vector<std::string> >("input_files", "Input files to read");
  params.addRequiredParam<std::vector<std::string> >("solve_order", "Order of the solve");

  params.addParam<Real>("start_time",      0.0,    "The start time of the simulation");
  params.addParam<Real>("end_time",        1.0e30, "The end time of the simulation");
  params.addRequiredParam<Real>("dt", "The timestep size between solves");
  params.addParam<Real>("dtmin",           0.0,    "The minimum timestep size in an adaptive run");
  params.addParam<Real>("dtmax",           1.0e30, "The maximum timestep size in an adaptive run");
  params.addParam<Real>("num_steps",       std::numeric_limits<Real>::max(),     "The number of timesteps in a transient run");
  params.addParam<int> ("n_startup_steps", 0,      "The number of timesteps during startup");
  params.addParam<bool>("trans_ss_check",  false,  "Whether or not to check for steady state conditions");
  params.addParam<Real>("ss_check_tol",    1.0e-08,"Whenever the relative residual changes by less than this the solution will be considered to be at steady state.");
  params.addParam<Real>("ss_tmin",         0.0,    "Minimum number of timesteps to take before checking for steady state conditions.");
  params.addParam<std::vector<Real> >("time_t", "The values of t");
  params.addParam<std::vector<Real> >("time_dt", "The values of dt");

  return params;
}

LooseCoupling::LooseCoupling(const std::string & name, InputParameters parameters) :
    Executioner(name, parameters),
    _shared_mesh(_mesh != NULL),
    _input_files(getParam<std::vector<std::string> >("input_files")),
    _solve_order(getParam<std::vector<std::string> >("solve_order")),
    _problem(*static_cast<CoupledProblem *>(ProblemFactory::instance()->create("CoupledProblem", "Coupled Problem", setupProblemParams("Coupled Problem", _mesh)))),
    _t_step(_problem.timeStep()),
    _time(_problem.time()),
    _time_old(_time),
    _input_dt(getParam<Real>("dt")),
    _dt(_problem.dt()),
    _dt_old(_problem.dtOld()),
    _prev_dt(-1),
    _reset_dt(false),
    _end_time(getParam<Real>("end_time")),
    _dtmin(getParam<Real>("dtmin")),
    _dtmax(getParam<Real>("dtmax")),
    _num_steps(getParam<Real>("num_steps")),
    _n_startup_steps(getParam<int>("n_startup_steps")),
    _act_wh(Moose::action_warehouse)
{
  _t_step = 0;
  _dt = _input_dt;
  _time = getParam<Real>("start_time");

  unsigned int n_problems = _input_files.size();
  _slave_parser.resize(n_problems);
  Moose::action_warehouse.clear();   // Clear the action warehouse
  for (unsigned int i = 0 ; i < n_problems; ++i)
  {
    std::string file_name = _input_files[i];

    std::cout << "  - parsing " << file_name << std::endl;
    _slave_parser[i] = new Parser(false);
    Moose::associateSyntax(*_slave_parser[i]);
    if (_shared_mesh)
    {
       InputParameters params = validParams<FEProblem>();
       params.set<std::string>("name") = "Moose Problem";
       params.set<MooseMesh *>("mesh") = _mesh;
       params.set<Problem *>("parent") = &_problem;
       FEProblem * subproblem = static_cast<FEProblem *>
         (ProblemFactory::instance()->create("FEProblem", "Moose Problem", params));
       //FEProblem * subproblem = new FEProblem(*_mesh, &_problem);
       _problem.addSubProblem(file_name, subproblem);
       _slave_parser[i]->_loose = true;
       _slave_parser[i]->_problem = subproblem;
    }
    _slave_parser[i]->parse(file_name);
  }
  _problem.transient(true);

  // need variables upfront
  executeBlocks("add_variable");
  executeBlocks("add_aux_variable");

  // execute the rest
  executeBlocks("add_material");
  executeBlocks("add_kernel");
  executeBlocks("add_bc");

  _problem.solveOrder(_solve_order);
  _problem.init();
}

LooseCoupling::~LooseCoupling()
{
  // This problem was built by the Factory and needs to be released by this destructor
  delete &_problem;
}

void
LooseCoupling::executeBlocks(const std::string & name)
{
  _act_wh.executeActionsWithAction(name);
}

void
LooseCoupling::execute()
{
  _problem.initialSetup();

  if (_output_initial)
    _problem.output();

  while (_time < _end_time)
  {
    //
    _dt = _input_dt;
    std::cout<<"DT: "<<_dt<<std::endl;

    std::cout << " Solving time step ";
    {
      OStringStream out;

      OSSInt(out,2,_t_step);
      out << ", time=";
      OSSRealzeroleft(out,9,6,_time);
      out <<  "...";
      std::cout << out.str() << std::endl;
    }


    for (std::vector<std::string>::iterator it = _solve_order.begin(); it != _solve_order.end(); ++it)
    {
      std::string problem_name = *it;

      SubProblem & subproblem = *_problem.subProblem(problem_name);

      subproblem.onTimestepBegin();

      // FIXME: FIX THIS
    //  setScaling();

  //    preSolve();

      subproblem.timestepSetup();

      // System Solve
      subproblem.solve();

  //    _converged = subproblem.converged();

  //    postSolve();

      subproblem.onTimestepEnd();
      //
    }

    _problem.output();

    // Increment time
    _t_step++;
    _time_old = _time;

    _time = _time_old + _dt;
  }

}

InputParameters
LooseCoupling::setupProblemParams(std::string name, MooseMesh * mesh)
{
  InputParameters params = validParams<FEProblem>();
  params.set<std::string>("name") = name;
  params.set<MooseMesh *>("mesh") = mesh;
  return params;
}
