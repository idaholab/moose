//Moose includes
#include "MooseSystem.h"
#include "FaceData.h"
#include "ElementData.h"
#include "ComputeQPSolution.h"

//libmesh includes
#include "numeric_vector.h"
#include "quadrature_gauss.h"
#include "fe_base.h"


FaceData::FaceData(MooseSystem & moose_system)
  :_moose_system(moose_system)
{
  sizeEverything();
}

FaceData::~FaceData()
{
  for (std::vector<std::map<FEType, FEBase*> >::iterator i = _fe_face.begin(); i != _fe_face.end(); ++i)
  {
    for (std::map<FEType, FEBase*>::iterator j = i->begin(); j != i->end(); ++j)
      delete j->second;
  }

  for (std::vector<QGauss *>::iterator i = _qface.begin(); i != _qface.end(); ++i)
  {
    delete *i;
  }
}

void
FaceData::sizeEverything()
{
  int n_threads = libMesh::n_threads();
  
  _current_node.resize(n_threads);
  _current_residual.resize(n_threads);
  _current_side.resize(n_threads);
  _fe_face.resize(n_threads);
  _qface.resize(n_threads);
  _q_point_face.resize(n_threads);
  _JxW_face.resize(n_threads);
  _phi_face.resize(n_threads);
  _dphi_face.resize(n_threads);
  _d2phi_face.resize(n_threads);
  _normals_face.resize(n_threads);

  _nodal_bc_var_dofs.resize(n_threads);
  _var_vals_face.resize(n_threads);
  _var_grads_face.resize(n_threads);
  _var_seconds_face.resize(n_threads);
  _var_vals_face_nodal.resize(n_threads);
}

void
FaceData::init()
{
  unsigned int n_vars = _moose_system.getNonlinearSystem()->n_vars();

  //Resize data arrays
  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    _boundary_to_var_nums[tid].resize(n_vars);
    _boundary_to_var_nums_nodal[tid].resize(n_vars);
    _nodal_bc_var_dofs[tid].resize(n_vars);
    _var_vals_face[tid].resize(n_vars);
    _var_grads_face[tid].resize(n_vars);
    _var_seconds_face[tid].resize(n_vars);
    _var_vals_face_nodal[tid].resize(n_vars);
  }

  //Max quadrature order was already found by Kernel::init()
  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
    _qface[tid] = new QGauss(_moose_system.getDim()-1, _moose_system._max_quadrature_order);

  for(unsigned int var=0; var < n_vars; var++)
  {
    // TODO: Fix this train wreck - where does DofMap belong?
    FEType fe_type = _moose_system._element_data->_dof_map->variable_type(var);

    for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
    {
      if(!_fe_face[tid][fe_type])
      {
        _fe_face[tid][fe_type] = FEBase::build(_moose_system.getDim(), fe_type).release();
        _fe_face[tid][fe_type]->attach_quadrature_rule(_qface[tid]);

        _q_point_face[tid][fe_type] = &_fe_face[tid][fe_type]->get_xyz();
        _JxW_face[tid][fe_type] = &_fe_face[tid][fe_type]->get_JxW();
        _phi_face[tid][fe_type] = &_fe_face[tid][fe_type]->get_phi();
        _dphi_face[tid][fe_type] = &_fe_face[tid][fe_type]->get_dphi();
        _normals_face[tid][fe_type] = &_fe_face[tid][fe_type]->get_normals();

        FEFamily family = fe_type.family;

        if(family == CLOUGH || family == HERMITE)
          _d2phi_face[tid][fe_type] = &_fe_face[tid][fe_type]->get_d2phi();
      }
    }
  }
}

  
void
FaceData::reinitBCs(THREAD_ID tid, const NumericVector<Number>& soln, const unsigned int side, const unsigned int boundary_id)
{
//  Moose::perf_log.push("reinit()","BoundaryCondition");

  _current_side[tid] = side;

  std::map<FEType, FEBase*>::iterator fe_it = _fe_face[tid].begin();
  std::map<FEType, FEBase*>::iterator fe_end = _fe_face[tid].end();

  // TODO: Train wreck fix
  for(;fe_it != fe_end; ++fe_it)
    fe_it->second->reinit(_moose_system._element_data->_current_elem[tid], _current_side[tid]);

  std::vector<unsigned int>::iterator var_nums_it = _boundary_to_var_nums[boundary_id].begin();
  std::vector<unsigned int>::iterator var_nums_end = _boundary_to_var_nums[boundary_id].end();

  for(;var_nums_it != var_nums_end; ++var_nums_it)
  {
    unsigned int var_num = *var_nums_it;

    // TODO: Train wreck fix
    FEType fe_type = _moose_system._element_data->_dof_map->variable_type(var_num);

    FEFamily family = fe_type.family;

    bool has_second_derivatives = (family == CLOUGH || family == HERMITE);

    // TODO: Train wreck fix
    std::vector<unsigned int> & var_dof_indices = _moose_system._element_data->_var_dof_indices[tid][var_num];

    _var_vals_face[tid][var_num].resize(_qface[tid]->n_points());
    _var_grads_face[tid][var_num].resize(_qface[tid]->n_points());

    if(has_second_derivatives)
      _var_seconds_face[tid][var_num].resize(_qface[tid]->n_points());

    const std::vector<std::vector<Real> > & static_phi_face = *_phi_face[tid][fe_type];
    const std::vector<std::vector<RealGradient> > & static_dphi_face= *_dphi_face[tid][fe_type];
    const std::vector<std::vector<RealTensor> > & static_d2phi_face= *_d2phi_face[tid][fe_type];

    for (unsigned int qp=0; qp<_qface[tid]->n_points(); qp++)
    {
      computeQpSolution(_var_vals_face[tid][var_num][qp], soln, var_dof_indices, qp, static_phi_face);
      computeQpGradSolution(_var_grads_face[tid][var_num][qp], soln, var_dof_indices, qp, static_dphi_face);

      if(has_second_derivatives)
        computeQpSecondSolution(_var_seconds_face[tid][var_num][qp], soln, var_dof_indices, qp, static_d2phi_face);
    }
  }

  std::vector<unsigned int>::iterator var_nums_nodal_it = _boundary_to_var_nums_nodal[boundary_id].begin();
  std::vector<unsigned int>::iterator var_nums_nodal_end = _boundary_to_var_nums_nodal[boundary_id].end();

  for(;var_nums_nodal_it != var_nums_nodal_end; ++var_nums_nodal_it)
  {
    unsigned int var_num = *var_nums_nodal_it;

    // TODO: Train wreck fix
    std::vector<unsigned int> & var_dof_indices = _moose_system._element_data->_var_dof_indices[tid][var_num];

    // TODO: Train wreck fix
    _var_vals_face_nodal[tid][var_num].resize(_moose_system._element_data->_current_elem[tid]->n_nodes());

    // TODO: Train wreck fix
    for(unsigned int i=0; i<_moose_system._element_data->_current_elem[tid]->n_nodes(); i++)
      _var_vals_face_nodal[tid][var_num][i] = soln(var_dof_indices[i]);
  }

//  Moose::perf_log.pop("reinit()","BoundaryCondition");
}

void
FaceData::reinitBCs(THREAD_ID tid, const NumericVector<Number>& soln, const Node & node, const unsigned int boundary_id, NumericVector<Number>& residual)
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

    _var_vals_face_nodal[tid][var_num].resize(1);

    _var_vals_face_nodal[tid][var_num][0] = soln(dof_number);
  }

//  Moose::perf_log.pop("reinit(node)","BoundaryCondition");
}
