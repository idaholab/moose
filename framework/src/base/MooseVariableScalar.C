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

#include "MooseVariableScalar.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "Assembly.h"
#include "SystemBase.h"

// libMesh
#include "numeric_vector.h"
#include "dof_map.h"

MooseVariableScalar::MooseVariableScalar(unsigned int var_num, unsigned int mvn, SystemBase & sys, Assembly & assembly) :
    _var_num(var_num),
    _moose_var_num(mvn),
    _subproblem(sys.subproblem()),
    _sys(sys),
    _assembly(assembly),
    _dof_map(sys.dofMap()),
    _scaling_factor(1.0)
{
}

MooseVariableScalar::~MooseVariableScalar()
{
  _u.release();
}

const std::string &
MooseVariableScalar::name()
{
  return _sys.system().variable(_var_num).name();
}

void
MooseVariableScalar::reinit()
{
  _dof_map.SCALAR_dof_indices(_dof_indices, _var_num);

  unsigned int n = _dof_indices.size();
  _u.resize(n);
  for (unsigned int i = 0; i < n; i++)
    _u[i] = (*_sys.currentSolution())(_dof_indices[i]);
}
