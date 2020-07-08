//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DualMortarPreconditioner.h"

// MOOSE includes
#include "FEProblem.h"
#include "MooseUtils.h"
#include "MooseVariableFE.h"
#include "NonlinearSystem.h"
#include "ComputeJacobianBlocksThread.h"
#include "MooseEnum.h"

#include "libmesh/coupling_matrix.h"
#include "libmesh/libmesh_common.h"
#include "libmesh/equation_systems.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/nonlinear_solver.h"
#include "libmesh/linear_implicit_system.h"
#include "libmesh/transient_system.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/sparse_matrix.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/mesh_base.h"
#include "libmesh/variable.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/parallel_object.h"
#include "libmesh/boundary_info.h"

registerMooseObjectAliased("MooseApp", DualMortarPreconditioner, "DMP");

defineLegacyParams(DualMortarPreconditioner);

InputParameters
DualMortarPreconditioner::validParams()
{
  InputParameters params = MoosePreconditioner::validParams();

  params.addParam<std::vector<NonlinearVariableName>>(
      "off_diag_row",
      "The off diagonal row you want to add into the matrix, it will be associated "
      "with an off diagonal column from the same position in off_diag_colum.");
  params.addParam<std::vector<NonlinearVariableName>>(
      "off_diag_column",
      "The off diagonal column you want to add into the matrix, it will be "
      "associated with an off diagonal row from the same position in "
      "off_diag_row.");
  params.addParam<std::vector<NonlinearVariableName>>(
      "coupled_groups",
      "List multiple space separated groups of comma separated variables. "
      "Off-diagonal jacobians will be generated for all pairs within a group.");
  params.addParam<bool>("full",
                        false,
                        "Set to true if you want the full set of couplings.  Simply "
                        "for convenience so you don't have to set every "
                        "off_diag_row and off_diag_column combination.");
  params.addRequiredParam<BoundaryID>("primary_boundary", "Primary side of the contact interface.");
  params.addRequiredParam<BoundaryID>("secondary_boundary", "Secondary side of the contact interface.");
  params.addRequiredParam<SubdomainID>("primary_subdomain", "Primary subdomain.");
  params.addRequiredParam<SubdomainID>("secondary_subdomain", "Secondary subdomain.");
  params.addRequiredParam<std::vector<std::string>>("preconditioner", "Preconditioner type.");

  return params;
}

DualMortarPreconditioner::DualMortarPreconditioner(const InputParameters & params)
  : MoosePreconditioner(params),
  Preconditioner<Number>(MoosePreconditioner::_communicator),
  _nl(_fe_problem.getNonlinearSystemBase()),
  _mesh(&_fe_problem.mesh()),
  _dofmap(& _nl.system().get_dof_map()),
  _n_vars(_nl.nVariables()),
  _K2ci(libmesh_make_unique<PetscMatrix<Number>>(MoosePreconditioner::_communicator)),
  _K2cc(libmesh_make_unique<PetscMatrix<Number>>(MoosePreconditioner::_communicator)),
  _D(libmesh_make_unique<PetscMatrix<Number>>(MoosePreconditioner::_communicator)),
  _M(libmesh_make_unique<PetscMatrix<Number>>(MoosePreconditioner::_communicator)),
  _MDinv(libmesh_make_unique<PetscMatrix<Number>>(MoosePreconditioner::_communicator)),
  _J_condensed(libmesh_make_unique<PetscMatrix<Number>>(MoosePreconditioner::_communicator)),
  _primary_boundary(getParam<BoundaryID>("primary_boundary")),
  _secondary_boundary(getParam<BoundaryID>("secondary_boundary")),
  _primary_subdomain(getParam<SubdomainID>("primary_subdomain")),
  _secondary_subdomain(getParam<SubdomainID>("secondary_subdomain")),
  _save_dofs(false),
  _init_timer(registerTimedSection("init", 2)),
  _apply_timer(registerTimedSection("apply", 1))
{
  // check if SubdomainID & BoundaryID are valid
  if(_mesh->meshSubdomains().find(_secondary_subdomain)==_mesh->meshSubdomains().end())
    mooseError("secondary subdomain ID ", _secondary_subdomain," does not exist.");
  if(_mesh->meshSubdomains().find(_primary_subdomain)==_mesh->meshSubdomains().end())
    mooseError("primary subdomain ID ", _primary_subdomain," does not exist.");
  if(_mesh->getBoundaryIDs().find(_secondary_boundary)==_mesh->getBoundaryIDs().end())
    mooseError("Secondary boundary ID ", _secondary_boundary," does not exist.");
  if(_mesh->getBoundaryIDs().find(_primary_boundary)==_mesh->getBoundaryIDs().end())
    mooseError("Secondary boundary ID ", _primary_boundary," does not exist.");


  // PC type
  const std::vector<std::string> & pc_type = getParam<std::vector<std::string>>("preconditioner");
  if (pc_type.size()>1)
    mooseWarning("We only use one preconditioner type in DMP, the ", pc_type[0], " preconditioner is utilized.");
  _pre_type = Utility::string_to_enum<PreconditionerType>(pc_type[0]);


  std::unique_ptr<CouplingMatrix> cm = libmesh_make_unique<CouplingMatrix>(_n_vars);
  bool full = getParam<bool>("full");

  if (!full)
  {
    // put 1s on diagonal
    for (unsigned int i = 0; i < _n_vars; i++)
      (*cm)(i, i) = 1;

    // off-diagonal entries from the off_diag_row and off_diag_column parameters
    std::vector<std::vector<unsigned int>> off_diag(_n_vars);
    for (unsigned int i = 0;
         i < getParam<std::vector<NonlinearVariableName>>("off_diag_row").size();
         i++)
    {
      unsigned int row =
          _nl.getVariable(0, getParam<std::vector<NonlinearVariableName>>("off_diag_row")[i])
              .number();
      unsigned int column =
          _nl.getVariable(0, getParam<std::vector<NonlinearVariableName>>("off_diag_column")[i])
              .number();
      (*cm)(row, column) = 1;
    }

    // off-diagonal entries from the coupled_groups parameters
    std::vector<NonlinearVariableName> groups =
        getParam<std::vector<NonlinearVariableName>>("coupled_groups");
    for (unsigned int i = 0; i < groups.size(); ++i)
    {
      std::vector<NonlinearVariableName> vars;
      MooseUtils::tokenize<NonlinearVariableName>(groups[i], vars, 1, ",");
      for (unsigned int j = 0; j < vars.size(); ++j)
        for (unsigned int k = j + 1; k < vars.size(); ++k)
        {
          unsigned int row = _nl.getVariable(0, vars[j]).number();
          unsigned int column = _nl.getVariable(0, vars[k]).number();
          (*cm)(row, column) = 1;
          (*cm)(column, row) = 1;
        }
    }
  }
  else
  {
    for (unsigned int i = 0; i < _n_vars; i++)
      for (unsigned int j = 0; j < _n_vars; j++)
        (*cm)(i, j) = 1;
  }

  _fe_problem.setCouplingMatrix(std::move(cm));

  _nl.attachPreconditioner(this);
}

DualMortarPreconditioner::~DualMortarPreconditioner() { this->clear(); }

void
DualMortarPreconditioner::getDofVarSubdomain()
{
  _dof_sets.resize(_n_vars);
  for (unsigned int vn = 0; vn < _n_vars; vn++)
  {
    Variable var = _nl.system().variable(vn);
    // loop through active subdomains of this variable
    for(auto it=var.active_subdomains().begin(); it != var.active_subdomains().end(); ++it)
    {
      SubdomainID sd=(SubdomainID)(*it);
      std::set<dof_id_type> dofs;
      // check dofs of each element
      ConstElemRange * active_local_elems = _mesh->getActiveLocalElementRange();
      for (const auto & elem : *active_local_elems)
      {
        if (elem->subdomain_id() == sd)
        {
          std::vector<dof_id_type> di;
          _dofmap->dof_indices(elem, di, vn);
          dofs.insert(di.begin(), di.end());
        }
      }
      _dof_sets[vn].insert(make_pair(sd, dofs));
    }
  }
}

void
DualMortarPreconditioner::getDofVarInterior()
{
  getDofVarSubdomain();
  _dof_sets_interior.resize(_n_vars);
  for (unsigned int vn = 0; vn < _n_vars; vn++)
  {
    Variable var = _nl.system().variable(vn);
    // loop through active subdomains of this variable
    for(auto it=var.active_subdomains().begin(); it != var.active_subdomains().end(); ++it)
    {
      SubdomainID sd=(SubdomainID)(*it);
      std::set<dof_id_type> dofs=_dof_sets[vn][sd];
      // remove dofs on the interface
      for (auto it_primary = _dof_sets_primary[vn].begin(); it_primary!=_dof_sets_primary[vn].end(); ++it_primary)
        dofs.erase(*it_primary);
      for (auto it_secondary = _dof_sets_secondary[vn].begin(); it_secondary!=_dof_sets_secondary[vn].end(); ++it_secondary)
        dofs.erase(*it_secondary);

      std::vector<dof_id_type> vec_dofs(dofs.begin(), dofs.end());
      _dof_sets_interior[vn].insert(make_pair(sd, vec_dofs));
    }
  }
}

void
DualMortarPreconditioner::getDofVarInterface()
{
  _dof_sets_primary.resize(_n_vars);
  _dof_sets_secondary.resize(_n_vars);
  for (unsigned int vn = 0; vn < _n_vars; vn++)
  {
    // loop over boundary nodes
    ConstBndNodeRange & range = * _mesh->getBoundaryNodeRange();
    std::vector<dof_id_type> di;
    for (const auto & bnode : range)
    {
      const Node * node_bdry = bnode->_node;
      BoundaryID boundary_id = bnode->_bnd_id;

      if (boundary_id == _secondary_boundary)
      {
          _dofmap->dof_indices(node_bdry, di, vn);
          for (auto it = di.begin(); it!=di.end(); ++it)
            if(std::find(_dof_sets_secondary[vn].begin(), _dof_sets_secondary[vn].end(), *it)==_dof_sets_secondary[vn].end())
              _dof_sets_secondary[vn].push_back(*it);
      }

      if (boundary_id == _primary_boundary)
      {
        _dofmap->dof_indices(node_bdry, di, vn);
        for (auto it = di.begin(); it!=di.end(); ++it)
          if(std::find(_dof_sets_primary[vn].begin(), _dof_sets_primary[vn].end(), *it)==_dof_sets_primary[vn].end())
            _dof_sets_primary[vn].push_back(*it);
      }
    }
  }
}

void
DualMortarPreconditioner::printDofSets()
{
  /// Dofs in the interior
  for (auto it1 = _dof_sets_interior.begin(); it1 != _dof_sets_interior.end(); it1++)
  {
    std::cout << "variable "<<std::endl;
    for (auto it2 = it1->begin(); it2 != it1->end(); it2++)
    {
      std::cout << " \tdofs in subdomain " << it2->first<<": ";
      for (auto it3 = it2->second.begin(); it3 != it2->second.end(); it3++)
        std::cout <<*it3 << " ";
      std::cout <<"\n";
    }
  }

  // primary
  std::cout<<"primary dofs:\n";
  for (auto it1=_dof_sets_primary.begin(); it1!=_dof_sets_primary.end(); it1++)
  {
    std::cout<< "\tvariable :";
    for (auto it2=it1->begin(); it2!=it1->end(); it2++)
      std::cout<<*it2<<" ";
    std::cout<<std::endl;
  }

  // secondary
  std::cout<<"secondary dofs:\n";
  for (auto it1=_dof_sets_secondary.begin(); it1!=_dof_sets_secondary.end(); it1++)
  {
    std::cout<< "\tvariable :";
    for (auto it2=it1->begin(); it2!=it1->end(); it2++)
      std::cout<<*it2<<" ";
    std::cout<<std::endl;
  }
}

void
DualMortarPreconditioner::condenseSystem()
{
  std::vector<dof_id_type> u1c=_dof_sets_primary[0];
  std::vector<dof_id_type> u2c=_dof_sets_secondary[0];

  std::vector<dof_id_type> lm=_dof_sets_secondary[1];

  std::vector<dof_id_type> u1i=_dof_sets_interior[0][_primary_subdomain];
  std::vector<dof_id_type> u2i=_dof_sets_interior[0][_secondary_subdomain];

  _matrix->create_submatrix(* _D, lm, u2c);
  _D->get_transpose(*_D);
  _matrix->create_submatrix(* _M, lm, u1c);
  _matrix->create_submatrix(* _MDinv, u1c, lm);
  _M->get_transpose(*_M);

  _matrix->create_submatrix(* _K2ci, u2c, u2i);
  _matrix->create_submatrix(* _K2cc, u2c, u2c);

  // invert _D:
  // _D should be strictly diagonal if dual_mortar approach is utilized
  // so we only need to compute the reciprocal number of the diagonal entries
  // to save memory, no new matrix is created
  for (unsigned int i=0; i<_D->m(); ++i)
  {
   Number tmp = 1.0/(*_D)(i,i);
   std::vector<numeric_index_type> row_i;
   row_i.push_back(i);
    _D->zero_rows(row_i, tmp);
  }


  // compute MDinv=_M*_D
  _M->matrix_matrix_mult(*_D, *_MDinv); // (should use empty initializer for _MDinv)

  // initialize _J_condensed

  _matrix->create_submatrix(* _J_condensed, _rows, _cols);

  // compute changed parts: MDinv*K2ci, MDinv*K2cc
  std::unique_ptr<PetscMatrix<Number>> MDinvK2ci(libmesh_make_unique<PetscMatrix<Number>>(MoosePreconditioner::_communicator)),
  MDinvK2cc(libmesh_make_unique<PetscMatrix<Number>>(MoosePreconditioner::_communicator));
  _matrix->create_submatrix( *MDinvK2ci, u1c, u2i); // get MDinvK2ci initialized (should use empty initializer here)
  _matrix->create_submatrix( *MDinvK2cc, u1c, u2c); // get MDinvK2cc initialized (should use empty initializer here)
  _MDinv->matrix_matrix_mult(*_K2ci, *MDinvK2ci);
  _MDinv->matrix_matrix_mult(*_K2cc, *MDinvK2cc);

  // add changed parts to _J_condensed
  // original system row_id: u1c
  // original system col_id: u2i, u2c
  std::vector<numeric_index_type> row_id_cond, col_id_cond_u2i, col_id_cond_u2c;
  std::map<numeric_index_type, numeric_index_type> row_id_cond_mp, col_id_cond_u2i_mp, col_id_cond_u2c_mp;

  for (auto it: index_range(u1c))
  {
    numeric_index_type lid=static_cast<numeric_index_type>(it);
    auto it_row = find (_rows.begin(), _rows.end(), u1c[it]);
    if (it_row != _rows.end())
    {
      numeric_index_type gid = std::distance(_rows.begin(), it_row);
      row_id_cond_mp.insert(std::make_pair(lid , gid));
    }
    else
      mooseError("DOF ", u1c[it]," does not exist in the rows of the condensed system");
  }

  for (auto it: index_range(u2i))
  {
    numeric_index_type lid=static_cast<numeric_index_type>(it);
    auto it_col = find(_cols.begin(), _cols.end(), u2i[it]);
    if (it_col != _cols.end())
    {
      numeric_index_type gid = std::distance(_cols.begin(), it_col);
      col_id_cond_u2i_mp.insert(std::make_pair(lid, gid));
    }
    else
      mooseError("DOF ", u2i[it]," does not exist in the columns of the condensed system");
  }

  for (auto it: index_range(u2c))
  {
    numeric_index_type lid=static_cast<numeric_index_type>(it);
    auto it_col = find(_cols.begin(), _cols.end(), u2c[it]);
    if (it_col != _cols.end())
    {
      numeric_index_type gid = std::distance(_cols.begin(), it_col);
      col_id_cond_u2c_mp.insert(std::make_pair(lid, gid));
    }
    else
      mooseError("DOF ", u2c[it]," does not exist in the columns of the condensed system");
  }

  MatSetOption(_J_condensed->mat(), MAT_NEW_NONZERO_ALLOCATION_ERR, PETSC_FALSE);
  _J_condensed->add_sparse_matrix(*MDinvK2ci, row_id_cond_mp, col_id_cond_u2i_mp, -1.0);
  _J_condensed->add_sparse_matrix(*MDinvK2cc, row_id_cond_mp, col_id_cond_u2c_mp, -1.0);
  _J_condensed->close();

  // _J_condensed->print_personal();

}

void
DualMortarPreconditioner::init()
{
  TIME_SECTION(_init_timer);
  if(!_save_dofs)
  {
    // Get DOFs on the secondary/primary and in subdomains for each variable
    getDofVarInterface();
    getDofVarInterior();

    // for debugging
    // printDofSets();

    // get row and col dofs for the condensed Jacobian
    std::vector<dof_id_type> u1c=_dof_sets_primary[0];
    std::vector<dof_id_type> u2c=_dof_sets_secondary[0];

    std::vector<dof_id_type> lm=_dof_sets_secondary[1];

    std::vector<dof_id_type> u1i=_dof_sets_interior[0][_primary_subdomain];
    std::vector<dof_id_type> u2i=_dof_sets_interior[0][_secondary_subdomain];
    // get row dofs
    _rows.reserve(_dofmap->n_dofs()-u2c.size());
    _rows.insert(_rows.end(), u1i.begin(), u1i.end());
    _rows.insert(_rows.end(), u1c.begin(), u1c.end());
    _rows.insert(_rows.end(), u2i.begin(), u2i.end());
    _rows.insert(_rows.end(), lm.begin(), lm.end());
    // get col dofs
    _cols.reserve(_dofmap->n_dofs()-lm.size());
    _cols.insert(_cols.end(), u1i.begin(), u1i.end());
    _cols.insert(_cols.end(), u1c.begin(), u1c.end());
    _cols.insert(_cols.end(), u2i.begin(), u2i.end());
    _cols.insert(_cols.end(), u2c.begin(), u2c.end());

    _save_dofs=true;
  }

  if (!_preconditioner)
    _preconditioner= Preconditioner<Number>::build_preconditioner(MoosePreconditioner::_communicator);


  _is_initialized=true;

}

void
DualMortarPreconditioner::print_node_info()
{
  NodeRange * range= _mesh->getActiveNodeRange();
  for (const auto & node : *range)
  {
    node->print_info();
  }
}

void
DualMortarPreconditioner::setup()
{
  condenseSystem();

  _preconditioner->set_matrix(*_J_condensed);
  _preconditioner->set_type(_pre_type);
  _preconditioner->init();
}

void
DualMortarPreconditioner::apply(const NumericVector<Number> & y, NumericVector<Number> & x)
{
  TIME_SECTION(_apply_timer);

  std::vector<dof_id_type> lm=_dof_sets_secondary[1];

  getCondensedXY(y, x);

  _preconditioner->apply(*_y_hat, *_x_hat);

  computeLM();

  // update x
  for (dof_id_type id1=0; id1<_cols.size(); ++id1)
  {
    dof_id_type id0=_cols[id1]; //id in the original system
    x.set(id0, (*_x_hat)(id1));
  }

  for (dof_id_type id1=0; id1<lm.size(); ++id1)
  {
    dof_id_type id0=lm[id1]; //id in the original system
    x.set(id0, (*_lambda)(id1));
  }

  x.close();
}

void
DualMortarPreconditioner::getCondensedXY(const NumericVector<Number> & y, NumericVector<Number> & x)
{
  std::vector<dof_id_type> u1c=_dof_sets_primary[0];
  std::vector<dof_id_type> u2c=_dof_sets_secondary[0];

  _x_hat=x.zero_clone(); _y_hat=y.zero_clone();
  _x_hat->init(_rows.size()); _y_hat->init(_rows.size());
  _r2c=y.zero_clone();
  _r2c->init(u2c.size());

  std::unique_ptr<NumericVector<Number>> mdinv_r2c=_r2c->zero_clone();
  mdinv_r2c->init(u1c.size());

  // get _r2c from the original y
  for (dof_id_type idx=0; idx<u2c.size(); ++idx)
  {
    dof_id_type id0=u2c[idx]; // row id in the original system
    _r2c->set(idx, y(id0));
  }

  _r2c->close();

  _MDinv->vector_mult(*mdinv_r2c, *_r2c);

  mdinv_r2c->close();


  for (dof_id_type idx=0; idx<_rows.size(); ++idx)
  {
    dof_id_type id0=_rows[idx];// row id in the original system
    // if id0 is in u1c, then need to subtract
    // otherwise, copy from y
    auto it_row = find(u1c.begin(), u1c.end(), id0);
    if (it_row != u1c.end())
      _y_hat->set(idx, y(id0)-(*mdinv_r2c)(std::distance(u1c.begin(), it_row)));
    else
      _y_hat->set(idx, y(id0));
  }

  for (dof_id_type idx=0; idx<_cols.size(); ++idx)
  {
    dof_id_type id0=_cols[idx];// col id in the original system
    _x_hat->set(idx, x(id0));
  }

  _y_hat->close();
  _x_hat->close();
}

void
DualMortarPreconditioner::computeLM()
{
  std::vector<dof_id_type> lm=_dof_sets_secondary[1];

  std::vector<dof_id_type> u2i=_dof_sets_interior[0][_secondary_subdomain];
  std::vector<dof_id_type> u2c=_dof_sets_secondary[0];

  _lambda=_r2c->zero_clone(); _lambda->init(lm.size());
  _x2i=_r2c->zero_clone(); _x2i->init(u2i.size());
  _x2c=_r2c->zero_clone(); _x2c->init(u2c.size());

  // get x2i, x2c from _x_hat
  for (dof_id_type id = 0; id<_cols.size(); ++id)
  {
    auto id2c =find(u2c.begin(), u2c.end(), _cols[id]);
    if(id2c!=u2c.end())
      _x2c->set(std::distance(u2c.begin(), id2c), (*_x_hat)(id));

    auto id2i=find(u2i.begin(), u2i.end(), _cols[id]);
    if(id2i!=u2i.end())
      _x2i->set(std::distance(u2i.begin(), id2i), (*_x_hat)(id));
  }

  _x2i->close();
  _x2c->close();

  std::unique_ptr<NumericVector<Number>> vec=_r2c->zero_clone(), tmp=_r2c->clone();
  // vec=_K2ci*_x2i;
  _K2ci->vector_mult (*vec,*_x2i);
  (*tmp)-=(*vec);
  // vec=_K2cc*_x2c;
  _K2cc->vector_mult (*vec,*_x2c);
  (*tmp)-=(*vec);

  _D->vector_mult(*_lambda, *tmp );

  vec->close();
  tmp->close();
  _lambda->close();
}

void
DualMortarPreconditioner::clear()
{}
