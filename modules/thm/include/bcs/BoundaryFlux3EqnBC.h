#ifndef BOUNDARYFLUX3EQNBC_H
#define BOUNDARYFLUX3EQNBC_H

#include "OneDIntegratedBC.h"
#include "BoundaryFluxBase.h"

class BoundaryFlux3EqnBC;

template <>
InputParameters validParams<BoundaryFlux3EqnBC>();

/**
 * Boundary conditions for the 1-D, 1-phase, variable-area Euler equations
 * using a boundary flux user object
 */
class BoundaryFlux3EqnBC : public OneDIntegratedBC
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
  std::map<unsigned int, unsigned int> getIndexMapping() const;

  /// Cross-sectional area, elemental
  const VariableValue & _A_elem;
  /// Cross-sectional area, linear
  const VariableValue & _A_linear;

  // coupled variables
  const VariableValue & _rhoA;
  const VariableValue & _rhouA;
  const VariableValue & _rhoEA;

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
