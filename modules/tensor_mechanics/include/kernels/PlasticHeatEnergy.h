//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"
#include "RankTwoTensor.h"

// Forward Declarations

/**
 * Provides a heat source from plastic deformation:
 * coeff * stress * plastic_strain_rate
 */
class PlasticHeatEnergy : public Kernel
{
public:
  static InputParameters validParams();

  PlasticHeatEnergy(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// coefficient of stress * plastic_strain_rate
  Real _coeff;

  /// optional parameter that allows multiple mechanics models to be defined
  const std::string _base_name;

  /// stress * plastic_strain_rate
  const MaterialProperty<Real> & _plastic_heat;

  /// d(plastic_heat)/d(total_strain)
  const MaterialProperty<RankTwoTensor> & _dplastic_heat_dstrain;

  /// umber of coupled displacement variables
  unsigned int _ndisp;

  /// MOOSE variable number for the displacement variables
  std::vector<unsigned int> _disp_var;
};
