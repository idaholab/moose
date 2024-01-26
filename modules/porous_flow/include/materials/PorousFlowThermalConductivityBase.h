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
template <bool is_ad>
class PorousFlowThermalConductivityBaseTempl : public PorousFlowMaterialVectorBase
{
public:
  static InputParameters validParams();

  PorousFlowThermalConductivityBaseTempl(const InputParameters & parameters);

protected:
  /// Thermal conducitivity at the qps
  GenericMaterialProperty<RealTensorValue, is_ad> & _la_qp;

  /// d(thermal conductivity at the qps)/d(PorousFlow variable)
  MaterialProperty<std::vector<RealTensorValue>> * const _dla_qp_dvar;
};

#define usingPorousFlowThermalConductivityMembers                                                  \
  using PorousFlowThermalConductivityBaseTempl<is_ad>::_dictator;                                  \
  using PorousFlowThermalConductivityBaseTempl<is_ad>::_num_phases;                                \
  using PorousFlowThermalConductivityBaseTempl<is_ad>::_num_var;                                   \
  using PorousFlowThermalConductivityBaseTempl<is_ad>::_qp;                                        \
  using PorousFlowThermalConductivityBaseTempl<is_ad>::_la_qp;                                     \
  using PorousFlowThermalConductivityBaseTempl<is_ad>::_dla_qp_dvar
