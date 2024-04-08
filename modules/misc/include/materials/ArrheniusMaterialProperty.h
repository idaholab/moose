/*************************************************/
/*           DO NOT MODIFY THIS HEADER           */
/*                                               */
/*                     BISON                     */
/*                                               */
/*    (c) 2015 Battelle Energy Alliance, LLC     */
/*            ALL RIGHTS RESERVED                */
/*                                               */
/*   Prepared by Battelle Energy Alliance, LLC   */
/*     Under Contract No. DE-AC07-05ID14517      */
/*     With the U. S. Department of Energy       */
/*                                               */
/*     See COPYRIGHT for full restrictions       */
/*************************************************/

#pragma once

#include "Material.h"

template <bool is_ad>
class ArrheniusMaterialPropertyTempl : public Material
{
public:
  static InputParameters validParams();
  ArrheniusMaterialPropertyTempl(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;
  virtual void initQpStatefulProperties() override;

  /// Arrhenius material property
  GenericMaterialProperty<Real, is_ad> & _diffusivity;

  /// Derivative of Arrhenius material property with respect to temperature
  GenericMaterialProperty<Real, is_ad> & _diffusivity_dT;

private:
  /// Coupled temperature variable
  const GenericVariableValue<is_ad> & _temperature;

  /// Vector of preexponentials
  const std::vector<Real> _D_0;

  /// Vector of activation energies
  const std::vector<Real> _Q;

  /// Gas constant
  const Real _R;

  /// Number of _D_0 and _Q parameters
  const unsigned int _number;

  /// Initial temperature
  const Real _initial_temperature;

  using Material::_qp;
};

typedef ArrheniusMaterialPropertyTempl<false> ArrheniusMaterialProperty;
typedef ArrheniusMaterialPropertyTempl<true> ADArrheniusMaterialProperty;
