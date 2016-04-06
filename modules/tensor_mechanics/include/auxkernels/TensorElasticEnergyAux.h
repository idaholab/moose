/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef TENSORELASTICENERGYAUX_H
#define TENSORELASTICENERGYAUX_H

#include "AuxKernel.h"
#include "RankTwoTensor.h"

//Forward declarations
class TensorElasticEnergyAux;

template<>
InputParameters validParams<TensorElasticEnergyAux>();


class TensorElasticEnergyAux : public AuxKernel
{
public:
  TensorElasticEnergyAux(const InputParameters & parameters);
  virtual ~TensorElasticEnergyAux() {}

protected:
  virtual Real computeValue();

  std::string _base_name;

  const MaterialProperty<RankTwoTensor> & _stress;
  const MaterialProperty<RankTwoTensor> & _elastic_strain;
};

#endif // TENSORELASTICENERGYAUX_H
