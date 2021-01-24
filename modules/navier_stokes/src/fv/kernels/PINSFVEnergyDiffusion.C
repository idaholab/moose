//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVEnergyDiffusion.h"

registerMooseObject("NavierStokesApp", PINSFVEnergyDiffusion);

InputParameters
PINSFVEnergyDiffusion::validParams()
{
  auto params = FVFluxKernel::validParams();
  params.addClassDescription("Diffusion term in the porous media incompressible Navier-Stokes "
                             "equations.");
  params.addRequiredCoupledVar("porosity", "Porosity variable");
  params.addRequiredParam<MaterialPropertyName>("k", "thermal conductivity");
  params.set<unsigned short>("ghost_layers") = 2;
  return params;
}

PINSFVEnergyDiffusion::PINSFVEnergyDiffusion(const InputParameters & params)
  : FVFluxKernel(params),
  _k_elem(getADMaterialProperty<Real>("k")),
  _k_neighbor(getNeighborADMaterialProperty<Real>("k")),
  _eps(coupledValue("porosity")),
  _eps_neighbor(coupledNeighborValue("porosity"))
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("INSFV is not supported by local AD indexing. In order to use INSFV, please run the "
             "configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif
}

ADReal
PINSFVEnergyDiffusion::computeQpResidual()
{
  /// Interpolate thermal conductivity on the face
  ADReal k_face;
  interpolate(Moose::FV::InterpMethod::Average,
              k_face,
              _k_elem[_qp],
              _k_neighbor[_qp],
              *_face_info,
              true);

  /// Compute porosity on the face
  ADReal eps_face;
  interpolate(Moose::FV::InterpMethod::Average,
              eps_face,
              _eps[_qp],
              _eps_neighbor[_qp],
              *_face_info,
              true);

  /// Compute the temperature gradient dotted with the surface normal
  auto dTdn = gradUDotNormal();

  return - eps_face * k_face * dTdn;
}
