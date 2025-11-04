//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "INSADTauMaterial.h"
#include "INSADMaterial.h"
#include "INSAD3Eqn.h"

registerMooseObject("NavierStokesApp", INSADTauMaterial);

// Make sure all symbols are generated
template class INSADTauMaterialTempl<INSADMaterial>;
