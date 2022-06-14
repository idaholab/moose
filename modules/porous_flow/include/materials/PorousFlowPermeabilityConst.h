//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowPermeabilityBase.h"

/**
 * Material designed to provide a constant permeability tensor
 */
template <bool is_ad>
class PorousFlowPermeabilityConstTempl : public PorousFlowPermeabilityBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowPermeabilityConstTempl(const InputParameters & parameters);

protected:
  void computeQpProperties() override;

  /// Constant value of permeability tensor
  const RealTensorValue _input_permeability;

  using PorousFlowPermeabilityBaseTempl<is_ad>::_dpermeability_qp_dgradvar;
  using PorousFlowPermeabilityBaseTempl<is_ad>::_dpermeability_qp_dvar;
  using PorousFlowPermeabilityBaseTempl<is_ad>::_num_var;
  using PorousFlowPermeabilityBaseTempl<is_ad>::_permeability_qp;
  using PorousFlowPermeabilityBaseTempl<is_ad>::_qp;
};

typedef PorousFlowPermeabilityConstTempl<false> PorousFlowPermeabilityConst;
typedef PorousFlowPermeabilityConstTempl<true> ADPorousFlowPermeabilityConst;
