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

Output::Output(Problem & problem) :
    _file_base("out"),
    _problem(problem),
    _time(_problem.time()),
    _dt(_problem.dt()),
    _interval(1),
    _screen_interval(1),
    _iteration_plot_start_time(std::numeric_limits<Real>::max())
{
}

Output::~Output()
{
  for (unsigned int i = 0; i < _outputters.size(); i++)
    delete _outputters[i];
}

void
Output::add(Output::Type type)
{
  Outputter *o = NULL;
  switch (type)
  {
  case EXODUS:
    o = new ExodusOutput(_problem.es());
    break;

  case NEMESIS:
    o = new NemesisOutput(_problem.es());
    break;

  case GMV:
    o = new GMVOutput(_problem.es());
    break;

  case TECPLOT:
    o = new TecplotOutput(_problem.es(), false);
    break;

  case TECPLOT_BIN:
    o = new TecplotOutput(_problem.es(), true);
    break;

  case XDA:
    o = new XDAOutput(_problem.es());
    break;

  default:
    mooseError("I do not know how to build and unknown outputter");
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
  FEProblem * mproblem( dynamic_cast<FEProblem*>(&_problem) );
  if (_time >= _iteration_plot_start_time && mproblem)
  {
    NonlinearSystem & nl = mproblem->getNonlinearSystem();
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
  if (output->_time >= output->_iteration_plot_start_time && its )
  {
    // Create an output time.  The time will be larger than the time of the previous
    // solution, and it will increase with each iteration.  Using 1e-3 indicates that
    // after 1000 nonlinear iterations, we'll overrun the next solution time.  That
    // should be more than enough.
    Real iteration_time( (output->_time-output->_dt) + its * output->_dt * 1e-3 );
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
