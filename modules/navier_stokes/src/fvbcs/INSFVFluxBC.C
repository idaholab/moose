//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVFluxBC.h"
#include "SystemBase.h"
#include "Assembly.h"
#include "MooseVariableBase.h"

InputParameters
INSFVFluxBC::validParams()
{
  auto params = FVFluxBC::validParams();
  params += INSFVMomentumResidualObject::validParams();
  return params;
}

INSFVFluxBC::INSFVFluxBC(const InputParameters & params)
  : FVFluxBC(params), INSFVMomentumResidualObject(*this)
{
}

void
INSFVFluxBC::addResidualAndJacobian(const ADReal & residual)
{
  const auto * const elem = (_face_type == FaceInfo::VarFaceNeighbors::ELEM)
                                ? &_face_info->elem()
                                : _face_info->neighborPtr();
  const auto dof_index = elem->dof_number(_sys.number(), _var.number(), 0);

  addResidualsAndJacobian(_assembly,
                          std::array<ADReal, 1>{{residual}},
                          std::array<dof_id_type, 1>{{dof_index}},
                          _var.scalingFactor());
}
