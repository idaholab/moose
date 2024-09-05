//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NEML2ParameterDerivativeToMOOSEMaterialProperty.h"
#include "NEML2Utils.h"

registerMooseObject("SolidMechanicsApp", NEML2ParameterDerivativeToRealMOOSEMaterialProperty);
registerMooseObject("SolidMechanicsApp",
                    NEML2ParameterDerivativeToStdVectorRealMOOSEMaterialProperty);
registerMooseObject("SolidMechanicsApp",
                    NEML2ParameterDerivativeToSymmetricRankTwoTensorMOOSEMaterialProperty);
registerMooseObject("SolidMechanicsApp",
                    NEML2ParameterDerivativeToSymmetricRankFourTensorMOOSEMaterialProperty);

#ifndef NEML2_ENABLED
#define NEML2ParameterDerivativeToMOOSEMaterialPropertyStub(name)                                  \
  NEML2ObjectStubImplementationOpen(name, Material);                                               \
  NEML2ObjectStubParam(UserObjectName, "execute_neml2_model_uo");                                  \
  NEML2ObjectStubParam(MaterialPropertyName, "moose_material_property");                           \
  NEML2ObjectStubParam(std::string, "neml2_variable");                                             \
  NEML2ObjectStubParam(std::string, "neml2_parameter_derivative");                                 \
  NEML2ObjectStubImplementationClose(name, Material)

NEML2ParameterDerivativeToMOOSEMaterialPropertyStub(
    NEML2ParameterDerivativeToRealMOOSEMaterialProperty);
NEML2ParameterDerivativeToMOOSEMaterialPropertyStub(
    NEML2ParameterDerivativeToStdVectorRealMOOSEMaterialProperty);
NEML2ParameterDerivativeToMOOSEMaterialPropertyStub(
    NEML2ParameterDerivativeToSymmetricRankTwoTensorMOOSEMaterialProperty);
NEML2ParameterDerivativeToMOOSEMaterialPropertyStub(
    NEML2ParameterDerivativeToSymmetricRankFourTensorMOOSEMaterialProperty);
#else

#include "ExecuteNEML2Model.h"

template <typename T>
InputParameters
NEML2ParameterDerivativeToMOOSEMaterialProperty<T>::validParams()
{
  auto params = Material::validParams();
  params.addClassDescription(
      "This object requests the computation of the derivative of a NEML2 output variable with "
      "respect to a NEML2 model parameter and saves the derivative as a MOOSE material property");

  params.addRequiredParam<UserObjectName>("execute_neml2_model_uo",
                                          "User object managing the execution of the NEML2 model.");
  params.addRequiredParam<MaterialPropertyName>("moose_material_property",
                                                "MOOSE material property to emit");
  params.addRequiredParam<std::string>("neml2_variable", "NEML2 output variable to read");
  params.addRequiredParam<std::string>(
      "neml2_parameter_derivative", "Return the derivative of neml2_variable with respect to this");

  return params;
}

template <typename T>
NEML2ParameterDerivativeToMOOSEMaterialProperty<T>::NEML2ParameterDerivativeToMOOSEMaterialProperty(
    const InputParameters & params)
  : Material(params),
    _execute_neml2_model(getUserObject<ExecuteNEML2Model>("execute_neml2_model_uo")),
    _prop(declareProperty<T>(getParam<MaterialPropertyName>("moose_material_property"))),
    _output_view(_execute_neml2_model.getOutputParameterDerivativeView(
        neml2::utils::parse<neml2::VariableName>(getParam<std::string>("neml2_variable")),
        getParam<std::string>("neml2_parameter_derivative")))
{
}

template <typename T>
void
NEML2ParameterDerivativeToMOOSEMaterialProperty<T>::computeProperties()
{
  if (!_execute_neml2_model.outputReady())
    return;

  // look up start index for current element
  const auto i = _execute_neml2_model.getBatchIndex(_current_elem->id());

  for (std::size_t qp = 0; qp < _qrule->n_points(); qp++)
    _prop[qp] = NEML2Utils::toMOOSE<T>(_output_view.batch_index({neml2::Size(i + qp)}));
}

template class NEML2ParameterDerivativeToMOOSEMaterialProperty<Real>;
template class NEML2ParameterDerivativeToMOOSEMaterialProperty<std::vector<Real>>;
template class NEML2ParameterDerivativeToMOOSEMaterialProperty<SymmetricRankTwoTensor>;
template class NEML2ParameterDerivativeToMOOSEMaterialProperty<SymmetricRankFourTensor>;

#endif
