/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef JINTEGRAL_H
#define JINTEGRAL_H

#include "ElementIntegralPostprocessor.h"
#include "CrackFrontDefinition.h"

// Forward Declarations
class JIntegral;

template <>
InputParameters validParams<JIntegral>();

/**
 * This postprocessor computes the J-Integral
 *
 */
class JIntegral : public ElementIntegralPostprocessor
{
public:
  JIntegral(const InputParameters & parameters);
  virtual Real getValue();

protected:
  virtual void initialSetup();
  virtual Real computeQpIntegral();
  const VariableValue & _scalar_q;
  /// The gradient of the scalar q field
  const VariableGradient & _grad_of_scalar_q;
  const CrackFrontDefinition * const _crack_front_definition;
  bool _has_crack_front_point_index;
  const unsigned int _crack_front_point_index;
  bool _treat_as_2d;
  const MaterialProperty<ColumnMajorMatrix> & _Eshelby_tensor;
  const MaterialProperty<RealVectorValue> * _J_thermal_term_vec;
  bool _convert_J_to_K;
  bool _has_symmetry_plane;
  Real _poissons_ratio;
  Real _youngs_modulus;
};

#endif // JINTEGRAL3D_H
