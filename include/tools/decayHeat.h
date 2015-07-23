#ifndef DECAYHEAT_H
#define DECAYHEAT_H

#include "CrowTools.h"

class decayHeat;

template<>
InputParameters validParams<decayHeat>();

class decayHeat : public CrowTools
{
public:
  decayHeat(const InputParameters & parameters);
  virtual ~decayHeat();
  double compute(double time);

protected:
  int _equation_type;

};


#endif /* DECAYHEAT_H_ */
