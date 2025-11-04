//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrackFrontNonlocalScalarMaterial.h"
#include "SolidMechanicsAppTypes.h"

registerMooseObject("SolidMechanicsApp", CrackFrontNonlocalScalarMaterial);

InputParameters
CrackFrontNonlocalScalarMaterial::validParams()
{
  InputParameters params = CrackFrontNonlocalMaterialBase::validParams();
  params.addClassDescription("Computes the average material at points provided by the "
                             "crack_front_definition vectorpostprocessor.");
  params.addRequiredParam<MaterialPropertyName>(
      "property_name", "Get name of material property to compute at crack front");
  ExecFlagEnum & exec = params.set<ExecFlagEnum>("execute_on");
  exec.addAvailableFlags(EXEC_XFEM_MARK);
  params.setDocString("execute_on", exec.getDocString());
  params.set<ExecFlagEnum>("execute_on") = {EXEC_XFEM_MARK, EXEC_TIMESTEP_END};
  return params;
}

CrackFrontNonlocalScalarMaterial::CrackFrontNonlocalScalarMaterial(
    const InputParameters & parameters)
  : CrackFrontNonlocalMaterialBase(parameters,
                                   parameters.get<MaterialPropertyName>("property_name")),
    _scalar(getMaterialProperty<Real>(_base_name + _property_name))
{
}

Real
CrackFrontNonlocalScalarMaterial::getQPCrackFrontScalar(const unsigned int qp,
                                                        const Point /*crack_face_normal*/) const
{
  return _scalar[qp];
}
