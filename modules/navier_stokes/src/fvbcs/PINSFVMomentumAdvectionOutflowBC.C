//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVMomentumAdvectionOutflowBC.h"
#include "PINSFVSuperficialVelocityVariable.h"
#include "SubProblem.h"
#include "MooseMesh.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", PINSFVMomentumAdvectionOutflowBC);

InputParameters
PINSFVMomentumAdvectionOutflowBC::validParams()
{
  InputParameters params = INSFVMomentumAdvectionOutflowBC::validParams();
  params.addRequiredCoupledVar("u", "The superficial velocity in the x direction.");
  params.addCoupledVar("v", "The superficial velocity in the y direction.");
  params.addCoupledVar("w", "The superficial velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>(NS::porosity, "The porosity");
  params.addClassDescription(
      "Outflow boundary condition for advecting momentum in the porous media momentum equation. "
      "This will impose a zero normal gradient on the boundary velocity.");
  return params;
}

PINSFVMomentumAdvectionOutflowBC::PINSFVMomentumAdvectionOutflowBC(const InputParameters & params)
  : INSFVMomentumAdvectionOutflowBC(params), _eps(getFunctor<ADReal>("porosity"))
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("PINSFV is not supported by local AD indexing. In order to use PINSFV, please run the "
             "configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif

  if (!dynamic_cast<const PINSFVSuperficialVelocityVariable *>(_u_var))
    paramError("u", "the u velocity must be a PINSFVSuperficialVelocityVariable.");

  if (_dim >= 2 && !dynamic_cast<const PINSFVSuperficialVelocityVariable *>(_v_var))
    paramError("v",
               "In two or more dimensions, the v velocity must be supplied and it must be an "
               "PINSFVSuperficialVelocityVariable.");

  if (_dim >= 3 && !dynamic_cast<const PINSFVSuperficialVelocityVariable *>(_w_var))
    paramError("w",
               "In three-dimensions, the w velocity must be supplied and it must be an "
               "PINSFVSuperficialVelocityVariable.");
}
