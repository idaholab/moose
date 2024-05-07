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

/**
 * A base class for the various Material related AuxKernal objects.
 * \p RT is short for return type
 */
template <typename T, bool is_ad, typename RT = Real>
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
  using MaterialAuxFunctorType =
      typename std::conditional<std::is_same_v<T, Real> || std::is_same_v<T, RealVectorValue>,
                                Moose::Functor<GenericType<T, is_ad>>,
                                void>::type;

protected:
  virtual RT computeValue() override;

  /// Perform a sanity check on teh retrieved value (e.g. to check dynamic sizes)
  virtual void checkFullValue() {}

  /// Returns material property values at quadrature points
  virtual RT getRealValue() = 0;

  /// Material property for this AuxKernel
  const GenericMaterialProperty<T, is_ad> * _property;

  /// Functor for this AuxKernel
  const MaterialAuxFunctorType * _functor;

  /// Evaluate at this quadrature point only
  const unsigned int _selected_qp;

  /// T Value evaluated from either the property or the functor
  GenericType<T, is_ad> _full_value;

private:
  /// Multiplier for the material property
  const Real _factor;

  /// Value to be added to the material property
  const RT _offset;
};

template <typename T, bool is_ad, typename RT>
InputParameters
MaterialAuxBaseTempl<T, is_ad, RT>::validParams()
{
  InputParameters params = AuxKernelTempl<RT>::validParams();
  params.addParam<MaterialPropertyName>(
      "property", "The scalar material property name (set either this or `functor`).");
  // functors do not support all types
  if constexpr (!std::is_same_v<MaterialAuxFunctorType, void>)
    params.addParam<MooseFunctorName>(
        "functor", "The scalar material property name (set either this or `property`).");
  params.addParam<Real>(
      "factor", 1, "The factor by which to multiply your material property for visualization");
  params.addParam<RT>("offset", 0, "The offset to add to your material property for visualization");

  params.addParam<unsigned int>(
      "selected_qp",
      "Evaluate the material property at a specified quadrature point. This only needs "
      "to be used if you are interested in a particular quadrature point in each element. "
      "Otherwise do not include this parameter in your input file.");
  params.addParamNamesToGroup("selected_qp", "Advanced");

  return params;
}

template <typename T, bool is_ad, typename RT>
MaterialAuxBaseTempl<T, is_ad, RT>::MaterialAuxBaseTempl(const InputParameters & parameters)
  : AuxKernelTempl<RT>(parameters),
    _property(this->isParamValid("property")
                  ? &this->template getGenericMaterialProperty<T, is_ad>("property")
                  : nullptr),
    _functor(this->isParamValid("functor")
                 ? [this]() {
                    if constexpr (!std::is_same_v<MaterialAuxFunctorType, void>)
                      return &this->template getFunctor<GenericType<T, is_ad>>("functor");
                    else
                    {
                      libmesh_ignore(this);
                      return nullptr;
                    }
                 }()
                 : nullptr),
    _selected_qp(this->isParamValid("selected_qp") ? this->template getParam<unsigned int>("selected_qp") : libMesh::invalid_uint),
    _factor(this->template getParam<Real>("factor")),
    _offset(this->template getParam<RT>("offset"))
{
  if (!_property == !_functor)
    mooseError("Specify either a `property` or a `functor` parameter.");
  if (_functor && _selected_qp != libMesh::invalid_uint)
    this->paramError("selected_qp",
                     "Selective quadrature point evaluation is not implemented for functors.");
}

template <typename T, bool is_ad, typename RT>
RT
MaterialAuxBaseTempl<T, is_ad, RT>::computeValue()
{
  // Material Property Values
  if (_property)
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
      _full_value = (*_property)[_selected_qp];
    }
    else
      _full_value = (*_property)[this->_qp];
  }

  // Functor Values
  if (_functor)
  {
    if constexpr (!std::is_same_v<MaterialAuxFunctorType, void>)
    {
      const auto elem_arg = this->makeElemArg(this->_current_elem);
      const auto state = this->determineState();
      _full_value = (*_functor)(elem_arg, state);
    }
    else
      mooseError("Unsupported functor type");
  }

  checkFullValue();

  return _factor * getRealValue() + _offset;
}

template <typename T = Real>
using MaterialAuxBase = MaterialAuxBaseTempl<T, false, Real>;
