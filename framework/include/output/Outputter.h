#ifndef OUTPUTTER_H
#define OUTPUTTER_H

#include <string>
#include "FormattedTable.h"
// libMesh
#include "equation_systems.h"

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

  virtual void meshChanged() = 0;
  virtual void sequence(bool state) = 0;

protected:
  EquationSystems & _es;
};

#endif /* OUTPUTTER_H */
