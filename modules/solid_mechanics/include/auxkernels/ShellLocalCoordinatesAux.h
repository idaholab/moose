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
#include "MooseEnum.h"

// Forward declarations

class ShellLocalCoordinatesAux : public AuxKernel
{
public:
  static InputParameters validParams();

  ShellLocalCoordinatesAux(const InputParameters & parameters);
  virtual ~ShellLocalCoordinatesAux() {}

protected:
  virtual Real computeValue() override;

  /// Base name of the material system used to calculate the elastic energy
  const std::string _base_name;

  MooseEnum _property;
  unsigned int _component;

  /// The local stress tensor
  const MaterialProperty<RankTwoTensor> * _local_coordinates;
};
