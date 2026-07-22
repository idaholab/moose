//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "MooseEnum.h"

// Selects the normal used to define each local secondary subpatch plane during 3D mortar segment
// mesh generation. Keep AVERAGED_NODAL_NORMAL first/default to preserve the historical behavior.
CreateMooseEnumClass(Mortar3DSubpatchPlane, AVERAGED_NODAL_NORMAL, GEOMETRIC_NORMAL);
