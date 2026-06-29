//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosLinearFVKernel.h"

namespace Moose::Kokkos
{

InputParameters
LinearFVKernel::validParams()
{
  auto params = LinearSystemContributionObject::validParams();
  params += BlockRestrictable::validParams();
  params += NonADFunctorInterface::validParams();
  params.registerBase("LinearFVKernel");
  return params;
}

LinearFVKernel::LinearFVKernel(const InputParameters & parameters)
  : LinearSystemContributionObject(parameters), BlockRestrictable(this), NonADFunctorInterface(this)
{
}

LinearFVKernel::LinearFVKernel(const LinearFVKernel & object)
  : LinearSystemContributionObject(object),
    BlockRestrictable(object, {}),
    NonADFunctorInterface(object, {})
{
}

} // namespace Moose::Kokkos
