//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADSurfaceTensionBC.h"

registerMooseObject("NavierStokesApp", INSADSurfaceTensionBC);

InputParameters
INSADSurfaceTensionBC::validParams()
{
  InputParameters params = ADVectorIntegratedBC::validParams();
  params.addClassDescription("Surface tension stresses.");
  return params;
}

INSADSurfaceTensionBC::INSADSurfaceTensionBC(const InputParameters & parameters)
  : ADVectorIntegratedBC(parameters),
    _surface_term_curvature(getADMaterialProperty<RealVectorValue>("surface_term_curvature")),
    _surface_term_gradient1(getADMaterialProperty<RealVectorValue>("surface_term_gradient1")),
    _surface_term_gradient2(getADMaterialProperty<RealVectorValue>("surface_term_gradient2"))
{
}

ADReal
INSADSurfaceTensionBC::computeQpResidual()
{
  return _test[_i][_qp] * (_surface_term_curvature[_qp] /*+ _surface_term_gradient1[_qp] +
                           _surface_term_gradient2[_qp]*/);
}
