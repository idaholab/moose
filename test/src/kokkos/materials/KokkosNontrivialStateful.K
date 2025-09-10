//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosNontrivialStateful.h"

registerKokkosMaterial("MooseApp", KokkosNontrivialStateful);

// The purpose of this material is to confirm that the load and store functions for Kokkos material
// properties are only instantiated when they were requested as stateful, so that it doesn't error
// when a user uses a non-trivially-copyable type as a Kokkos material property without writing
// custom dataStore but never requests it as stateful

InputParameters
KokkosNontrivialStateful::validParams()
{
  InputParameters params = Material::validParams();
  return params;
}

KokkosNontrivialStateful::KokkosNontrivialStateful(const InputParameters & parameters)
  : Material(parameters)
{
  _prop = declareKokkosProperty<NotTriviallyCopyable>("prop");
  // Uncommenting the following line will lead to a compile error that custom dataStore should be
  // provided
  // _old_prop = getKokkosMaterialPropertyOld<NotTriviallyCopyable>("prop");
}
