//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosCoupledVectorDiffusion.h"

registerKokkosResidualObject("MooseTestApp", KokkosCoupledVectorDiffusion);

InputParameters
KokkosCoupledVectorDiffusion::validParams()
{
  InputParameters params = Moose::Kokkos::VectorKernel::validParams();
  params.addRequiredCoupledVar("v",
                               "A vector variable that will determine the diffusion rate of u.");
  MooseEnum state("current old older", "current");
  params.addParam<MooseEnum>("state",
                             state,
                             "The 'age' of the gradient you want to use for coupled diffusion. The "
                             "options are: current (the default), old, and older.");
  return params;
}

KokkosCoupledVectorDiffusion::KokkosCoupledVectorDiffusion(const InputParameters & parameters)
  : Moose::Kokkos::VectorKernel(parameters),
    _current_state(getParam<MooseEnum>("state") == "current"),
    _grad_v(getParam<MooseEnum>("state") == "current" ? kokkosCoupledVectorGradient("v")
            : getParam<MooseEnum>("state") == "old"   ? kokkosCoupledVectorGradientOld("v")
                                                      : kokkosCoupledVectorGradientOlder("v")),
    _v_id(coupled("v"))
{
}
