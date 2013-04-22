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

#include "NodalNormalsPreprocessor.h"

Threads::spin_mutex nodal_normals_preprocessor_mutex;

template<>
InputParameters validParams<NodalNormalsPreprocessor>()
{
  InputParameters params = validParams<ElementUserObject>();
  params.addParam<BoundaryName>("corner_boundary", "Node set ID which contains the nodes that are in 'corners'.");

  return params;
}

NodalNormalsPreprocessor::NodalNormalsPreprocessor(const std::string & name, InputParameters parameters) :
    ElementUserObject(name, parameters),
    _aux(_fe_problem.getAuxiliarySystem()),
    _has_corners(isParamValid("corner_boundary")),
    _corner_boundary_id(_has_corners ? _mesh.getBoundaryID(getParam<BoundaryName>("corner_boundary")) : static_cast<BoundaryID>(-1)),
    _grad_phi(_assembly.feGradPhi(FEType(FIRST, LAGRANGE)))
{
}

NodalNormalsPreprocessor::~NodalNormalsPreprocessor()
{
}

void
NodalNormalsPreprocessor::initialize()
{
  NumericVector<Number> & sln = _aux.solution();
  _aux.system().zero_variable(sln, _aux.getVariable(_tid, "nodal_normal_x").number());
  _aux.system().zero_variable(sln, _aux.getVariable(_tid, "nodal_normal_y").number());
  _aux.system().zero_variable(sln, _aux.getVariable(_tid, "nodal_normal_z").number());
}

void
NodalNormalsPreprocessor::execute()
{
  NumericVector<Number> & sln = _aux.solution();

  for (unsigned int i = 0; i < _current_elem->n_nodes(); i++)
  {
    const Node * node = _current_elem->get_node(i);
    if (_mesh.isBoundaryNode(node->id()))
    {
      // it is a boundary node and not a part of 'corner boundary id'
      if (!_has_corners || !_mesh._mesh.boundary_info->has_boundary_id(node, _corner_boundary_id))
      {
        // but it is not a corner node, they will be treated differently later on
        dof_id_type dof_x = node->dof_number(_aux.number(), _fe_problem.getVariable(_tid, "nodal_normal_x").number(), 0);
        dof_id_type dof_y = node->dof_number(_aux.number(), _fe_problem.getVariable(_tid, "nodal_normal_y").number(), 0);
        dof_id_type dof_z = node->dof_number(_aux.number(), _fe_problem.getVariable(_tid, "nodal_normal_z").number(), 0);

        for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
        {
          Threads::spin_mutex::scoped_lock lock(nodal_normals_preprocessor_mutex);

          sln.add(dof_x, _JxW[qp] * _grad_phi[i][qp](0));
          sln.add(dof_y, _JxW[qp] * _grad_phi[i][qp](1));
          sln.add(dof_z, _JxW[qp] * _grad_phi[i][qp](2));
        }
      }
    }
  }
}

void
NodalNormalsPreprocessor::destroy()
{
}

void
NodalNormalsPreprocessor::finalize()
{
  _aux.solution().close();
}

void
NodalNormalsPreprocessor::threadJoin(const UserObject & /*uo*/)
{
}
