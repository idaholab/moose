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

registerADMooseObject("NavierStokesApp", PINSFVMomentumAdvectionOutflowBC);

InputParameters
PINSFVMomentumAdvectionOutflowBC::validParams()
{
  InputParameters params = FVMatAdvectionOutflowBC::validParams();
  params += INSFVFullyDevelopedFlowBC::validParams();
  params.addRequiredCoupledVar("u", "The velocity in the x direction.");
  params.addCoupledVar("v", "The velocity in the y direction.");
  params.addCoupledVar("w", "The velocity in the z direction.");
  params.addRequiredCoupledVar("porosity", "Porosity auxiliary variable");
  params.addClassDescription("Outflow boundary condition for advecting momentum. This will impose "
                             "a zero normal gradient on the boundary velocity.");
  return params;
}

PINSFVMomentumAdvectionOutflowBC::PINSFVMomentumAdvectionOutflowBC(const InputParameters & params)
  : FVMatAdvectionOutflowBC(params),
    INSFVFullyDevelopedFlowBC(params),
    _u_var(dynamic_cast<const PINSFVSuperficialVelocityVariable *>(getFieldVar("u", 0))),
    _v_var(dynamic_cast<const PINSFVSuperficialVelocityVariable *>(getFieldVar("v", 0))),
    _w_var(dynamic_cast<const PINSFVSuperficialVelocityVariable *>(getFieldVar("w", 0))),
    _eps(coupledValue("porosity")),
    _eps_neighbor(coupledNeighborValue("porosity")),
    _dim(_subproblem.mesh().dimension())
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("PINSFV is not supported by local AD indexing. In order to use PINSFV, please run the "
             "configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif

  if (!_u_var)
    paramError("u", "the u velocity must be a PINSFVSuperficialVelocityVariable.");

  if (_dim >= 2 && !_v_var)
    paramError("v",
               "In two or more dimensions, the v velocity must be supplied and it must be an "
               "PINSFVSuperficialVelocityVariable.");

  if (_dim >= 3 && !params.isParamValid("w"))
    paramError("w",
               "In three-dimensions, the w velocity must be supplied and it must be an "
               "PINSFVSuperficialVelocityVariable.");
}

ADReal
PINSFVMomentumAdvectionOutflowBC::computeQpResidual()
{
#ifdef MOOSE_GLOBAL_AD_INDEXING
  using namespace Moose::FV;

  ADRealVectorValue v(_u_var->getBoundaryFaceValue(*_face_info));
  if (_v_var)
    v(1) = _v_var->getBoundaryFaceValue(*_face_info);
  if (_w_var)
    v(2) = _w_var->getBoundaryFaceValue(*_face_info);

  ADReal adv_quant_boundary;
  interpolate(_advected_interp_method,
              adv_quant_boundary,
              _adv_quant_elem[_qp] / _eps[_qp],
              _adv_quant_neighbor[_qp] / _eps_neighbor[_qp],
              v,
              *_face_info,
              true);
  mooseAssert(_normal * v >= 0,
              "This boundary condition is for outflow but the flow is in the opposite direction of "
              "the boundary normal");
  return _normal * v * adv_quant_boundary;
#else
  mooseError("PINSFV is not supported by local AD indexing. In order to use PINSFV, please run the "
             "configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif
}
