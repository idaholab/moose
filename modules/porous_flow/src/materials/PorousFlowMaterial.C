/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowMaterial.h"

template<>
InputParameters validParams<PorousFlowMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<UserObjectName>("PorousFlowDictator", "The UserObject that holds the list of Porous-Flow variable names");
  params.addParam<bool>("always_resize_nodal_properties", true, "It set to false then the nodal properties of this Material will only be resized at the beginning of a simulation, which is computationally efficient if no mesh adaptivity is being used.");
  params.addClassDescription("This generalises MOOSE's Material class to allow for Materials that hold information related to the nodes in the finite element");
  return params;
}

PorousFlowMaterial::PorousFlowMaterial(const InputParameters & parameters) :
    Material(parameters),
    _nodal_material(false),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _always_resize(getParam<bool>("always_resize_nodal_properties"))
{
}

void
PorousFlowMaterial::initStatefulProperties(unsigned int n_points)
{
  if (_nodal_material)
  {
    sizeAllRequestedProperties();
    Material::initStatefulProperties(_current_elem->n_nodes()); //n_points); // should be _current_elem->n_nodes();
  }
  else
    Material::initStatefulProperties(n_points);
}


void
PorousFlowMaterial::computeProperties()
{
  if (_nodal_material) // & false) // get rid of false
  {
    if (_always_resize || _t_step <= 1)
      sizeAllRequestedProperties();
    for (_qp = 0; _qp < _current_elem->n_nodes(); ++_qp)
      computeQpProperties();
  }
  else
    Material::computeProperties();
}

void
PorousFlowMaterial::sizeNodalProperty(const std::string & prop_name)
{
  mooseAssert(_material_data->getMaterialPropertyStorage().hasProperty(prop_name), "PorousFlowMaterial can not find nodal property " << prop_name);
  //Moose::out << "resizing " << prop_name << " to size " << _current_elem->n_nodes() << "\n";
  const unsigned prop_id = _material_data->getMaterialPropertyStorage().retrievePropertyId(prop_name);
  // _material_data->props() returns MaterialProperties, which is a std::vector of PropertyValue.
  _material_data->props()[prop_id]->resize(_current_elem->n_nodes());
}

void
PorousFlowMaterial::sizeAllRequestedProperties()
{
  for (auto prop_name : getSuppliedItems())
    sizeNodalProperty(prop_name);
}
