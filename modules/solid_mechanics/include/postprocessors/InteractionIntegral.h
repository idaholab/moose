/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INTERACTIONINTEGRAL_H
#define INTERACTIONINTEGRAL_H

#include "ElementIntegralPostprocessor.h"
#include "CrackFrontDefinition.h"
#include "SymmTensor.h"

// Forward Declarations
class InteractionIntegral;

template <>
InputParameters validParams<InteractionIntegral>();

/**
 * This postprocessor computes the Interaction Integral
 *
 */
class InteractionIntegral : public ElementIntegralPostprocessor
{
public:
  InteractionIntegral(const InputParameters & parameters);

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
  const MaterialProperty<SymmTensor> & _stress;
  const MaterialProperty<SymmTensor> & _strain;
  const VariableGradient & _grad_disp_x;
  const VariableGradient & _grad_disp_y;
  const VariableGradient & _grad_disp_z;
  const bool _has_temp;
  const VariableGradient & _grad_temp;
  std::string _aux_stress_name;
  const MaterialProperty<ColumnMajorMatrix> & _aux_stress;
  std::string _aux_grad_disp_name;
  const MaterialProperty<ColumnMajorMatrix> & _aux_grad_disp;
  const MaterialProperty<Real> * _current_instantaneous_thermal_expansion_coef;
  Real _K_factor;
  bool _has_symmetry_plane;
  bool _t_stress;
  Real _poissons_ratio;
};

#endif // INTERACTIONINTEGRAL_H
