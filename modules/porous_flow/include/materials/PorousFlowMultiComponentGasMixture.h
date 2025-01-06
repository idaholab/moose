//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowMultiComponentFluidBase.h"

class SinglePhaseFluidProperties;
template <bool is_ad>
class PorousFlowMultiComponentGasMixtureTempl : public PorousFlowMultiComponentFluidBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowMultiComponentGasMixtureTempl(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /**
   * Compute mixture properties (density, viscosity, enthalpy etc)
   * @param p Fluid pressure
   * @param T Fluid temperature
   * @param X Mass fraction of each gas component (-)
   * @return mole fractions (-)
   */
  virtual ADReal mixtureDensity(const ADReal & p, const ADReal & T, const std::vector<ADReal> & x);
  virtual ADReal
  mixtureViscosity(const ADReal & p, const ADReal & T, const std::vector<ADReal> & x);
  virtual ADReal mixtureEnthalpy(const ADReal & p, const ADReal & T, const std::vector<ADReal> & x);
  virtual ADReal
  mixtureInternalEnergy(const ADReal & p, const ADReal & T, const std::vector<ADReal> & x);

  /**
   * Convert mixture mass fractions to mole fractions
   * @param X Mass fraction of each gas component (-)
   * @return mole fractions (-)
   */
  std::vector<ADReal> massFractionsToMoleFractions(std::vector<ADReal> & X);

  /// Fluid properties UserObject
  std::vector<const SinglePhaseFluidProperties *> _fp;

  /// fluid name vector
  const std::vector<UserObjectName> _fp_names;

  /// number of fluid component
  const unsigned int _n_components;

  /// number of mass fraction variable
  const unsigned int _X_components;

  /// mass fraction variables
  std::vector<const GenericVariableValue<is_ad> *> _X;

  /// Molar masses of each gas component
  std::vector<Real> _M;

  usingPorousFlowFluidPropertiesMembers;
  using PorousFlowMultiComponentFluidBaseTempl<is_ad>::_ddensity_dX;
  using PorousFlowMultiComponentFluidBaseTempl<is_ad>::_dviscosity_dX;
  using PorousFlowMultiComponentFluidBaseTempl<is_ad>::_dinternal_energy_dX;
  using PorousFlowMultiComponentFluidBaseTempl<is_ad>::_denthalpy_dX;
};

typedef PorousFlowMultiComponentGasMixtureTempl<false> PorousFlowMultiComponentGasMixture;
typedef PorousFlowMultiComponentGasMixtureTempl<true> ADPorousFlowMultiComponentGasMixture;
