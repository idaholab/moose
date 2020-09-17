//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVBase.h"
#include "InputParameters.h"
#include "SubProblem.h"
#include "MooseVariableFV.h"

InputParameters
NSFVBase::validParams()
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

  params.addParam<Real>("mu", 1, "The viscosity");
  params.addParam<Real>("rho", 1, "The density");
  return params;
}

NSFVBase::NSFVBase(const InputParameters & params)
  : _nsfv_subproblem(*params.getCheckedPointerParam<SubProblem *>("_subproblem")),
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
    mooseError("In two-dimensions, the v velocity must be supplied and it must be a finite volume "
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
}
