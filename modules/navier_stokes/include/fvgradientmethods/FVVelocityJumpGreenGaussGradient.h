//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "FVGreenGaussGradient.h"

class PorousRhieChowMassFlux;

/**
 * Green-Gauss velocity gradient that uses the one-sided baffle face states of a porous Rhie-Chow
 * object instead of interpolating velocity across pressure baffles.
 */
class FVVelocityJumpGreenGaussGradient : public FVGreenGaussGradient
{
public:
  static InputParameters validParams();
  FVVelocityJumpGreenGaussGradient(const InputParameters & params);

  /// Link this method to the porous Rhie-Chow object that supplies the baffle face states.
  void linkFlowSystem(const PorousRhieChowMassFlux & rc);

private:
  InternalFaceValues internalFaceValues(const FaceInfo & fi,
                                        const ElemInfo & elem_info,
                                        const ElemInfo & neighbor_info,
                                        const MooseLinearVariableFV<Real> & variable,
                                        Real elem_value,
                                        Real neighbor_value) const override;

  /// Porous Rhie-Chow object supplying baffle locations and face states after linkage.
  const PorousRhieChowMassFlux * _rhie_chow = nullptr;
};
