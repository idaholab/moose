#ifndef RDG3EQNMATERIAL_H
#define RDG3EQNMATERIAL_H

#include "Material.h"
#include "SinglePhaseFluidProperties.h"
#include "RDGSlopes3Eqn.h"

class RDG3EqnMaterial;

template <>
InputParameters validParams<RDG3EqnMaterial>();

/**
 * Reconstructed solution values for the 1-D, 1-phase, variable-area Euler equations
 *
 * This material applies the limited slopes for the primitive variable set
 * {p, u, T} and then computes the corresponding face values for the conserved
 * variables {rhoA, rhouA, rhoEA}.
 */
class RDG3EqnMaterial : public Material
{
public:
  RDG3EqnMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Cross-sectional area, piecewise constant
  const VariableValue & _A_avg;
  /// Cross-sectional area, linear
  const VariableValue & _A_linear;

  // piecewise constant conserved variables
  const VariableValue & _rhoA_avg;
  const VariableValue & _rhouA_avg;
  const VariableValue & _rhoEA_avg;

  /// Flow channel direction
  const MaterialProperty<RealVectorValue> & _dir;

  // reconstructed variable values
  MaterialProperty<Real> & _rhoA;
  MaterialProperty<Real> & _rhouA;
  MaterialProperty<Real> & _rhoEA;

  /// slopes user object
  const RDGSlopes3Eqn & _slopes_uo;

  /// fluid properties user object
  const SinglePhaseFluidProperties & _fp;
};

#endif
