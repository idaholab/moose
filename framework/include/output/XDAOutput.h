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

#ifndef XDAOUTPUT_H
#define XDAOUTPUT_H

#include "Outputter.h"
#include "FormattedTable.h"

// libMesh
#include "libmesh/libmesh_common.h"

class XDAOutput : public Outputter
{
public:
  XDAOutput(EquationSystems & es);
  virtual ~XDAOutput();

  virtual void output(const std::string & file_base, Real time);
  virtual void outputPps(const std::string & file_base, const FormattedTable & table, Real time);

  virtual void meshChanged();
  virtual void sequence(bool state);

  std::string getFileName(const std::string & file_base);

protected:
  /// number of the file
  int _file_num;
};

#endif /* XDAOUTPUT_H */
