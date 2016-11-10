#ifndef TABLEFUNCTION_H
#define TABLEFUNCTION_H

#include "CrowTools.h"
#include "InterpolationFunctions.h"

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
  InterpolationFunctions _interpolation;         ///< Interpolation class
};


#endif /* TABLEFUNCTION_H_ */
