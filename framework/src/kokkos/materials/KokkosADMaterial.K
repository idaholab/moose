//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosADMaterial.h"

namespace Moose::Kokkos
{

InputParameters
ADMaterial::validParams()
{
  InputParameters params = Material::validParams();
  return params;
}

ADMaterial::ADMaterial(const InputParameters & parameters) : Material(parameters) {}

void
ADMaterial::initialSetup()
{
  Material::initialSetup();

  _ad_setup.assign(*this,
                   DispatcherRegistry::getProperty("max_ad_slots", type()),
                   _configuration.displacements());
  _ad_slots = _ad_setup.table();
}

} // namespace Moose::Kokkos
