//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosGeneralUserObject.h"

namespace Moose::Kokkos
{

InputParameters
GeneralUserObject::validParams()
{
  InputParameters params = UserObject::validParams();
  params += ::MaterialPropertyInterface::validParams();
  params += ::TransientInterface::validParams();
  return params;
}

GeneralUserObject::GeneralUserObject(const InputParameters & parameters)
  : UserObject(parameters),
    ::MaterialPropertyInterface(this, Moose::EMPTY_BLOCK_IDS, Moose::EMPTY_BOUNDARY_IDS),
    ::TransientInterface(this)
{
}

GeneralUserObject::GeneralUserObject(const GeneralUserObject & object)
  : UserObject(object), ::MaterialPropertyInterface(object, {}), ::TransientInterface(object, {})
{
}

} // namespace Moose::Kokkos
