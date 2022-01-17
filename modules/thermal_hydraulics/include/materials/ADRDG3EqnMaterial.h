#pragma once

#include "Material.h"
#include "SlopeReconstruction1DInterface.h"

class SinglePhaseFluidProperties;

/**
 * Reconstructed solution values for the 1-D, 1-phase, variable-area Euler equations
 *
 * This material applies the limited slopes for the primitive variable set
 * {p, u, T} and then computes the corresponding face values for the conserved
 * variables {rhoA, rhouA, rhoEA}.
 */
class ADRDG3EqnMaterial : public Material, public SlopeReconstruction1DInterface<true>
{
public:
  ADRDG3EqnMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;
  virtual std::vector<ADReal> computeElementPrimitiveVariables(const Elem * elem) const override;

  /// Cross-sectional area, piecewise constant
  const ADVariableValue & _A_avg;
  /// Cross-sectional area, linear
  const ADVariableValue & _A_linear;

  // piecewise constant conserved variable values
  const ADVariableValue & _rhoA_avg;
  const ADVariableValue & _rhouA_avg;
  const ADVariableValue & _rhoEA_avg;

  // piecewise constant conserved variables
  MooseVariable * _A_var;
  MooseVariable * _rhoA_var;
  MooseVariable * _rhouA_var;
  MooseVariable * _rhoEA_var;

  /// Flow channel direction
  const MaterialProperty<RealVectorValue> & _dir;

  // reconstructed variable values
  ADMaterialProperty<Real> & _rhoA;
  ADMaterialProperty<Real> & _rhouA;
  ADMaterialProperty<Real> & _rhoEA;

  /// fluid properties user object
  const SinglePhaseFluidProperties & _fp;

  /// Number of slopes
  static const unsigned int _n_slopes = 3;
  /// Indices for slope array
  enum SlopeIndex
  {
    PRESSURE = 0,
    VELOCITY = 1,
    TEMPERATURE = 2
  };

public:
  static InputParameters validParams();
};
