//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceKernel.h"

/**
 * InterfaceKernel for modeling heat transfer across a thin layer.
 */
class ThinLayerHeatTransfer : public InterfaceKernel
{
public:
  static InputParameters validParams();
  ThinLayerHeatTransfer(const InputParameters &);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type) override;
  virtual Real computeQpJacobian(Moose::DGJacobianType type) override;

  /// The specific heat material property of the thin layer
  const MaterialProperty<Real> & _specific_heat;

  /// The density material property of the thin layer
  const MaterialProperty<Real> & _density;

  /// The heat source material property of the thin layer
  const MaterialProperty<Real> & _heat_source;

  /// The thermal conductivity material property of the thin layer
  const MaterialProperty<Real> & _thermal_conductivity;

  // The thin layer thickness
  const Real _thickness;

  /// Time derivative of temperature variable
  const VariableValue & _u_dot;

  /// Derivative of u_dot with respect to u
  const VariableValue & _du_dot_du;

  /// Time derivative of neighbor temperature variable
  const VariableValue & _u_dot_neighbor;

  /// Derivative of neighbor_value_dot with respect to u
  const VariableValue & _du_dot_du_neighbor;
};
