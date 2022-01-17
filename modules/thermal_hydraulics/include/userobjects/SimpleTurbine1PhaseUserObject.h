#pragma once

#include "VolumeJunction1PhaseUserObject.h"

class SimpleTurbine1PhaseUserObject;

/**
 * Computes and caches flux and residual vectors for a 1-phase turbine
 *
 * This class computes and caches the following quantities:
 * \li residuals for the scalar variables associated with the turbine, and
 * \li fluxes between the flow channels and the turbine.
 */
class SimpleTurbine1PhaseUserObject : public VolumeJunction1PhaseUserObject
{
public:
  SimpleTurbine1PhaseUserObject(const InputParameters & params);

protected:
  virtual void computeFluxesAndResiduals(const unsigned int & c) override;

  /// Flag determining if turbine is operating or not
  const bool & _on;
  /// Turbine power, [W]
  const Real & _W_dot;

public:
  static InputParameters validParams();
};
