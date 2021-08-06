//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMassAdvectionOutflowBC.h"
#include "INSFVVelocityVariable.h"
#include "SubProblem.h"
#include "MooseMesh.h"

registerADMooseObject("NavierStokesApp", INSFVMassAdvectionOutflowBC);

InputParameters
INSFVMassAdvectionOutflowBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params += INSFVFullyDevelopedFlowBC::validParams();
  params.addClassDescription("Outflow boundary condition for advecting mass.");
  params.addRequiredParam<Real>("rho", "The value for the density");
  params.declareControllable("rho");
  params.addRequiredCoupledVar("u", "The velocity in the x direction.");
  params.addCoupledVar("v", "The velocity in the y direction.");
  params.addCoupledVar("w", "The velocity in the z direction.");
  return params;
}

INSFVMassAdvectionOutflowBC::INSFVMassAdvectionOutflowBC(const InputParameters & params)
  : FVFluxBC(params),
    INSFVFullyDevelopedFlowBC(params),
    _rho(getParam<Real>("rho")),
    _u_var(dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("u", 0))),
    _v_var(dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("v", 0))),
    _w_var(dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("w", 0))),
    _dim(_subproblem.mesh().dimension())
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("INSFV is not supported by local AD indexing. In order to use INSFV, please run the "
             "configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif

  if (!_u_var)
    paramError("u", "the u velocity must be an INSFVVelocityVariable.");

  if (_dim >= 2 && !_v_var)
    paramError("v",
               "In two or more dimensions, the v velocity must be supplied and it must be an "
               "INSFVVelocityVariable.");

  if (_dim >= 3 && !params.isParamValid("w"))
    paramError("w",
               "In three-dimensions, the w velocity must be supplied and it must be an "
               "INSFVVelocityVariable.");
}

ADReal
INSFVMassAdvectionOutflowBC::computeQpResidual()
{
#ifdef MOOSE_GLOBAL_AD_INDEXING
  ADRealVectorValue v(_u_var->getBoundaryFaceValue(*_face_info));
  if (_v_var)
    v(1) = _v_var->getBoundaryFaceValue(*_face_info);
  if (_w_var)
    v(2) = _w_var->getBoundaryFaceValue(*_face_info);

  return _normal * v * _rho;
#else
  mooseError("INSFV is not supported by local AD indexing. In order to use INSFV, please run the "
             "configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif
}
