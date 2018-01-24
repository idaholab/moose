//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOWMATERIALVECTORBASE_H
#define POROUSFLOWMATERIALVECTORBASE_H

#include "PorousFlowMaterial.h"
#include "DerivativeMaterialInterface.h"

class PorousFlowMaterialVectorBase;

template <>
InputParameters validParams<PorousFlowMaterialVectorBase>();

/**
 * Base class for all PorousFlow vector materials
 */
class PorousFlowMaterialVectorBase : public DerivativeMaterialInterface<PorousFlowMaterial>
{
public:
  PorousFlowMaterialVectorBase(const InputParameters & parameters);

protected:
  /// Number of phases
  const unsigned int _num_phases;

  /// Number of fluid components
  const unsigned int _num_components;

  /// Number of PorousFlow variables
  const unsigned int _num_var;
};

#endif // POROUSFLOWMATERIALVECTORBASE_H
