//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"
#include "RankTwoTensor.h"

// Forward declarations
template <bool is_ad>
class KineticEnergyAuxTempl : public AuxKernel
{
public:
  static InputParameters validParams();

  KineticEnergyAuxTempl(const InputParameters & parameters);
  virtual ~KineticEnergyAuxTempl() {}

protected:
  virtual Real computeValue();

  /// Base name of the material system used to calculate the elastic energy
  const std::string _base_name;

  /// The stress tensor
  const GenericMaterialProperty<Real, is_ad> & _density;

  const VariableValue & _vel_x;
  const VariableValue & _vel_y;
  const VariableValue & _vel_z;
};

typedef KineticEnergyAuxTempl<false> KineticEnergyAux;
typedef KineticEnergyAuxTempl<true> ADKineticEnergyAux;
