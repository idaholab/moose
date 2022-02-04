//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVElementalKernel.h"
#include "INSFVMomentumResidualObject.h"

/**
 * Body force that contributes to the Rhie-Chow interpolation
 */
class INSFVBodyForce : public FVElementalKernel, public INSFVMomentumResidualObject
{
public:
  INSFVBodyForce(const InputParameters & params);
  static InputParameters validParams();

  void gatherRCData(const Elem &) override {}
  void gatherRCData(const FaceInfo &) override {}

protected:
  ADReal computeQpResidual() override;

  /// Scale factor
  const Real & _scale;

  /// The functor describing the body force
  const Moose::Functor<ADReal> & _functor;

  /// Optional Postprocessor value
  const PostprocessorValue & _postprocessor;
};
