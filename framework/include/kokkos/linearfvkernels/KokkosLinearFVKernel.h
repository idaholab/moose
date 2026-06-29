//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosLinearSystemContributionObject.h"

#include "BlockRestrictable.h"
#include "NonADFunctorInterface.h"

namespace Moose::Kokkos
{

class FVDatum;

/**
 * Base class for Kokkos linear finite volume kernels that contribute to the linear system
 */
class LinearFVKernel : public LinearSystemContributionObject,
                       public BlockRestrictable,
                       public NonADFunctorInterface
{
public:
  static InputParameters validParams();

  LinearFVKernel(const InputParameters & parameters);
  LinearFVKernel(const LinearFVKernel & object);

  /**
   * Default methods to prevent compile errors when matrix contributions are not defined in the
   * derived class. Kernels using these defaults skip matrix dispatch.
   */
  ///@{
  template <typename Derived>
  KOKKOS_FUNCTION Real computeMatrixContribution(const FVDatum &) const
  {
    ::Kokkos::abort("Default computeMatrixContribution() should never be called. Make sure you "
                    "properly redefined this method in your class without typos.");

    return 0;
  }

  template <typename Derived>
  KOKKOS_FUNCTION Real computeNeighborMatrixContribution(const FVDatum &) const
  {
    ::Kokkos::abort("Default computeNeighborMatrixContribution() should never be called. Make sure "
                    "you properly redefined this method in your class without typos.");

    return 0;
  }
  ///@}

  /**
   * Functions used to check if users have overriden the hook methods, whose calculations can be
   * skipped when not overriden
   * @returns The function pointer of the default hook method
   */
  ///@{
  template <typename Derived>
  static auto defaultMatrixContribution()
  {
    return &LinearFVKernel::computeMatrixContribution<Derived>;
  }

  template <typename Derived>
  static auto defaultNeighborMatrixContribution()
  {
    return &LinearFVKernel::computeNeighborMatrixContribution<Derived>;
  }
  ///@}
};

} // namespace Moose::Kokkos
