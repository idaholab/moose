//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NEML2ParameterDerivativeToMOOSEMaterialProperty.h"
#include "ExecuteNEML2Model.h"

registerMooseObject("SolidMechanicsApp", NEML2ParameterDerivativeToRealMOOSEMaterialProperty);
registerMooseObject("SolidMechanicsApp",
                    NEML2ParameterDerivativeToStdVectorRealMOOSEMaterialProperty);
registerMooseObject("SolidMechanicsApp",
                    NEML2ParameterDerivativeToSymmetricRankTwoTensorMOOSEMaterialProperty);
registerMooseObject("SolidMechanicsApp",
                    NEML2ParameterDerivativeToSymmetricRankFourTensorMOOSEMaterialProperty);

#ifndef NEML2_ENABLED
NEML2ObjectStubImplementation(NEML2ParameterDerivativeToRealMOOSEMaterialProperty, Material);
NEML2ObjectStubImplementation(NEML2ParameterDerivativeToStdVectorRealMOOSEMaterialProperty,
                              Material);
NEML2ObjectStubImplementation(NEML2ParameterDerivativeToSymmetricRankTwoTensorMOOSEMaterialProperty,
                              Material);
NEML2ObjectStubImplementation(
    NEML2ParameterDerivativeToSymmetricRankFourTensorMOOSEMaterialProperty, Material);
#else

template <typename T>
InputParameters
NEML2ParameterDerivativeToMOOSEMaterialProperty<T>::validParams()
{
  auto params = Material::validParams();
  params.addClassDescription(
      "Provide an output from a NEML2 model as a MOOSE material property of type " +
      demangle(typeid(T).name()) + ".");

  params.addRequiredParam<UserObjectName>("execute_neml2_model_uo",
                                          "User object managing the execution of the NEML2 model.");
  params.addRequiredParam<MaterialPropertyName>("moose_material_property",
                                                "MOOSE material property to emit");
  params.addRequiredParam<std::string>("neml2_variable", "NEML2 output variable to read");
  params.addRequiredParam<std::string>(
      "neml2_parameter_derivative", "Return the derivative of neml2_variable with respect to this");

  // provide an optional initialization of the moose property (because we don't really know if it is
  // going to become stateful or not)
  params.addParam<MaterialPropertyName>("moose_material_property_init",
                                        "Optional material property as the initial condition");

  return params;
}

template <typename T>
NEML2ParameterDerivativeToMOOSEMaterialProperty<T>::NEML2ParameterDerivativeToMOOSEMaterialProperty(
    const InputParameters & params)
  : Material(params),
    _execute_neml2_model(getUserObject<ExecuteNEML2Model>("execute_neml2_model_uo")),
    _prop(declareProperty<T>(getParam<MaterialPropertyName>("moose_material_property"))),
    _prop0(isParamValid("moose_material_property_init")
               ? &getMaterialProperty<T>("moose_material_property_init")
               : nullptr),
    _output_view(_execute_neml2_model.getOutputParameterDerivativeView(
        neml2::utils::parse<neml2::VariableName>(getParam<std::string>("neml2_variable")),
        getParam<std::string>("neml2_parameter_derivative")))
{
}

template <typename T>
void
NEML2ParameterDerivativeToMOOSEMaterialProperty<T>::initQpStatefulProperties()
{
  if (_prop0)
    _prop[_qp] = (*_prop0)[_qp];
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
