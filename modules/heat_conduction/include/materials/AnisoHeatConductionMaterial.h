//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DerivativeMaterialInterface.h"
#include "Material.h"

class Function;

/**
 * Calculates thermal conductivity and specific heat of the material
 */
template <bool is_ad>
class AnisoHeatConductionMaterialTempl : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  AnisoHeatConductionMaterialTempl(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  const unsigned int _dim;

  const Real _ref_temp;
  const bool _has_temp;
  const GenericVariableValue<is_ad> & _T;
  const unsigned int _T_var;
  const VariableName _T_name;

  const std::string _base_name;

  const RankTwoTensor _user_provided_thermal_conductivity;
  GenericMaterialProperty<RankTwoTensor, is_ad> & _thermal_conductivity;
  MaterialProperty<RankTwoTensor> & _dthermal_conductivity_dT;
  const Function * const _thermal_conductivity_temperature_coefficient_function;

  GenericMaterialProperty<Real, is_ad> & _specific_heat;
  MaterialProperty<Real> & _dspecific_heat_dT;
  const Function * const _specific_heat_function;

  const MooseArray<ADPoint> * _ad_q_point;

private:
  auto genericQPoints();
};

typedef AnisoHeatConductionMaterialTempl<false> AnisoHeatConductionMaterial;
typedef AnisoHeatConductionMaterialTempl<true> ADAnisoHeatConductionMaterial;
