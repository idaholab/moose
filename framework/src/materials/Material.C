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
  params.addParam<std::vector<SubdomainName> >("block", "The id or name of the block (subdomain) that this material represents.");
  params.addParam<std::vector<BoundaryName> >("boundary", "The id or name of the boundary that this material represents.");

  params.addPrivateParam<std::string>("built_by_action", "add_material");
  return params;
}


Material::Material(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    SetupInterface(parameters),
    Coupleable(parameters, false),
    ScalarCoupleable(parameters),
    FunctionInterface(parameters),
    UserObjectInterface(parameters),
    TransientInterface(parameters),
    MaterialPropertyInterface(parameters),
    PostprocessorInterface(parameters),
    _problem(*parameters.get<Problem *>("_problem")),
    _subproblem(*parameters.get<SubProblem *>("_subproblem")),
    _displaced_subproblem(parameters.get<SubProblem *>("_subproblem_displaced")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _bnd(parameters.get<bool>("_bnd")),
    _material_data(*parameters.get<MaterialData *>("_material_data")),
    _qrule(_bnd ? _subproblem.qRuleFace(_tid) : _subproblem.qRule(_tid)),
    _JxW(_bnd ? _subproblem.JxWFace(_tid) : _subproblem.JxW(_tid)),
    _coord(_subproblem.coords(_tid)),
    _q_point(_bnd ? _subproblem.pointsFace(_tid) : _subproblem.points(_tid)),
    _normals(_subproblem.assembly(_tid).normals()),
    _current_elem(_subproblem.elem(_tid)),
    _mesh(_subproblem.mesh()),
    _dim(_mesh.dimension()),
    _coord_sys(_subproblem.coordSystem(_tid)),
    _block_id(_mesh.getSubdomainIDs(parameters.get<std::vector<SubdomainName> >("block"))),
    _real_zero(_problem._real_zero[_tid]),
    _zero(_problem._zero[_tid]),
    _grad_zero(_problem._grad_zero[_tid]),
    _second_zero(_problem._second_zero[_tid]),
    _has_stateful_property(false)
{
/*
  for (unsigned int i = 0; i < _coupled_to.size(); i++)
  {
    std::string coupled_var_name = _coupled_to[i];

    //Is it in the nonlinear system or the aux system?
    if (_moose_system.hasVariable(coupled_var_name))
    {
      unsigned int coupled_var_num = _moose_system.getVariableNumber(coupled_var_name);
      _data._var_nums.insert(coupled_var_num);
    }
    //Look for it in the Aux system
    else if (_moose_system.hasAuxVariable(coupled_var_name))
    {
      unsigned int coupled_var_num = _moose_system.getAuxVariableNumber(coupled_var_name);
      _data._aux_var_nums.insert(coupled_var_num);
    }
    else
      mooseError("Coupled variable '" + coupled_var_name + "' not found.");
  }
  */
}

Material::~Material()
{
  // TODO: Implement destructor to clean up after the _qp_prev and _qp_curr data objects

  //std::for_each(_qp_prev.begin(), _qp_prev.end(), DeleteFunctor());
  //std::for_each(_qp_curr.begin(), _qp_curr.end(), DeleteFunctor());
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
  else
    for (_qp = 0; _qp < n_points; ++_qp)
      computeQpProperties();
}

void
Material::initQpStatefulProperties()
{
  libmesh_do_once(mooseWarning(std::string("Material \"") + _name + "\" declares one or more stateful properties but initQpStatefulProperties() was not overridden in the derived class."));
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

