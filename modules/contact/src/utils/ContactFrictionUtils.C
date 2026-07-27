//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ContactFrictionUtils.h"

#include "MooseError.h"

#include "libmesh/elem.h"
#include "libmesh/fe_map.h"

#include "metaphysicl/raw_type.h"

#include <algorithm>
#include <limits>

namespace Moose
{
namespace Contact
{

MooseEnum
frictionCoefficientRegularizationOptions()
{
  MooseEnum options(getFrictionCoefficientRegularizationOptions(), "NONE");
  options.addDocumentation("NONE", "Use the supplied Coulomb friction coefficient.");
  options.addDocumentation(
      "ARCTAN_SLIP",
      "Scale the Coulomb friction coefficient by an arctangent function of the slip increment.");
  return options;
}

ContactTangentialFrame
buildContactTangentialFrame(const Elem & elem,
                            const Point & reference_point,
                            const RealVectorValue & preferred_normal)
{
  if (elem.dim() != 1 && elem.dim() != 2)
    mooseError("Elastic-slip contact frames require a one- or two-dimensional surface element.");

  std::array<RealVectorValue, 2> covariant_tangents;
  for (const auto direction : make_range(elem.dim()))
    covariant_tangents[direction] = FEMap::map_deriv(elem.dim(), &elem, direction, reference_point);

  const Real local_scale = std::max(elem.hmin(), std::numeric_limits<Real>::min());
  constexpr Real frame_tolerance = 128.0 * std::numeric_limits<Real>::epsilon();
  const auto normal_norm = preferred_normal.norm();
  if (normal_norm <= frame_tolerance)
    mooseError("Cannot construct an elastic-slip contact frame from a degenerate surface normal.");

  ContactTangentialFrame frame;
  frame[2] = preferred_normal / normal_norm;

  if (elem.dim() == 2)
  {
    const auto first_projected =
        covariant_tangents[0] - (covariant_tangents[0] * frame[2]) * frame[2];
    const auto second_projected =
        covariant_tangents[1] - (covariant_tangents[1] * frame[2]) * frame[2];
    // Preserve the first material coordinate direction so the frame cannot switch tangents during
    // deformation. Use the second direction only when the first is geometrically degenerate.
    frame[0] =
        first_projected.norm() > TOLERANCE * local_scale ? first_projected : second_projected;
  }
  else
    frame[0] = covariant_tangents[0] - (covariant_tangents[0] * frame[2]) * frame[2];

  const auto tangent_norm = frame[0].norm();
  if (tangent_norm <= TOLERANCE * local_scale)
    mooseError("Cannot construct an elastic-slip contact frame from lower-dimensional element ",
               elem.id(),
               " because its material tangent is degenerate.");
  frame[0] /= tangent_norm;
  frame[1] = elem.dim() == 2 ? frame[2].cross(frame[0]) : RealVectorValue();
  return frame;
}

ADReal
tangentialSlipMagnitude(const ADRealVectorValue & slip)
{
  const ADReal squared_norm = slip * slip;
  if (MetaPhysicL::raw_value(squared_norm) == 0.0)
    return 0.0;
  using std::sqrt;
  return sqrt(squared_norm);
}

ElasticSlipReturn
elasticSlipReturnMap(const ADRealVectorValue & trial_elastic_gap,
                     const ADReal & friction_coefficient,
                     const ADReal & contact_pressure,
                     const Real elastic_slip)
{
  mooseAssert(elastic_slip > 0.0, "The elastic-slip return map requires positive elastic slip");

  if (MetaPhysicL::raw_value(friction_coefficient) <= 0.0 ||
      MetaPhysicL::raw_value(contact_pressure) <= 0.0)
    return {ADRealVectorValue(), RealVectorValue()};

  const ADReal capacity = friction_coefficient * contact_pressure;
  const ADReal tangential_stiffness = capacity / elastic_slip;
  const ADRealVectorValue trial_multiplier = tangential_stiffness * trial_elastic_gap;
  const ADReal trial_norm = tangentialSlipMagnitude(trial_multiplier);

  if (MetaPhysicL::raw_value(trial_norm) <= MetaPhysicL::raw_value(capacity))
    return {trial_multiplier, MetaPhysicL::raw_value(trial_elastic_gap)};

  const ADRealVectorValue multiplier = capacity * trial_multiplier / trial_norm;
  const RealVectorValue returned_elastic_gap =
      elastic_slip * MetaPhysicL::raw_value(trial_multiplier / trial_norm);
  return {multiplier, returned_elastic_gap};
}

ADReal
arctanFrictionCoefficient(const ADReal & mu,
                          const ADReal & slip_increment,
                          const Real reference_slip)
{
  mooseAssert(reference_slip > 0.0,
              "Friction coefficient regularization requires a positive reference slip");
  using std::atan;
  return mu * (2.0 / libMesh::pi) * atan(slip_increment / reference_slip);
}

} // namespace Contact
} // namespace Moose
