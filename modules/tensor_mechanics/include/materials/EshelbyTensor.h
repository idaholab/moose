//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ESHELBYTENSOR_H
#define ESHELBYTENSOR_H

#include "Material.h"
#include "DerivativeMaterialInterface.h"

class RankTwoTensor;

class EshelbyTensor;

template <>
InputParameters validParams<EshelbyTensor>();

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
