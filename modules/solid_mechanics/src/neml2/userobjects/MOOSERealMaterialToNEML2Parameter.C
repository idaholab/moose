//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MOOSERealMaterialToNEML2Parameter.h"
#include "NEML2Utils.h"

registerMooseObject("SolidMechanicsApp", MOOSERealMaterialToNEML2Parameter);

#ifndef NEML2_ENABLED
#define MOOSEMRealMaterialToNEML2ParameterStub(name)                                               \
  NEML2ObjectStubImplementationOpen(name, MOOSEToNEML2Parameter);                                  \
  NEML2ObjectStubParam(MaterialPropertyName, "moose_material_property");                           \
  NEML2ObjectStubImplementationClose(name, MOOSEToNEML2Parameter)
MOOSEMRealMaterialToNEML2ParameterStub(MOOSERealMaterialToNEML2Parameter)
#else

InputParameters
MOOSERealMaterialToNEML2Parameter::validParams()
{
  auto params = MOOSEToNEML2Parameter::validParams();
  params.addClassDescription("This object gathers a real material property from MOOSE and use it "
                             "as a NEML2 model parameter.");

  params.addRequiredParam<MaterialPropertyName>("moose_material_property",
                                                "MOOSE material property to read from");
  return params;
}

MOOSERealMaterialToNEML2Parameter::MOOSERealMaterialToNEML2Parameter(const InputParameters & params)
  : MOOSEToNEML2Parameter(params),
    _mat_prop(getGenericMaterialProperty<Real, false>("moose_material_property"))
{
}

torch::Tensor
MOOSERealMaterialToNEML2Parameter::convertQpMOOSEData() const
{
  return NEML2Utils::toNEML2<Real>(_mat_prop[_qp]);
}

#endif
