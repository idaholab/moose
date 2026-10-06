//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosDeformation.h"

#include "Coupleable.h"

namespace Moose::Kokkos
{

Deformation::Deformation(const Coupleable & object, const std::string & param)
  : _disp(object.kokkosCoupledValues(param)), _grad_disp(object.kokkosCoupledGradients(param))
{
}

} // namespace Moose::Kokkos
