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
 * A base class for the various Material related AuxKernal objects
 */
template <typename T, bool is_ad>
class MaterialStdVectorAuxBaseTempl : public MaterialAuxBaseTempl<std::vector<T>, is_ad>
{
public:
  static InputParameters validParams();

  MaterialStdVectorAuxBaseTempl(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// index of the vecor element
  unsigned int _index;

  // Explicitly declare the origin of the following inherited members
  // https://isocpp.org/wiki/faq/templates#nondependent-name-lookup-members
  using MaterialAuxBaseTempl<std::vector<T>, is_ad, Real>::_qp;
  using MaterialAuxBaseTempl<std::vector<T>, is_ad, Real>::_prop;
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

template <typename T, bool is_ad>
Real
MaterialStdVectorAuxBaseTempl<T, is_ad>::computeValue()
{
  mooseAssert(_prop[_qp].size() > _index,
              "MaterialStdVectorRealGradientAux: You chose to extract component "
                  << _index << " but your Material property only has size " << _prop[_qp].size());
  return MaterialAuxBaseTempl<std::vector<T>, is_ad>::computeValue();
}

template <typename T = Real>
using MaterialStdVectorAuxBase = MaterialStdVectorAuxBaseTempl<T, false>;
