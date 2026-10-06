//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "MooseTypes.h"

#include <cmath>

namespace NS
{
namespace FV
{

/** Coefficients obtained by eliminating the two one-sided interface pressures on a jump face. */
struct PressureJumpInterfaceData
{
  Real elem_conductance = 0.0;
  Real neighbor_conductance = 0.0;
  Real elem_correction = 0.0;
  Real neighbor_correction = 0.0;
  Real transmissibility = 0.0;
  Real correction = 0.0;
  bool valid = false;
};

/**
 * Construct the conservative half-cell interface coefficients for a diagonal diffusion tensor.
 * Both half-cell displacement vectors point in the element-to-neighbor orientation.
 */
inline PressureJumpInterfaceData
pressureJumpInterfaceData(const RealVectorValue & normal,
                          const Point & elem_to_face,
                          const Point & face_to_neighbor,
                          const RealVectorValue & elem_diffusion,
                          const RealVectorValue & neighbor_diffusion,
                          const RealVectorValue & elem_gradient,
                          const RealVectorValue & neighbor_gradient,
                          const Real face_area,
                          const bool use_nonorthogonal_correction)
{
  PressureJumpInterfaceData data;

  const auto side_data = [&](const Point & half_cell,
                             const RealVectorValue & diffusion,
                             const RealVectorValue & gradient,
                             Real & conductance,
                             Real & correction)
  {
    RealVectorValue diffusion_normal;
    for (const auto i : make_range(Moose::dim))
      diffusion_normal(i) = diffusion(i) * normal(i);

    const Real normal_diffusion = normal * diffusion_normal;
    const Real normal_distance = half_cell * normal;
    if (!std::isfinite(normal_distance) || normal_distance <= 0.0 ||
        !std::isfinite(normal_diffusion) || normal_diffusion <= 0.0 || face_area <= 0.0)
      return false;

    conductance = face_area * normal_diffusion / normal_distance;

    RealVectorValue correction_vector = diffusion_normal - normal_diffusion * normal;
    if (use_nonorthogonal_correction)
      correction_vector += normal_diffusion * (normal - half_cell / normal_distance);

    correction = face_area * correction_vector * gradient;
    return std::isfinite(conductance) && conductance > 0.0 && std::isfinite(correction);
  };

  if (!side_data(elem_to_face,
                 elem_diffusion,
                 elem_gradient,
                 data.elem_conductance,
                 data.elem_correction) ||
      !side_data(face_to_neighbor,
                 neighbor_diffusion,
                 neighbor_gradient,
                 data.neighbor_conductance,
                 data.neighbor_correction))
    return data;

  data.transmissibility = 1.0 / (1.0 / data.elem_conductance + 1.0 / data.neighbor_conductance);
  data.correction = data.transmissibility * (data.elem_correction / data.elem_conductance +
                                             data.neighbor_correction / data.neighbor_conductance);
  data.valid = std::isfinite(data.transmissibility) && data.transmissibility > 0.0 &&
               std::isfinite(data.correction);
  return data;
}

/** Compute the single element-to-neighbor flux after eliminating the interface pressures. */
inline Real
pressureJumpFlux(const PressureJumpInterfaceData & data,
                 const Real elem_pressure,
                 const Real neighbor_pressure,
                 const Real elem_jump)
{
  return data.transmissibility * (elem_pressure - neighbor_pressure - elem_jump) - data.correction;
}
}
}
