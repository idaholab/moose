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

class Problem;
class Outputter;

class Output
{
public:
  enum Type {
    EXODUS,
    GMV,
    TECPLOT,
    TECPLOT_BIN,
    XDA
  };

  Output(Problem & problem);
  virtual ~Output();

  void add(Type type);

  void timestepSetup();

  void output();

#ifdef LIBMESH_HAVE_PETSC
  static PetscErrorCode iterationOutput(SNES, PetscInt its, PetscReal fnorm, void *);
#endif

  // FIXME: right now, it is here - might go somewhere else?
  void outputPps(const FormattedTable & table);
  void outputInput();

  void fileBase(const std::string & file_base) { _file_base = file_base; }
  std::string & fileBase() { return _file_base; }

  void interval(int interval) { _interval = interval; }
  int interval() { return _interval; }

  void meshChanged();
  void sequence(bool state);

  void iterationPlotStartTime(Real t)
  {
    _iteration_plot_start_time = t;
  }
  Real iterationPlotStartTime()
  {
    return _iteration_plot_start_time;
  }

protected:
  std::string _file_base;

  Problem & _problem;
  Real & _time;
  Real & _dt;
  int _interval;
  Real _iteration_plot_start_time;

  std::vector<Outputter *> _outputters;
};

#endif /* OUTPUTTER_H */
