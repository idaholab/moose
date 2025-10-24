//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NonADFunctorInterface.h"

InputParameters
NonADFunctorInterface::validParams()
{
  return FunctorInterface::validParams();
}

NonADFunctorInterface::NonADFunctorInterface(const MooseObject * const moose_object)
  : FunctorInterface(moose_object)
{
}

#ifdef MOOSE_KOKKOS_ENABLED
NonADFunctorInterface::NonADFunctorInterface(const NonADFunctorInterface & object,
                                             const Moose::Kokkos::FunctorCopy & key)
  : FunctorInterface(object, key)
{
}
#endif
