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

  return params;
}

NodalNormalsPreprocessor::NodalNormalsPreprocessor(const std::string & name, InputParameters parameters) :
    ElementUserObject(name, parameters),
    _nx(_fe_problem.getAuxiliarySystem().getVector("nx")),
    _ny(_fe_problem.getAuxiliarySystem().getVector("ny")),
    _nz(_fe_problem.getAuxiliarySystem().getVector("nz")),
    _grad_phi(_assembly.feGradPhi(FEType(FIRST, LAGRANGE)))
{
}

NodalNormalsPreprocessor::~NodalNormalsPreprocessor()
{
}

void
NodalNormalsPreprocessor::initialize()
{
  _nx.zero();
  _ny.zero();
  _nz.zero();
}

void
NodalNormalsPreprocessor::execute()
{
  for (unsigned int i = 0; i < _current_elem->n_nodes(); i++)
  {
    const Node * node = _current_elem->get_node(i);
    if (_mesh.isBoundaryNode(node->id()))
    {
      dof_id_type dof = node->id();
      for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
      {
        Threads::spin_mutex::scoped_lock lock(nodal_normals_preprocessor_mutex);

        _nx.add(dof, _JxW[qp] * _grad_phi[i][qp](0));
        _ny.add(dof, _JxW[qp] * _grad_phi[i][qp](1));
        _nz.add(dof, _JxW[qp] * _grad_phi[i][qp](2));
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
  _nx.close();
  _ny.close();
  _nz.close();
}

void
NodalNormalsPreprocessor::threadJoin(const UserObject & /*uo*/)
{
}
