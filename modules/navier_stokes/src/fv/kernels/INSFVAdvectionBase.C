//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVAdvectionBase.h"

#include "InputParameters.h"
#include "SubProblem.h"
#include "MooseVariableFV.h"

std::unordered_map<const MooseApp *,
                   std::vector<std::unordered_map<const Elem *, VectorValue<ADReal>>>>
    INSFVAdvectionBase::_rc_a_coeffs;

InputParameters
INSFVAdvectionBase::validParams()
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

  params.addRequiredParam<MaterialPropertyName>("mu", "The viscosity");
  params.addRequiredParam<Real>("rho", "The value for the density");
  params.declareControllable("rho");
  return params;
}

INSFVAdvectionBase::INSFVAdvectionBase(const InputParameters & params)
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
    _rho(params.get<Real>("rho"))
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("INSFV is not supported by local AD indexing. In order to use INSFV, please run the "
             "configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif

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

const VectorValue<ADReal> &
INSFVAdvectionBase::rcCoeff(const Elem & elem, const ADReal & mu) const
{
  mooseError("You have to override me");
}

#ifdef MOOSE_GLOBAL_AD_INDEXING
VectorValue<ADReal>
INSFVAdvectionBase::coeffCalculator(const Elem & elem, const ADReal & mu) const
{
  mooseError("You have to override me");
}
#else
VectorValue<ADReal>
INSFVAdvectionBase::coeffCalculator(const Elem &, const ADReal &) const
{
  mooseError("INSFV is not supported by local AD indexing. In order to use INSFV, please run the "
             "configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
}
#endif

void
INSFVAdvectionBase::clearRCCoeffs()
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
