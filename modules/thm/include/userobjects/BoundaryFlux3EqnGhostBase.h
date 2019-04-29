#pragma once

#include "BoundaryFluxBase.h"
#include "RDGFluxBase.h"

class BoundaryFlux3EqnGhostBase;

template <>
InputParameters validParams<BoundaryFlux3EqnGhostBase>();

/**
 * Computes boundary fluxes for the 1-D, variable-area Euler equations using a
 * numerical flux user object and a ghost cell solution
 */
class BoundaryFlux3EqnGhostBase : public BoundaryFluxBase
{
public:
  BoundaryFlux3EqnGhostBase(const InputParameters & parameters);

protected:
  virtual void calcFlux(unsigned int iside,
                        dof_id_type ielem,
                        const std::vector<Real> & U1,
                        const RealVectorValue & normal,
                        std::vector<Real> & flux) const override;

  virtual void calcJacobian(unsigned int iside,
                            dof_id_type ielem,
                            const std::vector<Real> & U1,
                            const RealVectorValue & normal,
                            DenseMatrix<Real> & jac1) const override;

  /**
   * Gets the solution vector in the ghost cell
   *
   * @param[in] U1  solution vector in the boundary (interior) cell
   *
   * @returns solution vector in the ghost cell
   */
  virtual std::vector<Real> getGhostCellSolution(const std::vector<Real> & U1) const = 0;

  /**
   * Gets the Jacobian matrix of the ghost cell solution w.r.t. the interior solution
   *
   * @param[in] U1  solution vector in the boundary (interior) cell
   *
   * @returns Jacobian matrix of the ghost cell solution w.r.t. the interior solution
   */
  virtual DenseMatrix<Real> getGhostCellSolutionJacobian(const std::vector<Real> & U1) const = 0;

  /// Numerical flux user object
  const RDGFluxBase & _numerical_flux;
};
