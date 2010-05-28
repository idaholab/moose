//Moose includes
#include "FaceData.h"
#include "MooseSystem.h"
#include "ComputeQPSolution.h"

//libmesh includes
#include "numeric_vector.h"

FaceData::FaceData(MooseSystem & moose_system) :
  QuadrPtData(moose_system),
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
  _var_vals.resize(n_threads);
  _var_grads.resize(n_threads);
  _var_seconds.resize(n_threads);
  _var_vals_nodal.resize(n_threads);
}

void FaceData::init()
{
  unsigned int n_vars = _moose_system.getNonlinearSystem()->n_vars();
  int dim = _moose_system.getDim();

  //Resize data arrays
  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    // BCs
    _boundary_to_var_nums[tid].resize(n_vars);
    _boundary_to_var_nums_nodal[tid].resize(n_vars);
    _nodal_bc_var_dofs[tid].resize(n_vars);
    _var_vals[tid].resize(n_vars);
    _var_grads[tid].resize(n_vars);
    _var_seconds[tid].resize(n_vars);
    _var_vals_nodal[tid].resize(n_vars);
  }

  //Max quadrature order was already found by Kernel::init()
  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
    _qrule[tid] = new QGauss(dim - 1, _moose_system._max_quadrature_order);

  for(unsigned int var=0; var < n_vars; var++)
  {
    // TODO: Replicate dof_map
    FEType fe_type = _moose_system._element_data._dof_map->variable_type(var);

    for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
    {
      if(!_fe[tid][fe_type])
      {
        _fe[tid][fe_type] = FEBase::build(dim, fe_type).release();
        _fe[tid][fe_type]->attach_quadrature_rule(_qrule[tid]);

        _q_point[tid][fe_type] = &_fe[tid][fe_type]->get_xyz();
        _JxW[tid][fe_type] = &_fe[tid][fe_type]->get_JxW();
        _phi[tid][fe_type] = &_fe[tid][fe_type]->get_phi();
        _dphi[tid][fe_type] = &_fe[tid][fe_type]->get_dphi();
        _normals[tid][fe_type] = &_fe[tid][fe_type]->get_normals();

        FEFamily family = fe_type.family;

        if(family == CLOUGH || family == HERMITE)
          _d2phi[tid][fe_type] = &_fe[tid][fe_type]->get_d2phi();
      }
    }
  }

}

void FaceData::reinit(THREAD_ID tid, const NumericVector<Number>& soln, const unsigned int side, const unsigned int boundary_id)
{
//  Moose::perf_log.push("reinit()","BoundaryCondition");

  _current_side[tid] = side;

  std::map<FEType, FEBase*>::iterator fe_it = _fe[tid].begin();
  std::map<FEType, FEBase*>::iterator fe_end = _fe[tid].end();

  for(;fe_it != fe_end; ++fe_it)
    fe_it->second->reinit(_moose_system._element_data._current_elem[tid], _current_side[tid]);

  std::vector<unsigned int>::iterator var_nums_it = _boundary_to_var_nums[boundary_id].begin();
  std::vector<unsigned int>::iterator var_nums_end = _boundary_to_var_nums[boundary_id].end();

  for(;var_nums_it != var_nums_end; ++var_nums_it)
  {
    unsigned int var_num = *var_nums_it;

    FEType fe_type = _moose_system._element_data._dof_map->variable_type(var_num);

    FEFamily family = fe_type.family;

    bool has_second_derivatives = (family == CLOUGH || family == HERMITE);

    std::vector<unsigned int> & var_dof_indices = _moose_system._element_data._var_dof_indices[tid][var_num];

    _var_vals[tid][var_num].resize(_qrule[tid]->n_points());
    _var_grads[tid][var_num].resize(_qrule[tid]->n_points());

    if(has_second_derivatives)
      _var_seconds[tid][var_num].resize(_qrule[tid]->n_points());

    const std::vector<std::vector<Real> > & static_phi_face = *_phi[tid][fe_type];
    const std::vector<std::vector<RealGradient> > & static_dphi_face= *_dphi[tid][fe_type];
    const std::vector<std::vector<RealTensor> > & static_d2phi_face= *_d2phi[tid][fe_type];

    for (unsigned int qp=0; qp<_qrule[tid]->n_points(); qp++)
    {
      computeQpSolution(_var_vals[tid][var_num][qp], soln, var_dof_indices, qp, static_phi_face);
      computeQpGradSolution(_var_grads[tid][var_num][qp], soln, var_dof_indices, qp, static_dphi_face);

      if(has_second_derivatives)
        computeQpSecondSolution(_var_seconds[tid][var_num][qp], soln, var_dof_indices, qp, static_d2phi_face);
    }
  }

  std::vector<unsigned int>::iterator var_nums_nodal_it = _boundary_to_var_nums_nodal[boundary_id].begin();
  std::vector<unsigned int>::iterator var_nums_nodal_end = _boundary_to_var_nums_nodal[boundary_id].end();

  for(;var_nums_nodal_it != var_nums_nodal_end; ++var_nums_nodal_it)
  {
    unsigned int var_num = *var_nums_nodal_it;

    std::vector<unsigned int> & var_dof_indices = _moose_system._element_data._var_dof_indices[tid][var_num];

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

  std::vector<unsigned int>::iterator var_nums_nodal_it = _boundary_to_var_nums_nodal[boundary_id].begin();
  std::vector<unsigned int>::iterator var_nums_nodal_end = _boundary_to_var_nums_nodal[boundary_id].end();

  unsigned int nonlinear_system_number = _moose_system.getNonlinearSystem()->number();

  for(;var_nums_nodal_it != var_nums_nodal_end; ++var_nums_nodal_it)
  {
    unsigned int var_num = *var_nums_nodal_it;

    //The zero is the component... that works fine for lagrange FE types.
    unsigned int dof_number = node.dof_number(nonlinear_system_number, var_num, 0);

    _nodal_bc_var_dofs[tid][var_num] = dof_number;

    _var_vals_nodal[tid][var_num].resize(1);

    _var_vals_nodal[tid][var_num][0] = soln(dof_number);
  }

//  Moose::perf_log.pop("reinit(node)","BoundaryCondition");
}
