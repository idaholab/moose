//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RadialReturnCreepStressUpdateBase.h"
#include "libmesh/quadrature_gauss.h"
#include "libmesh/string_to_enum.h"

template <bool is_ad>
InputParameters
RadialReturnCreepStressUpdateBaseTempl<is_ad>::validParams()
{
  InputParameters params = RadialReturnStressUpdateTempl<is_ad>::validParams();
  params.set<std::string>("effective_inelastic_strain_name") = "effective_creep_strain";
  params.addParam<std::string>(
      "serd_integration_order",
      "FIFTH",
      "Gaussian quadrature order for computing the strain energy rate density");
  return params;
}

template <bool is_ad>
RadialReturnCreepStressUpdateBaseTempl<is_ad>::RadialReturnCreepStressUpdateBaseTempl(
    const InputParameters & parameters)
  : RadialReturnStressUpdateTempl<is_ad>(parameters),
    _creep_strain(this->template declareGenericProperty<RankTwoTensor, is_ad>(this->_base_name +
                                                                              "creep_strain")),
    _creep_strain_old(
        this->template getMaterialPropertyOld<RankTwoTensor>(this->_base_name + "creep_strain")),
    _serd_integration_order(this->template getParam<std::string>("serd_integration_order"))
{
}

template <bool is_ad>
void
RadialReturnCreepStressUpdateBaseTempl<is_ad>::initQpStatefulProperties()
{
  _creep_strain[_qp].zero();

  RadialReturnStressUpdateTempl<is_ad>::initQpStatefulProperties();
}

template <bool is_ad>
void
RadialReturnCreepStressUpdateBaseTempl<is_ad>::propagateQpStatefulProperties()
{
  _creep_strain[_qp] = _creep_strain_old[_qp];

  propagateQpStatefulPropertiesRadialReturn();
}

template <bool is_ad>
Real
RadialReturnCreepStressUpdateBaseTempl<is_ad>::computeStressDerivative(
    const Real /*effective_trial_stress*/, const Real /*scalar*/)
{
  mooseError("computeStressDerivative called: no stress derivative computation is needed for AD");
}

template <>
Real
RadialReturnCreepStressUpdateBaseTempl<false>::computeStressDerivative(
    const Real effective_trial_stress, const Real scalar)
{
  return -(computeDerivative(effective_trial_stress, scalar) + 1.0) / this->_three_shear_modulus;
}

template <bool is_ad>
void
RadialReturnCreepStressUpdateBaseTempl<is_ad>::computeStressFinalize(
    const GenericRankTwoTensor<is_ad> & plastic_strain_increment)
{
  _creep_strain[_qp] = _creep_strain_old[_qp] + plastic_strain_increment;
}

template <bool is_ad>
GenericReal<is_ad>
RadialReturnCreepStressUpdateBaseTempl<is_ad>::computeCreepStrainRate(
    const GenericReal<is_ad> & /*stress_eq*/)
{
  mooseError("Derived creep models must implement computeCreepStrainRate");
}

template <bool is_ad>
Real
RadialReturnCreepStressUpdateBaseTempl<is_ad>::computeStrainEnergyRateDensity(
    const GenericMaterialProperty<RankTwoTensor, is_ad> & stress,
    const GenericMaterialProperty<RankTwoTensor, is_ad> & strain_rate)
{
  using std::sqrt;

  const QGauss qrule(1, Utility::string_to_enum<Order>(_serd_integration_order));
  const auto & weights = qrule.get_weights();
  const auto & points = qrule.get_points();

  const GenericReal<is_ad> effective_stress = sqrt(3.0 * stress[_qp].secondInvariant());
  const GenericReal<is_ad> effective_strain_rate =
      sqrt(2.0 / 3.0 * strain_rate[_qp].doubleContraction(strain_rate[_qp]));

  Real serd = MetaPhysicL::raw_value(effective_stress * effective_strain_rate);
  for (const auto i : index_range(points))
  {
    const GenericReal<is_ad> integration_stress = 0.5 * (points[i](0) + 1.0) * effective_stress;
    serd -= 0.5 * weights[i] *
            MetaPhysicL::raw_value(effective_stress * computeCreepStrainRate(integration_stress));
  }

  return serd;
}

template class RadialReturnCreepStressUpdateBaseTempl<false>;
template class RadialReturnCreepStressUpdateBaseTempl<true>;
