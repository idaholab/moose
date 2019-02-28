#ifndef FLOWREGIMESIMPLEMATERIAL_H
#define FLOWREGIMESIMPLEMATERIAL_H

#include "Material.h"
#include "DerivativeMaterialInterfaceTHM.h"

class FlowRegimeSimpleMaterial;

template <>
InputParameters validParams<Material>();

/**
 * Computes the flow regime map for simple closures
 *
 * This class does nothing, becuase simple closures do not assume any flow regimes
 */
class FlowRegimeSimpleMaterial : public DerivativeMaterialInterfaceTHM<Material>
{
public:
  FlowRegimeSimpleMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Heat flux partitioning coefficient
  MaterialProperty<Real> & _kappa_liquid;
  MaterialProperty<Real> & _dkappa_liquid_dbeta;

  /// Liquid volume fraction
  const VariableValue & _alpha_liquid;
  const MaterialProperty<Real> & _dalpha_liquid_dbeta;
};

#endif /* FLOWREGIMESIMPLEMATERIAL_H */
