//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVMeshAdvection.h"
#include "INSFVMomentumResidualObject.h"

/**
 * Implements a momentum source/sink term proportional to the divergence of the mesh velocity
 */
class INSFVMomentumMeshAdvection : public INSFVMeshAdvection, public INSFVMomentumResidualObject
{
public:
  static InputParameters validParams();

  INSFVMomentumMeshAdvection(const InputParameters & parameters);
  virtual void gatherRCData(const Elem & elem) override;
  virtual void gatherRCData(const FaceInfo &) override {}

protected:
  /// Whether to add this object's contribution to the Rhie-Chow coefficients
  const bool _add_to_a;
};
