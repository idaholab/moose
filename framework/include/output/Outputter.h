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

#ifndef OUTPUTTER_H
#define OUTPUTTER_H

#include "Moose.h"
#include "FormattedTable.h"

// libMesh
#include "libmesh/equation_systems.h"

#include <vector>
#include <string>

class Problem;

class Outputter
{
public:
  Outputter(EquationSystems & es);
  virtual ~Outputter();

  /**
   * Outputs the data
   */
  virtual void output(const std::string & file_base, Real time) = 0;
  virtual void outputPps(const std::string & file_base, const FormattedTable & table, Real time) = 0;
  /// Returns a Boolean indicating whether this Outputter supports PPS values
  virtual bool supportsPpsOutput() { return false; }

  virtual void outputInput() {}

  virtual void meshChanged() = 0;
  virtual void sequence(bool state) = 0;

  void setOutputVariables(std::vector<std::string> output_variables) { _output_variables = output_variables; }

protected:
  EquationSystems & _es;

  /// The variables to be output
  std::vector<std::string> _output_variables;
};

#endif /* OUTPUTTER_H */
