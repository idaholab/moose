//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVMomentumPressureFlux.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", PINSFVMomentumPressureFlux);

InputParameters
PINSFVMomentumPressureFlux::validParams()
{
  auto params = FVFluxKernel::validParams();
  params.addClassDescription("Viscous diffusion term, div(mu grad(u)), in the porous media "
                             "incompressible Navier-Stokes momentum equation.");
  params.addRequiredCoupledVar("porosity", "Porosity auxiliary variable");
  params.addRequiredCoupledVar("p", "Pressure variable");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");
  params.set<unsigned short>("ghost_layers") = 2;
  return params;
}

PINSFVMomentumPressureFlux::PINSFVMomentumPressureFlux(const InputParameters & params)
  : FVFluxKernel(params),
  _eps(coupledValue("porosity")),
  _eps_neighbor(coupledNeighborValue("porosity")),
  _p_elem(adCoupledValue(NS::pressure)),
  _p_neighbor(adCoupledNeighborValue(NS::pressure)),
  _p_var(dynamic_cast<const INSFVPressureVariable *>(getFieldVar("p", 0))),
  _index(getParam<MooseEnum>("momentum_component"))
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("PINSFV is not supported by local AD indexing. In order to use PINSFV, please run "
             "the configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif
}

bool
PINSFVMomentumPressureFlux::skipForBoundary(const FaceInfo & fi) const
{
  return false;
}

ADReal
PINSFVMomentumPressureFlux::computeQpResidual()
{
  ADReal eps_p_interface;
  ADReal p_elem = _p_var->getElemValue(&_face_info->elem());
  ADReal p_neighbor = _p_var->getNeighborValue(_face_info->neighborPtr(), *_face_info, p_elem);

  Moose::FV::interpolate(Moose::FV::InterpMethod::Average,
                        eps_p_interface,
                        _p_elem[_qp] * _eps[_qp],
                        _p_neighbor[_qp] * _eps_neighbor[_qp],
                        *_face_info,
                        true);
  return eps_p_interface * _normal(_index);
}
