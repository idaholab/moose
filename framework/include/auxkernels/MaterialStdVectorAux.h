//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MaterialStdVectorAuxBase.h"

/**
 * AuxKernel for outputting a std::vector material-property component to an AuxVariable
 */
template <bool is_ad>
class MaterialStdVectorAuxTempl : public MaterialStdVectorAuxBaseTempl<Real, is_ad>
{
public:
  static InputParameters validParams();

  MaterialStdVectorAuxTempl(const InputParameters & parameters);

protected:
  virtual Real getRealValue() override;

  /// whether or not selected_qp has been set
  const bool _has_selected_qp;

  /// The std::vector will be evaluated at this quadpoint only
  const unsigned int _selected_qp;

  using MaterialStdVectorAuxBaseTempl<Real, is_ad>::_qp;
  using MaterialStdVectorAuxBaseTempl<Real, is_ad>::_q_point;
  using MaterialStdVectorAuxBaseTempl<Real, is_ad>::_prop;
  using MaterialStdVectorAuxBaseTempl<Real, is_ad>::_index;
};

typedef MaterialStdVectorAuxTempl<false> MaterialStdVectorAux;
typedef MaterialStdVectorAuxTempl<true> ADMaterialStdVectorAux;
