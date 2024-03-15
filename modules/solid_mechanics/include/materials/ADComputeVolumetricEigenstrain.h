//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeEigenstrainBase.h"

/**
 * ADComputeVolumetricEigenstrain computes an eigenstrain that is defined by a set of scalar
 * material properties that summed together define the volumetric change.;
 */
class ADComputeVolumetricEigenstrain : public ADComputeEigenstrainBase
{
public:
  static InputParameters validParams();

  ADComputeVolumetricEigenstrain(const InputParameters & parameters);

protected:
  virtual void computeQpEigenstrain();

  /// The material properties that define volumetric change
  std::vector<const ADMaterialProperty<Real> *> _volumetric_materials;
};
