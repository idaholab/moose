//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADComputeIncrementalSmallStrain.h"

/**
 * ADCompute1DIncrementalStrain defines a strain increment only for incremental
 * small strains in 1D problems, handling strains in other two directions.
 * ADCompute1DIncrementalStrain contains virtual methods to define the displacement gradients
 * as a general nonzero value.
 */
class ADCompute1DIncrementalStrain : public ADComputeIncrementalSmallStrain
{
public:
  static InputParameters validParams();

  ADCompute1DIncrementalStrain(const InputParameters & parameters);

protected:
  /**
   * Computes the current and old deformation gradients with the assumptions for
   * axisymmetric 1D problems, and returns the total strain increment tensor
   */
  void computeTotalStrainIncrement(ADRankTwoTensor & total_strain_increment) override;

  /**
   * Computes the current dUy/dY; as a virtual function, this function is
   * overwritten for the specific geometries defined by inheriting classes
   */
  virtual ADReal computeGradDispYY() = 0;

  /**
   * Computes the old dUy/dY; as a virtual function, this function is
   * overwritten for the specific geometries defined by inheriting classes
   */
  virtual Real computeGradDispYYOld() = 0;

  /**
   * Computes the current dUz/dZ; as a virtual function, this function is
   * overwritten for the specific geometries defined by inheriting classes
   */
  virtual ADReal computeGradDispZZ() = 0;

  /**
   * Computes the old dUz/dZ; as a virtual function, this function is
   * overwritten for the specific geometries defined by inheriting classes
   */
  virtual Real computeGradDispZZOld() = 0;
};
