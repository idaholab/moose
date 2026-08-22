//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernelScalarBase.h"
#include "ADRankTwoTensorForward.h"

class Function;

/**
 * Assembles the contributions of the area integral performed over the elements
 * to the residual for the out-of-plane strain scalar variable using automatic differentiation.
 */
class ADGeneralizedPlaneStrain : public ADKernelScalarBase
{
public:
  static InputParameters validParams();

  ADGeneralizedPlaneStrain(const InputParameters & parameters);

protected:
  void initialSetup() override;
  ADReal computeQpResidual() override;
  ADReal computeScalarQpResidual() override;

private:
  /// Base name of the material system
  const std::string _base_name;

  /// The stress tensor
  const ADMaterialProperty<RankTwoTensor> & _stress;

  /// Function defining the applied out-of-plane pressure
  const Function * const _out_of_plane_pressure_function;

  /// Material property defining the applied out-of-plane pressure
  const MaterialProperty<Real> & _out_of_plane_pressure_material;

  /// Factor applied to the out-of-plane pressure
  const Real _pressure_factor;

  /// Direction of the out-of-plane strain scalar variable
  unsigned int _out_of_plane_direction;
};
