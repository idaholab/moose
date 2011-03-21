#ifndef OUTPUTTER_H_
#define OUTPUTTER_H_

#include <string>
#include "FormattedTable.h"
// libMesh
#include "equation_systems.h"

namespace Moose {

class Problem;

class Outputter {
public:
  Outputter(EquationSystems & es);
  virtual ~Outputter();

  /**
   * Outputs the data
   */
  virtual void output(const std::string & file_base, Real time) = 0;
  virtual void outputPps(const std::string & file_base, const FormattedTable & table, Real time) = 0;

protected:
  EquationSystems & _es;
};

} // namespace

#endif /* OUTPUTTER_H_ */
