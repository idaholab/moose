#ifndef BOUNDARYFLUX3EQNBC_H
#define BOUNDARYFLUX3EQNBC_H

#include "OneDIntegratedBC.h"
#include "RDGIndices3Eqn.h"
#include "BoundaryFluxBase.h"

class BoundaryFlux3EqnBC;

template <>
InputParameters validParams<BoundaryFlux3EqnBC>();

/**
 * Boundary conditions for the 1-D, 1-phase, variable-area Euler equations
 *
 * The boundary fluxes are computed using the reconstructed linear solution,
 * passed in using a material, but their Jacobians are approximated using the
 * constant monomial cell-average solution, coupled in as variables. This
 * approximation has been found to be perfectly sufficient and avoids the
 * complexities and expense of the perfect Jacobian, which would require chain
 * rule with the Jacobians of the solution slopes.
 */
class BoundaryFlux3EqnBC : public OneDIntegratedBC, public RDGIndices3Eqn
{
public:
  BoundaryFlux3EqnBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /**
   * Creates the mapping of coupled variable index to index in Euler system
   *
   * @returns the mapping of coupled variable index to index in Euler system
   */
  virtual std::map<unsigned int, unsigned int> getIndexMapping() const;

  /// Cross-sectional area, piecewise constant
  const VariableValue & _A_avg;
  /// Cross-sectional area, linear
  const VariableValue & _A_linear;

  // piecewise constant variable values in interior cells
  const VariableValue & _rhoA_avg;
  const VariableValue & _rhouA_avg;
  const VariableValue & _rhoEA_avg;

  // reconstructed variable values
  const MaterialProperty<Real> & _rhoA;
  const MaterialProperty<Real> & _rhouA;
  const MaterialProperty<Real> & _rhoEA;

  // coupled variable indices
  const unsigned int _rhoA_var;
  const unsigned int _rhouA_var;
  const unsigned int _rhoEA_var;

  /// map of coupled variable index to equations variable index convention
  const std::map<unsigned int, unsigned int> _jmap;

  /// index within the Euler system of the equation upon which this BC acts
  const unsigned int _equation_index;

  /// boundary flux user object
  const BoundaryFluxBase & _flux;
};

#endif
