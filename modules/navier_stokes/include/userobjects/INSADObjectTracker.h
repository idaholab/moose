//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"
#include "MooseTypes.h"
#include "MooseEnum.h"
#include "MooseError.h"

#include "libmesh/vector_value.h"

template <typename>
class ADMaterialProperty;
template <typename>
class MaterialProperty;

/**
 * Object for tracking what kernels have been added to an INSAD simulation. This is used to
 * determine what properties to calculate in the INSADMaterial, which is important particularly for
 * ensuring that we have consistenly included all the strong terms for stabilization methods like
 * PSPG and SUPG
 */
class INSADObjectTracker : public GeneralUserObject
{
public:
  static InputParameters validParams()
  {
    InputParameters params = GeneralUserObject::validParams();
    params.addClassDescription("User object used to track the kernels added to an INS simulation "
                               "and determine what properties to calculate in INSADMaterial");
    return params;
  }

  INSADObjectTracker(const InputParameters & parameters) : GeneralUserObject(parameters) {}

  virtual void initialize() final {}
  virtual void execute() final {}
  virtual void finalize() final {}

  bool hasBoussinesq() const { return _has_boussinesq; }
  bool hasGravity() const { return _has_gravity; }
  bool hasTransient() const { return _has_transient; }
  bool integratePByParts() const
  {
    if (!_integrate_p_by_parts_set)
      mooseError("Requesting integrate_p_by_parts, but it has not yet been set");
    return _integrate_p_by_parts;
  }
  const std::string & viscousForm() const
  {
    if (!_viscous_form_set)
      mooseError("Requesting the viscous form, but it has not yet been set");
    return _viscous_form;
  }

  void setHasBoussinesq(bool has_boussinesq) { _has_boussinesq = has_boussinesq; }
  void setHasGravity(bool has_gravity) { _has_gravity = has_gravity; }
  void setHasTransient(bool has_transient) { _has_transient = has_transient; }
  void setIntegratePByParts(bool integrate_p_by_parts)
  {
    if (_integrate_p_by_parts_set && (integrate_p_by_parts != _integrate_p_by_parts))
      mooseError("Two INSAD objects have set different values for integrate_p_by_parts");

    _integrate_p_by_parts = integrate_p_by_parts;
    _integrate_p_by_parts_set = true;
  }

  void setViscousForm(const MooseEnum & viscous_form)
  {
    if (!(viscous_form == "laplace" || viscous_form == "traction"))
      mooseError("invalid value for viscous_form");
    if (_viscous_form_set && (_viscous_form != static_cast<std::string>(viscous_form)))
      mooseError("Two INSAD objects have set different values for the viscous form");

    _viscous_form = static_cast<std::string>(viscous_form);
    _viscous_form_set = true;
  }

  const RealVectorValue & gravityVector() const { return _gravity_vector; }
  void setGravityVector(const RealVectorValue & gravity_vector)
  {
    if (_gravity_vector_set && (_gravity_vector != gravity_vector))
      mooseError("Two INSAD objects are using inconsistent values for the gravity vector");
    _gravity_vector = gravity_vector;
    _gravity_vector_set = true;
  }

  const ADMaterialProperty<Real> * alpha() const
  {
    if (!_alpha_set)
      mooseError("Requesting the the thermal expansion coefficieint, but it has not been set yet");
    return _alpha;
  }
  void setAlpha(const ADMaterialProperty<Real> * alpha)
  {
    if (_alpha_set && (_alpha != alpha))
      mooseError(
          "Two INSAD objects have set different thermal expansion coefficient material properties");
    _alpha = alpha;
    _alpha_set = true;
  }

  const MaterialProperty<Real> * tRef() const
  {
    if (!_t_ref_set)
      mooseError("Requesting the reference temperature, but it has not been set yet");
    return _t_ref;
  }
  void setTRef(const MaterialProperty<Real> * t_ref)
  {
    if (_t_ref_set && (_t_ref != t_ref))
      mooseError("Two INSAD objects have set reference temperature material properties");
    _t_ref = t_ref;
    _t_ref_set = true;
  }

  const ADVariableValue * t() const
  {
    if (!_t_set)
      mooseError("Requesting the temperature variable, but it has not been set yet");
    return _t;
  }
  void setT(const ADVariableValue * t)
  {
    if (_t_set && (_t != t))
      mooseError("Two INSAD objects have set temperature variable values");
    _t = t;
    _t_set = true;
  }

private:
  bool _has_boussinesq = false;
  bool _has_gravity = false;
  bool _has_transient = false;
  bool _integrate_p_by_parts = true;
  bool _integrate_p_by_parts_set = false;

  /// Default constructor initializes every component to zero
  RealVectorValue _gravity_vector;

  bool _gravity_vector_set = false;

  /// the thermal expansion coefficient
  const ADMaterialProperty<Real> * _alpha = nullptr;

  bool _alpha_set = false;

  /// the reference temperature
  const MaterialProperty<Real> * _t_ref = nullptr;

  bool _t_ref_set = false;

  /// the temperature
  const ADVariableValue * _t = nullptr;

  bool _t_set = false;

  /// The form the of the viscous term. Options are either laplace or traction
  std::string _viscous_form;

  bool _viscous_form_set = false;
};
