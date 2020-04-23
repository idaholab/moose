//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADMaterial.h"
#include "RankTwoTensor.h"

/// ADRankTwoCartesianComponent computes selected components from a Rank-2 tensors.
class ADRankTwoCartesianComponent : public ADMaterial
{
public:
  static InputParameters validParams();

  ADRankTwoCartesianComponent(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

private:
  const ADMaterialProperty<RankTwoTensor> & _tensor;

  /// Name of the stress/strain to be calculated
  const std::string _property_name;

  /// Stress/strain value returned from calculation
  ADMaterialProperty<Real> & _property;

  /// Tensor component
  const unsigned int _i;
  /// Tensor component
  const unsigned int _j;
};
