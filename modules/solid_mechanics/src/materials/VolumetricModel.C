/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "VolumetricModel.h"

template <>
InputParameters
validParams<VolumetricModel>()
{
  return validParams<Material>();
}

VolumetricModel::VolumetricModel(const InputParameters & parameters) : Material(parameters) {}

VolumetricModel::~VolumetricModel() {}
