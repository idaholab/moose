//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RadialReturnCreepStressUpdateBase.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/quadrature_gauss.h"

template <bool is_ad>
InputParameters
RadialReturnCreepStressUpdateBaseTempl<is_ad>::validParams()
{
  InputParameters params = RadialReturnStressUpdateTempl<is_ad>::validParams();
  params.set<std::string>("effective_inelastic_strain_name") = "effective_creep_strain";
  params.addParam<std::string>(
      "serd_integration_order",
      "FIFTH",
      "numerical integration order for computing strain energy rate density");
  return params;
}

template <bool is_ad>
RadialReturnCreepStressUpdateBaseTempl<is_ad>::RadialReturnCreepStressUpdateBaseTempl(
    const InputParameters & parameters)
  : RadialReturnStressUpdateTempl<is_ad>(parameters),
    _creep_strain(this->template declareGenericProperty<RankTwoTensor, is_ad>(this->_base_name +
                                                                              "creep_strain")),
    _creep_strain_old(
        this->template getMaterialPropertyOld<RankTwoTensor>(this->_base_name + "creep_strain"))
{
  _serd_integration_order = parameters.get<std::string>("serd_integration_order");
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
  mooseError("This is a base class. Developers need to write their own creep law");
}

template <bool is_ad>
Real
RadialReturnCreepStressUpdateBaseTempl<is_ad>::computeStrainEnergyRateDensity(
    const GenericMaterialProperty<RankTwoTensor, is_ad> & stress,
    const GenericMaterialProperty<RankTwoTensor, is_ad> & strain_rate)
{
  // Create a Gauss quadrature rule of the specified order
  std::unique_ptr<QGauss> qrule =
      std::make_unique<QGauss>(1, Utility::string_to_enum<Order>(_serd_integration_order));

  // Get the weights and points
  std::vector<Real> weights = qrule->get_weights();
  std::vector<Point> points = qrule->get_points();

  const GenericReal<is_ad> sigma_eq = std::sqrt(3.0 * stress[_qp].secondInvariant());
  const GenericReal<is_ad> eps_eq =
      std::sqrt(2.0 / 3.0 * strain_rate[_qp].doubleContraction(strain_rate[_qp]));

  Real integral = MetaPhysicL::raw_value(sigma_eq * eps_eq);
  // Perform the integral using Gaussian quadrature
  for (unsigned int k = 0; k < points.size(); ++k)
  {
    // Transform Gaussian points to the interval [0, sigma]
    GenericReal<is_ad> sigma_eq_tmp = 0.5 * (points[k](0) + 1) * sigma_eq; // Map to [0, sigma_eq]
    GenericReal<is_ad> strain_rate_tmp = computeCreepStrainRate(sigma_eq_tmp);
    integral -= 0.5 * weights[k] * MetaPhysicL::raw_value(sigma_eq * strain_rate_tmp);
  }
  return integral;
}

template class RadialReturnCreepStressUpdateBaseTempl<false>;
template class RadialReturnCreepStressUpdateBaseTempl<true>;
