#ifndef DECAYHEAT_H
#define DECAYHEAT_H

#include "CrowTools.h"

class DecayHeat;

template<>
InputParameters validParams<DecayHeat>();

class DecayHeat : public CrowTools
{
public:
  DecayHeat(const InputParameters & parameters);
  virtual ~DecayHeat();
  double compute(double time);

protected:
  int _equation_type;

};


#endif /* DECAYHEAT_H_ */
