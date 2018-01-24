//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
