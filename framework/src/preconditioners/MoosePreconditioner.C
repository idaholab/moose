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

#include "MoosePreconditioner.h"
#include "FEProblem.h"

template<>
InputParameters validParams<MoosePreconditioner>()
{
  InputParameters params = validParams<MooseObject>();
  params.addPrivateParam<FEProblem *>("_fe_problem");
  params.addPrivateParam<std::string>("built_by_action", "add_preconditioning");
  return params;
}


MoosePreconditioner::MoosePreconditioner(const std::string & name, InputParameters params) :
    MooseObject(name, params),
    _fe_problem(*getParam<FEProblem *>("_fe_problem"))
{
}

MoosePreconditioner::~MoosePreconditioner()
{
}

void
MoosePreconditioner::copyVarValues(MeshBase & mesh,
                                   const unsigned int from_system, const unsigned int from_var, const NumericVector<Number> & from_vector,
                                   const unsigned int to_system, const unsigned int to_var, NumericVector<Number> & to_vector)
{
  MeshBase::node_iterator it = mesh.local_nodes_begin();
  MeshBase::node_iterator it_end = mesh.local_nodes_end();

  for(;it!=it_end;++it)
  {
    Node * node = *it;

    unsigned int n_comp = node->n_comp(from_system, from_var);

    mooseAssert(node->n_comp(from_system, from_var) == node->n_comp(to_system, to_var), "Number of components does not match in each system in PBP");

    for(unsigned int i=0; i<n_comp; i++)
    {
      unsigned int from_dof = node->dof_number(from_system,from_var,i);
      unsigned int to_dof = node->dof_number(to_system,to_var,i);

      to_vector.set(to_dof,from_vector(from_dof));
    }
  }
}
