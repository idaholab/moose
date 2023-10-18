//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementIntegralPostprocessor.h"

/**
 * Homogenization of Temperature-Dependent Thermal Conductivity in Composite
 * Materials, Journal of Thermophysics and Heat Transfer, Vol. 15, No. 1,
 * January-March 2001.
 */
class HomogenizedThermalConductivity : public ElementIntegralPostprocessor
{
public:
  static InputParameters validParams();

  HomogenizedThermalConductivity(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  using Postprocessor::getValue;
  virtual Real getValue() const override;
  virtual void threadJoin(const UserObject & y) override;
  virtual void finalize() override;

protected:
  virtual Real computeQpIntegral() override;

  /// the row index of the homogenized thermal conductivity tensor that is returned
  const unsigned int _row;
  /// the column index of the homogenized thermal conductivity tensor that is returned
  const unsigned int _col;
  /// a scale factor multiplied to the result
  const Real _scale;
  /// dimension of the mesh
  const unsigned int _dim;
  ///@{ heterogeneous diffusion coefficient as scalar and tensor
  const MaterialProperty<Real> * _diffusion_coefficient;
  const MaterialProperty<RankTwoTensor> * _tensor_diffusion_coefficient;
  ///@}
  /// the gradients of the characteristic functions usually denoted chi in the literature
  std::vector<const VariableGradient *> _grad_chi;
  /// volume of the integration domain
  Real _volume;
  /// the integral value that is being accumulated
  Real _integral_value;
};
