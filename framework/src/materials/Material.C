//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "Material.h"

template <>
InputParameters
validParams<Material>()
{
  InputParameters params = validParams<MaterialBase>();
  params += validParams<BlockRestrictable>();
  params += validParams<BoundaryRestrictable>();
  params += validParams<MaterialPropertyInterface>();
  MooseEnum const_option("NONE=0 ELEMENT=1 SUBDOMAIN=2", "none");
  params.addParam<MooseEnum>(
      "constant_on",
      const_option,
      "When ELEMENT, MOOSE will only call computeQpProperties() for the 0th "
      "quadrature point, and then copy that value to the other qps."
      "When SUBDOMAIN, MOOSE will only call computeSubdomainProperties() for the 0th "
      "quadrature point, and then copy that value to the other qps. Evaluations on element qps "
      "will be skipped");
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");
  return params;
}

Material::Material(const InputParameters & parameters)
  : MaterialBase(parameters),
    Coupleable(this, false),
    MaterialPropertyInterface(this, blockIDs(), boundaryIDs()),
    _bnd(_material_data_type != Moose::BLOCK_MATERIAL_DATA),
    _neighbor(_material_data_type == Moose::NEIGHBOR_MATERIAL_DATA),
    _q_point(_bnd ? _assembly.qPointsFace() : _assembly.qPoints()),
    _qrule(_bnd ? _assembly.qRuleFace() : _assembly.qRule()),
    _JxW(_bnd ? _assembly.JxWFace() : _assembly.JxW()),
    _current_elem(_neighbor ? _assembly.neighbor() : _assembly.elem()),
    _current_subdomain_id(_neighbor ? _assembly.currentNeighborSubdomainID()
                                    : _assembly.currentSubdomainID()),
    _current_side(_neighbor ? _assembly.neighborSide() : _assembly.side()),
    _constant_option(getParam<MooseEnum>("constant_on").getEnum<ConstantTypeEnum>())
{
  // Fill in the MooseVariable dependencies
  const std::vector<MooseVariableFEBase *> & coupled_vars = getCoupledMooseVars();
  for (const auto & var : coupled_vars)
    addMooseVariableDependency(var);
}

void
Material::subdomainSetup()
{
  if (_constant_option == ConstantTypeEnum::SUBDOMAIN)
  {
    unsigned int nqp = _fe_problem.getMaxQps();

    MaterialProperties & props = materialData().props();
    for (const auto & prop_id : _supplied_prop_ids)
      props[prop_id]->resize(nqp);

    _qp = 0;
    computeSubdomainProperties();

    for (const auto & prop_id : _supplied_prop_ids)
    {
      for (decltype(nqp) qp = 1; qp < nqp; ++qp)
        props[prop_id]->qpCopy(qp, props[prop_id], 0);
    }
  }
}

void
Material::computeProperties()
{
  if (_constant_option == ConstantTypeEnum::SUBDOMAIN)
    return;

  // Reference to *all* the MaterialProperties in the MaterialData object, not
  // just the ones for this Material.
  MaterialProperties & props = _material_data->props();

  // If this Material has the _constant_on_elem flag set, we take the
  // value computed for _qp==0 and use it at all the quadrature points
  // in the Elem.
  if (_constant_option == ConstantTypeEnum::ELEMENT)
  {
    // Compute MaterialProperty values at the first qp.
    _qp = 0;
    computeQpProperties();

    // Now copy the values computed at qp 0 to all the other qps.
    for (const auto & prop_id : _supplied_regular_prop_ids)
    {
      auto nqp = _qrule->n_points();
      for (decltype(nqp) qp = 1; qp < nqp; ++qp)
        props[prop_id]->qpCopy(qp, props[prop_id], 0);
    }
    for (const auto & prop_id : _supplied_ad_prop_ids)
    {
      auto nqp = _qrule->n_points();
      for (decltype(nqp) qp = 1; qp < nqp; ++qp)
        props[prop_id]->qpCopy(qp, props[prop_id], 0);
    }
    copyDualNumbersToValues();
  }
  else
  {
    for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
      computeQpProperties();
    copyDualNumbersToValues();
  }
}

// void
// Material::resetProperties()
// {
//   for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
//     resetQpProperties();
// }

void
Material::computePropertiesAtQp(unsigned int qp)
{
  _qp = qp;
  computeQpProperties();
}

void
Material::initStatefulProperties(unsigned int n_points)
{
  for (_qp = 0; _qp < n_points; ++_qp)
    initQpStatefulProperties();

  // checking for statefulness of properties via this loop is necessary
  // because owned props might have been promoted to stateful by calls to
  // getMaterialProperty[Old/Older] from other objects.  In these cases, this
  // object won't otherwise know that it owns stateful properties.
  for (auto & prop : _supplied_props)
    if (_material_data->getMaterialPropertyStorage().isStatefulProp(prop) &&
        !_overrides_init_stateful_props)
      mooseError(std::string("Material \"") + name() +
                 "\" provides one or more stateful "
                 "properties but initQpStatefulProperties() "
                 "was not overridden in the derived class.");
}
