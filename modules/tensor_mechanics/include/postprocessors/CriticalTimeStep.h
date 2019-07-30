//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementPostprocessor.h"
#include "ComputeStressBase.h"

// Forward Declarations
class CriticalTimeStep;

template <>
InputParameters validParams<CriticalTimeStep>();

/**
 * This postprocessor computes an average element size (h) for the whole domain.
 */
class CriticalTimeStep : public ElementPostprocessor
{
public:
  CriticalTimeStep(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;

  virtual Real getValue() override;
  virtual void threadJoin(const UserObject & y) override;

protected:


  virtual void computeQpStress();

  /// Name of the elasticity tensor material property
  const std::string _elasticity_tensor_name;
  /// Elasticity tensor material property
  const MaterialProperty<RankFourTensor> & _elasticity_tensor;

  Real _total_size;
  int _elems;
  Real _poiss_rat;
  Real _young_mod;
  Real _mat_dens;
};
