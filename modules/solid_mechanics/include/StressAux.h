#ifndef STRESSAUX_H
#define STRESSAUX_H

#include "AuxKernel.h"

class StressAux;

template<>
InputParameters validParams<StressAux>();


class StressAux : public AuxKernel
{
public:
  StressAux( const std::string & name, MooseSystem & moose_system, InputParameters parameters );

  virtual ~StressAux() {}

protected:
  virtual Real computeValue();

  const unsigned _index;
//   MaterialProperty<RealTensorValue> & _stress;

};

#endif // STRESSAUX_H
