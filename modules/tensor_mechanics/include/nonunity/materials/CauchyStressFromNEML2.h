/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/*                       BlackBear                              */
/*                                                              */
/*           (c) 2017 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#pragma once

#ifdef NEML2_ENABLED
#include "neml2/tensors/LabeledVector.h"
#include "neml2/tensors/LabeledMatrix.h"
#endif

#include "NEML2SolidMechanicsInterface.h"
#include "ComputeLagrangianObjectiveStress.h"

/**
 * This material performs the objective stress update using a NEML2 material model.
 */
class CauchyStressFromNEML2 : public NEML2SolidMechanicsInterface<ComputeLagrangianObjectiveStress>
{
public:
  static InputParameters validParams();
  CauchyStressFromNEML2(const InputParameters & parameters);

protected:
  virtual void computeQpSmallStress() override {}

#ifdef NEML2_ENABLED
public:
  virtual void initialSetup() override;
  virtual void computeProperties() override;

protected:
  virtual void initQpStatefulProperties() override;

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
  std::map<neml2::LabeledAxisAccessor, MaterialProperty<std::vector<Real>> *> _state_vars;

  /// The old state variables of the NEML2 material model (stored as MOOSE material properties)
  std::map<neml2::LabeledAxisAccessor, const MaterialProperty<std::vector<Real>> *> _state_vars_old;
#endif
};
