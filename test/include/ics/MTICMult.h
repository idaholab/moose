#ifndef MTICMULT_H
#define MTICMULT_H

#include "InitialCondition.h"

class MTICMult;

template<>
InputParameters validParams<MTICMult>();

/**
 *
 */
class MTICMult : public InitialCondition
{
public:
  MTICMult(const std::string & name, InputParameters parameters);
  virtual ~MTICMult();

  virtual Real value(const Point & /*p*/);

protected:
  VariableValue & _var1;
  Real _factor;
};


#endif /* MTICMULT_H */
