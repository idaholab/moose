#ifndef TABLEFUNCTION_H
#define TABLEFUNCTION_H

#include "CrowTools.h"
#include "Interpolation_Functions.h"

class TableFunction;

template<>
InputParameters validParams<TableFunction>();


class TableFunction : public CrowTools
{
public:
  TableFunction(const InputParameters & parameters);
  virtual ~TableFunction();
  double compute(double time);

protected:
  Interpolation_Functions _interpolation;         ///< Interpolation class
};


#endif /* TABLEFUNCTION_H_ */
