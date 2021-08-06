//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVMomentumPressureFlux.h"
#include "PINSFVSuperficialVelocityVariable.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", PINSFVMomentumPressureFlux);

InputParameters
PINSFVMomentumPressureFlux::validParams()
{
  auto params = FVFluxKernel::validParams();
  params += INSFVMomentumResidualObject::validParams();
  params.addClassDescription("Momentum pressure term eps grad_P, as a flux kernel "
                             "using the divergence theoreom, in the porous media "
                             "incompressible Navier-Stokes momentum equation. This kernel "
                             "is also executed on boundaries.");
  params.addRequiredCoupledVar(NS::porosity, "Porosity auxiliary variable");
  params.addRequiredCoupledVar(NS::pressure, "Pressure variable");
  params.addDeprecatedCoupledVar("p", NS::pressure, "1/1/2022");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>("momentum_component",
                                     momentum_component,
                                     "The component of the momentum equation that this kernel "
                                     "applies to.");
  params.set<bool>("force_boundary_execution") = true;
  return params;
}

PINSFVMomentumPressureFlux::PINSFVMomentumPressureFlux(const InputParameters & params)
  : FVFluxKernel(params),
    INSFVMomentumResidualObject(*this),
    _eps(coupledValue(NS::porosity)),
    _eps_neighbor(coupledNeighborValue(NS::porosity)),
    _p_elem(adCoupledValue(NS::pressure)),
    _p_neighbor(adCoupledNeighborValue(NS::pressure)),
    _index(getParam<MooseEnum>("momentum_component"))
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("PINSFV is not supported by local AD indexing. In order to use PINSFV, please run "
             "the configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif
  if (!dynamic_cast<PINSFVSuperficialVelocityVariable *>(&_var))
    mooseError("PINSFVMomentumPressureFlux may only be used with a superficial velocity, "
               "of variable type PINSFVSuperficialVelocityVariable.");
}

ADReal
PINSFVMomentumPressureFlux::computeQpResidual()
{
  ADReal eps_p_interface;

  Moose::FV::interpolate(Moose::FV::InterpMethod::Average,
                         eps_p_interface,
                         _eps[_qp] * _p_elem[_qp],
                         _eps_neighbor[_qp] * _p_neighbor[_qp],
                         *_face_info,
                         true);
  return eps_p_interface * _normal(_index);
}
