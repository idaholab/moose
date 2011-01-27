#ifndef CREEPSTRAINAUX_H
#define CREEPSTRAINAUX_H

#include "AuxKernel.h"

class CreepStrainAux;

template<>
InputParameters validParams<CreepStrainAux>();


class CreepStrainAux : public AuxKernel
{
public:
  CreepStrainAux( const std::string & name, InputParameters parameters );

  virtual ~CreepStrainAux() {}

protected:
  virtual Real computeValue();

  const unsigned _index;
  MaterialProperty<RealTensorValue> & _creep_strain;

};

#endif // CREEPSTRAINAUX_H
