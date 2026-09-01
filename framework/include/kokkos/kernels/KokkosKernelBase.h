//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosResidualObject.h"
#include "KokkosLocalParallelInterface.h"

#include "BlockRestrictable.h"
#include "MaterialPropertyInterface.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"

namespace Moose::Kokkos
{

/**
 * The base class for Kokkos kernels
 */
class KernelBase : public ResidualObject,
                   public BlockRestrictable,
                   public CoupleableMooseVariableDependencyIntermediateInterface,
                   public MaterialPropertyInterface,
                   public LocalParallelInterface
{
public:
  static InputParameters validParams();

  /**
   * Constructor
   * @param field_type The MOOSE variable field type
   */
  KernelBase(const InputParameters & parameters, Moose::VarFieldType field_type);
  /**
   * Copy constructor for parallel dispatch
   */
  KernelBase(const KernelBase & object);

  /**
   * Compute the local contribution to the action of this kernel's Jacobian on the Kokkos
   * matrix-free direction vector, accumulating the result into the Kokkos matrix-free action
   * vector. Only supported for kernels with factored (precomputed) Jacobian hooks, e.g.
   * KernelGrad/KernelValue; the default implementation errors out.
   */
  virtual void computeJacobianVectorProduct()
  {
    mooseError("computeJacobianVectorProduct() is not implemented for Kokkos kernel type '",
               type(),
               "'.");
  }
};

} // namespace Moose::Kokkos
