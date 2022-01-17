#pragma once

#include "RadiativeHeatFluxBCBase.h"

/**
 * Radiative heat transfer boundary condition for a plate heat structure
 */
class ADRadiativeHeatFluxBC : public ADRadiativeHeatFluxBCBase
{
public:
  ADRadiativeHeatFluxBC(const InputParameters & parameters);

protected:
  virtual ADReal coefficient() const override;

  /// View factor function
  const Function & _view_factor_fn;

  /// Post-processor by which to scale boundary condition
  const PostprocessorValue & _scale_pp;

public:
  static InputParameters validParams();
};
