//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosFunction.h"

#include "FunctionInterface.h"
#include "FEProblemBase.h"

FunctionInterface::FunctionInterface(const FunctionInterface & object,
                                     const Moose::Kokkos::FunctorCopy &)
  : _fni_object(object._fni_object),
    _fni_params(object._fni_params),
    _fni_feproblem(object._fni_feproblem),
    _fni_tid(object._fni_tid)
{
}

Moose::Kokkos::Function
FunctionInterface::getKokkosFunction(const std::string & name) const
{
  return getKokkosFunctionByName(_fni_params.get<FunctionName>(name));
}

Moose::Kokkos::Function
FunctionInterface::getKokkosFunctionByName(const FunctionName & name) const
{
#ifdef MOOSE_ENABLE_KOKKOS_GPU
  _fni_object.mooseError(
      "Retrieving a Kokkos function as abstract type is currently not supported for GPU.");
#endif

  if (!_fni_object.isKokkosObject())
    _fni_object.mooseError("Attempted to retrieve a Kokkos function from a non-Kokkos object.");

  return _fni_feproblem.getKokkosFunction(name);
}

bool
FunctionInterface::hasKokkosFunction(const std::string & param_name) const
{
  return hasKokkosFunctionByName(_fni_params.get<FunctionName>(param_name));
}

bool
FunctionInterface::hasKokkosFunctionByName(const FunctionName & name) const
{
  if (!_fni_object.isKokkosObject())
    _fni_object.mooseError("Attempted to retrieve a Kokkos function from a non-Kokkos object.");

  return _fni_feproblem.hasKokkosFunction(name);
}

const Moose::FunctionBase *
FunctionInterface::getKokkosFunctionByNameHelper(const FunctionName & name) const
{
  if (!_fni_object.isKokkosObject())
    _fni_object.mooseError("Attempted to retrieve a Kokkos function from a non-Kokkos object.");

  return &_fni_feproblem.getKokkosFunction<Moose::FunctionBase>(name);
}
