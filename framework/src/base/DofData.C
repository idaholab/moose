/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Orieneted Simulation Environment */
/*                                                              */
/*            @ 2010 Battelle Energy Alliance, LLC              */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

//Moose includes
#include "MooseSystem.h"
#include "DofData.h"
#include "ComputeQPSolution.h"

//libmesh includes
#include "quadrature_gauss.h"
#include "dof_map.h"
#include "fe_base.h"

DofData::DofData(MooseSystem & moose_system) :
  _moose_system(moose_system),
  _current_elem(NULL)
{

}

DofData::~DofData()
{
  for (std::vector<DenseSubVector<Number> *>::iterator i = _var_Res.begin(); i != _var_Res.end(); ++i)
    delete *i;

  for (std::vector<DenseMatrix<Number> *>::iterator i = _var_Kes.begin(); i != _var_Kes.end(); ++i)
    delete *i;

  for (std::vector<DenseMatrix<Number> *>::iterator i = _var_Kns.begin(); i != _var_Kns.end(); ++i)
    delete *i;
}

void
DofData::init()
{
  unsigned int n_vars = _moose_system.getNonlinearSystem()->n_vars();
  unsigned int n_aux_vars = _moose_system.getAuxSystem()->n_vars();

  _var_dof_indices.resize(n_vars);
  _aux_var_dofs.resize(n_aux_vars);
  _aux_var_dof_indices.resize(n_aux_vars);

  _var_Res.resize(n_vars);
  _var_Kes.resize(n_vars);
  _var_Kns.resize(n_vars);
}

void
DofData::reinitRes(int var_num, DenseVector<Number> & Re, unsigned int position, unsigned int num_dofs)
{
  if (_var_Res[var_num])
    delete _var_Res[var_num];

  _var_Res[var_num] = new DenseSubVector<Number>(Re, position, num_dofs);
}

void
DofData::reinitKes(int var_num, unsigned int num_dofs)
{
  if (_var_Kes[var_num])
    delete _var_Kes[var_num];
  _var_Kes[var_num] = new DenseMatrix<Number>(num_dofs, num_dofs);
}

void
DofData::reinitKns(int var_num, unsigned int num_dofs, unsigned int num_n_dofs)
{
  if (_var_Kns[var_num])
    delete _var_Kns[var_num];
  _var_Kns[var_num] = new DenseMatrix<Number>(num_dofs, num_n_dofs);
}
