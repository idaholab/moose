/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWDEPENDENCIES_H
#define POROUSFLOWDEPENDENCIES_H

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

#endif // POROUSFLOWDEPENDENCIES_H
