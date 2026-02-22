//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosUserObject.h"

#include "MaterialPropertyInterface.h"
#include "TransientInterface.h"

namespace Moose::Kokkos
{

class GeneralUserObject : public UserObject,
                          public ::MaterialPropertyInterface,
                          public ::TransientInterface
{
public:
  static InputParameters validParams();

  GeneralUserObject(const InputParameters & parameters);

  /**
   * Copy constructor for parallel dispatch
   */
  GeneralUserObject(const GeneralUserObject & object);
};

} // namespace Moose::Kokkos
