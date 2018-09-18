//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef JINTEGRAL_H
#define JINTEGRAL_H

#include "ElementIntegralPostprocessor.h"
#include "CrackFrontDefinition.h"

// Forward Declarations
class JIntegral;
template <typename>
class RankTwoTensorTempl;
typedef RankTwoTensorTempl<Real> RankTwoTensor;

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
  virtual Real computeIntegral();
  const CrackFrontDefinition * const _crack_front_definition;
  bool _has_crack_front_point_index;
  const unsigned int _crack_front_point_index;
  bool _treat_as_2d;
  const MaterialProperty<RankTwoTensor> & _Eshelby_tensor;
  const MaterialProperty<RealVectorValue> * _J_thermal_term_vec;
  bool _convert_J_to_K;
  bool _has_symmetry_plane;
  Real _poissons_ratio;
  Real _youngs_modulus;
  unsigned int _ring_index;
  unsigned int _ring_first;
  MooseEnum _q_function_type;
  std::vector<Real> _q_curr_elem;
  const std::vector<std::vector<Real>> * _phi_curr_elem;
  const std::vector<std::vector<RealGradient>> * _dphi_curr_elem;
};

#endif // JINTEGRAL3D_H
