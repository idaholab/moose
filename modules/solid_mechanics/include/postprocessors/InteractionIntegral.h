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

//Forward Declarations
class InteractionIntegral;

template<>
InputParameters validParams<InteractionIntegral>();

/**
 * This postprocessor computes the Interaction Integral
 *
 */
class InteractionIntegral: public ElementIntegralPostprocessor
{
public:
  InteractionIntegral(const std::string & name, InputParameters parameters);

  virtual Real getValue();

protected:
  virtual void initialSetup();
  virtual Real computeQpIntegral();
  /// The gradient of the scalar q field
  VariableGradient & _grad_of_scalar_q;
  const CrackFrontDefinition * const _crack_front_definition;
  bool _has_crack_front_node_index;
  const unsigned int _crack_front_node_index;
  bool _treat_as_2d;
  MaterialProperty<ColumnMajorMatrix> & _Eshelby_tensor;
  MaterialProperty<SymmTensor> & _stress;
  MaterialProperty<SymmTensor> & _strain;
  VariableGradient & _grad_disp_x;
  VariableGradient & _grad_disp_y;
  VariableGradient & _grad_disp_z;
  std::string _aux_stress_name;
  MaterialProperty<ColumnMajorMatrix> & _aux_stress;
  std::string _aux_disp_name;
  MaterialProperty<ColumnMajorMatrix> & _aux_disp;
  std::string _aux_grad_disp_name;
  MaterialProperty<ColumnMajorMatrix> & _aux_grad_disp;
  std::string _aux_strain_name;
  MaterialProperty<ColumnMajorMatrix> & _aux_strain;
  Real _K_factor;
  bool _has_symmetry_plane;
};

#endif //INTERACTIONINTEGRAL_H
