//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVPenaltyBernoulli.h"
#include "NS.h"
#include "SystemBase.h"
#include "Assembly.h"
#include "SubProblem.h"

registerMooseObject("NavierStokesApp", PINSFVPenaltyBernoulli);

InputParameters
PINSFVPenaltyBernoulli::validParams()
{
  InputParameters params = FVInterfaceKernel::validParams();
  params.set<unsigned short>("ghost_layers") = 2;
  params.addRequiredParam<Real>("penalty", "The penalty");
  params.addRequiredParam<NonlinearVariableName>("v1", "The y-velocity on the '1' side");
  params.addRequiredParam<NonlinearVariableName>("v2", "The y-velocity on the '2' side");
  params.addRequiredParam<NonlinearVariableName>(NS::pressure + "1",
                                                 "The pressure on the '1' side");
  params.addRequiredParam<NonlinearVariableName>(NS::pressure + "2",
                                                 "The pressure on the '2' side");
  params.addRequiredParam<AuxVariableName>(NS::porosity + "1", "The porosity on the '1' side");
  params.addRequiredParam<AuxVariableName>(NS::porosity + "2", "The porosity on the '2' side");
  params.addRequiredParam<Real>(NS::density, "The density");
  return params;
}

PINSFVPenaltyBernoulli::PINSFVPenaltyBernoulli(const InputParameters & params)
  : FVInterfaceKernel(params),
    _penalty(getParam<Real>("penalty")),
    _v1(_sys.getFVVariable<Real>(_tid, getParam<NonlinearVariableName>("v1"))),
    _v2(_sys.getFVVariable<Real>(_tid,
                                 isParamValid("v2") ? getParam<NonlinearVariableName>("v2")
                                                    : getParam<NonlinearVariableName>("v1"))),
    _p1(_sys.getFVVariable<Real>(_tid, getParam<NonlinearVariableName>(NS::pressure + "1"))),
    _p2(_sys.getFVVariable<Real>(_tid,
                                 isParamValid(NS::pressure + "2")
                                     ? getParam<NonlinearVariableName>(NS::pressure + "2")
                                     : getParam<NonlinearVariableName>(NS::pressure + "1"))),
    _eps1(_subproblem.systemBaseAuxiliary().getFVVariable<Real>(
        _tid, getParam<AuxVariableName>(NS::porosity + "1"))),
    _eps2(_subproblem.systemBaseAuxiliary().getFVVariable<Real>(
        _tid,
        isParamValid(NS::porosity + "2") ? getParam<AuxVariableName>(NS::porosity + "2")
                                         : getParam<AuxVariableName>(NS::porosity + "1"))),
    _rho(getParam<Real>(NS::density))
{
}

ADReal
PINSFVPenaltyBernoulli::computeQpResidual()
{
  const auto u1 =
      var1().getBoundaryFaceValue(*_face_info) / _eps1.getBoundaryFaceValue(*_face_info);
  const auto u2 =
      var2().getBoundaryFaceValue(*_face_info) / _eps2.getBoundaryFaceValue(*_face_info);
  const auto v1 = _v1.getBoundaryFaceValue(*_face_info) / _eps1.getBoundaryFaceValue(*_face_info);
  const auto v2 = _v2.getBoundaryFaceValue(*_face_info) / _eps2.getBoundaryFaceValue(*_face_info);
  const auto p1 = _p1.getBoundaryFaceValue(*_face_info);
  const auto p2 = _p2.getBoundaryFaceValue(*_face_info);
  return _penalty * (_rho * (u1 * u1 + v1 * v1) / 2 + p1 - (_rho * (u2 * u2 + v2 * v2) / 2 + p2));
}

void
PINSFVPenaltyBernoulli::computeResidual(const FaceInfo & fi)
{
  setupData(fi);

  const auto r = MetaPhysicL::raw_value(fi.faceArea() * fi.faceCoord() * computeQpResidual());

  processResidual(r, var1().number(), !_elem_is_one);
}

#ifdef MOOSE_GLOBAL_AD_INDEXING
void
PINSFVPenaltyBernoulli::computeJacobian(const FaceInfo & fi)
{
  setupData(fi);

  const auto r = fi.faceArea() * fi.faceCoord() * computeQpResidual();

  _assembly.processResidualAndJacobian(r,
                                       _elem_is_one ? var1().dofIndices()[0]
                                                    : var1().dofIndicesNeighbor()[0],
                                       _vector_tags,
                                       _matrix_tags);
}
#else
void
PINSFVPenaltyBernoulli::computeJacobian(const FaceInfo &)
{
}
#endif
