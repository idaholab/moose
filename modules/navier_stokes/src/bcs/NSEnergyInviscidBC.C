//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "NSEnergyInviscidBC.h"
#include "NS.h"

// FluidProperties includes
#include "IdealGasFluidProperties.h"

InputParameters
NSEnergyInviscidBC::validParams()
{
  InputParameters params = NSIntegratedBC::validParams();
  params.addClassDescription("This class corresponds to the inviscid part of the 'natural' "
                             "boundary condition for the energy equation.");
  params.addRequiredCoupledVar(NS::temperature, "temperature");
  return params;
}

NSEnergyInviscidBC::NSEnergyInviscidBC(const InputParameters & parameters)
  : NSIntegratedBC(parameters),
    _temperature(coupledValue(NS::temperature)),
    // Object for computing deriviatives of pressure
    _pressure_derivs(*this)
{
}

Real
NSEnergyInviscidBC::qpResidualHelper(Real pressure, Real un)
{
  return (_rho_et[_qp] + pressure) * un * _test[_i][_qp];
}

Real
NSEnergyInviscidBC::qpResidualHelper(Real rho, RealVectorValue u, Real /*pressure*/)
{
  // return (rho*(cv*_temperature[_qp] + 0.5*u.norm_sq()) + pressure) * (u*_normals[_qp]) *
  // _test[_i][_qp];
  // We can also expand pressure in terms of rho... does this make a difference?
  // Then we don't use the input pressure value.
  return rho * (_fp.gamma() * _fp.cv() * _temperature[_qp] + 0.5 * u.norm_sq()) *
         (u * _normals[_qp]) * _test[_i][_qp];
}

// (U4+p) * d(u.n)/dX
Real
NSEnergyInviscidBC::qpJacobianTermA(unsigned var_number, Real pressure)
{
  Real result = 0.0;

  switch (var_number)
  {
    case 0: // density
    {
      // Velocity vector object
      RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

      result = -(vel * _normals[_qp]);
      break;
    }

    case 1:
    case 2:
    case 3: // momentums
      result = _normals[_qp](var_number - 1);
      break;

    case 4: // energy
      result = 0.;
      break;

    default:
      mooseError("Shouldn't get here!");
      break;
  }

  // Notice the division by _rho[_qp] here.  This comes from taking the
  // derivative wrt to either density or momentum.
  return (_rho_et[_qp] + pressure) / _rho[_qp] * result * _phi[_j][_qp] * _test[_i][_qp];
}

// d(U4)/dX * (u.n)
Real
NSEnergyInviscidBC::qpJacobianTermB(unsigned var_number, Real un)
{
  Real result = 0.0;
  switch (var_number)
  {
    case 0: // density
    case 1:
    case 2:
    case 3: // momentums
    {
      result = 0.;
      break;
    }

    case 4: // energy
    {
      result = _phi[_j][_qp] * un * _test[_i][_qp];
      break;
    }

    default:
      mooseError("Shouldn't get here!");
      break;
  }

  return result;
}

// d(p)/dX * (u.n)
Real
NSEnergyInviscidBC::qpJacobianTermC(unsigned var_number, Real un)
{
  return _pressure_derivs.get_grad(var_number) * _phi[_j][_qp] * un * _test[_i][_qp];
}
