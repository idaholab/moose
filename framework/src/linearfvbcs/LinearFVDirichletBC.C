//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVDirichletBC.h"

registerMooseObject("MooseApp", LinearFVDirichletBC);

InputParameters
LinearFVDirichletBC::validParams()
{
  InputParameters params = LinearFVBoundaryCondition::validParams();
  params.addRequiredParam<Real>("value", "The value of this boundary condifition.");
  return params;
}

LinearFVDirichletBC::LinearFVDirichletBC(const InputParameters & parameters)
  : LinearFVBoundaryCondition(parameters), _value(getParam<Real>("value"))
{
}

Real
LinearFVDirichletBC::computeBoundaryValue(const FaceInfo * const /*face_info*/)
{
  return _value;
}

Real
LinearFVDirichletBC::computeBoundaryNormalGradient(const FaceInfo * const /*face_info*/)
{
  return 0.0; // Need to change this
}

Real
LinearFVDirichletBC::computeBoundaryValueMatrixContribution(const FaceInfo * const /*face_info*/)
{
  return 0.0;
}
Real
LinearFVDirichletBC::computeBoundaryValueRHSContribution(const FaceInfo * const /*face_info*/)
{
  return _value;
}

Real
LinearFVDirichletBC::computeBoundaryGradientMatrixContribution(const FaceInfo * const face_info) const
{
  return face_info->faceArea() /
         (face_info->neighborPtr() ? face_info->dCNMag() / 2 : face_info->dCNMag());
}

Real
LinearFVDirichletBC::computeBoundaryGradientRHSContribution(const FaceInfo * const face_info) const
{
  return _value * face_info->faceArea() /
         (face_info->neighborPtr() ? face_info->dCNMag() / 2 : face_info->dCNMag());
}
