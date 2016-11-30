/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowNodalValueMaterial.h"

template<>
InputParameters validParams<PorousFlowNodalValueMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addParam<bool>("always_resize_nodal_properties", true, "It set to false then the nodal properties of this Material will only be resized at the beginning of a simulation, which is computationally efficient if no mesh adaptivity is being used.");
  params.addClassDescription("This Material ensures that the size of each material property is equal to the number of nodes in the element");
  return params;
}

PorousFlowNodalValueMaterial::PorousFlowNodalValueMaterial(const InputParameters & parameters) :
    Material(parameters),
    _always_resize(getParam<bool>("always_resize_nodal_properties")),
    _nodenum(0)
{
}

void
PorousFlowNodalValueMaterial::initStatefulProperties(unsigned int n_points)
{
  Material::initStatefulProperties(n_points);

  sizeNodalStatefulProperties();
  for (_nodenum = 0; _nodenum < _current_elem->n_nodes(); ++_nodenum)
    initNodalStatefulProperties();
}

void
PorousFlowNodalValueMaterial::sizeNodalStatefulProperties()
{
}

void
PorousFlowNodalValueMaterial::initNodalStatefulProperties()
{
}

void
PorousFlowNodalValueMaterial::computeProperties()
{
  Material::computeProperties();

  if (_always_resize || _t_step <= 1)
    sizeNodalProperties();

  for (_nodenum = 0; _nodenum < _current_elem->n_nodes(); ++_nodenum)
    computeNodalProperties();
}

void
PorousFlowNodalValueMaterial::sizeNodalProperties()
{
}

void
PorousFlowNodalValueMaterial::computeNodalProperties()
{
}

void
PorousFlowNodalValueMaterial::sizeNodalProperty(const std::string & prop_name)
{
  mooseAssert(_material_data->getMaterialPropertyStorage().hasProperty(prop_name), "PorousFlowNodalValueMaterial can not find nodal property " << prop_name);
  const unsigned prop_id = _material_data->getMaterialPropertyStorage().retrievePropertyId(prop_name);
  // _material_data->props() returns MaterialProperties, which is a std::vector of PropertyValue.
  _material_data->props()[prop_id]->resize(_current_elem->n_nodes());
}

void
PorousFlowNodalValueMaterial::sizeAllRequestedProperties()
{
  for (auto prop_name : getRequestedItems())
    sizeNodalProperty(prop_name);
}
