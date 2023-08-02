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

#include "neml2/tensors/LabeledVector.h"
#include "neml2/tensors/LabeledMatrix.h"
#include "neml2/models/Model.h"

#include "NEML2SolidMechanicsInterface.h"
#include "BatchMaterial.h"

#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include "SymmetricRankTwoTensor.h"
#include "SymmetricRankFourTensor.h"

typedef BatchMaterial<BatchMaterialUtils::TupleStd,
                      // Outputs: stress, internal variables, dstress/dstrain
                      std::tuple<RankTwoTensor, RankFourTensor>,
                      // Inputs:
                      //   strain
                      //   temperature
                      BatchMaterialUtils::GatherMatProp<RankTwoTensor>,
                      BatchMaterialUtils::GatherVariable>
    CauchyStressFromNEML2UOParent;

/**
 * This userobject gathers input variables required for an objective stress integration from all
 * quadrature points. The batched input vector is sent through a NEML2 material model to perform the
 * constitutive update.
 */
class CauchyStressFromNEML2UO : public NEML2SolidMechanicsInterface<CauchyStressFromNEML2UOParent>
{
public:
  static InputParameters validParams();

  CauchyStressFromNEML2UO(const InputParameters & params);

  virtual void timestepSetup() override;

  virtual void batchCompute() override;

protected:
  /// Advance state and forces in time
  virtual void advanceStep();

  /// Update the forces driving the material model update
  virtual void updateForces();

  /// Apply the predictor to set current trial state
  virtual void applyPredictor();

  /// Perform the material update
  virtual void solve();

  /// The input vector of the material model
  neml2::LabeledVector _in;

  /// The output vector of the material model
  neml2::LabeledVector _out;

  /// The derivative of the output vector w.r.t. the input vector
  neml2::LabeledMatrix _dout_din;
};
