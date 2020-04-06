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

/**
 * Base class for materials that provide thermal conducitivity
 */
class PorousFlowThermalConductivityBase : public PorousFlowMaterialVectorBase
{
public:
  static InputParameters validParams();

  PorousFlowThermalConductivityBase(const InputParameters & parameters);

protected:
  /// Thermal conducitivity at the qps
  MaterialProperty<RealTensorValue> & _la_qp;

  /// d(thermal conductivity at the qps)/d(PorousFlow variable)
  MaterialProperty<std::vector<RealTensorValue>> & _dla_qp_dvar;
};
