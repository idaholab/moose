//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVElementalKernel.h"

/**
 * Body force that contributes to the Rhie-Chow interpolation
 */
class INSFVBodyForce : public INSFVElementalKernel
{
public:
  INSFVBodyForce(const InputParameters & params);
  static InputParameters validParams();

  using INSFVElementalKernel::gatherRCData;
  void gatherRCData(const Elem &) override;

protected:
  /// Scale factor
  const Real & _scale;

  /// The functor describing the body force
  const Moose::Functor<ADReal> & _functor;

  /// Optional Postprocessor value
  const PostprocessorValue & _postprocessor;
};
