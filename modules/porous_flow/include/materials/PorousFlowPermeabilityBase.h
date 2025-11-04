//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowMaterialVectorBase.h"

/**
 * Base class Material designed to provide the permeability tensor.
 */
template <bool is_ad>
class PorousFlowPermeabilityBaseTempl : public PorousFlowMaterialVectorBase
{
public:
  static InputParameters validParams();

  PorousFlowPermeabilityBaseTempl(const InputParameters & parameters);

protected:
  /// Quadpoint permeability
  GenericMaterialProperty<RealTensorValue, is_ad> & _permeability_qp;

  /// d(quadpoint permeability)/d(PorousFlow variable)
  MaterialProperty<std::vector<RealTensorValue>> * const _dpermeability_qp_dvar;

  /// d(quadpoint permeability)/d(grad(PorousFlow variable))
  MaterialProperty<std::vector<std::vector<RealTensorValue>>> * const _dpermeability_qp_dgradvar;
};

#define usingPorousFlowPermeabilityBaseMembers                                                     \
  using PorousFlowPermeabilityBaseTempl<is_ad>::_dpermeability_qp_dgradvar;                        \
  using PorousFlowPermeabilityBaseTempl<is_ad>::_dpermeability_qp_dvar;                            \
  using PorousFlowPermeabilityBaseTempl<is_ad>::_num_var;                                          \
  using PorousFlowPermeabilityBaseTempl<is_ad>::_permeability_qp;                                  \
  using PorousFlowPermeabilityBaseTempl<is_ad>::_qp;                                               \
  using PorousFlowPermeabilityBaseTempl<is_ad>::_dictator;                                         \
  using Coupleable::coupledValue

typedef PorousFlowPermeabilityBaseTempl<false> PorousFlowPermeabilityBase;
typedef PorousFlowPermeabilityBaseTempl<true> ADPorousFlowPermeabilityBase;
