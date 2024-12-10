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
  params.addParam<bool>("include_gradient_terms",
                        false,
                        "If the surface tension should include the gradient terms (increases "
                        "fidelity, decreases stability)");
  return params;
}

INSADSurfaceTensionBC::INSADSurfaceTensionBC(const InputParameters & parameters)
  : ADVectorIntegratedBC(parameters),
    _surface_term_curvature(getADMaterialProperty<RealVectorValue>("surface_term_curvature")),
    _surface_term_gradient1(getADMaterialProperty<RealVectorValue>("surface_term_gradient1")),
    _surface_term_gradient2(getADMaterialProperty<RealVectorValue>("surface_term_gradient2")),
    _include_gradient_terms(getParam<bool>("include_gradient_terms")),
    _curvature_factor(_subproblem.mesh().dimension() == 3 ? 1.0 : -1.0)
{
}

ADReal
INSADSurfaceTensionBC::computeQpResidual()
{
  auto force = _curvature_factor * _surface_term_curvature[_qp];

  if (_include_gradient_terms)
    force += _surface_term_gradient1[_qp] + _surface_term_gradient2[_qp];
  return -_test[_i][_qp] * force;
}
