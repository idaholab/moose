//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADComputeSmallStrain.h"

/**
 * ADCompute1DSmallStrain defines a strain tensor, assuming small strains,
 * in 1D problems, handling strains in other two directions.
 * ADCompute1DSmallStrain contains virtual methods to define the strain_yy and strain_zz
 * as a general nonzero value.
 */
class ADCompute1DSmallStrain : public ADComputeSmallStrain
{
public:
  static InputParameters validParams();

  ADCompute1DSmallStrain(const InputParameters & parameters);

  void computeProperties() override;

protected:
  /// Computes the strain_yy; as a virtual function, this function is
  /// overwritten for the specific geometries defined by inheriting classes
  virtual ADReal computeStrainYY() = 0;

  /// Computes the strain_zz; as a virtual function, this function is
  /// overwritten for the specific geometries defined by inheriting classes
  virtual ADReal computeStrainZZ() = 0;
};
