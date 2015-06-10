/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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

  const MaterialProperty<SymmTensor> & _stress;
  const MaterialProperty<SymmTensor> & _elastic_strain;
};

#endif // ELASTICENERGYAUX_H
