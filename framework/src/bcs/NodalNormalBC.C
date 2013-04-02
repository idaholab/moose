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

#include "NodalNormalBC.h"

template<>
InputParameters validParams<NodalNormalBC>()
{
  InputParameters params = validParams<NodalBC>();

  return params;
}

NodalNormalBC::NodalNormalBC(const std::string & name, InputParameters parameters) :
    NodalBC(name, parameters),
    _nx(_fe_problem.getAuxiliarySystem().getVector("nx")),
    _ny(_fe_problem.getAuxiliarySystem().getVector("ny")),
    _nz(_fe_problem.getAuxiliarySystem().getVector("nz"))
{
}

NodalNormalBC::~NodalNormalBC()
{
}

void
NodalNormalBC::computeResidual(NumericVector<Number> & residual)
{
  // compute the normal
  dof_id_type dof = _current_node->id();
  _normal = Point(_nx(dof), _ny(dof), _nz(dof));

  NodalBC::computeResidual(residual);
}
