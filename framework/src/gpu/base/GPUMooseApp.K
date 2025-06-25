//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUHeader.h"

#include "MooseApp.h"

void
MooseApp::queryGPUs()
{
  unsigned int num_gpus = Kokkos::num_devices();

  _has_gpus = comm().verify(num_gpus > 0);
}
