#ifndef MOMENTUMFREESLIPBC_H
#define MOMENTUMFREESLIPBC_H

#include "NodalNormalBC.h"

class MomentumFreeSlipBC;

template <>
InputParameters validParams<MomentumFreeSlipBC>();

/**
 * Boundary condition that applies free slip condition at nodes
 */
class MomentumFreeSlipBC : public NodalNormalBC
{
public:
  MomentumFreeSlipBC(const InputParameters & parameters);
  virtual ~MomentumFreeSlipBC();

  virtual bool shouldApply() override;

protected:
  virtual Real computeQpResidual() override;

  /// The dimension of the mesh
  const unsigned int _mesh_dimension;

  /// Momentum in x-direction
  const VariableValue & _rho_u;
  /// Momentum in y-direction
  const VariableValue & _rho_v;
  /// Momentum in z-direction
  const VariableValue & _rho_w;
};

#endif /* MOMENTUMFREESLIPBC_H */
