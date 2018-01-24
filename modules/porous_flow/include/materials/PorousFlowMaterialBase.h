/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWMATERIALBASE_H
#define POROUSFLOWMATERIALBASE_H

#include "PorousFlowMaterial.h"
#include "DerivativeMaterialInterface.h"

class PorousFlowMaterialBase;

template <>
InputParameters validParams<PorousFlowMaterialBase>();

/**
 * Base class for all PorousFlow materials that provide phase-dependent properties.
 * These include: fluid properties, relative permeabilities and capillary pressures.
 * and relative permeability classes. This base class checks that the specified fluid
 * phase index is valid, and provides a stringified version of the phase index to use
 * in the material property names.
 */
class PorousFlowMaterialBase : public DerivativeMaterialInterface<PorousFlowMaterial>
{
public:
  PorousFlowMaterialBase(const InputParameters & parameters);

protected:
  /// Phase number of fluid
  const unsigned int _phase_num;

  /// Stringified fluid phase number
  const std::string _phase;
};

#endif // POROUSFLOWMATERIALBASE_H
