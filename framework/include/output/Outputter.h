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
#include "MooseTypes.h"
#include "FormattedTable.h"
#include "Restartable.h"

// libMesh
#include "libmesh/equation_systems.h"

#include <vector>
#include <string>

class Problem;

class Outputter : public Restartable
{
public:
  /**
   * Constructor
   *
   * @param es The EquationSystems object to output
   * @param sub_problem The sub_problem this Outputter is associated with
   * @param name The name of the Outputter (like Exodus, VTK, etc.)
   */
  Outputter(EquationSystems & es, SubProblem & sub_problem, std::string name);
  virtual ~Outputter();

  /**
   * Outputs the data
   */
  virtual void output(const std::string & file_base, Real time, unsigned int t_step) = 0;
  virtual void outputPps(const std::string & file_base, const FormattedTable & table, Real time) = 0;
  /// Returns a Boolean indicating whether this Outputter supports PPS values
  virtual bool supportsPpsOutput() { return false; }

  virtual void outputInput() {}

  virtual void meshChanged() = 0;
  virtual void sequence(bool state) = 0;

  void setOutputVariables(std::vector<VariableName> output_variables);

  /// Set (or reset) the output position
  virtual void setOutputPosition(const Point & /*p*/) {}

  /**
   * Set the append flag on this Outputter.  Derived classes may or
   * may not make use of this flag when they are created.
   */
  virtual void setAppend(bool b);

protected:
  EquationSystems & _es;

  /// The variables to be output
  std::vector<std::string> _output_variables;

  /// The flag which gets set by the setAppend() function
  bool _append;
};

#endif /* OUTPUTTER_H */
