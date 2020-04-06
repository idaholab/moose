#pragma once

#include "RadiativeHeatFluxBCBase.h"

/**
 * Radiative heat transfer boundary condition for a plate heat structure
 */
class RadiativeHeatFluxBC : public RadiativeHeatFluxBCBase
{
public:
  RadiativeHeatFluxBC(const InputParameters & parameters);

protected:
  virtual Real coefficient() const override;

  /// View factor function
  const Function & _view_factor_fn;

  /// Post-processor by which to scale boundary condition
  const PostprocessorValue & _scale_pp;

public:
  static InputParameters validParams();
};
