//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MatDiffusion.h"

registerMooseObject("MooseApp", MatDiffusion);
registerMooseObject("MooseApp", ADMatDiffusion);

template <bool is_ad>
InputParameters
MatDiffusionTempl<is_ad>::validParams()
{
  InputParameters params = MatDiffusionBaseParent<is_ad>::validParams();
  params.addClassDescription(
      "Diffusion equation Kernel that takes an isotropic Diffusivity from a material property");
  return params;
}

template <bool is_ad>
MatDiffusionTempl<is_ad>::MatDiffusionTempl(const InputParameters & parameters)
  : MatDiffusionBaseParent<is_ad>(parameters)
{
}

template class MatDiffusionTempl<false>;
template class MatDiffusionTempl<true>;
