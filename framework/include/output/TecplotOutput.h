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

#ifndef TECPLOTOUTPUT_H
#define TECPLOTOUTPUT_H

#include "Outputter.h"
#include "FormattedTable.h"

// libMesh
#include "libmesh_common.h"

class TecplotOutput : public Outputter
{
public:
  TecplotOutput(EquationSystems & es, bool binary);
  virtual ~TecplotOutput();

  virtual void output(const std::string & file_base, Real time);
  virtual void outputPps(const std::string & file_base, const FormattedTable & table, Real time);

  virtual void meshChanged();
  virtual void sequence(bool state);

protected:
  int _file_num;                        ///< number of the file
  bool _binary;                         ///< true if outputing in binary format

  std::string getFileName(const std::string & file_base);
};

#endif /* TECPLOTOUTPUT_H */
