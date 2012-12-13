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

#include "Output.h"
#include "FEProblem.h"
#include "NonlinearSystem.h"

#include "Outputter.h"
#include "ExodusOutput.h"
#include "NemesisOutput.h"
#include "GMVOutput.h"
#include "XDAOutput.h"
#include "TecplotOutput.h"

Output::Output(FEProblem & fe_problem, EquationSystems & eq) :
    _file_base("out"),
    _fe_problem(fe_problem),
    _eq(eq),
    _time(_fe_problem.time()),
    _dt(_fe_problem.dt()),
    _interval(1),
    _screen_interval(1),
    _iteration_plot_start_time(std::numeric_limits<Real>::max()),
    _last_iteration_output_time(0.0)
{
}

Output::~Output()
{
  for (unsigned int i = 0; i < _outputters.size(); i++)
    delete _outputters[i];
}

void
Output::init()
{
  // put nodal variables into _output_vars
  std::vector<std::string> vars = _fe_problem.getVariableNames();
  for (unsigned int i = 0; i < vars.size(); ++i)
  {
    std::string var_name = vars[i];
    if (_fe_problem.hasVariable(var_name))
    {
      MooseVariable & v = _fe_problem.getVariable(0, var_name);
      if (v.isNodal())
        _output_variables.push_back(var_name);
    }
  }
}

void
Output::add(Output::Type type, bool output_input)
{
  Outputter *o = NULL;
  switch (type)
  {
  case EXODUS:
    o = new ExodusOutput(_eq, output_input);
    break;

  case NEMESIS:
    o = new NemesisOutput(_eq);
    break;

  case GMV:
    o = new GMVOutput(_eq);
    break;

  case TECPLOT:
    o = new TecplotOutput(_eq, false);
    break;

  case TECPLOT_BIN:
    o = new TecplotOutput(_eq, true);
    break;

  case XDA:
    o = new XDAOutput(_eq);
    break;

  default:
    mooseError("I do not know how to build and unknown outputter");
    break;
  }

  _outputter_types.insert(type);

  o->setOutputVariables(_output_variables);

  _outputters.push_back(o);
}

void
Output::output()
{
  for (unsigned int i = 0; i < _outputters.size(); i++)
    _outputters[i]->output(_file_base, _time);
}

void
Output::timestepSetup()
{
#ifdef LIBMESH_HAVE_PETSC
  if (_time >= _iteration_plot_start_time)
  {
    NonlinearSystem & nl = _fe_problem.getNonlinearSystem();
    PetscNonlinearSolver<Number> * petsc_solver = dynamic_cast<PetscNonlinearSolver<Number> *>(nl.sys().nonlinear_solver.get());
    SNES snes = petsc_solver->snes();
#if PETSC_VERSION_LESS_THAN(2,3,3)
    PetscErrorCode ierr =
      SNESSetMonitor (snes, Output::iterationOutput, this, PETSC_NULL);
#else
    // API name change in PETSc 2.3.3
    PetscErrorCode ierr =
      SNESMonitorSet (snes, Output::iterationOutput, this, PETSC_NULL);
#endif
    CHKERRABORT(libMesh::COMM_WORLD,ierr);
  }
#endif
}

#ifdef LIBMESH_HAVE_PETSC
PetscErrorCode
Output::iterationOutput(SNES, PetscInt its, PetscReal /*fnorm*/, void * _output)
{
  Output * output = static_cast<Output*>(_output);
  mooseAssert(output, "Error in iterationOutput");
  if (output->_time >= output->_iteration_plot_start_time)
  {
    // Create an output time.  The time will be larger than the time of the previous
    // solution, and it will increase with each iteration.  Using 1e-3 indicates that
    // after 1000 nonlinear iterations, we'll overrun the next solution time.  That
    // should be more than enough.

    Real last_time = output->_time - output->_dt;

    // If the last iteration output time happened before the last solve finished then advance to after that time
    // if not just keep incrementing time
    if(output->_last_iteration_output_time <= last_time)
      output->_last_iteration_output_time = last_time + output->_dt*1e-2;
    else
      output->_last_iteration_output_time += output->_dt*1e-2;

    Real iteration_time = output->_last_iteration_output_time;
    std::cout << "  Writing iteration plot for NL step " << its << " at time " << iteration_time << std::endl;
    for (unsigned int i(0); i < output->_outputters.size(); ++i)
    {
      output->_outputters[i]->output(output->_file_base, iteration_time);
    }
  }
  return 0;
}
#endif

bool
Output::isOutputterActive(Type type)
{
  return _outputter_types.find(type) != _outputter_types.end();
}

bool
Output::PpsFileOutputEnabled()
{
  bool supports_pps_output = false;
  for (unsigned int i = 0; i < _outputters.size(); i++)
    supports_pps_output |= _outputters[i]->supportsPpsOutput();
  return supports_pps_output;
}

void
Output::outputPps(const FormattedTable & table)
{
  for (unsigned int i = 0; i < _outputters.size(); i++)
    _outputters[i]->outputPps(_file_base, table, _time);
}

void
Output::outputInput()
{
  for (unsigned int i = 0; i < _outputters.size(); i++)
    _outputters[i]->outputInput();
}

void
Output::outputSolutionHistory()
{
  NonlinearSystem & nl_sys = _fe_problem.getNonlinearSystem();

  std::ofstream slh_file;
  slh_file.open((_file_base + ".slh").c_str(), std::ios::app);
  slh_file << nl_sys._current_nl_its;

  for(unsigned int i=0; i<nl_sys._current_l_its.size(); i++)
    slh_file << " " << nl_sys._current_l_its[i];

  slh_file<<std::endl;
}

void
Output::interval(unsigned int interval)
{
  _interval = interval;
}

int
Output::interval()
{
  return _interval;
}

void
Output::screen_interval(unsigned int screen_interval)
{
  _screen_interval = screen_interval;
}

int
Output::screen_interval()
{
  return _screen_interval;
}

void
Output::meshChanged()
{
  for (unsigned int i = 0; i < _outputters.size(); i++)
    _outputters[i]->meshChanged();
}

void
Output::sequence(bool state)
{
  for (unsigned int i = 0; i < _outputters.size(); i++)
    _outputters[i]->sequence(state);
}

void
Output::iterationPlotStartTime(Real t)
{
  _iteration_plot_start_time = t;
}

Real
Output::iterationPlotStartTime()
{
  return _iteration_plot_start_time;
}
