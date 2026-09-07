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
    mooseError(
        "computeJacobianVectorProduct() is not implemented for Kokkos kernel type '", type(), "'.");
  }

  /**
   * Compute the local contribution to the Kokkos matrix-free Jacobian diagonal, accumulating the
   * result into the Kokkos matrix-free diagonal vector. Only supported for kernels with factored
   * (precomputed) Jacobian hooks, e.g. KernelGrad/KernelValue; the default implementation errors
   * out.
   */
  virtual void computeJacobianDiagonal()
  {
    mooseError(
        "computeJacobianDiagonal() is not implemented for Kokkos kernel type '", type(), "'.");
  }

  /**
   * Accumulate this kernel's linearization into the system's quadrature-point Jacobian cache.
   * Only supported for kernels that define the quadrature-point Jacobian tensor hook; the default
   * implementation errors out.
   */
  virtual void computeQpJacobianCache()
  {
    mooseError(
        "computeQpJacobianCache() is not implemented for Kokkos kernel type '", type(), "'.");
  }

  /**
   * Get whether this kernel's linearization is gathered into the system's quadrature-point
   * Jacobian cache. A kernel that is covered by the cache does not supply its own matrix-free
   * operator action or diagonal, since the cache's consumers already account for it.
   * @returns Whether the cache covers this kernel
   */
  virtual bool usesQpJacobianCache() const { return false; }

  /**
   * Get which blocks of the quadrature-point linearization this kernel can populate. Kernels that
   * define the quadrature-point Jacobian tensor hook override this to declare their mask; the union
   * over the kernels active on a subdomain bounds the work the cache's consumers must do.
   * @returns The QpJacobianBlock mask
   */
  virtual unsigned int qpJacobianBlocks() const { return QP_JACOBIAN_NONE; }
};

} // namespace Moose::Kokkos
