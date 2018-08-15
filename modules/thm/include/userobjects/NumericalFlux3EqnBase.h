#ifndef NUMERICALFLUX3EQNBASE_H
#define NUMERICALFLUX3EQNBASE_H

#include "RDGFluxBase.h"
#include "RDGIndices3Eqn.h"

class NumericalFlux3EqnBase;

template <>
InputParameters validParams<NumericalFlux3EqnBase>();

/**
 * Abstract base class for computing and caching internal or boundary fluxes for RDG
 * for the 3-equation model of 1-phase flow.
 *
 * This base class adds interfaces useful for testing purposes; specifically,
 * since some fluxes have different regions, depending on flow conditions, it
 * is important to ensure that all are tested. To help with this, an interface
 * is added to retrieve the last entered region.
 */
class NumericalFlux3EqnBase : public RDGFluxBase, public RDGIndices3Eqn
{
public:
  NumericalFlux3EqnBase(const InputParameters & parameters);

  /**
   * Returns the index of the region last entered
   *
   * Here "region" refers to a code path taken. For some fluxes, such as centered
   * fluxes, there is just a single code path, but for others, such as those
   * using an approximate Riemann solver, there are multiple. Riemann solvers
   * have "regions" defined by the characteristic waves.
   */
  unsigned int getLastRegionIndex() const { return _last_region_index; }

  /**
   * Returns the total possible number of regions
   *
   * Here "region" refers to a code path taken. For some fluxes, such as centered
   * fluxes, there is just a single code path, but for others, such as those
   * using an approximate Riemann solver, there are multiple. Riemann solvers
   * have "regions" defined by the characteristic waves.
   */
  virtual unsigned int getNumberOfRegions() const = 0;

protected:
  /// Index describing the region last entered, which is useful for testing and debugging
  mutable unsigned int _last_region_index;
};

#endif // NUMERICALFLUX3EQNBASE_H
