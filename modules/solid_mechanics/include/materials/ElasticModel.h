//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ELASTICMODEL_H
#define ELASTICMODEL_H

#include "ConstitutiveModel.h"

class ElasticModel;

template <>
InputParameters validParams<ElasticModel>();

class ElasticModel : public ConstitutiveModel
{
public:
  ElasticModel(const InputParameters & parameters);
  virtual ~ElasticModel();

protected:
  /// Compute the stress (sigma += deltaSigma)
  virtual void computeStress(const Elem & current_elem,
                             const SymmElasticityTensor & elasticity_tensor,
                             const SymmTensor & stress_old,
                             SymmTensor & strain_increment,
                             SymmTensor & stress_new);
};

#endif
