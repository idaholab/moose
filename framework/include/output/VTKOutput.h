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

#ifndef VTKOUTPUTTER_H
#define VTKOUTPUTTER_H

#include "Outputter.h"
#include "FormattedTable.h"

// libMesh
#include "libmesh/libmesh_common.h"
#include "libmesh/vtk_io.h"

class VTKOutput : public Outputter
{
public:
  VTKOutput(EquationSystems & es, SubProblem & sub_problem);
  virtual ~VTKOutput();

  virtual void output(const std::string & file_base, Real time, unsigned int t_step);
  virtual void outputPps(const std::string & file_base, const FormattedTable & table, Real time);

  virtual void meshChanged();
  virtual void sequence(bool state);

protected:
  VTKIO * _out;

  std::string getFileName(const std::string & file_base, unsigned int t_step);
};

#endif /* OUTPUTTER_H */
