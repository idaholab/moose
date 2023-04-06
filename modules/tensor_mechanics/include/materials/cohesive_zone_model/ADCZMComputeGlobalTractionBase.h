//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfaceMaterial.h"

/**
 * AD equivalent of CZMComputeGlobalTractionBase
 */
class ADCZMComputeGlobalTractionBase : public InterfaceMaterial
{
public:
  static InputParameters validParams();
  ADCZMComputeGlobalTractionBase(const InputParameters & parameters);

protected:
  void computeQpProperties() override;

  // method computing the equilibrium traction, rotate local traction to the global coordinate
  // system
  virtual void computeEquilibriumTracion() = 0;

  /// Base name of the material system
  const std::string _base_name;

  /// the value of the traction in global and interface coordinates
  ///@{
  ADMaterialProperty<RealVectorValue> & _traction_global;
  const ADMaterialProperty<RealVectorValue> & _interface_traction;
  ///@}

  /// the rotation matrix trnasforming from interface to global coordinates
  const ADMaterialProperty<RankTwoTensor> & _czm_total_rotation;
};
