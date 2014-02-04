
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

#include "Material.h"
#include "SubProblem.h"
#include "MaterialData.h"

// system includes
#include <iostream>

template<>
InputParameters validParams<Material>()
{
  InputParameters params = validParams<MooseObject>();
  params += validParams<BlockRestrictable>();
  params += validParams<BoundaryRestrictable>();

  params.addParam<bool>("use_displaced_mesh", false, "Whether or not this object should use the displaced mesh for computation.  Note that in the case this is true but no displacements are provided in the Mesh block the undisplaced mesh will still be used.");
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");

  params.registerBase("Material");

  return params;
}


Material::Material(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    BlockRestrictable(name, parameters),
    BoundaryRestrictable(name, parameters),
    SetupInterface(parameters),
    Coupleable(parameters, false),
    MooseVariableDependencyInterface(),
    ScalarCoupleable(parameters),
    FunctionInterface(parameters),
    UserObjectInterface(parameters),
    TransientInterface(parameters, name, "materials"),
    MaterialPropertyInterface(parameters),
    PostprocessorInterface(parameters),
    DependencyResolverInterface(),
    Restartable(name, parameters, "Materials"),
    ZeroInterface(parameters),
    _subproblem(*parameters.get<SubProblem *>("_subproblem")),
    _fe_problem(*parameters.get<FEProblem *>("_fe_problem")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid)),
    _bnd(parameters.get<bool>("_bnd")),
    _neighbor(getParam<bool>("_neighbor")),
    _material_data(*parameters.get<MaterialData *>("_material_data")),
    _qp(std::numeric_limits<unsigned int>::max()),
    _qrule(_bnd ? _assembly.qRuleFace() : _assembly.qRule()),
    _JxW(_bnd ? _assembly.JxWFace() : _assembly.JxW()),
    _coord(_assembly.coordTransformation()),
    _q_point(_bnd ? _assembly.qPointsFace() : _assembly.qPoints()),
    _normals(_assembly.normals()),
    _current_elem(_neighbor ? _assembly.neighbor() : _assembly.elem()),
    _current_side(_neighbor ? _assembly.neighborSide() : _assembly.side()),
    _mesh(_subproblem.mesh()),
//    _dim(_mesh.dimension()),
    _coord_sys(_assembly.coordSystem()),
    _has_stateful_property(false)
{
  // Fill in the MooseVariable dependencies
  const std::vector<MooseVariable *> & coupled_vars = getCoupledMooseVars();
  for(unsigned int i=0; i<coupled_vars.size(); i++)
    addMooseVariableDependency(coupled_vars[i]);
}

Material::~Material()
{
}

void
Material::computeProperties()
{
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    computeQpProperties();
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
  mooseDoOnce(mooseWarning(std::string("Material \"") + _name + "\" declares one or more stateful properties but initQpStatefulProperties() was not overridden in the derived class."));
}

void
Material::computeQpProperties()
{
}

void
Material::timeStepSetup()
{}

QpData *
Material::createData()
{
  return NULL;
}

void
Material::checkStatefulSanity() const
{
  for (std::map<std::string, int>::const_iterator it = _props_to_flags.begin(); it != _props_to_flags.end(); ++it)
  {
    if (static_cast<int>(it->second) % 2 == 0) // Only Stateful properties declared!
      mooseError("Material '" << _name << "' has stateful properties declared but not associated \"current\" properties." << it->second);
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
    _subproblem.storeMatPropName(*it, prop_name);
  }

  // Store material properites for the boundary ids
  for (std::set<BoundaryID>::const_iterator it = boundaryIDs().begin(); it != boundaryIDs().end(); ++it)
  {
    // \TODO: see ticket #2192
    // Only save this prop as a "supplied" prop is it was registered as a result of a call to declareProperty not getMaterialProperty
    // if (!is_get)
    //  _supplied_props.insert(prop_name);
    _fe_problem.storeMatPropName(*it, prop_name);
    _subproblem.storeMatPropName(*it, prop_name);
  }
}
