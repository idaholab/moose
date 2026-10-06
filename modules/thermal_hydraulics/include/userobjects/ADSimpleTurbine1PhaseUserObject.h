//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADJunctionParallelChannels1PhaseUserObject.h"

class ADSimpleTurbine1PhaseUserObject;

/**
 * Computes and caches flux and residual vectors for a 1-phase turbine
 *
 * This class computes and caches the following quantities:
 * \li residuals for the scalar variables associated with the turbine, and
 * \li fluxes between the flow channels and the turbine.
 */
class ADSimpleTurbine1PhaseUserObject : public ADJunctionParallelChannels1PhaseUserObject
{
public:
  ADSimpleTurbine1PhaseUserObject(const InputParameters & params);

protected:
  virtual void computeFluxesAndResiduals(const unsigned int & c) override;

  /// Function determining if turbine is operating or not (0=false, 1=true)
  const Function & _on_fn;
  /// Function specifying the turbine power [W]
  const Function & _W_dot_fn;

public:
  static InputParameters validParams();
};
