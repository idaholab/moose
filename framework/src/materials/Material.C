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

InputParameters
Material::validParams()
{

  InputParameters params = MaterialBase::validParams();
  params += MaterialPropertyInterface::validParams();
  MooseEnum const_option("NONE=0 ELEMENT=1 SUBDOMAIN=2", "none");
  params.addParam<MooseEnum>(
      "constant_on",
      const_option,
      "When ELEMENT, MOOSE will only call computeQpProperties() for the 0th "
      "quadrature point, and then copy that value to the other qps."
      "When SUBDOMAIN, MOOSE will only call computeQpProperties() for the 0th "
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
    _constant_option(computeConstantOption()),
    _ghostable(true)
{
  // 1. Fill in the MooseVariable dependencies
  // 2. For ghost calculations we need to check and see whether this has any finite element
  // variables. If it does, then this material doesn't support ghost calculations
  // 3. For the purpose of ghost calculations, we will error if this material couples in both finite
  // element and finite volume variables.
  const std::vector<MooseVariableFieldBase *> & coupled_vars = getCoupledMooseVars();
  bool has_fe_vars = false;
  bool has_fv_vars = false;
  for (auto * const var : coupled_vars)
  {
    addMooseVariableDependency(var);
    if (var->isFV())
      has_fv_vars = true;
    else
    {
      has_fe_vars = true;
      _ghostable = false;
    }
  }

  // Note that this check will not catch a case in which a finite volume consumer needs a
  // non-variable-based property ghosted, but that non-variable-based property is computed within a
  // material that has finite element coupling (but not finite volume coupling)
  if (has_fe_vars && has_fv_vars)
    mooseError(
        "Your material ",
        this->name(),
        " couples in both FE and FV vars. To support ghost calculations which some FV "
        "consumers may need, multiphysics simulations should define separate materials for "
        "coupling in finite element and finite volume variables because we do not have a user "
        "friendly way of running DerivedMaterial::computeQpProperties and saying 'compute this "
        "property because it doesn't depend on finite element variables' or 'don't compute this "
        "property because it *does* depend on finite element variables'");
}

void
Material::subdomainSetup()
{
  if (_constant_option == ConstantTypeEnum::SUBDOMAIN)
  {
    auto nqp = _fe_problem.getMaxQps();

    MaterialProperties & props = materialData().props();
    for (const auto & prop_id : _supplied_prop_ids)
      props[prop_id]->resize(nqp);

    _qp = 0;
    computeQpProperties();

    for (const auto & prop_id : _supplied_prop_ids)
      for (decltype(nqp) qp = 1; qp < nqp; ++qp)
        props[prop_id]->qpCopy(qp, props[prop_id], 0);
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

  // If this Material ist set to be constant over elements, we take the
  // value computed for _qp == 0 and use it at all the quadrature points
  // in the element.
  if (_constant_option == ConstantTypeEnum::ELEMENT)
  {
    // Compute MaterialProperty values at the first qp.
    _qp = 0;
    computeQpProperties();

    // Now copy the values computed at qp 0 to all the other qps.
    for (const auto & prop_id : _supplied_prop_ids)
    {
      auto nqp = _qrule->n_points();
      for (decltype(nqp) qp = 1; qp < nqp; ++qp)
        props[prop_id]->qpCopy(qp, props[prop_id], 0);
    }
  }
  else
    for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
      computeQpProperties();
}

Material::ConstantTypeEnum
Material::computeConstantOption()
{
  auto co = getParam<MooseEnum>("constant_on").getEnum<ConstantTypeEnum>();

  // If the material is operating on a boundary we'll have to _at least_ run it
  // once per element, as there is no boundarySetup, and boundaries are worked
  // on as they are encountered on the elements while looping elements.
  if (_bnd && co == ConstantTypeEnum::SUBDOMAIN)
    co = ConstantTypeEnum::ELEMENT;

  return co;
}

MaterialBase &
Material::getMaterialByName(const std::string & name, bool no_warn, bool no_dep)
{
  if (!no_dep && _mi_feproblem.getCurrentExecuteOnFlag() != EXEC_INITIAL)
    mooseError("To ensure dependency resolution, discrete materials must be retrieved during "
               "initial setup. This is a code problem.");

  MaterialBase & discrete_mat = MaterialPropertyInterface::getMaterialByName(name, no_warn);

  if (!no_dep)
  {
    // Insert the materials requested by the discrete material into the host material who
    // retrieves this discrete material
    const auto & discrete_requested = discrete_mat.getRequestedItems();
    _requested_props.insert(discrete_requested.begin(), discrete_requested.end());
  }

  return discrete_mat;
}

void
Material::resolveOptionalProperties()
{
  for (auto & proxy : _optional_property_proxies)
    proxy->resolve(*this);
}
