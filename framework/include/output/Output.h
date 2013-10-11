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

#include "Moose.h"
#include "MooseTypes.h"
#include "FormattedTable.h"

// libMesh
#include "libmesh/libmesh_common.h"
#include "libmesh/petsc_nonlinear_solver.h"

#include <string>
#include <vector>

class FEProblem;
class Outputter;

class Output
{
public:
  enum Type {
    EXODUS,
    NEMESIS,
    GMV,
    VTK,
    TECPLOT,
    TECPLOT_BIN,
    XDA,
    XDR
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

  void interval(unsigned int interval);
  int interval();
  void screen_interval(unsigned int screen_interval);
  int screen_interval();

  void meshChanged();
  void sequence(bool state);

  bool isOutputterActive(Type type);

  void iterationPlotStartTime(Real t);

  Real iterationPlotStartTime();

  /**
   * Sets the variables to be output.
   *
   * Must be called before any outputers are added!
   */
  void setOutputVariables(std::vector<VariableName> output_variables) { _output_variables = output_variables; }

  bool PpsFileOutputEnabled();
  /// sets the time interval at which the solution should be output
  void setTimeIntervalOutput(Real time_interval);
  /// if outputing at a set time interval is set
  bool useTimeInterval();
  /// the set time interval at which output occurs, in useTimeInterval returns true
  Real timeinterval();
  ///if output occured at this time step
  bool wasOutput();
  ///sets if output has occured at time step
  void setOutput(bool b);

  /// Set (or reset) the output position
  void setOutputPosition(Point p);

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

  /// The variables to be output
  std::vector<VariableName> _output_variables;

  Real _last_iteration_output_time;
  ///if to use time interval output
  bool _time_interval;
  ///the output interval to use
  Real _time_interval_output_interval;
  bool _output;
};

#endif /* OUTPUTTER_H */
