//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosVectorPostprocessor.h"
#include "KokkosGeneralUserObject.h"

namespace Moose
{
namespace Kokkos
{

class GeneralVectorPostprocessor : public GeneralUserObject, public VectorPostprocessor
{
public:
  static InputParameters validParams();

  GeneralVectorPostprocessor(const InputParameters & parameters);
};

} // namespace Kokkos
} // namespace Moose
