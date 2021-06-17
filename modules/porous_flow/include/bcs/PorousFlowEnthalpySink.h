//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"

class SinglePhaseFluidProperties;
class PorousFlowDictator;
class Function;

/**
 * Applies a flux sink of heat energy to a boundary with specified mass flux and inlet temperature.
 */
class PorousFlowEnthalpySink : public IntegratedBC
{
public:
  static InputParameters validParams();

  PorousFlowEnthalpySink(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// PorousFlowDictator UserObject
  const PorousFlowDictator & _dictator;

  /// Pressure (from aux variable)
  const VariableValue * _pressure;

  /// The phase number
  const unsigned int _ph;

  /// The mass flux
  const Function & _m_func;

  /// Computed nodal values of porepressure of the phases
  const MaterialProperty<std::vector<Real>> & _pp;

  /// d(porepressure)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> & _dpp_dvar;

  /// Specified inlet temperature
  const Real & _T_in;

  /// Fluid properties UserObject
  const SinglePhaseFluidProperties & _fp;

  /// Derivative of residual with respect to the jvar variable
  Real jac(unsigned int jvar) const;
};
