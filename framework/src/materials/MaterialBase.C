//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MaterialBase.h"
#include "SubProblem.h"
#include "Assembly.h"
#include "Executioner.h"
#include "Transient.h"

#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<MaterialBase>()
{
  InputParameters params = validParams<MooseObject>();
  params += validParams<TransientInterface>();
  params += validParams<RandomInterface>();

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
                        "The user must call computeProperties() after retrieving the MaterialBase "
                        "via MaterialBasePropertyInterface::getMaterialBase(). "
                        "Non-computed MaterialBases are not sorted for dependencies.");

  params.addPrivateParam<bool>("_neighbor", false);

  // Outputs
  params += validParams<OutputInterface>();
  params.set<std::vector<OutputName>>("outputs") = {"none"};
  params.addParam<std::vector<std::string>>(
      "output_properties",
      "List of material properties, from this material, to output (outputs "
      "must also be defined to an output type)");

  params.addParamNamesToGroup("outputs output_properties", "Outputs");
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");
  params.registerBase("MaterialBase");
  return params;
}

MaterialBase::MaterialBase(const InputParameters & parameters)
  : MooseObject(parameters),
    SetupInterface(this),
    BlockRestrictable(this),
    BoundaryRestrictable(this, blockIDs(), false), // false for being _not_ nodal
    MooseVariableDependencyInterface(),
    ScalarCoupleable(this),
    FunctionInterface(this),
    DistributionInterface(this),
    UserObjectInterface(this),
    TransientInterface(this),
    PostprocessorInterface(this),
    VectorPostprocessorInterface(this),
    DependencyResolverInterface(),
    Restartable(this, "MaterialBases"),
    MeshChangedInterface(parameters),

    // The false flag disables the automatic call buildOutputVariableHideList;
    // for MaterialBase objects the hide lists are handled by MaterialBaseOutputAction
    OutputInterface(parameters, false),
    RandomInterface(parameters,
                    *parameters.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base"),
                    parameters.get<THREAD_ID>("_tid"),
                    false),
    _subproblem(*getCheckedPointerParam<SubProblem *>("_subproblem")),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid)),
    _qp(std::numeric_limits<unsigned int>::max()),
    _coord(_assembly.coordTransformation()),
    _normals(_assembly.normals()),
    _mesh(_subproblem.mesh()),
    _coord_sys(_assembly.coordSystem()),
    _compute(getParam<bool>("compute")),
    _has_stateful_property(false)
{
}

void
MaterialBase::initStatefulProperties(unsigned int n_points)
{
  for (_qp = 0; _qp < n_points; ++_qp)
    initQpStatefulProperties();

  // checking for statefulness of properties via this loop is necessary
  // because owned props might have been promoted to stateful by calls to
  // getMaterialProperty[Old/Older] from other objects.  In these cases, this
  // object won't otherwise know that it owns stateful properties.
  for (auto & prop : _supplied_props)
    if (materialData().getMaterialPropertyStorage().isStatefulProp(prop) &&
        !_overrides_init_stateful_props)
      mooseError(std::string("Material \"") + name() +
                 "\" provides one or more stateful "
                 "properties but initQpStatefulProperties() "
                 "was not overridden in the derived class.");
}

void
MaterialBase::initQpStatefulProperties()
{
  _overrides_init_stateful_props = false;
}

void
MaterialBase::checkStatefulSanity() const
{
  for (const auto & it : _props_to_flags)
    if (static_cast<int>(it.second) % 2 == 0) // Only Stateful properties declared!
      mooseError("Material '", name(), "' requests undefined stateful property '", it.first, "'");
}

void
MaterialBase::registerPropName(std::string prop_name,
                               bool is_get,
                               Prop_State state,
                               bool is_declared_ad)
{
  if (!is_get)
  {
    _supplied_props.insert(prop_name);
    const auto & property_id = materialData().getPropertyId(prop_name);
    _supplied_prop_ids.insert(property_id);
    if (is_declared_ad)
      _supplied_ad_prop_ids.insert(property_id);
    else
      _supplied_regular_prop_ids.insert(property_id);

    _props_to_flags[prop_name] |= static_cast<int>(state);
    if (static_cast<int>(state) % 2 == 0)
      _has_stateful_property = true;
  }

  registerMaterials(prop_name);
}

std::set<OutputName>
MaterialBase::getOutputs()
{
  const std::vector<OutputName> & out = getParam<std::vector<OutputName>>("outputs");
  return std::set<OutputName>(out.begin(), out.end());
}

void
MaterialBase::computeSubdomainProperties()
{
  mooseError("MaterialBase::computeSubdomainQpProperties in Material '",
             name(),
             "' needs to be implemented");
}

void
MaterialBase::subdomainSetup()
{
  mooseError("MaterialBase::subdomainSetup in Material'", name(), "' needs to be implemented");
}

void
MaterialBase::computeProperties()
{
  mooseError("MaterialBase::computeProperties in Material '", name(), "' needs to be implemented");
}

void
MaterialBase::computeQpProperties()
{
}

void
MaterialBase::resetProperties()
{
  for (_qp = 0; _qp < qRule().n_points(); ++_qp)
    resetQpProperties();
}

void
MaterialBase::resetQpProperties()
{
  if (!_compute)
    mooseDoOnce(mooseWarning("You disabled the computation of this (",
                             name(),
                             ") material by MOOSE, but have not overridden the 'resetQpProperties' "
                             "method, this can lead to unintended values being set for material "
                             "property values."));
}

void
MaterialBase::computePropertiesAtQp(unsigned int qp)
{
  _qp = qp;
  computeQpProperties();
}

void
MaterialBase::checkExecutionStage()
{
  if (_fe_problem.startedInitialSetup())
    mooseError("Material properties must be retrieved during material object construction to "
               "ensure correct dependency resolution.");
}

void
MaterialBase::copyDualNumbersToValues()
{
  if (!_fe_problem.currentlyComputingJacobian() || !_fe_problem.usingADMatProps())
    return;

  MaterialProperties & props = materialData().props();
  for (_qp = 0; _qp < qRule().n_points(); ++_qp)
    for (const auto & prop_id : _supplied_ad_prop_ids)
      props[prop_id]->copyDualNumberToValue(_qp);
}
