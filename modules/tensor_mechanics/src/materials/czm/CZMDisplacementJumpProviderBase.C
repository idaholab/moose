//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CZMDisplacementJumpProviderBase.h"
#include "RotationMatrix.h"

InputParameters
CZMDisplacementJumpProviderBase::validParams()
{
  InputParameters params = InterfaceMaterial::validParams();
  params.addClassDescription("Base class used to compute the displacement jump accross a czm "
                             "interface in local coordinates");
  params.addRequiredCoupledVar("displacements",
                               "The string of displacements suitable for the problem statement");
  params.suppressParameter<bool>("use_displaced_mesh");
  return params;
}

CZMDisplacementJumpProviderBase::CZMDisplacementJumpProviderBase(const InputParameters & parameters)
  : InterfaceMaterial(parameters),
    _normals(_assembly.normals()),
    _ndisp(coupledComponents("displacements")),
    _disp(3),
    _disp_neighbor(3),
    _displacement_jump_global(declareProperty<RealVectorValue>("displacement_jump_global")),
    _interface_displacement_jump(declareProperty<RealVectorValue>("interface_displacement_jump")),
    _Q0(declareProperty<RealTensorValue>("czm_reference_rotation"))
{
  if (_ndisp > 3 || _ndisp < 1)
    mooseError("the CZM material requires 1, 2 or 3 displacement variables");
}

void
CZMDisplacementJumpProviderBase::initialSetup()
{
  // initializing the displacement vectors
  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    _disp[i] = &coupledValue("displacements", i);
    _disp_neighbor[i] = &coupledNeighborValue("displacements", i);
  }

  // All others zero (so this will work naturally for 2D and 1D problems)
  for (unsigned int i = _ndisp; i < 3; i++)
  {
    _disp[i] = &_zero;
    _disp_neighbor[i] = &_zero;
  }
}

void
CZMDisplacementJumpProviderBase::computeQpProperties()
{

  _Q0[_qp] = RotationMatrix::rotVec1ToVec2(RealVectorValue(1, 0, 0), _normals[_qp]);

  // computing the displacement jump
  for (unsigned int i = 0; i < _ndisp; i++)
    _displacement_jump_global[_qp](i) = (*_disp_neighbor[i])[_qp] - (*_disp[i])[_qp];
  for (unsigned int i = _ndisp; i < 3; i++)
    _displacement_jump_global[_qp](i) = 0;

  computeLocalDisplacementJump();
}
