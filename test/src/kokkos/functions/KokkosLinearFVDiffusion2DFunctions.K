//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosLinearFVDiffusion2DFunctions.h"

registerKokkosFunction("MooseTestApp", KokkosLinearFV2DExactSolution);
registerKokkosFunction("MooseTestApp", KokkosLinearFV2DDiffusionCoefficient);
registerKokkosFunction("MooseTestApp", KokkosLinearFV2DSourceFunction);

InputParameters
KokkosLinearFV2DExactSolution::validParams()
{
  InputParameters params = Moose::Kokkos::FunctionBase::validParams();
  params.addClassDescription("Exact solution used by the Kokkos LinearFV 2D diffusion test.");
  return params;
}

KokkosLinearFV2DExactSolution::KokkosLinearFV2DExactSolution(const InputParameters & parameters)
  : Moose::Kokkos::FunctionBase(parameters)
{
}

InputParameters
KokkosLinearFV2DDiffusionCoefficient::validParams()
{
  InputParameters params = Moose::Kokkos::FunctionBase::validParams();
  params.addClassDescription(
      "Diffusion coefficient used by the Kokkos LinearFV 2D diffusion test.");
  return params;
}

KokkosLinearFV2DDiffusionCoefficient::KokkosLinearFV2DDiffusionCoefficient(
    const InputParameters & parameters)
  : Moose::Kokkos::FunctionBase(parameters)
{
}

InputParameters
KokkosLinearFV2DSourceFunction::validParams()
{
  InputParameters params = Moose::Kokkos::FunctionBase::validParams();
  params.addClassDescription("Source term used by the Kokkos LinearFV 2D diffusion test.");
  return params;
}

KokkosLinearFV2DSourceFunction::KokkosLinearFV2DSourceFunction(const InputParameters & parameters)
  : Moose::Kokkos::FunctionBase(parameters)
{
}
