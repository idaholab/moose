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
#include "AuxKernel.h"
#include "Assembly.h"

// Forward declarations
template <typename T, bool is_ad, bool is_functor, typename RT>
struct PropSetter;

/**
 * A base class for the various Material related AuxKernal objects.
 * \p RT is short for return type
 */
template <typename T, bool is_ad, bool is_functor = false, typename RT = Real>
class MaterialAuxBaseTempl : public AuxKernelTempl<RT>
{
public:
  static InputParameters validParams();

  /**
   * Class constructor
   * @param parameters The input parameters for this object
   */
  MaterialAuxBaseTempl(const InputParameters & parameters);

  /// Functors really only work for Real and RealVectorValue for now :(
  using PropertyType = typename std::conditional<is_functor,
                                                 Moose::Functor<Moose::GenericType<T, is_ad>>,
                                                 GenericMaterialProperty<T, is_ad>>::type;

protected:
  virtual RT computeValue() override;

  /// Perform a sanity check on teh retrieved value (e.g. to check dynamic sizes)
  virtual void checkFullValue() {}

  /// Returns material property values at quadrature points
  virtual RT getRealValue() = 0;

  /// (Functor)Material property for this AuxKernel
  const PropertyType & _prop;

  /// Evaluate at this quadrature point only
  const unsigned int _selected_qp;

  /// T Value evaluated from either the property or the functor
  Moose::GenericType<T, is_ad> _full_value;

private:
  /// Multiplier for the material property
  const Real _factor;

  /// Value to be added to the material property
  const RT _offset;

  /// ID of the subdomain currently being iterated over
  const SubdomainID & _current_subdomain_id;

  /// Hack to avoid "dangling reference" warnings from gcc 13
  friend struct PropSetter<T, is_ad, is_functor, RT>;
};

template <typename T, bool is_ad, bool is_functor, typename RT>
InputParameters
MaterialAuxBaseTempl<T, is_ad, is_functor, RT>::validParams()
{
  InputParameters params = AuxKernelTempl<RT>::validParams();
  if constexpr (is_functor)
    params.addRequiredParam<MooseFunctorName>("functor", "The functor name.");
  else
    params.addRequiredParam<MaterialPropertyName>("property", "The material property name.");

  params.addParam<Real>(
      "factor", 1, "The factor by which to multiply your material property for visualization");
  params.addParam<RT>("offset", 0, "The offset to add to your material property for visualization");

  if constexpr (!is_functor)
  {
    params.addParam<unsigned int>(
        "selected_qp",
        "Evaluate the material property at a specified quadrature point. This only needs "
        "to be used if you are interested in a particular quadrature point in each element. "
        "Otherwise do not include this parameter in your input file.");
    params.addParamNamesToGroup("selected_qp", "Advanced");
  }

  return params;
}

template <typename T, bool is_ad, bool is_functor, typename RT>
struct PropSetter
{
  static const Moose::Functor<Moose::GenericType<T, is_ad>> &
  prop(MaterialAuxBaseTempl<T, is_ad, is_functor, RT> & mabt)
  {
    return mabt.template getFunctor<Moose::GenericType<T, is_ad>>("functor");
  }
};

template <typename T, bool is_ad, typename RT>
struct PropSetter<T, is_ad, false, RT>
{
  static const GenericMaterialProperty<T, is_ad> &
  prop(MaterialAuxBaseTempl<T, is_ad, false, RT> & mabt)
  {
    return mabt.template getGenericMaterialProperty<T, is_ad>("property");
  }
};

template <typename T, bool is_ad, bool is_functor, typename RT>
MaterialAuxBaseTempl<T, is_ad, is_functor, RT>::MaterialAuxBaseTempl(
    const InputParameters & parameters)
  : AuxKernelTempl<RT>(parameters),
    _prop(PropSetter<T, is_ad, is_functor, RT>::prop(*this)),
    _selected_qp(this->isParamValid("selected_qp")
                     ? this->template getParam<unsigned int>("selected_qp")
                     : libMesh::invalid_uint),
    _factor(this->template getParam<Real>("factor")),
    _offset(this->template getParam<RT>("offset")),
    _current_subdomain_id(this->_assembly.currentSubdomainID())
{
}

template <typename T, bool is_ad, bool is_functor, typename RT>
RT
MaterialAuxBaseTempl<T, is_ad, is_functor, RT>::computeValue()
{
  // Functor Values
  if constexpr (is_functor)
  {
    if (this->isNodal())
    {
      const Moose::NodeArg node_arg{this->_current_node, _current_subdomain_id};
      const auto state = this->determineState();
      _full_value = _prop(node_arg, state);
    }
    else
    {
      const auto elem_arg = this->makeElemArg(this->_current_elem);
      const auto state = this->determineState();
      _full_value = _prop(elem_arg, state);
    }
  }
  // Material Properties
  else
  {
    if (_selected_qp != libMesh::invalid_uint)
    {
      if (_selected_qp >= this->_q_point.size())
      {
        Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
        this->paramError("selected_qp",
                         "Trying to evaluate qp ",
                         _selected_qp,
                         " but there are only ",
                         this->_q_point.size(),
                         " quadrature points in the element");
      }
      _full_value = _prop[_selected_qp];
    }
    else
      _full_value = _prop[this->_qp];
  }

  checkFullValue();
  return _factor * getRealValue() + _offset;
}

template <typename T = Real>
using MaterialAuxBase = MaterialAuxBaseTempl<T, false, false, Real>;
