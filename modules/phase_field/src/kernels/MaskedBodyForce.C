//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaskedBodyForce.h"

registerMooseObject("PhaseFieldApp", MaskedBodyForce);

InputParameters
MaskedBodyForce::validParams()
{
  InputParameters params = MatBodyForce::validParams();
  params.addClassDescription("Customization of MatBodForce which uses a material property, scalar, "
                             "and/or postprocessor to provide a source term PDE contribution.");
  params.renameParam("material_property", "mask", "Material property defining the mask.");
  return params;
}
