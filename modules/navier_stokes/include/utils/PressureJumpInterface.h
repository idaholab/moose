//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "MooseTypes.h"
#include "libmesh/int_range.h"

#include <cmath>

namespace NS
{
namespace FV
{

/** Complete face operator obtained by eliminating the two one-sided interface pressures. */
struct PressureJumpInterfaceData
{
  bool is_baffle = false;
  Real elem_jump = 0.0;
  Real elem_conductance = 0.0;
  Real neighbor_conductance = 0.0;
  Real elem_correction = 0.0;
  Real neighbor_correction = 0.0;
  /// One-sided H/A mass fluxes A rho_s (H/A)_s . n, oriented along the face normal
  Real elem_hbya_flux = 0.0;
  Real neighbor_hbya_flux = 0.0;
  Real transmissibility = 0.0;
  Real correction = 0.0;
  Real elem_normal_distance = 0.0;
  Real neighbor_normal_distance = 0.0;
  Real elem_normal_diffusion = 0.0;
  Real neighbor_normal_diffusion = 0.0;
  Real face_area = 0.0;
  bool valid = false;
};

/**
 * Construct the conservative half-cell interface coefficients for a diagonal diffusion tensor.
 * Both half-cell direction vectors point in the element-to-neighbor orientation.
 */
inline PressureJumpInterfaceData
pressureJumpInterfaceData(const RealVectorValue & normal,
                          const Point & elem_to_face,
                          const Point & face_to_neighbor,
                          const RealVectorValue & elem_diffusion,
                          const RealVectorValue & neighbor_diffusion,
                          const RealVectorValue & elem_gradient,
                          const RealVectorValue & neighbor_gradient,
                          const RealVectorValue & elem_rho_hbya,
                          const RealVectorValue & neighbor_rho_hbya,
                          const Real face_area,
                          const bool use_nonorthogonal_correction,
                          const Real elem_jump)
{
  PressureJumpInterfaceData data;
  data.is_baffle = true;
  data.elem_jump = elem_jump;
  data.face_area = face_area;

  const auto side_data = [&](const Point & half_cell,
                             const RealVectorValue & diffusion,
                             const RealVectorValue & gradient,
                             Real & conductance,
                             Real & correction,
                             Real & normal_distance,
                             Real & normal_diffusion)
  {
    RealVectorValue diffusion_normal;
    for (const auto i : make_range(Moose::dim))
      diffusion_normal(i) = diffusion(i) * normal(i);

    normal_diffusion = normal * diffusion_normal;
    normal_distance = half_cell * normal;
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

  const bool elem_valid = side_data(elem_to_face,
                                    elem_diffusion,
                                    elem_gradient,
                                    data.elem_conductance,
                                    data.elem_correction,
                                    data.elem_normal_distance,
                                    data.elem_normal_diffusion);
  const bool neighbor_valid = side_data(face_to_neighbor,
                                        neighbor_diffusion,
                                        neighbor_gradient,
                                        data.neighbor_conductance,
                                        data.neighbor_correction,
                                        data.neighbor_normal_distance,
                                        data.neighbor_normal_diffusion);
  if (!elem_valid || !neighbor_valid)
    return data;

  data.elem_hbya_flux = face_area * (elem_rho_hbya * normal);
  data.neighbor_hbya_flux = face_area * (neighbor_rho_hbya * normal);
  data.transmissibility = 1.0 / (1.0 / data.elem_conductance + 1.0 / data.neighbor_conductance);
  data.correction = data.transmissibility * (data.elem_correction / data.elem_conductance +
                                             data.neighbor_correction / data.neighbor_conductance);
  data.valid = std::isfinite(data.transmissibility) && data.transmissibility > 0.0 &&
               std::isfinite(data.correction) && std::isfinite(data.elem_hbya_flux) &&
               std::isfinite(data.neighbor_hbya_flux);
  return data;
}

/**
 * Compute the H/A part of the face mass flux after eliminating the interface pressures. Each side
 * carries mdot = -h_s + (pressure flux of side s), so continuity of mdot weights the side H/A
 * fluxes h_s with the same series weights T / T_s as the pressure.
 */
inline Real
pressureJumpHbyAFlux(const PressureJumpInterfaceData & data)
{
  return data.transmissibility * (data.elem_hbya_flux / data.elem_conductance +
                                  data.neighbor_hbya_flux / data.neighbor_conductance);
}

/** Compute the pressure part of the element-to-neighbor flux after eliminating the interface
 * pressures. The face mass flux is this minus pressureJumpHbyAFlux. */
inline Real
pressureJumpFlux(const PressureJumpInterfaceData & data,
                 const Real elem_pressure,
                 const Real neighbor_pressure)
{
  return data.transmissibility * (elem_pressure - neighbor_pressure - data.elem_jump) -
         data.correction;
}

/** Recover one side's face pressure from the common oriented face mass flux. */
inline Real
pressureJumpOneSidedFaceValue(const PressureJumpInterfaceData & data,
                              const Real cell_pressure,
                              const Real mass_flux,
                              const bool elem_side)
{
  if (elem_side)
    return cell_pressure -
           (mass_flux + data.elem_hbya_flux + data.elem_correction) / data.elem_conductance;

  return cell_pressure + (mass_flux + data.neighbor_hbya_flux + data.neighbor_correction) /
                             data.neighbor_conductance;
}
}
}
