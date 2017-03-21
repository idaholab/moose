/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ELASTICMODEL_H
#define ELASTICMODEL_H

#include "ConstitutiveModel.h"

class ElasticModel : public ConstitutiveModel
{
public:
  ElasticModel(const InputParameters & parameters);
  virtual ~ElasticModel();

protected:
  /// Compute the stress (sigma += deltaSigma)
  virtual void computeStress(const Elem & current_elem,
                             unsigned qp,
                             const SymmElasticityTensor & elasticity_tensor,
                             const SymmTensor & stress_old,
                             SymmTensor & strain_increment,
                             SymmTensor & stress_new);
};

template <>
InputParameters validParams<ElasticModel>();

#endif
