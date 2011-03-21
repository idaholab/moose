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

class Problem;
class Outputter;

class Output
{
public:
  Output(Problem & problem);
  virtual ~Output();

  void addExodus();

  void output();
  // FIXME: right now, it is here - might go somewhere else?
  void outputPps(const FormattedTable & table);

  void fileBase(const std::string & file_base) { _file_base = file_base; }
  std::string & fileBase() { return _file_base; }

  void interval(int interval) { _interval = interval; }
  int interval() { return _interval; }

  void meshChanged();
  void sequence(bool state);

protected:
  std::string _file_base;

  Problem & _problem;
  Real & _time;
  int _interval;

  std::vector<Outputter *> _outputters;
};

#endif /* OUTPUTTER_H_ */
