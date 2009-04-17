#include "BoundaryCondition.h"

#include "numeric_vector.h"

#include "Moose.h"
#include "dof_map.h"

BoundaryCondition::BoundaryCondition(std::string name,
                                     Parameters parameters,
                                     std::string var_name,
                                     bool integrated,
                                     unsigned int boundary_id,
                                     std::vector<std::string> coupled_to,
                                     std::vector<std::string> coupled_as)
  :Kernel(name, parameters, var_name, integrated, coupled_to, coupled_as),
   _boundary_id(boundary_id),
   _side_elem(NULL),
   _JxW_face(*_static_JxW_face[_fe_type]),
   _phi_face(*_static_phi_face[_fe_type]),
   _dphi_face(*_static_dphi_face[_fe_type]),
   _d2phi_face(*_static_d2phi_face[_fe_type]),
   _normals_face(*_static_normals_face[_fe_type]),
   _q_point_face(*_static_q_point_face[_fe_type]),
   _u_face(integrated ? _var_vals_face[_var_num] : _var_vals_face_nodal[_var_num]),
   _grad_u_face(integrated ? _var_grads_face[_var_num] : _grad_zero),
   _second_u_face(integrated ? _var_seconds_face[_var_num] : _second_zero)
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
  //Max quadrature order was already found by Kernel::init()
  _qface = new QGauss(_dim-1,_max_quadrature_order);

  for(unsigned int var=0; var < _system->n_vars(); var++)
  {
    FEType fe_type = _dof_map->variable_type(var);

    if(!_fe_face[fe_type])
    {
      _fe_face[fe_type] = FEBase::build(_dim, fe_type).release();
      _fe_face[fe_type]->attach_quadrature_rule(_qface);

      _static_q_point_face[fe_type] = &_fe_face[fe_type]->get_xyz();
      _static_JxW_face[fe_type] = &_fe_face[fe_type]->get_JxW();
      _static_phi_face[fe_type] = &_fe_face[fe_type]->get_phi();
      _static_dphi_face[fe_type] = &_fe_face[fe_type]->get_dphi();
      _static_normals_face[fe_type] = &_fe_face[fe_type]->get_normals();

      FEFamily family = fe_type.family;

      if(family == CLOUGH || family == HERMITE)
        _static_d2phi_face[fe_type] = &_fe[fe_type]->get_d2phi();
    }
  }
}

void BoundaryCondition::reinit(const NumericVector<Number>& soln, const unsigned int side, const unsigned int boundary_id)
{
  Moose::perf_log.push("reinit()","BoundaryCondition");

  _current_side = side;

  std::map<FEType, FEBase*>::iterator fe_it = _fe_face.begin();
  std::map<FEType, FEBase*>::iterator fe_end = _fe_face.end();

  for(;fe_it != fe_end; ++fe_it)
    fe_it->second->reinit(_current_elem, _current_side);  

  std::vector<unsigned int>::iterator var_nums_it = _boundary_to_var_nums[boundary_id].begin();
  std::vector<unsigned int>::iterator var_nums_end = _boundary_to_var_nums[boundary_id].end();

  for(;var_nums_it != var_nums_end; ++var_nums_it)
  {
    unsigned int var_num = *var_nums_it;

    FEType fe_type = _dof_map->variable_type(var_num);

    FEFamily family = fe_type.family;

    bool has_second_derivatives = (family == CLOUGH || family == HERMITE);

    std::vector<unsigned int> & var_dof_indices = _var_dof_indices[var_num];

    _var_vals_face[var_num].resize(_qface->n_points());
    _var_grads_face[var_num].resize(_qface->n_points());

    if(has_second_derivatives)
      _var_seconds_face[var_num].resize(_qface->n_points());

    const std::vector<std::vector<Real> > & static_phi_face = *_static_phi_face[fe_type];
    const std::vector<std::vector<RealGradient> > & static_dphi_face= *_static_dphi_face[fe_type];
    const std::vector<std::vector<RealTensor> > & static_d2phi_face= *_static_d2phi_face[fe_type];

    for (unsigned int qp=0; qp<_qface->n_points(); qp++)
    {
      computeQpSolution(_var_vals_face[var_num][qp], soln, var_dof_indices, qp, static_phi_face);
      computeQpGradSolution(_var_grads_face[var_num][qp], soln, var_dof_indices, qp, static_dphi_face);

      if(has_second_derivatives)
        computeQpSecondSolution(_var_seconds_face[var_num][qp], soln, var_dof_indices, qp, static_d2phi_face);
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



void BoundaryCondition::reinit(const NumericVector<Number>& soln, const Node & node, const unsigned int boundary_id, NumericVector<Number>& residual)
{
  Moose::perf_log.push("reinit(node)","BoundaryCondition");

  _current_node = &node;

  _current_residual = &residual;

  std::vector<unsigned int>::iterator var_nums_nodal_it = _boundary_to_var_nums_nodal[boundary_id].begin();
  std::vector<unsigned int>::iterator var_nums_nodal_end = _boundary_to_var_nums_nodal[boundary_id].end();

  unsigned int nonlinear_system_number = _system->number();
  
  for(;var_nums_nodal_it != var_nums_nodal_end; ++var_nums_nodal_it)
  {
    unsigned int var_num = *var_nums_nodal_it;

    //The zero is the component... that works fine for lagrange FE types.
    unsigned int dof_number = node.dof_number(nonlinear_system_number, var_num, 0);

    _nodal_bc_var_dofs[var_num] = dof_number;

    _var_vals_face_nodal[var_num].resize(1);

    _var_vals_face_nodal[var_num][0] = soln(dof_number);
  }

  Moose::perf_log.pop("reinit(node)","BoundaryCondition");
}


void
BoundaryCondition::computeResidual()
{
//  Moose::perf_log.push("computeResidual()","BoundaryCondition");

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

//  Moose::perf_log.pop("computeResidual()","BoundaryCondition");
}

void
BoundaryCondition::computeJacobian()
{
//  Moose::perf_log.push("computeJacobian()","BoundaryCondition");

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

//  Moose::perf_log.pop("computeJacobian()","BoundaryCondition");
}

void
BoundaryCondition::computeJacobianBlock(DenseMatrix<Number> & Ke, unsigned int ivar, unsigned int jvar)
{
//  Moose::perf_log.push("computeJacobianBlock()","BoundaryCondition");

  if(_integrated)
    for (_qp=0; _qp<_qface->n_points(); _qp++)
      for (_i=0; _i<_phi_face.size(); _i++)
        for (_j=0; _j<_phi_face.size(); _j++)
        {
          if(ivar ==jvar)
            Ke(_i,_j) += _JxW_face[_qp]*computeQpJacobian();
          else
            Ke(_i,_j) += _JxW_face[_qp]*computeQpOffDiagJacobian(jvar);
        }
  
  else
  {
    for(_i=0; _i<_phi_face.size(); _i++)
    {
      if(_current_elem->is_node_on_side(_i,_current_side))
      {
        //Zero out the row and put 1 on the diagonal
        for(_j=0; _j<_phi_face.size(); _j++)
          Ke(_i,_j) = 0;

        if(ivar ==jvar)
          Ke(_i,_i) = 1;
      }
    }
  }

//  Moose::perf_log.pop("computeJacobianBlock()","BoundaryCondition");
}

void
BoundaryCondition::computeAndStoreResidual()
{
  _qp = 0;
  _current_residual->set(_nodal_bc_var_dofs[_var_num], computeQpResidual());
}

std::vector<Real> &
BoundaryCondition::coupledValFace(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"BC "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    libmesh_error();
  }
  
  if(_integrated)
    return _var_vals_face[_coupled_as_to_var_num[name]];

  return _var_vals_face_nodal[_coupled_as_to_var_num[name]];
}

std::vector<RealGradient> &
BoundaryCondition::coupledGradFace(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"BC "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    libmesh_error();
  }

  if(_integrated)
    return _var_grads_face[_coupled_as_to_var_num[name]];

  error();
}

const Node * BoundaryCondition::_current_node;
NumericVector<Number> * BoundaryCondition::_current_residual;
unsigned int BoundaryCondition::_current_side;
std::map<FEType, FEBase *> BoundaryCondition::_fe_face;
QGauss * BoundaryCondition::_qface;
std::map<FEType, const std::vector<Point> *> BoundaryCondition::_static_q_point_face;
std::map<FEType, const std::vector<Real> *> BoundaryCondition::_static_JxW_face;
std::map<FEType, const std::vector<std::vector<Real> > *> BoundaryCondition::_static_phi_face;
std::map<FEType, const std::vector<std::vector<RealGradient> > *> BoundaryCondition::_static_dphi_face;
std::map<FEType, const std::vector<std::vector<RealTensor> > *> BoundaryCondition::_static_d2phi_face;
std::map<FEType, const std::vector<Point> *> BoundaryCondition::_static_normals_face;
std::map<unsigned int, std::vector<unsigned int> > BoundaryCondition::_boundary_to_var_nums;
std::map<unsigned int, std::vector<unsigned int> > BoundaryCondition::_boundary_to_var_nums_nodal;
std::map<unsigned int, unsigned int> BoundaryCondition::_nodal_bc_var_dofs;
std::map<unsigned int, std::vector<Real> > BoundaryCondition::_var_vals_face;
std::map<unsigned int, std::vector<RealGradient> > BoundaryCondition::_var_grads_face;
std::map<unsigned int, std::vector<RealTensor> > BoundaryCondition::_var_seconds_face;
std::map<unsigned int, std::vector<Real> > BoundaryCondition::_var_vals_face_nodal;
