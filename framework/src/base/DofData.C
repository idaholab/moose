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
}
