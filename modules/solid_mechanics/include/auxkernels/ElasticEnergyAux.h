#ifndef ELASTICENERGYAUX_H
#define ELASTICENERGYAUX_H

#include "AuxKernel.h"

class ElasticEnergyAux;
class SymmTensor;

template<>
InputParameters validParams<ElasticEnergyAux>();


class ElasticEnergyAux : public AuxKernel
{
public:
  ElasticEnergyAux( const std::string & name, InputParameters parameters );

  virtual ~ElasticEnergyAux() {}

protected:
  virtual Real computeValue();

  MaterialProperty<SymmTensor> & _stress;
  MaterialProperty<SymmTensor> & _elastic_strain;

};

#endif // ELASTICENERGYAUX_H
