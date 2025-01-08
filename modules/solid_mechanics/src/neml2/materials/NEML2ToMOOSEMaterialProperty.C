//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NEML2ToMOOSEMaterialProperty.h"
#include "NEML2ModelExecutor.h"

#define RegisterNEML2ToMOOSEMaterialProperty(alias)                                                \
  registerMooseObject("SolidMechanicsApp", NEML2ToMOOSE##alias##MaterialProperty)

RegisterNEML2ToMOOSEMaterialProperty(Real);
RegisterNEML2ToMOOSEMaterialProperty(SymmetricRankTwoTensor);
RegisterNEML2ToMOOSEMaterialProperty(SymmetricRankFourTensor);
RegisterNEML2ToMOOSEMaterialProperty(RealVectorValue);
RegisterNEML2ToMOOSEMaterialProperty(RankTwoTensor);
RegisterNEML2ToMOOSEMaterialProperty(RankFourTensor);

template <typename T>
InputParameters
NEML2ToMOOSEMaterialProperty<T>::validParams()
{
  auto params = Material::validParams();
  params.addClassDescription(
      NEML2Utils::docstring("Provide an output (or its derivative) from a NEML2 model as a MOOSE "
                            "material property of type " +
                            demangle(typeid(T).name()) + "."));

  params.addRequiredParam<UserObjectName>(
      "neml2_executor",
      NEML2Utils::docstring("User object managing the execution of the NEML2 model."));
  params.addRequiredParam<MaterialPropertyName>(
      "to_moose",
      NEML2Utils::docstring(
          "MOOSE material property used to store the NEML2 output variable (or its derivative)"));
  params.addRequiredParam<std::string>("from_neml2",
                                       NEML2Utils::docstring("NEML2 output variable to read from"));
  params.addParam<std::string>(
      "neml2_input_derivative",
      NEML2Utils::docstring(
          "If supplied return the derivative of the NEML2 output variable with respect to this"));
  params.addParam<std::string>(
      "neml2_parameter_derivative",
      NEML2Utils::docstring(
          "If supplied return the derivative of neml2_variable with respect to this"));

  // provide an optional initialization of the moose property (because we don't really know if it is
  // going to become stateful or not)
  params.addParam<MaterialPropertyName>(
      "moose_material_property_init",
      NEML2Utils::docstring("Optional material property as the initial condition"));

  return params;
}

template <typename T>
NEML2ToMOOSEMaterialProperty<T>::NEML2ToMOOSEMaterialProperty(const InputParameters & params)
  : Material(params)
#ifdef NEML2_ENABLED
    ,
    _execute_neml2_model(getUserObject<NEML2ModelExecutor>("neml2_executor")),
    _prop(declareProperty<T>(getParam<MaterialPropertyName>("to_moose"))),
    _prop0(isParamValid("moose_material_property_init")
               ? &getMaterialProperty<T>("moose_material_property_init")
               : nullptr),
    _value(
        !isParamValid("neml2_input_derivative")
            ? (!isParamValid("neml2_parameter_derivative")
                   ? _execute_neml2_model.getOutput(
                         NEML2Utils::parseVariableName(getParam<std::string>("from_neml2")))
                   : _execute_neml2_model.getOutputParameterDerivative(
                         NEML2Utils::parseVariableName(getParam<std::string>("from_neml2")),
                         getParam<std::string>("neml2_parameter_derivative")))
            : _execute_neml2_model.getOutputDerivative(
                  NEML2Utils::parseVariableName(getParam<std::string>("from_neml2")),
                  NEML2Utils::parseVariableName(getParam<std::string>("neml2_input_derivative"))))
#endif
{
  NEML2Utils::assertNEML2Enabled();
}

#ifdef NEML2_ENABLED
template <typename T>
void
NEML2ToMOOSEMaterialProperty<T>::computeProperties()
{
  // See issue #28971: Using _prop0 to set initial condition for this possibly stateful property may
  // not work. As a workaround, we set the initial condition here when _t_step == 0.
  if (_t_step == 0 && _prop0)
  {
    _prop.set() = _prop0->get();
    return;
  }

  if (!_execute_neml2_model.outputReady())
    return;

  // look up start index for current element
  const auto i = _execute_neml2_model.getBatchIndex(_current_elem->id());
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    NEML2Utils::copyTensorToMOOSEData(_value.batch_index({neml2::Size(i + _qp)}), _prop[_qp]);
}
#endif

#define InstantiateNEML2ToMOOSEMaterialProperty(T) template class NEML2ToMOOSEMaterialProperty<T>

InstantiateNEML2ToMOOSEMaterialProperty(Real);
InstantiateNEML2ToMOOSEMaterialProperty(SymmetricRankTwoTensor);
InstantiateNEML2ToMOOSEMaterialProperty(SymmetricRankFourTensor);
InstantiateNEML2ToMOOSEMaterialProperty(RealVectorValue);
InstantiateNEML2ToMOOSEMaterialProperty(RankTwoTensor);
InstantiateNEML2ToMOOSEMaterialProperty(RankFourTensor);
