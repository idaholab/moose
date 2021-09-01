#pragma once

#include "ADKernel.h"

class ADHeatFluxFromHeatStructureBaseUserObject;

class ADOneDHeatFluxBase : public ADKernel
{
public:
  ADOneDHeatFluxBase(const InputParameters & parameters);

protected:
  /// User object that computes the heat flux
  const ADHeatFluxFromHeatStructureBaseUserObject & _q_uo;

public:
  static InputParameters validParams();
};
