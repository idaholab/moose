//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "FVAdvectedVanLeer.h"

#include <limits>

registerMooseObject("MooseApp", FVAdvectedVanLeer);

InputParameters
FVAdvectedVanLeer::validParams()
{
  InputParameters params = FVInterpolationMethod::validParams();
  params.addClassDescription(
      "TVD Van Leer interpolation for advected quantities using cell gradients.");
  params.addParam<bool>("use_deferred_correction",
                        false,
                        "Lag limiter gradient contributions and treat the high-order correction "
                        "explicitly (deferred correction).");
  return params;
}

FVAdvectedVanLeer::FVAdvectedVanLeer(const InputParameters & params)
  : FVInterpolationMethod(params), _deferred_correction(getParam<bool>("use_deferred_correction"))
{
  setAdvectedFaceInterpolator(buildAdvectedFaceInterpolator<FVAdvectedVanLeer>(true));
}

FVInterpolationMethod::AdvectedInterpolationResult
FVAdvectedVanLeer::advectedInterpolate(const FaceInfo & face,
                                       Real elem_value,
                                       Real neighbor_value,
                                       const VectorValue<Real> * elem_grad,
                                       const VectorValue<Real> * neighbor_grad,
                                       Real mass_flux) const
{
  mooseAssert(elem_grad && neighbor_grad,
              "Van Leer advected interpolation requires both element and neighbor gradients.");

  const bool elem_is_upwind = mass_flux >= 0.0;
  const auto limiter = Moose::FV::Limiter<Real>::build(Moose::FV::LimiterType::VanLeer);

  // Upwind/downwind selection
  const Real phi_upwind = elem_is_upwind ? elem_value : neighbor_value;
  const Real phi_downwind = elem_is_upwind ? neighbor_value : elem_value;

  const auto grad_upwind = elem_is_upwind ? elem_grad : neighbor_grad;
  const auto grad_downwind = elem_is_upwind ? neighbor_grad : elem_grad;

  const auto beta = limiter->limit(phi_upwind,
                                   phi_downwind,
                                   grad_upwind,
                                   grad_downwind,
                                   elem_is_upwind ? face.dCN() : Point(-face.dCN()),
                                   /*max_value*/ std::numeric_limits<Real>::max(),
                                   /*min_value*/ std::numeric_limits<Real>::lowest(),
                                   &face,
                                   elem_is_upwind);

  const Real w_f = elem_is_upwind ? face.gC() : (1.0 - face.gC());
  const Real g = beta * (1.0 - w_f);

  AdvectedInterpolationResult result;
  if (elem_is_upwind)
    result.weights_high = std::make_pair(1.0 - g, g);
  else
    result.weights_high = std::make_pair(g, 1.0 - g);

  if (_deferred_correction)
  {
    const Real abs_flux = std::abs(mass_flux) + std::numeric_limits<Real>::epsilon();
    const Real w_upwind = std::min(std::max(0.5 * (1.0 + mass_flux / abs_flux), 0.0), 1.0);
    result.weights_matrix = std::make_pair(w_upwind, 1.0 - w_upwind);
    result.has_correction = true;
  }
  else
  {
    result.weights_matrix = result.weights_high;
    result.has_correction = false;
  }

  return result;
}
