//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifdef NEML2_ENABLED
#include "neml2/tensors/LabeledVector.h"
#include "neml2/tensors/LabeledMatrix.h"
#endif

#include "ComputeLagrangianObjectiveStress.h"
#include "NEML2SolidMechanicsInterface.h"

/**
 * This material performs the objective stress update using a NEML2 material model.
 */
class CauchyStressFromNEML2 : public NEML2SolidMechanicsInterface<ComputeLagrangianObjectiveStress>
{
public:
  static InputParameters validParams();
  CauchyStressFromNEML2(const InputParameters & parameters);

#ifndef NEML2_ENABLED
protected:
  virtual void computeQpSmallStress() override {}

#else
  virtual void initialSetup() override;
  virtual void computeProperties() override;

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpSmallStress() override;

  /// Advance state and forces in time
  virtual void advanceStep();

  /// Update the forces driving the material model update
  virtual void updateForces();

  /// Apply the predictor to set current trial state
  virtual void applyPredictor();

  /// Perform the material update
  virtual void solve();

  // @{ Variables and properties computed by MOOSE
  /// The old mechanical strain
  const MaterialProperty<RankTwoTensor> * _mechanical_strain_old;

  /// The temperature
  const VariableValue * _temperature;

  /// The old temperature
  const VariableValue * _temperature_old;
  // @}

  /// The input vector of the material model
  neml2::LabeledVector _in;

  /// The output vector of the material model
  neml2::LabeledVector _out;

  /// The derivative of the output vector w.r.t. the input vector
  neml2::LabeledMatrix _dout_din;

  /// The state variables of the NEML2 material model (stored as MOOSE material properties)
  std::map<neml2::VariableName, MaterialProperty<std::vector<Real>> *> _state_vars;

  /// The old state variables of the NEML2 material model (stored as MOOSE material properties)
  std::map<neml2::VariableName, const MaterialProperty<std::vector<Real>> *> _state_vars_old;

#endif // NEML2_ENABLED
};
