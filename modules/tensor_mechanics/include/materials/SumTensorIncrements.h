/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SUMTENSORINCREMENTS_H
#define SUMTENSORINCREMENTS_H

#include "Material.h"
#include "RankTwoTensor.h"
#include "DerivativeMaterialInterface.h"

class SumStraubIncrements;

/**
 * SumTensorIncrements update a tensor by summing tensor increments passed as property
 */
class SumTensorIncrements : public DerivativeMaterialInterface<Material>
{
public:
  SumTensorIncrements(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

  std::vector<MaterialPropertyName> _property_names;
  unsigned int _num_property;

  MaterialProperty<RankTwoTensor> & _tensor;
  MaterialProperty<RankTwoTensor> & _tensor_old;
  MaterialProperty<RankTwoTensor> & _tensor_increment;

  std::vector<const MaterialProperty<RankTwoTensor> *> _coupled_tensor_increments;
};

#endif
