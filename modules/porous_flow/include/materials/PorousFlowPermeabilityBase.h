//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
