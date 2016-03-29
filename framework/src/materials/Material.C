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

// libMesh includes
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<Material>()
{
  InputParameters params = validParams<MooseObject>();
  params += validParams<BlockRestrictable>();
  params += validParams<BoundaryRestrictable>();
  params += validParams<TransientInterface>();
  params += validParams<RandomInterface>();
  params += validParams<MaterialPropertyInterface>();

  params.addParam<bool>("use_displaced_mesh", false, "Whether or not this object should use the displaced mesh for computation.  Note that in the case this is true but no displacements are provided in the Mesh block the undisplaced mesh will still be used.");
  params.addParam<bool>("compute", true, "When false MOOSE will not call compute methods on this material, compute then must be called retrieving the Material object via MaterialPropertyInterface::getMaterial and calling the computeProerties method.");

  // Outputs
  params += validParams<OutputInterface>();
  params.set<std::vector<OutputName> >("outputs") =  std::vector<OutputName>(1, "none");
  params.addParam<std::vector<std::string> >("output_properties", "List of material properties, from this material, to output (outputs must also be defined to an output type)");

  params.addParamNamesToGroup("outputs output_properties", "Outputs");
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");
  params.registerBase("Material");

  return params;
}


Material::Material(const InputParameters & parameters) :
    MooseObject(parameters),
    BlockRestrictable(parameters),
    BoundaryRestrictable(parameters, blockIDs()),
    SetupInterface(this),
    Coupleable(parameters, false),
    MooseVariableDependencyInterface(),
    ScalarCoupleable(parameters),
    FunctionInterface(this),
    UserObjectInterface(this),
    TransientInterface(this),
    MaterialPropertyInterface(parameters, blockIDs(), boundaryIDs()),
    PostprocessorInterface(this),
    DependencyResolverInterface(),
    Restartable(parameters, "Materials"),
    ZeroInterface(parameters),
    MeshChangedInterface(parameters),

    // The false flag disables the automatic call buildOutputVariableHideList;
    // for Material objects the hide lists are handled by MaterialOutputAction
    OutputInterface(parameters, false),
    RandomInterface(parameters, *parameters.get<FEProblem *>("_fe_problem"), parameters.get<THREAD_ID>("_tid"), false),
    _subproblem(*parameters.get<SubProblem *>("_subproblem")),
    _fe_problem(*parameters.get<FEProblem *>("_fe_problem")),
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
    _has_stateful_property(false)
{
  // Fill in the MooseVariable dependencies
  const std::vector<MooseVariable *> & coupled_vars = getCoupledMooseVars();
  for (unsigned int i=0; i<coupled_vars.size(); i++)
    addMooseVariableDependency(coupled_vars[i]);
}


void
Material::initStatefulProperties(unsigned int n_points)
{
  if (_has_stateful_property)
    for (_qp = 0; _qp < n_points; ++_qp)
      initQpStatefulProperties();
}


void
Material::initQpStatefulProperties()
{
  mooseDoOnce(mooseWarning(std::string("Material \"") + name() + "\" declares one or more stateful properties but initQpStatefulProperties() was not overridden in the derived class."));
}

void
Material::checkStatefulSanity() const
{
  for (std::map<std::string, int>::const_iterator it = _props_to_flags.begin(); it != _props_to_flags.end(); ++it)
  {
    if (static_cast<int>(it->second) % 2 == 0) // Only Stateful properties declared!
      mooseError("Material '" << name() << "' has stateful properties declared but not associated \"current\" properties." << it->second);
  }
}


void
Material::registerPropName(std::string prop_name, bool is_get, Material::Prop_State state)
{
  if (!is_get)
  {
    _props_to_flags[prop_name] |= static_cast<int>(state);
    if (static_cast<int>(state) % 2 == 0)
      _has_stateful_property = true;
  }

  // Store material properties for block ids
  for (std::set<SubdomainID>::const_iterator it = blockIDs().begin(); it != blockIDs().end(); ++it)
  {
    // Only save this prop as a "supplied" prop is it was registered as a result of a call to declareProperty not getMaterialProperty
    if (!is_get)
      _supplied_props.insert(prop_name);
    _fe_problem.storeMatPropName(*it, prop_name);
  }

  // Store material properties for the boundary ids
  for (std::set<BoundaryID>::const_iterator it = boundaryIDs().begin(); it != boundaryIDs().end(); ++it)
  {
    // Only save this prop as a "supplied" prop is it was registered as a result of a call to declareProperty not getMaterialProperty
    if (!is_get)
      _supplied_props.insert(prop_name);
    _fe_problem.storeMatPropName(*it, prop_name);
  }
}


std::set<OutputName>
Material::getOutputs()
{
  const std::vector<OutputName> & out = getParam<std::vector<OutputName> >("outputs");
  return std::set<OutputName>(out.begin(), out.end());
}


bool
Material::hasBlockMaterialPropertyHelper(const std::string & name)
{
  // Reference to the warehouse
  const MaterialWarehouse<Material> & warehouse = _fe_problem.getMaterialWarehouse();

  // Complete set of ids that this object is active
  const std::set<SubdomainID> & ids = hasBlocks(Moose::ANY_BLOCK_ID) ? meshBlockIDs() : blockIDs();

  // Loop over each id for this object
  for (std::set<SubdomainID>::const_iterator id_it = ids.begin(); id_it != ids.end(); ++id_it)
  {
    // Storage of material properties that have been DECLARED on this id
    std::set<std::string> declared_props;

    // If block materials exist, populated the set of properties that were declared
    if (warehouse[_material_data_type].hasActiveBlockObjects(*id_it))
    {
      const std::vector<MooseSharedPointer<Material> > & mats = warehouse[_material_data_type].getActiveBlockObjects(*id_it);
      for (std::vector<MooseSharedPointer<Material> >::const_iterator mat_it = mats.begin(); mat_it != mats.end(); ++mat_it)
      {
        const std::set<std::string> & mat_props = (*mat_it)->getSuppliedItems();
        declared_props.insert(mat_props.begin(), mat_props.end());
      }
    }

    // If the supplied property is not in the list of properties on the current id, return false
    if (declared_props.find(name) == declared_props.end())
      return false;
  }

  // If you get here the supplied property is defined on all blocks
  return true;
}


void
Material::computeProperties()
{
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    computeQpProperties();
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
    mooseDoOnce(mooseWarning("You disabled the computation of this (" << name() << ") material by MOOSE, but have not overridden the 'resetQpProperties' method, this can lead to unintended values being set for material property values."));
}


void
Material::computeProperties(unsigned int qp)
{
  _qp = qp;
  computeQpProperties();
}
