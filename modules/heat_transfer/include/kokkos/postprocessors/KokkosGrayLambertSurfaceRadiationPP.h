//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosGeneralPostprocessor.h"
#include "KokkosGrayLambertSurfaceRadiationBase.h"

/**
 * Kokkos postprocessor extracting information from the KokkosGrayLambertSurfaceRadiationBase user
 * object
 */
class KokkosGrayLambertSurfaceRadiationPP : public Moose::Kokkos::GeneralPostprocessor
{
public:
  static InputParameters validParams();

  KokkosGrayLambertSurfaceRadiationPP(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void compute() override {}
  virtual void finalize() override;
  virtual PostprocessorValue getValue() const override { return _value; }

  /// Evaluate the requested quantity through the virtual user object hooks
  void extractValue();

private:
  /// Surface radiation user object
  const Moose::Kokkos::VirtualUserObject<KokkosGrayLambertSurfaceRadiationBase> _glsr_uo;
  /// Requested return type
  const MooseEnum _return_type;
  /// Boundary of interest
  const BoundaryID _bnd_id;
  /// Device buffer holding the requested quantity
  Moose::Kokkos::Array<Real> _buffer;
  /// The requested quantity
  Real _value;
};
