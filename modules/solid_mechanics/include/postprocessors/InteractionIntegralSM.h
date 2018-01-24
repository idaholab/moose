/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INTERACTIONINTEGRALSM_H
#define INTERACTIONINTEGRALSM_H

#include "ElementIntegralPostprocessor.h"
#include "CrackFrontDefinition.h"
#include "SymmTensor.h"

// Forward Declarations
class InteractionIntegralSM;
class RankTwoTensor;

template <>
InputParameters validParams<InteractionIntegralSM>();

/**
 * This postprocessor computes the Interaction Integral
 *
 */
class InteractionIntegralSM : public ElementIntegralPostprocessor
{
public:
  InteractionIntegralSM(const InputParameters & parameters);

  virtual Real getValue();

  static MooseEnum qFunctionType();
  static MooseEnum sifModeType();

protected:
  virtual void initialSetup();
  virtual Real computeQpIntegral();
  virtual Real computeIntegral();
  void computeAuxFields(RankTwoTensor & aux_stress, RankTwoTensor & grad_disp);
  void computeTFields(RankTwoTensor & aux_stress, RankTwoTensor & grad_disp);

  unsigned int _ndisp;
  const CrackFrontDefinition * const _crack_front_definition;
  bool _has_crack_front_point_index;
  const unsigned int _crack_front_point_index;
  bool _treat_as_2d;
  const MaterialProperty<SymmTensor> & _stress;
  const MaterialProperty<SymmTensor> & _strain;
  std::vector<const VariableGradient *> _grad_disp;
  const bool _has_temp;
  const VariableGradient & _grad_temp;
  const MaterialProperty<Real> * _current_instantaneous_thermal_expansion_coef;
  Real _K_factor;
  bool _has_symmetry_plane;
  Real _poissons_ratio;
  Real _youngs_modulus;
  unsigned int _ring_index;
  std::vector<Real> _q_curr_elem;
  const std::vector<std::vector<Real>> * _phi_curr_elem;
  const std::vector<std::vector<RealGradient>> * _dphi_curr_elem;
  Real _kappa;
  Real _shear_modulus;
  Real _r;
  Real _theta;

private:
  enum class QMethod
  {
    Geometry,
    Topology
  };

  const QMethod _q_function_type;

  enum class SifMethod
  {
    KI,
    KII,
    KIII,
    T
  };

  const SifMethod _sif_mode;
};

#endif // INTERACTIONINTEGRALSM_H
