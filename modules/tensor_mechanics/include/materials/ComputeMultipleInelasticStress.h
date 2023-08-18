//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeMultipleInelasticStressBase.h"

/**
 * ComputeMultipleInelasticStress computes the stress, the consistent tangent
 * operator (or an approximation to it), and a decomposition of the strain
 * into elastic and inelastic parts.  By default finite strains are assumed.
 *
 * The elastic strain is calculated by subtracting the computed inelastic strain
 * increment tensor from the mechanical strain tensor.  Mechanical strain is
 * considered as the sum of the elastic and inelastic (plastic, creep, ect) strains.
 *
 * This material is used to call the recompute iterative materials of a number
 * of specified inelastic models that inherit from StressUpdateBase.  It iterates
 * over the specified inelastic models until the change in stress is within
 * a user-specified tolerance, in order to produce the stress, the consistent
 * tangent operator and the elastic and inelastic strains for the time increment.
 */

class ComputeMultipleInelasticStress : public ComputeMultipleInelasticStressBase
{
public:
  static InputParameters validParams();

  ComputeMultipleInelasticStress(const InputParameters & parameters);

protected:
  virtual std::vector<MaterialName> getInelasticModelNames() override;

  virtual void updateQpState(RankTwoTensor & elastic_strain_increment,
                             RankTwoTensor & combined_inelastic_strain_increment) override;
};
