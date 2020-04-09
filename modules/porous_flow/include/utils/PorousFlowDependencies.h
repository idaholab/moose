//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DependencyResolver.h"

/**
 * Holds the PorousFlow dependencies of kernels, auxkernels, materials, etc.
 * For instance the Kernel PorousFlowAdvectiveFlux depends on the Kernel
 * PorousFlowDarcyBase, and the Materials PorousFlowMassFraction (nodal version)
 * and PorousFlowRelativePermeability (the nodal version).
 *
 * The main purpose of this class is to enable Actions to easily determine
 * which Materials they should add, given that they have to add certain Kernels
 * and AuxKernels.
 *
 * It is envisaged that as more Kernels, etc, are added to PorousFlow, then the
 * dependencies constained in this class will be updated to enable Actions
 * to work efficiently
 */
class PorousFlowDependencies
{
public:
  PorousFlowDependencies();

protected:
  /**
   * All dependencies of kernels, auxkernels, materials, etc, are stored in _dependencies
   */
  DependencyResolver<std::string> _deps;
};
