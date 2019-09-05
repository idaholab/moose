//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowMaterial.h"
#include "libmesh/quadrature.h"
#include <limits>

template <>
InputParameters
validParams<PorousFlowMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names");
  params.addParam<bool>(
      "at_nodes", false, "Evaluate Material properties at nodes instead of quadpoints");
  params.addPrivateParam<std::string>("pf_material_type", "pf_material");
  params.addClassDescription("This generalises MOOSE's Material class to allow for Materials that "
                             "hold information related to the nodes in the finite element");
  return params;
}

PorousFlowMaterial::PorousFlowMaterial(const InputParameters & parameters)
  : Material(parameters),
    _nodal_material(getParam<bool>("at_nodes")),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _pressure_variable_name("pressure_variable"),
    _saturation_variable_name("saturation_variable"),
    _temperature_variable_name("temperature_variable"),
    _mass_fraction_variable_name("mass_fraction_variable")
{
}

void
PorousFlowMaterial::initialSetup()
{
  if (_nodal_material)
    _material_data->onlyResizeIfSmaller(true);
}

void
PorousFlowMaterial::initStatefulProperties(unsigned int n_points)
{
  if (_nodal_material)
  {
    // size the properties to max(number_of_nodes, number_of_quadpoints)
    sizeNodalProperties();

    // compute the values for number_of_nodes
    Material::initStatefulProperties(_current_elem->n_nodes());
  }
  else
    Material::initStatefulProperties(n_points);
}

void
PorousFlowMaterial::computeProperties()
{
  if (_nodal_material)
  {
    // size the properties to max(number_of_nodes, number_of_quadpoints)
    sizeNodalProperties();

    // compute the values for number_of_nodes
    for (_qp = 0; _qp < _current_elem->n_nodes(); ++_qp)
      computeQpProperties();
  }
  else
    Material::computeProperties();
}

void
PorousFlowMaterial::sizeNodalProperties()
{
  /*
   * For nodal materials, the Properties should be sized as the maximum of
   * the number of nodes and the number of quadpoints.
   * We only actually need "number of nodes" pieces of information, which are
   * computed by computeProperties(), so the n_points - _current_elem->n_nodes()
   * elements at the end of the std::vector will always be zero, but they
   * are needed because MOOSE does copy operations (etc) that assumes that
   * the std::vector is sized to number of quadpoints.
   *
   * On boundary materials, the number of nodes may be larger than the number of
   * qps on the face of the element, in which case the remaining entries in the
   * material properties storage will be zero.
   */
  _material_data->resize(std::max(_current_elem->n_nodes(), _qrule->n_points()));
}

unsigned
PorousFlowMaterial::nearestQP(unsigned nodenum) const
{
  unsigned nearest_qp = 0;
  Real smallest_dist = std::numeric_limits<Real>::max();
  for (unsigned qp = 1; qp < _qrule->n_points(); ++qp)
  {
    const Real this_dist = (_current_elem->point(nodenum) - _q_point[qp]).norm();
    if (this_dist < smallest_dist)
    {
      nearest_qp = qp;
      smallest_dist = this_dist;
    }
  }
  return nearest_qp;
}
