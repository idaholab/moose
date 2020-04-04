//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "NSThermalBC.h"
#include "NS.h"

// FluidProperties includes
#include "IdealGasFluidProperties.h"

registerMooseObject("NavierStokesApp", NSThermalBC);

InputParameters
NSThermalBC::validParams()
{
  InputParameters params = NodalBC::validParams();
  params.addClassDescription("NS thermal BC.");
  params.addRequiredCoupledVar(NS::density, "density");
  params.addRequiredParam<Real>("initial", "Initial temperature");
  params.addRequiredParam<Real>("final", "Final temperature");
  params.addRequiredParam<Real>("duration",
                                "Time over which temperature ramps up from initial to final");
  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "The name of the user object for fluid properties");
  return params;
}

NSThermalBC::NSThermalBC(const InputParameters & parameters)
  : NodalBC(parameters),
    _rho_var(coupled(NS::density)),
    _rho(coupledValue(NS::density)),
    _initial(getParam<Real>("initial")),
    _final(getParam<Real>("final")),
    _duration(getParam<Real>("duration")),
    _fp(getUserObject<IdealGasFluidProperties>("fluid_properties"))
{
}

Real
NSThermalBC::computeQpResidual()
{
  Real value;

  // For constant temperature, set _initial = _final, or set _duration=0 and set _final.
  //
  // T(t) = T_i + (T_f - T_i) * sin (pi/2 * t/t_d)
  if (_t < _duration)
    value = _initial + (_final - _initial) * std::sin((0.5 * libMesh::pi) * _t / _duration);
  else
    value = _final;

  // For the total energy, the essential BC is:
  // rho*E = rho*(c_v*T + |u|^2/2)
  //
  // or, in residual form, (In general, this BC is coupled to the velocity variables.)
  // rho*E - rho*(c_v*T + |u|^2/2) = 0
  //
  // ***at a no-slip wall*** this further reduces to (no coupling to velocity variables):
  // rho*E - rho*cv*T = 0

  // std::ios_base::fmtflags flags = Moose::out.flags();
  // Moose::out << std::scientific << std::setprecision(16);
  // Moose::out << "rho*E                        =" << _u[_qp] << std::endl;
  // Moose::out << "(_p[_qp] * _c_v[_qp] * value)=" << (_p[_qp] * _c_v[_qp] * value) << std::endl;
  // //Moose::out << "_c_v[_qp]                    =" << _c_v[_qp] << std::endl;
  // Moose::out.flags(flags);
  return _u[_qp] - (_rho[_qp] * _fp.cv() * value);
}
