//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVAdvectionBase.h"

#ifdef MOOSE_GLOBAL_AD_INDEXING

#include "InputParameters.h"
#include "SubProblem.h"
#include "MooseVariableFV.h"

std::unordered_map<const MooseApp *, std::vector<std::unordered_map<const Elem *, ADReal>>>
    NSFVAdvectionBase::_rc_a_coeffs;

InputParameters
NSFVAdvectionBase::validParams()
{
  InputParameters params = emptyInputParameters();
  params.addRequiredCoupledVar("pressure", "The pressure variable.");
  params.addRequiredCoupledVar("u", "The velocity in the x direction.");
  params.addCoupledVar("v", "The velocity in the y direction.");
  params.addCoupledVar("w", "The velocity in the z direction.");

  MooseEnum velocity_interp_method("average rc", "rc");

  params.addParam<MooseEnum>(
      "velocity_interp_method",
      velocity_interp_method,
      "The interpolation to use for the velocity. Options are "
      "'average' and 'rc' which stands for Rhie-Chow. The default is Rhie-Chow.");

  params.addRequiredParam<Real>("mu", "The viscosity");
  params.addRequiredParam<Real>("rho", "The density");
  return params;
}

NSFVAdvectionBase::NSFVAdvectionBase(const InputParameters & params)
  : _nsfv_app(*params.getCheckedPointerParam<MooseApp *>("_moose_app")),
    _nsfv_subproblem(*params.getCheckedPointerParam<SubProblem *>("_subproblem")),
    _nsfv_tid(params.get<THREAD_ID>("_tid")),
    _p_var(dynamic_cast<const MooseVariableFV<Real> *>(&_nsfv_subproblem.getVariable(
        _nsfv_tid, params.get<std::vector<VariableName>>("pressure").front()))),
    _u_var(dynamic_cast<const MooseVariableFV<Real> *>(&_nsfv_subproblem.getVariable(
        _nsfv_tid, params.get<std::vector<VariableName>>("u").front()))),
    _v_var(params.isParamValid("v")
               ? dynamic_cast<const MooseVariableFV<Real> *>(&_nsfv_subproblem.getVariable(
                     _nsfv_tid, params.get<std::vector<VariableName>>("v").front()))
               : nullptr),
    _w_var(params.isParamValid("w")
               ? dynamic_cast<const MooseVariableFV<Real> *>(&_nsfv_subproblem.getVariable(
                     _nsfv_tid, params.get<std::vector<VariableName>>("w").front()))
               : nullptr),
    _mu(params.get<Real>("mu")),
    _rho(params.get<Real>("rho"))
{
  if (!_p_var)
    mooseError("the pressure must be a finite volume variable.");

  if (!_u_var)
    mooseError("the u velocity must be a finite volume variable.");

  if (_nsfv_subproblem.mesh().dimension() >= 2 && !_v_var)
    mooseError(
        "In two or more dimensions, the v velocity must be supplied and it must be a finite volume "
        "variable.");

  if (_nsfv_subproblem.mesh().dimension() >= 3 && !params.isParamValid("w"))
    mooseError("In three-dimensions, the w velocity must be supplied and it must be a finite "
               "volume variable.");

  const auto & velocity_interp_method = params.get<MooseEnum>("velocity_interp_method");
  if (velocity_interp_method == "average")
    _velocity_interp_method = Moose::FV::InterpMethod::Average;
  else if (velocity_interp_method == "rc")
    _velocity_interp_method = Moose::FV::InterpMethod::RhieChow;
  else
    mooseError("Unrecognized interpolation type ",
               static_cast<std::string>(velocity_interp_method));

  if (_nsfv_tid == 0)
  {
    auto & vec_of_coeffs_map = _rc_a_coeffs[&_nsfv_app];
    vec_of_coeffs_map.resize(libMesh::n_threads());
  }
}

const ADReal &
NSFVAdvectionBase::rcCoeff(const Elem & elem) const
{
  auto it = _rc_a_coeffs.find(&_nsfv_app);
  mooseAssert(it != _rc_a_coeffs.end(),
              "No RC coeffs structure exists for the given MooseApp pointer");
  mooseAssert(_nsfv_tid < it->second.size(),
              "The RC coeffs structure size "
                  << it->second.size() << " is greater than or equal to the provided thread ID "
                  << _nsfv_tid);
  auto & my_map = it->second[_nsfv_tid];

  auto rc_map_it = my_map.find(&elem);

  if (rc_map_it != my_map.end())
    return rc_map_it->second;

  // Returns a pair with the first being an iterator pointing to the key-value pair and the second a
  // boolean denoting whether a new insertion took place
  auto emplace_ret = my_map.emplace(&elem, coeffCalculator(elem));

  mooseAssert(emplace_ret.second, "We should have inserted a new key-value pair");

  return emplace_ret.first->second;
}

ADReal
NSFVAdvectionBase::coeffCalculator(const Elem & elem) const
{
  ADReal coeff = 0;

  ADRealVectorValue elem_velocity(_u_var->getElemValue(&elem));

  if (_v_var)
    elem_velocity(1) = _v_var->getElemValue(&elem);
  if (_w_var)
    elem_velocity(2) = _w_var->getElemValue(&elem);

  auto action_functor = [&coeff, &elem_velocity, this](const Elem & /*functor_elem*/,
                                                       const Elem * const neighbor,
                                                       const FaceInfo * const fi,
                                                       const Point & surface_vector,
                                                       Real coord,
                                                       const bool /*elem_has_info*/) {
    mooseAssert(fi, "We need a non-null FaceInfo");
    ADRealVectorValue neighbor_velocity(_u_var->getNeighborValue(neighbor, *fi, elem_velocity(0)));
    if (_v_var)
      neighbor_velocity(1) = _v_var->getNeighborValue(neighbor, *fi, elem_velocity(1));
    if (_w_var)
      neighbor_velocity(2) = _w_var->getNeighborValue(neighbor, *fi, elem_velocity(2));

    ADRealVectorValue interp_v;
    Moose::FV::interpolate(Moose::FV::InterpMethod::Average,
                           interp_v,
                           elem_velocity,
                           neighbor_velocity,
                           *fi,
                           neighbor == fi->neighborPtr());

    ADReal mass_flow = _rho * interp_v * surface_vector;

    coeff += -mass_flow;

    // Now add the viscous flux
    coeff += _mu * fi->faceArea() * coord / (fi->elemCentroid() - fi->neighborCentroid()).norm();
  };

  Moose::FV::loopOverElemFaceInfo(elem, _nsfv_subproblem.mesh(), _nsfv_subproblem, action_functor);

  return coeff;
}

void
NSFVAdvectionBase::clearRCCoeffs()
{
  auto it = _rc_a_coeffs.find(&_nsfv_app);
  mooseAssert(it != _rc_a_coeffs.end(),
              "No RC coeffs structure exists for the given MooseApp pointer");
  mooseAssert(_nsfv_tid < it->second.size(),
              "The RC coeffs structure size "
                  << it->second.size() << " is greater than or equal to the provided thread ID "
                  << _nsfv_tid);
  it->second[_nsfv_tid].clear();
}

#endif
