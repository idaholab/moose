/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef GENERALIZEDMAXWELLMODEL_H
#define GENERALIZEDMAXWELLMODEL_H

#include "GeneralizedMaxwellBase.h"

class GeneralizedMaxwellModel;

template <>
InputParameters validParams<GeneralizedMaxwellModel>();

/**
 * Represents a generalized Maxwell model with constant and isotropic
 * mechanical properties.
 */
class GeneralizedMaxwellModel : public GeneralizedMaxwellBase
{
public:
  GeneralizedMaxwellModel(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpViscoelasticProperties();

  RankFourTensor _C0;
  std::vector<RankFourTensor> _Ci;
  std::vector<Real> _eta_i;
};

#endif // GENERALIZEDMAXWELLMODEL_H
