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

// libMesh
#include "libmesh_common.h"
#include "exodusII_io.h"

class ExodusOutput : public Outputter
{
public:
  ExodusOutput(EquationSystems & es);
  virtual ~ExodusOutput();

  virtual void output(const std::string & file_base, Real time);
  virtual void outputPps(const std::string & file_base, const FormattedTable & table, Real time);
  virtual void outputInput();

  virtual void meshChanged();
  virtual void sequence(bool state) { _seq = state; }

protected:
  ExodusII_IO * _out;

  bool _seq;
  int _file_num;                        /// number of the file
  int _num;                             /// the number of timestep within the file
  std::string getFileName(const std::string & file_base);
};

#endif /* OUTPUTTER_H */
