//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComponentMaterialPropertyInterface.h"
#include "Factory.h"
#include "FEProblemBase.h"

InputParameters
ComponentMaterialPropertyInterface::validParams()
{
  auto params = ActionComponent::validParams();
  params.addParam<std::vector<std::string>>(
      "property_names",
      {},
      "List of material properties that should be defined on this ActionComponent");
  params.addParam<std::vector<MooseFunctorName>>(
      "property_values",
      {},
      "Functors that provide the values of the material property on this ActionComponent");

  // TODO: make the Physics set these three parameters
  params.addParam<bool>("use_ad_for_properties",
                        true,
                        "Whether to use automatic differentiation for the properties defined");
  params.addParam<bool>("define_material_properties",
                        true,
                        "If true, define material properties from the values provided");
  params.addParam<bool>("define_functor_properties",
                        true,
                        "If true, define functor properties from the values provided");

  params.addParamNamesToGroup("property_names property_values use_ad_for_properties "
                              "define_material_properties define_functor_properties",
                              "Material and Functor Property");
  return params;
}

ComponentMaterialPropertyInterface::ComponentMaterialPropertyInterface(
    const InputParameters & params)
  : ActionComponent(params),
    _property_names(getParam<std::vector<std::string>>("property_names")),
    _property_functors(getParam<std::vector<MooseFunctorName>>("property_values")),
    _use_ad_for_props(getParam<bool>("use_ad_for_properties"))
{
  addRequiredTask("add_material");
  if (_property_names.size() != _property_functors.size())
    paramError("property_names", "Should be the same size as property functors");
}

void
ComponentMaterialPropertyInterface::addMaterials()
{
  if (getParam<bool>("define_material_properties") && _property_names.size())
  {
    // Add a material that makes material properties available on the blocks of the component.
    // The idea is to make the values/functors available under the same property name across all
    // components. Then the Physics / kernels can just use the name of the property
    InputParameters params = getFactory().getValidParams("MaterialFunctorConverter");
    params.set<std::vector<SubdomainName>>("block") = _blocks;

    // Type conversion
    std::vector<MaterialPropertyName> property_names(_property_names.size());
    std::transform(_property_names.begin(),
                   _property_names.end(),
                   property_names.begin(),
                   [](const std::string & val) { return MaterialPropertyName(val); });
    if (_use_ad_for_props)
      params.set<std::vector<MaterialPropertyName>>("ad_props_out") = property_names;
    else
      params.set<std::vector<MaterialPropertyName>>("reg_props_out") = property_names;
    params.set<std::vector<MooseFunctorName>>("functors_in") = _property_functors;
    getProblem().addMaterial(
        "MaterialFunctorConverter", name() + "_local_material_properties", params);
  }

  // Add a functor material that makes the functor material properties on the blocks of the
  // component. This makes the functors available under the same functor name across all components
  if (getParam<bool>("define_functor_properties") && _property_names.size())
  {
    // Add a material that makes material properties available on the blocks of the component.
    // The idea is to make the values/functors available under the same property name across all
    // components. Then the Physics / kernels can just use the name of the property
    const auto mat_type = _use_ad_for_props ? "ADGenericFunctorMaterial" : "GenericFunctorMaterial";
    InputParameters params = getFactory().getValidParams(mat_type);
    params.set<std::vector<SubdomainName>>("block") = _blocks;
    params.set<std::vector<std::string>>("prop_names") = _property_names;
    params.set<std::vector<MooseFunctorName>>("prop_values") = _property_functors;
    getProblem().addMaterial(mat_type, name() + "_local_functor_properties", params);
  }
}

bool
ComponentMaterialPropertyInterface::hasProperty(const std::string & property_name) const
{
  return std::find(_property_names.begin(), _property_names.end(), property_name) !=
         _property_names.end();
}

/// Return the name of the functor for that property
const MooseFunctorName &
ComponentMaterialPropertyInterface::getPropertyValue(const std::string & property_name,
                                                     const std::string & requestor_name) const
{
  for (const auto i : index_range(_property_names))
    if (_property_names[i] == property_name)
      return _property_functors[i];
  mooseError("Property '",
             property_name,
             "' was requested on Component '",
             name(),
             "' by Physics '",
             requestor_name,
             "'");
}
