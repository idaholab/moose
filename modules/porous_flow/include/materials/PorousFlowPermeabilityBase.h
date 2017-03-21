/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWPERMEABILITYBASE_H
#define POROUSFLOWPERMEABILITYBASE_H

#include "PorousFlowMaterialVectorBase.h"

// Forward Declarations
class PorousFlowPermeabilityBase;

template <>
InputParameters validParams<PorousFlowPermeabilityBase>();

/**
 * Base class Material designed to provide the permeability tensor.
 */
class PorousFlowPermeabilityBase : public PorousFlowMaterialVectorBase
{
public:
  PorousFlowPermeabilityBase(const InputParameters & parameters);

protected:
  /// quadpoint permeability
  MaterialProperty<RealTensorValue> & _permeability_qp;

  /// d(quadpoint permeability)/d(PorousFlow variable)
  MaterialProperty<std::vector<RealTensorValue>> & _dpermeability_qp_dvar;

  /// d(quadpoint permeability)/d(grad(PorousFlow variable))
  MaterialProperty<std::vector<std::vector<RealTensorValue>>> & _dpermeability_qp_dgradvar;
};

#endif // POROUSFLOWPERMEABILITYBASE_H
