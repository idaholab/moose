//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PFFRACTUREBULKRATEBASE_H
#define PFFRACTUREBULKRATEBASE_H

#include "Kernel.h"

// Forward Declarations
class RankTwoTensor;
class PFFractureBulkRateBase;

template <>
InputParameters validParams<PFFractureBulkRateBase>();

/**
 * Phase field based fracture model
 * This kernel computes the residual and Jacobian for bulk free energy contribution to c
 * Refer to Formulation: Miehe et. al., Int. J. Num. Methods Engg., 2010, 83. 1273-1311 Equation 63
 */
class PFFractureBulkRateBase : public Kernel
{
public:
  PFFractureBulkRateBase(const InputParameters & parameters);

protected:
  /// Critical energy release rate for fracture
  const MaterialProperty<Real> & _gc_prop;

  /// Contribution of umdamaged strain energy to damage evolution
  const MaterialProperty<Real> & _G0_pos;

  /// Variation of undamaged strain energy driving damage evolution with strain
  const MaterialProperty<RankTwoTensor> * _dG0_pos_dstrain;

  /// Coupled displacement variables
  const unsigned int _ndisp;
  std::vector<unsigned int> _disp_var;
  std::string _base_name;

  /// Diffuse crack width, controls damage zone thickness
  const Real _width;

  /// Viscosity parameter ( viscosity -> 0, rate independent )
  const Real _viscosity;
};

#endif // PFFRACTUREBULKRATE_H
