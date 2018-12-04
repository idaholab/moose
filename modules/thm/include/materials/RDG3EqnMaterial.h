#ifndef RDG3EQNMATERIAL_H
#define RDG3EQNMATERIAL_H

#include "Material.h"
#include "SinglePhaseFluidProperties.h"
#include "FlowModel.h"

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

  /**
   * Gets limited slopes for the primitive variables.
   *
   * The interface for this function returns a vector of \c RealGradient values,
   * but for 1-D cases like this, it does not make sense to return a 3-D vector;
   * only a single value is needed to represent the gradient along the 1-D
   * direction. This function outputs these 1-D gradient values in the first
   * entry of the \c RealGradient values; the other 2 entries are not used.
   *
   * @param[in] elem   Element for which to get slopes
   *
   * @returns Vector of a number of slopes for the element, where the first
   *          entry of each is the slope in the 1-D direction, and the other
   *          entries are zero.
   */
  std::vector<RealGradient> getElementSlopes(const Elem * elem) const;

  /**
   * Computes the cell-average primitive variable values for an element
   *
   * @param[in] elem   Element for which to get values
   * @param[in] i      Index of storage in \c U
   * @param[out] U     Vector in which to store solution values
   */
  void computeElementPrimitiveVariables(const Elem * elem,
                                        const unsigned int & i,
                                        std::vector<std::vector<Real>> & U) const;

  /// Slope reconstruction scheme
  const FlowModel::ESlopeReconstructionType _scheme;

  /// Cross-sectional area, piecewise constant
  const VariableValue & _A_avg;
  /// Cross-sectional area, linear
  const VariableValue & _A_linear;

  // piecewise constant conserved variable values
  const VariableValue & _rhoA_avg;
  const VariableValue & _rhouA_avg;
  const VariableValue & _rhoEA_avg;

  // piecewise constant conserved variables
  MooseVariable * _A_var;
  MooseVariable * _rhoA_var;
  MooseVariable * _rhouA_var;
  MooseVariable * _rhoEA_var;

  /// Flow channel direction
  const MaterialProperty<RealVectorValue> & _dir;

  // reconstructed variable values
  MaterialProperty<Real> & _rhoA;
  MaterialProperty<Real> & _rhouA;
  MaterialProperty<Real> & _rhoEA;

  /// fluid properties user object
  const SinglePhaseFluidProperties & _fp;

  /// Number of sides
  static const unsigned int _n_side;
  /// Number of elemental values in stencil for computing slopes
  static const unsigned int _n_sten;
};

#endif
