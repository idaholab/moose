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
  params.addPrivateParam<std::string>("built_by_action", "add_material");
  return params;
}


Material::Material(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    BlockRestrictable(parameters),
    BoundaryRestrictable(parameters),
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
    _subproblem(*parameters.get<SubProblem *>("_subproblem")),
    _displaced_subproblem(parameters.get<SubProblem *>("_subproblem_displaced")),
    _fe_problem(*parameters.get<FEProblem *>("_fe_problem")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid)),
    _bnd(parameters.get<bool>("_bnd")),
    _neighbor(getParam<bool>("_neighbor")),
    _material_data(*parameters.get<MaterialData *>("_material_data")),
    _qrule(_bnd ? _assembly.qRuleFace() : _assembly.qRule()),
    _JxW(_bnd ? _assembly.JxWFace() : _assembly.JxW()),
    _coord(_assembly.coordTransformation()),
    _q_point(_bnd ? _assembly.qPointsFace() : _assembly.qPoints()),
    _normals(_assembly.normals()),
    _current_elem(_neighbor ? _assembly.neighbor() : _assembly.elem()),
    _current_side(_neighbor ? _assembly.neighborSide() : _assembly.side()),
    _mesh(_subproblem.mesh()),
    _dim(_mesh.dimension()),
    _coord_sys(_assembly.coordSystem()),
    _real_zero(_subproblem._real_zero[_tid]),
    _zero(_subproblem._zero[_tid]),
    _grad_zero(_subproblem._grad_zero[_tid]),
    _second_zero(_subproblem._second_zero[_tid]),
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
