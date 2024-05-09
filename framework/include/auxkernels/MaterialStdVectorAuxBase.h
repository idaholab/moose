//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MaterialAuxBase.h"

/**
 * A base class for the various Material related AuxKernel objects
 */
template <typename T, bool is_ad>
class MaterialStdVectorAuxBaseTempl : public MaterialAuxBaseTempl<std::vector<T>, is_ad>
{
public:
  static InputParameters validParams();

  MaterialStdVectorAuxBaseTempl(const InputParameters & parameters);

protected:
  /// index of the vector element
  unsigned int _index;

  using MaterialAuxBaseTempl<std::vector<T>, is_ad>::_full_value;
};

template <typename T, bool is_ad>
InputParameters
MaterialStdVectorAuxBaseTempl<T, is_ad>::validParams()
{
  InputParameters params = MaterialAuxBaseTempl<T, is_ad>::validParams();
  params.addParam<unsigned int>("index", 0, "The index to consider for this kernel");
  return params;
}

template <typename T, bool is_ad>
MaterialStdVectorAuxBaseTempl<T, is_ad>::MaterialStdVectorAuxBaseTempl(
    const InputParameters & parameters)
  : MaterialAuxBaseTempl<std::vector<T>, is_ad>(parameters),
    _index(this->template getParam<unsigned int>("index"))
{
}

template <typename T = Real>
using MaterialStdVectorAuxBase = MaterialStdVectorAuxBaseTempl<T, false>;
