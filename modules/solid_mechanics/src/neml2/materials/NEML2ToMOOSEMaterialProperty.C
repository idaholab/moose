//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NEML2ToMOOSEMaterialProperty.h"
#include "ExecuteNEML2Model.h"

registerMooseObject("SolidMechanicsApp", NEML2ToRealMOOSEMaterialProperty);
registerMooseObject("SolidMechanicsApp", NEML2ToStdVectorRealMOOSEMaterialProperty);
registerMooseObject("SolidMechanicsApp", NEML2ToSymmetricRankTwoTensorMOOSEMaterialProperty);
registerMooseObject("SolidMechanicsApp", NEML2ToSymmetricRankFourTensorMOOSEMaterialProperty);

#ifndef NEML2_ENABLED
NEML2ObjectStubImplementation(NEML2ToRealMOOSEMaterialProperty, Material);
NEML2ObjectStubImplementation(NEML2ToStdVectorRealMOOSEMaterialProperty, Material);
NEML2ObjectStubImplementation(NEML2ToSymmetricRankTwoTensorMOOSEMaterialProperty, Material);
NEML2ObjectStubImplementation(NEML2ToSymmetricRankFourTensorMOOSEMaterialProperty, Material);
#else

template <typename T>
InputParameters
NEML2ToMOOSEMaterialProperty<T>::validParams()
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
  params.addParam<std::string>(
      "neml2_input_derivative",
      "If supplied return the derivative of neml2_variable with respect to this");

  // provide an optional initialization of the moose property (because we don't really know if it is
  // going to become stateful or not)
  params.addParam<MaterialPropertyName>("moose_material_property_init",
                                        "Optional material property as the initial condition");

  return params;
}

template <typename T>
NEML2ToMOOSEMaterialProperty<T>::NEML2ToMOOSEMaterialProperty(const InputParameters & params)
  : Material(params),
    _execute_neml2_model(getUserObject<ExecuteNEML2Model>("execute_neml2_model_uo")),
    _prop(declareProperty<T>(getParam<MaterialPropertyName>("moose_material_property"))),
    _prop0(isParamValid("moose_material_property_init")
               ? &getMaterialProperty<T>("moose_material_property_init")
               : nullptr),
    _output_view(
        !isParamValid("neml2_input_derivative")
            ? _execute_neml2_model.getOutputView(
                  neml2::utils::parse<neml2::VariableName>(getParam<std::string>("neml2_variable")))
            : _execute_neml2_model.getOutputDerivativeView(
                  neml2::utils::parse<neml2::VariableName>(getParam<std::string>("neml2_variable")),
                  neml2::utils::parse<neml2::VariableName>(
                      getParam<std::string>("neml2_input_derivative"))))
{
}

template <typename T>
void
NEML2ToMOOSEMaterialProperty<T>::initQpStatefulProperties()
{
  if (_prop0)
    _prop[_qp] = (*_prop0)[_qp];
}

template <typename T>
void
NEML2ToMOOSEMaterialProperty<T>::computeProperties()
{
  if (!_execute_neml2_model.outputReady())
    return;

  // look up start index for current element
  const auto i = _execute_neml2_model.getBatchIndex(_current_elem->id());

  for (std::size_t qp = 0; qp < _qrule->n_points(); qp++)
    _prop[qp] = NEML2Utils::toMOOSE<T>(_output_view.batch_index({neml2::Size(i + qp)}));
}

template class NEML2ToMOOSEMaterialProperty<Real>;
template class NEML2ToMOOSEMaterialProperty<std::vector<Real>>;
template class NEML2ToMOOSEMaterialProperty<SymmetricRankTwoTensor>;
template class NEML2ToMOOSEMaterialProperty<SymmetricRankFourTensor>;

#endif
