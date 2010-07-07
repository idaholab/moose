//Moose includes
#include "FaceData.h"
#include "MooseSystem.h"
#include "ComputeQPSolution.h"

//libmesh includes
#include "fe_base.h"
#include "numeric_vector.h"

FaceData::FaceData(MooseSystem & moose_system) :
  QuadraturePointData(moose_system),
  _moose_system(moose_system)
{
  sizeEverything();
}

FaceData::~FaceData()
{
}

void FaceData::sizeEverything()
{
  int n_threads = libMesh::n_threads();

  _current_node.resize(n_threads);
  _current_residual.resize(n_threads);
  _current_side.resize(n_threads);
  _normals.resize(n_threads);

  _nodal_bc_var_dofs.resize(n_threads);
  _var_vals_nodal.resize(n_threads);
}

void FaceData::init()
{
  QuadraturePointData::init();

  unsigned int n_vars = _moose_system.getNonlinearSystem()->n_vars();
  int dim = _moose_system.getDim();

  //Resize data arrays
  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    _nodal_bc_var_dofs[tid].resize(n_vars);
    _var_vals_nodal[tid].resize(n_vars);
  }

  //Max quadrature order was already found by Kernel::init()
  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
    _qrule[tid] = new QGauss(dim - 1, _moose_system._max_quadrature_order);

  for(unsigned int var=0; var < n_vars; var++)
  {
    // TODO: Replicate dof_map
    FEType fe_type = _moose_system._dof_map->variable_type(var);

    for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
    {
      if(!_fe[tid][fe_type])
      {
        _fe[tid][fe_type] = FEBase::build(dim, fe_type).release();
        _fe[tid][fe_type]->attach_quadrature_rule(_qrule[tid]);

        _q_point[tid][fe_type] = &_fe[tid][fe_type]->get_xyz();
        _JxW[tid][fe_type] = &_fe[tid][fe_type]->get_JxW();
        _phi[tid][fe_type] = &_fe[tid][fe_type]->get_phi();
        _grad_phi[tid][fe_type] = &_fe[tid][fe_type]->get_dphi();
        _normals[tid][fe_type] = &_fe[tid][fe_type]->get_normals();

        FEFamily family = fe_type.family;

        if(family == CLOUGH || family == HERMITE)
          _second_phi[tid][fe_type] = &_fe[tid][fe_type]->get_d2phi();
      }
    }
  }

}

void FaceData::reinit(THREAD_ID tid, const NumericVector<Number>& soln, const Elem * elem, const unsigned int side, const unsigned int boundary_id)
{
//  Moose::perf_log.push("reinit()","BoundaryCondition");

  _current_side[tid] = side;

  std::map<FEType, FEBase*>::iterator fe_it = _fe[tid].begin();
  std::map<FEType, FEBase*>::iterator fe_end = _fe[tid].end();

  for(;fe_it != fe_end; ++fe_it)
    fe_it->second->reinit(elem, _current_side[tid]);

  QuadraturePointData::reinit(tid, boundary_id, soln, elem);

  for (std::set<unsigned int>::iterator it = _boundary_to_var_nums_nodal[boundary_id].begin();
       it != _boundary_to_var_nums_nodal[boundary_id].end();
       ++it)
  {
    unsigned int var_num = *it;

    std::vector<unsigned int> & var_dof_indices = _moose_system._var_dof_indices[tid][var_num];

    _var_vals_nodal[tid][var_num].resize(_moose_system._element_data._current_elem[tid]->n_nodes());

    for(unsigned int i=0; i<_moose_system._element_data._current_elem[tid]->n_nodes(); i++)
      _var_vals_nodal[tid][var_num][i] = soln(var_dof_indices[i]);
  }

//  Moose::perf_log.pop("reinit()","BoundaryCondition");
}

void FaceData::reinit(THREAD_ID tid, const NumericVector<Number>& soln, const Node & node, const unsigned int boundary_id, NumericVector<Number>& residual)
{
//  Moose::perf_log.push("reinit(node)","BoundaryCondition");

  _current_node[tid] = &node;
  _current_residual[tid] = &residual;

  unsigned int nonlinear_system_number = _moose_system.getNonlinearSystem()->number();

  for (std::set<unsigned int>::iterator it = _boundary_to_var_nums_nodal[boundary_id].begin();
       it != _boundary_to_var_nums_nodal[boundary_id].end();
       ++it)
  {
    unsigned int var_num = *it;

    //The zero is the component... that works fine for lagrange FE types.
    unsigned int dof_number = node.dof_number(nonlinear_system_number, var_num, 0);

    _nodal_bc_var_dofs[tid][var_num] = dof_number;

    _var_vals_nodal[tid][var_num].resize(1);

    _var_vals_nodal[tid][var_num][0] = soln(dof_number);
  }

//  Moose::perf_log.pop("reinit(node)","BoundaryCondition");
}
