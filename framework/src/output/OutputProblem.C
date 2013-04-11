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

#include "OutputProblem.h"
#include "MooseMesh.h"

#include "libmesh/equation_systems.h"
#include "libmesh/explicit_system.h"
#include "libmesh/mesh_function.h"

template<>
InputParameters validParams<OutputProblem>()
{
  InputParameters params = validParams<Problem>();
  params.addRequiredParam<FEProblem *>("mproblem", "The FE problem containing this OutputProblem");
  params.addRequiredParam<unsigned int>("refinements", "The number of refinements to use in the oversampled mesh");
  params.addParam<Point>("position", "Set a positional offset.  This vector will get added to the nodal cooardinates to move the domain.");
  return params;
}


OutputProblem::OutputProblem(const std::string & name, InputParameters parameters):
    Problem(name, parameters),
    _mproblem(*parameters.get<FEProblem *>("mproblem")),
    _mesh(_mproblem.mesh().clone()),
    _eq(_mesh),
    _out(_mproblem, _eq),
    _mesh_might_change(true)
{
  // The mesh in this system will be finer than the nonlinear system mesh
  MeshRefinement mesh_refinement(_mesh);
  mesh_refinement.uniformly_refine(parameters.get<unsigned int>("refinements"));

  if(isParamValid("position"))
  {
    _position = getParam<Point>("position");
    moveMesh();
  }

  EquationSystems & source_es = _mproblem.es();

  unsigned int num_systems = source_es.n_systems();

  if(!getParam<unsigned int>("refinements") && !_mproblem.adaptivity().isOn()) // If the mesh isn't changing
    _mesh_might_change = false;

  if(_mesh_might_change)
  {
    _mesh_functions.resize(num_systems);

    for(unsigned int sys_num=0; sys_num<num_systems; sys_num++)
    {
      System & source_sys = source_es.get_system(sys_num);

      // Add the system to the es
      ExplicitSystem & dest_sys = _eq.add_system<ExplicitSystem>(source_sys.name());

      unsigned int num_vars = source_sys.n_vars();

      // The system may be empty (auxiliary system is optional)
      if (num_vars)
      {
        _mesh_functions[sys_num].resize(num_vars);

        _serialized_solution = NumericVector<Number>::build().release();
        _serialized_solution->init(source_sys.n_dofs(), false, SERIAL);

        // Need to pull down a full copy of this vector on every processor so we can get values in parallel
        source_sys.solution->localize(*_serialized_solution);

        // Add the variables to the system... simultaneously creating MeshFunctions for them.
        for(unsigned int var_num=0; var_num<num_vars; var_num++)
        {
          // Create a variable in the dest_sys to match... but of LINEAR LAGRANGE type
          dest_sys.add_variable(source_sys.variable_name(var_num), FEType());

          _mesh_functions[sys_num][var_num] = new MeshFunction(source_es,
                                                               *_serialized_solution,
                                                               source_sys.get_dof_map(),
                                                               var_num);
          _mesh_functions[sys_num][var_num]->init();
        }
      }
    }
  }
  else
  {
    for(unsigned int sys_num=0; sys_num<num_systems; sys_num++)
    {
      System & source_sys = source_es.get_system(sys_num);

      // Add the system to the es
      ExplicitSystem & dest_sys = _eq.add_system<ExplicitSystem>(source_sys.name());

      unsigned int num_vars = source_sys.n_vars();

      // The system may be empty (auxiliary system is optional)
      if (num_vars)
      {
        // Add the variables to the system...
        for(unsigned int var_num=0; var_num<num_vars; var_num++)
        {
          // Create a variable in the dest_sys to match...
          dest_sys.add_variable(source_sys.variable_name(var_num), source_sys.variable_type(var_num));
        }
      }
    }
  }

  _eq.init();

}

OutputProblem::~OutputProblem()
{
  if(_mesh_might_change)
  {
    for (unsigned int sys_num=0; sys_num < _mesh_functions.size(); ++sys_num)
      for (unsigned int var_num=0; var_num < _mesh_functions[sys_num].size(); ++var_num)
        delete _mesh_functions[sys_num][var_num];

    delete _serialized_solution;
    delete &_mesh;
  }
}

void
OutputProblem::init()
{
  EquationSystems & source_es = _mproblem.es();

  for (unsigned int sys_num=0; sys_num < source_es.n_systems(); ++sys_num)
  {
    if (_mesh_might_change)
    {
      if(_mesh_functions[sys_num].size())
      {
        System & source_sys = source_es.get_system(sys_num);
        System & dest_sys = _eq.get_system(sys_num);

        _serialized_solution->clear();
        _serialized_solution->init(source_sys.n_dofs(), false, SERIAL);
        source_sys.solution->localize(*_serialized_solution);

        for (unsigned int var_num=0; var_num < _mesh_functions[sys_num].size(); ++var_num)
        {

          delete _mesh_functions[sys_num][var_num];
          // TODO: Why do we need to recreate these MeshFunctions each time?
          _mesh_functions[sys_num][var_num] = new MeshFunction(source_es,
                                                               *_serialized_solution,
                                                               source_sys.get_dof_map(),
                                                               var_num);
          _mesh_functions[sys_num][var_num]->init();
        }


        MeshBase::const_node_iterator nd     = _mesh.localNodesBegin();
        MeshBase::const_node_iterator nd_end = _mesh.localNodesEnd();

        // Now loop over the nodes of the 'To' mesh setting values for each variable.
        for(;nd != nd_end; ++nd)
          for(unsigned int var_num=0; var_num < _mesh_functions[sys_num].size(); ++var_num)
            // 0 is for the value component
            dest_sys.solution->set((*nd)->dof_number(sys_num, var_num, 0), (*_mesh_functions[sys_num][var_num])(**nd - _position));
      }
    }
    else
    {
      System & source_sys = source_es.get_system(sys_num);
      System & dest_sys = _eq.get_system(sys_num);

      (*dest_sys.solution) = *(source_sys.solution);
    }
  }
}

void
OutputProblem::timestepSetup()
{
  _out.timestepSetup();
}

void
OutputProblem::outputPps(const FormattedTable & table)
{
  _out.outputPps(table);
}

void
OutputProblem::outputInput()
{
  _out.outputInput();
}

void
OutputProblem::setPosition(const Point & p)
{
  _position = p;
  moveMesh();
}

void
OutputProblem::moveMesh()
{
  MeshBase::node_iterator nd     = _mesh.getMesh().nodes_begin();
  MeshBase::node_iterator nd_end = _mesh.getMesh().nodes_end();

  for(;nd != nd_end; ++nd)
  {
    Node & node = *(*nd);
    node += _position;
  }
}
