#ifndef MTICSUM_H
#define MTICSUM_H

#include "InitialCondition.h"

class MTICSum;

template<>
InputParameters validParams<MTICSum>();

/**
 *
 */
class MTICSum : public InitialCondition
{
public:
  MTICSum(const std::string & name, InputParameters parameters);
  virtual ~MTICSum();

  virtual Real value(const Point & /*p*/);

protected:
  VariableValue & _var1;
  VariableValue & _var2;
};


#endif /* MTICSUM_H */
