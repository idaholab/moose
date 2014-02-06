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

#ifndef EXODUSOUTPUTTER_H
#define EXODUSOUTPUTTER_H

#include "Outputter.h"
#include "FormattedTable.h"
#include "MooseApp.h"

// libMesh
#include "libmesh/libmesh_common.h"
#include "libmesh/exodusII_io.h"

#include <string>

class ExodusOutput : public Outputter
{
public:
  ExodusOutput(MooseApp & app, EquationSystems & es, bool output_input, SubProblem & sub_problem, std::string name);
  virtual ~ExodusOutput();

  virtual void output(const std::string & file_base, Real time, unsigned int t_step);
  virtual void outputPps(const std::string & file_base, const FormattedTable & table, Real time);
  virtual bool supportsPpsOutput() { return true; }
  virtual void outputInput();

  virtual void meshChanged();
  virtual void sequence(bool state) { _seq = state; }

  /// Set (or reset) the output position
  virtual void setOutputPosition(const Point & p);

protected:
  MooseApp & _app;

  ExodusII_IO * _out;

  bool & _seq;
  /// number of the file
  int & _file_num;
  /// the number of timestep within the file
  int & _num;

  bool _output_input;
  std::string getFileName(const std::string & file_base);

  bool & _first;

  bool & _mesh_just_changed;

  /**
   * Since we need to allocate the ExodusII_IO object in two different
   * locations, use this helper function to make sure it gets
   * allocated the same way no matter what.
   */
  void allocateExodusObject();
};

#endif /* EXODUSOUTPUTTER_H */
