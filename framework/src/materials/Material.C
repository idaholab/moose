/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

// MOOSE includes
#include "Material.h"
#include "SubProblem.h"
#include "MaterialData.h"
#include "Assembly.h"
#include "Executioner.h"
#include "Transient.h"

// libMesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<Material>()
{
  InputParameters params = validParams<MooseObject>();
  params += validParams<BlockRestrictable>();
  params += validParams<BoundaryRestrictable>();
  params += validParams<TransientInterface>();
  params += validParams<RandomInterface>();
  params += validParams<MaterialPropertyInterface>();

  params.addParam<bool>("use_displaced_mesh",
                        false,
                        "Whether or not this object should use the "
                        "displaced mesh for computation.  Note that "
                        "in the case this is true but no "
                        "displacements are provided in the Mesh block "
                        "the undisplaced mesh will still be used.");
  params.addParam<bool>("compute",
                        true,
                        "When false, MOOSE will not call compute methods on this material. "
                        "The user must call computeProperties() after retrieving the Material "
                        "via MaterialPropertyInterface::getMaterial(). "
                        "Non-computed Materials are not sorted for dependencies.");
  params.addParam<bool>("constant_on_elem",
                        false,
                        "When true, MOOSE will only call computeQpProperties() for the 0th "
                        "quadrature point, and then copy that value to the other qps.");

  // Outputs
  params += validParams<OutputInterface>();
  params.set<std::vector<OutputName>>("outputs") = {"none"};
  params.addParam<std::vector<std::string>>(
      "output_properties",
      "List of material properties, from this material, to output (outputs "
      "must also be defined to an output type)");

  params.addParamNamesToGroup("outputs output_properties", "Outputs");
  params.addParamNamesToGroup("use_displaced_mesh constant_on_elem", "Advanced");
  params.registerBase("Material");

  return params;
}

Material::Material(const InputParameters & parameters)
  : MooseObject(parameters),
    BlockRestrictable(parameters),
    BoundaryRestrictable(parameters, blockIDs(), false), // false for being _not_ nodal
    SetupInterface(this),
    Coupleable(this, false),
    MooseVariableDependencyInterface(),
    ScalarCoupleable(this),
    FunctionInterface(this),
    DistributionInterface(this),
    UserObjectInterface(this),
    TransientInterface(this),
    MaterialPropertyInterface(this, blockIDs(), boundaryIDs()),
    PostprocessorInterface(this),
    VectorPostprocessorInterface(this),
    DependencyResolverInterface(),
    Restartable(parameters, "Materials"),
    ZeroInterface(parameters),
    MeshChangedInterface(parameters),

    // The false flag disables the automatic call buildOutputVariableHideList;
    // for Material objects the hide lists are handled by MaterialOutputAction
    OutputInterface(parameters, false),
    RandomInterface(parameters,
                    *parameters.get<FEProblemBase *>("_fe_problem_base"),
                    parameters.get<THREAD_ID>("_tid"),
                    false),
    _subproblem(*parameters.get<SubProblem *>("_subproblem")),
    _fe_problem(*parameters.get<FEProblemBase *>("_fe_problem_base")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid)),
    _bnd(_material_data_type != Moose::BLOCK_MATERIAL_DATA),
    _neighbor(_material_data_type == Moose::NEIGHBOR_MATERIAL_DATA),
    _qp(std::numeric_limits<unsigned int>::max()),
    _qrule(_bnd ? _assembly.qRuleFace() : _assembly.qRule()),
    _JxW(_bnd ? _assembly.JxWFace() : _assembly.JxW()),
    _coord(_assembly.coordTransformation()),
    _q_point(_bnd ? _assembly.qPointsFace() : _assembly.qPoints()),
    _normals(_assembly.normals()),
    _current_elem(_neighbor ? _assembly.neighbor() : _assembly.elem()),
    _current_side(_neighbor ? _assembly.neighborSide() : _assembly.side()),
    _mesh(_subproblem.mesh()),
    _coord_sys(_assembly.coordSystem()),
    _compute(getParam<bool>("compute")),
    _constant_on_elem(getParam<bool>("constant_on_elem")),
    _has_stateful_property(false)
{
  // Fill in the MooseVariable dependencies
  const std::vector<MooseVariable *> & coupled_vars = getCoupledMooseVars();
  for (const auto & var : coupled_vars)
    addMooseVariableDependency(var);
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
      mooseError(std::string("Material \"") + name() + "\" provides one or more stateful "
                                                       "properties but initQpStatefulProperties() "
                                                       "was not overridden in the derived class.");
}

void
Material::initQpStatefulProperties()
{
  _overrides_init_stateful_props = false;
}

void
Material::checkStatefulSanity() const
{
  for (const auto & it : _props_to_flags)
    if (static_cast<int>(it.second) % 2 == 0) // Only Stateful properties declared!
      mooseError("Material '", name(), "' requests undefined stateful property '", it.first, "'");
}

void
Material::registerPropName(std::string prop_name, bool is_get, Material::Prop_State state)
{
  if (!is_get)
  {
    _supplied_props.insert(prop_name);
    _supplied_prop_ids.insert(_material_data->getPropertyId(prop_name));

    _props_to_flags[prop_name] |= static_cast<int>(state);
    if (static_cast<int>(state) % 2 == 0)
      _has_stateful_property = true;
  }

  // Store material properties for block ids
  for (const auto & block_id : blockIDs())
    _fe_problem.storeMatPropName(block_id, prop_name);

  // Store material properties for the boundary ids
  for (const auto & boundary_id : boundaryIDs())
    _fe_problem.storeMatPropName(boundary_id, prop_name);
}

std::set<OutputName>
Material::getOutputs()
{
  const std::vector<OutputName> & out = getParam<std::vector<OutputName>>("outputs");
  return std::set<OutputName>(out.begin(), out.end());
}

void
Material::computeProperties()
{
  // If this Material has the _constant_on_elem flag set, we take the
  // value computed for _qp==0 and use it at all the quadrature points
  // in the Elem.
  if (_constant_on_elem)
  {
    // Compute MaterialProperty values at the first qp.
    _qp = 0;
    computeQpProperties();

    // Reference to *all* the MaterialProperties in the MaterialData object, not
    // just the ones for this Material.
    MaterialProperties & props = _material_data->props();

    // Now copy the values computed at qp 0 to all the other qps.
    for (const auto & prop_id : _supplied_prop_ids)
    {
      auto nqp = _qrule->n_points();
      for (decltype(nqp) qp = 1; qp < nqp; ++qp)
        props[prop_id]->qpCopy(qp, props[prop_id], 0);
    }
  }
  else
  {
    for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
      computeQpProperties();
  }
}

void
Material::computeQpProperties()
{
}

void
Material::resetProperties()
{
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    resetQpProperties();
}

void
Material::resetQpProperties()
{
  if (!_compute)
    mooseDoOnce(mooseWarning("You disabled the computation of this (",
                             name(),
                             ") material by MOOSE, but have not overridden the 'resetQpProperties' "
                             "method, this can lead to unintended values being set for material "
                             "property values."));
}

void
Material::computePropertiesAtQp(unsigned int qp)
{
  _qp = qp;
  computeQpProperties();
}

void
Material::checkExecutionStage()
{
  if (_fe_problem.startedInitialSetup())
    mooseError("Material properties must be retrieved during material object construction to "
               "ensure correct dependency resolution.");
}
