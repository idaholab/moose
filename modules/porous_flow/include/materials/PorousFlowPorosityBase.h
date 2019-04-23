//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowMaterialVectorBase.h"

class PorousFlowPorosityBase;

template <>
InputParameters validParams<PorousFlowPorosityBase>();

/**
 * Base class Material designed to provide the porosity.
 */
class PorousFlowPorosityBase : public PorousFlowMaterialVectorBase
{
public:
  PorousFlowPorosityBase(const InputParameters & parameters);

protected:
  /// Computed porosity at the nodes or quadpoints
  MaterialProperty<Real> & _porosity;

  /// d(porosity)/d(PorousFlow variable)
  MaterialProperty<std::vector<Real>> & _dporosity_dvar;

  /// d(porosity)/d(grad PorousFlow variable)
  MaterialProperty<std::vector<RealGradient>> & _dporosity_dgradvar;
};

