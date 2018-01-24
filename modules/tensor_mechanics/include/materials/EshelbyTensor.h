/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ESHELBYTENSOR_H
#define ESHELBYTENSOR_H

#include "Material.h"
#include "DerivativeMaterialInterface.h"

class RankTwoTensor;

/**
 * EshelbyTensor defines a strain increment and rotation increment, for finite strains.
 */
class EshelbyTensor : public DerivativeMaterialInterface<Material>
{
public:
  EshelbyTensor(const InputParameters & parameters);

  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

protected:
  std::string _base_name;

  MaterialProperty<Real> & _sed;
  const MaterialProperty<Real> & _sed_old;
  MaterialProperty<RankTwoTensor> & _eshelby_tensor;
  const MaterialProperty<RankTwoTensor> & _stress;
  const MaterialProperty<RankTwoTensor> & _stress_old;
  const MaterialProperty<RankTwoTensor> * _strain_increment;
  const MaterialProperty<RankTwoTensor> * _mechanical_strain;
  std::vector<const VariableGradient *> _grad_disp;

  MaterialProperty<RealVectorValue> & _J_thermal_term_vec;
  const bool _has_temp;
  const VariableGradient & _grad_temp;
  const MaterialProperty<RankTwoTensor> * _total_deigenstrain_dT;
};

#endif // ESHELBYTENSOR_H
