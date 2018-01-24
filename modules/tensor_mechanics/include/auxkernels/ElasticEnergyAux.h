/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ELASTICENERGYAUX_H
#define ELASTICENERGYAUX_H

#include "AuxKernel.h"
#include "RankTwoTensor.h"

// Forward declarations
class ElasticEnergyAux;

template <>
InputParameters validParams<ElasticEnergyAux>();

class ElasticEnergyAux : public AuxKernel
{
public:
  ElasticEnergyAux(const InputParameters & parameters);
  virtual ~ElasticEnergyAux() {}

protected:
  virtual Real computeValue();

  std::string _base_name;

  const MaterialProperty<RankTwoTensor> & _stress;
  const MaterialProperty<RankTwoTensor> & _elastic_strain;
};

#endif // ELASTICENERGYAUX_H
