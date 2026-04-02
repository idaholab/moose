//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosLinearFVSource.h"

registerKokkosLinearFVKernel("MooseApp", KokkosLinearFVSource);

InputParameters
KokkosLinearFVSource::validParams()
{
  InputParameters params = Moose::Kokkos::LinearFVElementalKernel::validParams();
  params.addRequiredParam<FunctionName>("source_density", "The volumetric source density.");
  params.addParam<FunctionName>("scaling_factor", "1", "Scale factor for the source term.");
  params.addClassDescription("Kokkos LinearFV source kernel.");
  return params;
}

KokkosLinearFVSource::KokkosLinearFVSource(const InputParameters & parameters)
  : Moose::Kokkos::LinearFVElementalKernel(parameters),
    _source_density(getKokkosFunction("source_density")),
    _scale(getKokkosFunction("scaling_factor"))
{
}
