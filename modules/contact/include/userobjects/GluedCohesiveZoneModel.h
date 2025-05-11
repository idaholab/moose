//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CohesiveZoneModelBase.h"
#include "TwoVector.h"

/**
 * User object that interface pressure resulting from a simple traction separation law.
 */
class GluedCohesiveZoneModel : virtual public PenaltyWeightedGapUserObject,
                               virtual public WeightedVelocitiesUserObject,
                               virtual public CohesiveZoneModelBase
{
public:
  static InputParameters validParams();

  GluedCohesiveZoneModel(const InputParameters & parameters);

  // Getters for analysis output
  Real getLocalDisplacementNormal(const Node * const node) const;
  Real getLocalDisplacementTangential(const Node * const node) const;

protected:
  virtual bool constrainedByOwner() const override { return false; }

  /// Encapsulate the CZM constitutive behavior.
  virtual void computeCZMTraction(const Node * const node) override;

  // Penalty stiffness for bilinear traction model
  const Real _penalty_stiffness_czm;
};
