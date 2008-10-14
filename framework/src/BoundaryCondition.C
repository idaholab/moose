#include "BoundaryCondition.h"

#include "numeric_vector.h"

#include "Moose.h"

BoundaryCondition::BoundaryCondition(Parameters parameters,
                                     std::string var_name,
                                     bool integrated,
                                     unsigned int boundary_id,
                                     std::vector<std::string> coupled_to,
                                     std::vector<std::string> coupled_as)
  :Kernel(parameters, var_name, integrated, coupled_to, coupled_as),
   _boundary_id(boundary_id),
   _side_elem(NULL),
   _JxW_face(*_static_JxW_face),
   _phi_face(*_static_phi_face),
   _dphi_face(*_static_dphi_face),
   _normals_face(*_static_normals_face),
   _q_point_face(*_static_q_point_face),
   _u_face(integrated ? _var_vals_face[_var_num] : _var_vals_face_nodal[_var_num]),
   _grad_zero(0),
   _grad_u_face(integrated ? _var_grads_face[_var_num] : _grad_zero)
{
  if(_integrated)
    addVarNums(_boundary_to_var_nums[_boundary_id]);
  else
    addVarNums(_boundary_to_var_nums_nodal[_boundary_id]);
}

void BoundaryCondition::addVarNums(std::vector<unsigned int> & var_nums)
{
  // If this variable isn't known yet... make it so
  if(std::find(var_nums.begin(), var_nums.end(), _var_num) == var_nums.end())
    var_nums.push_back(_var_num);

  for(unsigned int i=0;i<_coupled_to.size();i++)
  {
    std::string coupled_var_name=_coupled_to[i];

    unsigned int coupled_var_num = _system->variable_number(coupled_var_name);

    if(std::find(var_nums.begin(),var_nums.end(),coupled_var_num) == var_nums.end())
      var_nums.push_back(coupled_var_num);
  }
} 

void BoundaryCondition::init()
{
  _fe_face = FEBase::build(_dim, _fe_type);
  _qface = new QGauss(_dim-1,_fe_type.default_quadrature_order());
  _fe_face->attach_quadrature_rule(_qface);

  _static_q_point_face = &_fe_face->get_xyz();
  _static_JxW_face = &_fe_face->get_JxW();
  _static_phi_face = &_fe_face->get_phi();
  _static_dphi_face = &_fe_face->get_dphi();
  _static_normals_face = &_fe_face->get_normals();
}

void BoundaryCondition::reinit(const NumericVector<Number>& soln, const unsigned int side, const unsigned int boundary_id)
{
  Moose::perf_log.push("reinit()","BoundaryCondition");

  _current_side = side;
  _fe_face->reinit(_current_elem, _current_side);  

  std::vector<unsigned int>::iterator var_nums_it = _boundary_to_var_nums[boundary_id].begin();
  std::vector<unsigned int>::iterator var_nums_end = _boundary_to_var_nums[boundary_id].end();

  for(;var_nums_it != var_nums_end; ++var_nums_it)
  {
    unsigned int var_num = *var_nums_it;

    std::vector<unsigned int> & var_dof_indices = _var_dof_indices[var_num];

    _var_vals_face[var_num].resize(_qface->n_points());
    _var_grads_face[var_num].resize(_qface->n_points());

    for (unsigned int qp=0; qp<_qface->n_points(); qp++)
    {
      computeQpSolution(_var_vals_face[var_num][qp], soln, var_dof_indices, qp, *_static_phi_face);
      computeQpGradSolution(_var_grads_face[var_num][qp], soln, var_dof_indices, qp, *_static_dphi_face);
    }
  }

  std::vector<unsigned int>::iterator var_nums_nodal_it = _boundary_to_var_nums_nodal[boundary_id].begin();
  std::vector<unsigned int>::iterator var_nums_nodal_end = _boundary_to_var_nums_nodal[boundary_id].end();
  
  for(;var_nums_nodal_it != var_nums_nodal_end; ++var_nums_nodal_it)
  {
    unsigned int var_num = *var_nums_nodal_it;

    std::vector<unsigned int> & var_dof_indices = _var_dof_indices[var_num];

    _var_vals_face_nodal[var_num].resize(_current_elem->n_nodes());

    for(unsigned int i=0; i<_current_elem->n_nodes(); i++)
      _var_vals_face_nodal[var_num][i] = soln(var_dof_indices[i]);
  }
  
  Moose::perf_log.pop("reinit()","BoundaryCondition");
}

void
BoundaryCondition::computeResidual()
{
  Moose::perf_log.push("computeResidual()","BoundaryCondition");

  DenseSubVector<Number> & var_Re = *_var_Res[_var_num];

  if(_integrated)
    for (_qp=0; _qp<_qface->n_points(); _qp++)
      for (_i=0; _i<_phi_face.size(); _i++)
	var_Re(_i) += _JxW_face[_qp]*computeQpResidual();
  else
  {
    //Use _qp to keep things standard at the leaf level
    //_qp is really looping over nodes right now.
    for(_qp=0; _qp<_current_elem->n_nodes(); _qp++)
      if(_current_elem->is_node_on_side(_qp,_current_side))
	var_Re(_qp) = computeQpResidual();
  }

  Moose::perf_log.pop("computeResidual()","BoundaryCondition");
}

void
BoundaryCondition::computeJacobian()
{
  Moose::perf_log.push("computeJacobian()","BoundaryCondition");

  DenseSubMatrix<Number> & var_Ke = *_var_Kes[_var_num];

  if(_integrated)
    for (_qp=0; _qp<_qface->n_points(); _qp++)
      for (_i=0; _i<_phi_face.size(); _i++)
        for (_j=0; _j<_phi_face.size(); _j++)
          var_Ke(_i,_j) += _JxW_face[_qp]*computeQpJacobian();
  else
  {
    for(_i=0; _i<_phi_face.size(); _i++)
    {
      if(_current_elem->is_node_on_side(_i,_current_side))
      {
        //Zero out the row and put 1 on the diagonal
        for(_j=0; _j<_phi_face.size(); _j++)
          var_Ke(_i,_j) = 0;
        
	var_Ke(_i,_i) = 1;
      }
    }
  }

  Moose::perf_log.pop("computeJacobian()","BoundaryCondition");
}

std::vector<Real> &
BoundaryCondition::coupledValFace(std::string name)
{
  if(_integrated)
    return _var_vals_face[_coupled_as_to_var_num[name]];

  return _var_vals_face_nodal[_coupled_as_to_var_num[name]];
}

std::vector<RealGradient> &
BoundaryCondition::coupledGradFace(std::string name)
{
  if(_integrated)
    return _var_grads_face[_coupled_as_to_var_num[name]];

  error();
}

unsigned int BoundaryCondition::_current_side;
AutoPtr<FEBase> BoundaryCondition::_fe_face;
QGauss * BoundaryCondition::_qface;
const std::vector<Point> * BoundaryCondition::_static_q_point_face;
const std::vector<Real> * BoundaryCondition::_static_JxW_face;
const std::vector<std::vector<Real> > * BoundaryCondition::_static_phi_face;
const std::vector<std::vector<RealGradient> > * BoundaryCondition::_static_dphi_face;
const std::vector<Point> * BoundaryCondition::_static_normals_face;
std::map<unsigned int, std::vector<unsigned int> > BoundaryCondition::_boundary_to_var_nums;
std::map<unsigned int, std::vector<unsigned int> > BoundaryCondition::_boundary_to_var_nums_nodal;
std::map<unsigned int, std::vector<Real> > BoundaryCondition::_var_vals_face;
std::map<unsigned int, std::vector<RealGradient> > BoundaryCondition::_var_grads_face;
std::map<unsigned int, std::vector<Real> > BoundaryCondition::_var_vals_face_nodal;
