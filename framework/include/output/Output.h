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

#ifndef OUTPUT_H
#define OUTPUT_H

#include <string>
#include <vector>
#include "FormattedTable.h"
// libMesh
#include "libmesh_common.h"

//PETSc includes
#include "petsc_nonlinear_solver.h"

class FEProblem;
class Outputter;

class Output
{
public:
  enum Type {
    EXODUS,
    NEMESIS,
    GMV,
    TECPLOT,
    TECPLOT_BIN,
    XDA
  };

  // FIXME: this is not good - the whole Output class has to be refactored with respect to OutputProblem class
  Output(FEProblem & problem, EquationSystems & eq);
  virtual ~Output();

  void init();

  void add(Type type, bool output_input=true);

  // TODO: move to OutputProblem
  void timestepSetup();

  void output();

#ifdef LIBMESH_HAVE_PETSC
  static PetscErrorCode iterationOutput(SNES, PetscInt its, PetscReal fnorm, void *);
#endif

  // FIXME: right now, it is here - might go somewhere else?
  void outputPps(const FormattedTable & table);
  void outputInput();
  void outputSolutionHistory();

  void fileBase(const std::string & file_base) { _file_base = file_base; }
  std::string & fileBase() { return _file_base; }

  void interval(unsigned int interval) { _interval = interval; }
  int interval() { return _interval; }
  void screen_interval(unsigned int screen_interval) { _screen_interval = screen_interval; }
  int screen_interval() { return _screen_interval; }

  void meshChanged();
  void sequence(bool state);

  bool isOutputterActive(Type type);

  void iterationPlotStartTime(Real t)
  {
    _iteration_plot_start_time = t;
  }
  Real iterationPlotStartTime()
  {
    return _iteration_plot_start_time;
  }

  /**
   * Sets the variables to be output.
   *
   * Must be called before any outputers are added!
   */
  void setOutputVariables(std::vector<std::string> output_variables) { _output_variables = output_variables; }

protected:
  std::string _file_base;

  FEProblem & _fe_problem;
  EquationSystems & _eq;
  Real & _time;
  Real & _dt;
  unsigned int _interval;
  unsigned int _screen_interval;
  Real _iteration_plot_start_time;

  std::vector<Outputter *> _outputters;
  std::set<Type> _outputter_types;

  std::vector<std::string> _output_variables;           ///< The variables to be output

  Real _last_iteration_output_time;
};

#endif /* OUTPUTTER_H */
