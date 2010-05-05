//MOOSE includes
#include "Moose.h"
#include "BoundaryCondition.h"

//libMesh includes
#include "numeric_vector.h"
#include "dof_map.h"

template<>
InputParameters validParams<BoundaryCondition>()
{
  InputParameters params;
  params.addRequiredParam<std::string>("variable", "The name of the variable that this boundary condition applies to");
  params.addRequiredParam<std::vector<unsigned int> >("boundary", "The list of boundary IDs from the mesh where this boundary condition applies");
  params.addParam<std::vector<std::string> >("coupled_to", "The list of variable names this boundary condition is coupled to.");
  params.addParam<std::vector<std::string> >("coupled_as", "The list of variable names as referenced inside of this boundary condition which correspond with the coupled_to names");
  return params;
}

BoundaryCondition::BoundaryCondition(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :Kernel(name, moose_system, parameters),
//          parameters,
//          parameters.get<std::string>("variable"),
//          parameters.get<bool>("_integrated"),
//          parameters.have_parameter<std::vector<std::string> >("coupled_to") ? parameters.get<std::vector<std::string> >("coupled_to") : std::vector<std::string>(0),
//          parameters.have_parameter<std::vector<std::string> >("coupled_as") ? parameters.get<std::vector<std::string> >("coupled_as") : std::vector<std::string>(0)),
   _boundary_id(parameters.get<unsigned int>("_boundary_id")),
   _side_elem(NULL),
   _JxW_face(*_static_JxW_face[_tid][_fe_type]),
   _phi_face(*_static_phi_face[_tid][_fe_type]),
   _dphi_face(*_static_dphi_face[_tid][_fe_type]),
   _d2phi_face(*_static_d2phi_face[_tid][_fe_type]),
   _normals_face(*_static_normals_face[_tid][_fe_type]),
   _qface(_static_qface[_tid]),
   _q_point_face(*_static_q_point_face[_tid][_fe_type]),
   _current_side(_static_current_side[_tid]),
   _current_node(_static_current_node[_tid]),
   _current_residual(_static_current_residual[_tid]),
   _u_face(_integrated ? _var_vals_face[_tid][_var_num] : _var_vals_face_nodal[_tid][_var_num]),
   _grad_u_face(_integrated ? _var_grads_face[_tid][_var_num] : _grad_zero),
   _second_u_face(_integrated ? _var_seconds_face[_tid][_var_num] : _second_zero)
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

void
BoundaryCondition::sizeEverything()
{
  int n_threads = libMesh::n_threads();

  _static_current_node.resize(n_threads);
  _static_current_residual.resize(n_threads);
  _static_current_side.resize(n_threads);
  _fe_face.resize(n_threads);
  _static_qface.resize(n_threads);
  _static_q_point_face.resize(n_threads);
  _static_JxW_face.resize(n_threads);
  _static_phi_face.resize(n_threads);
  _static_dphi_face.resize(n_threads);
  _static_d2phi_face.resize(n_threads);
  _static_normals_face.resize(n_threads);

  _nodal_bc_var_dofs.resize(n_threads);
  _var_vals_face.resize(n_threads);
  _var_grads_face.resize(n_threads);
  _var_seconds_face.resize(n_threads);
  _var_vals_face_nodal.resize(n_threads);
}

void BoundaryCondition::init()
{
  unsigned int n_vars = _system->n_vars();
  unsigned int n_aux_vars = _aux_system->n_vars();

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
    _static_qface[tid] = new QGauss(_dim-1,_max_quadrature_order);

  for(unsigned int var=0; var < _system->n_vars(); var++)
  {
    FEType fe_type = _dof_map->variable_type(var);

    for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
    {
      if(!_fe_face[tid][fe_type])
      {
        _fe_face[tid][fe_type] = FEBase::build(_dim, fe_type).release();
        _fe_face[tid][fe_type]->attach_quadrature_rule(_static_qface[tid]);

        _static_q_point_face[tid][fe_type] = &_fe_face[tid][fe_type]->get_xyz();
        _static_JxW_face[tid][fe_type] = &_fe_face[tid][fe_type]->get_JxW();
        _static_phi_face[tid][fe_type] = &_fe_face[tid][fe_type]->get_phi();
        _static_dphi_face[tid][fe_type] = &_fe_face[tid][fe_type]->get_dphi();
        _static_normals_face[tid][fe_type] = &_fe_face[tid][fe_type]->get_normals();

        FEFamily family = fe_type.family;

        if(family == CLOUGH || family == HERMITE)
          _static_d2phi_face[tid][fe_type] = &_fe_face[tid][fe_type]->get_d2phi();
      }
    }
  }
}

void BoundaryCondition::reinit(THREAD_ID tid, const NumericVector<Number>& soln, const unsigned int side, const unsigned int boundary_id)
{
//  Moose::perf_log.push("reinit()","BoundaryCondition");

  _static_current_side[tid] = side;

  std::map<FEType, FEBase*>::iterator fe_it = _fe_face[tid].begin();
  std::map<FEType, FEBase*>::iterator fe_end = _fe_face[tid].end();

  for(;fe_it != fe_end; ++fe_it)
    fe_it->second->reinit(_static_current_elem[tid], _static_current_side[tid]);

  std::vector<unsigned int>::iterator var_nums_it = _boundary_to_var_nums[boundary_id].begin();
  std::vector<unsigned int>::iterator var_nums_end = _boundary_to_var_nums[boundary_id].end();

  for(;var_nums_it != var_nums_end; ++var_nums_it)
  {
    unsigned int var_num = *var_nums_it;

    FEType fe_type = _dof_map->variable_type(var_num);

    FEFamily family = fe_type.family;

    bool has_second_derivatives = (family == CLOUGH || family == HERMITE);

    std::vector<unsigned int> & var_dof_indices = _var_dof_indices[tid][var_num];

    _var_vals_face[tid][var_num].resize(_static_qface[tid]->n_points());
    _var_grads_face[tid][var_num].resize(_static_qface[tid]->n_points());

    if(has_second_derivatives)
      _var_seconds_face[tid][var_num].resize(_static_qface[tid]->n_points());

    const std::vector<std::vector<Real> > & static_phi_face = *_static_phi_face[tid][fe_type];
    const std::vector<std::vector<RealGradient> > & static_dphi_face= *_static_dphi_face[tid][fe_type];
    const std::vector<std::vector<RealTensor> > & static_d2phi_face= *_static_d2phi_face[tid][fe_type];

    for (unsigned int qp=0; qp<_static_qface[tid]->n_points(); qp++)
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

    std::vector<unsigned int> & var_dof_indices = _var_dof_indices[tid][var_num];

    _var_vals_face_nodal[tid][var_num].resize(_static_current_elem[tid]->n_nodes());

    for(unsigned int i=0; i<_static_current_elem[tid]->n_nodes(); i++)
      _var_vals_face_nodal[tid][var_num][i] = soln(var_dof_indices[i]);
  }
  
//  Moose::perf_log.pop("reinit()","BoundaryCondition");
}



void BoundaryCondition::reinit(THREAD_ID tid, const NumericVector<Number>& soln, const Node & node, const unsigned int boundary_id, NumericVector<Number>& residual)
{
//  Moose::perf_log.push("reinit(node)","BoundaryCondition");

  _static_current_node[tid] = &node;

  _static_current_residual[tid] = &residual;

  std::vector<unsigned int>::iterator var_nums_nodal_it = _boundary_to_var_nums_nodal[boundary_id].begin();
  std::vector<unsigned int>::iterator var_nums_nodal_end = _boundary_to_var_nums_nodal[boundary_id].end();

  unsigned int nonlinear_system_number = _system->number();
  
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

unsigned int
BoundaryCondition::boundaryID()
{
  return _boundary_id;
}

void
BoundaryCondition::computeResidual()
{
//  Moose::perf_log.push("computeResidual()","BoundaryCondition");

  DenseSubVector<Number> & var_Re = *_var_Res[_tid][_var_num];

  if(_integrated)
    for (_qp=0; _qp<_qface->n_points(); _qp++)
      for (_i=0; _i<_phi_face.size(); _i++)
	var_Re(_i) += _scaling_factor[_var_num]*_JxW_face[_qp]*computeQpResidual();
  else
  {
    //Use _qp to keep things standard at the leaf level
    //_qp is really looping over nodes right now.
    for(_qp=0; _qp<_current_elem->n_nodes(); _qp++)
      if(_current_elem->is_node_on_side(_qp,_current_side))
	var_Re(_qp) = _scaling_factor[_var_num]*computeQpResidual();
  }

//  Moose::perf_log.pop("computeResidual()","BoundaryCondition");
}

void
BoundaryCondition::computeJacobian()
{
//  Moose::perf_log.push("computeJacobian()","BoundaryCondition");

  DenseMatrix<Number> & var_Ke = *_var_Kes[_tid][_var_num];

  if(_integrated)
    for (_qp=0; _qp<_qface->n_points(); _qp++)
      for (_i=0; _i<_phi_face.size(); _i++)
        for (_j=0; _j<_phi_face.size(); _j++)
          var_Ke(_i,_j) += _scaling_factor[_var_num]*_JxW_face[_qp]*computeQpJacobian();
  else
  {
    for(_i=0; _i<_phi_face.size(); _i++)
    {
      if(_current_elem->is_node_on_side(_i,_current_side))
      {
        //Zero out the row and put 1 on the diagonal
        for(_j=0; _j<_phi_face.size(); _j++)
          var_Ke(_i,_j) = 0;
        
	var_Ke(_i,_i) = _scaling_factor[_var_num]*1;
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
  _current_residual->set(_nodal_bc_var_dofs[_tid][_var_num], _scaling_factor[_var_num]*computeQpResidual());
}

Real
BoundaryCondition::computeIntegral()
{
//  Moose::perf_log.push("computeIntegral()",_name);

  Real sum = 0;
  
  for (_qp=0; _qp<_qface->n_points(); _qp++)
      sum += _JxW_face[_qp]*computeQpIntegral();
  
//  Moose::perf_log.pop("computeIntegral()",_name);
  return sum;
}


bool
BoundaryCondition::isIntegrated()
{
  return _integrated;
}

InputParameters &
BoundaryCondition::setIntegratedParam(InputParameters & params, bool integrated)
{
  params.set<bool>("_integrated") = integrated;
  return params;
}

std::vector<Real> &
BoundaryCondition::coupledValFace(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"BC "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    mooseError("");
  }
  
  if(_integrated)
    return _var_vals_face[_tid][_coupled_as_to_var_num[name]];

  return _var_vals_face_nodal[_tid][_coupled_as_to_var_num[name]];
}

std::vector<RealGradient> &
BoundaryCondition::coupledGradFace(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"BC "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    mooseError("");
  }

  if(_integrated)
    return _var_grads_face[_tid][_coupled_as_to_var_num[name]];

  mooseError("");
}

std::vector<const Node *> BoundaryCondition::_static_current_node;
std::vector<NumericVector<Number> *> BoundaryCondition::_static_current_residual;
std::vector<unsigned int> BoundaryCondition::_static_current_side;
std::vector<std::map<FEType, FEBase *> > BoundaryCondition::_fe_face;
std::vector<QGauss *> BoundaryCondition::_static_qface;
std::vector<std::map<FEType, const std::vector<Point> *> > BoundaryCondition::_static_q_point_face;
std::vector<std::map<FEType, const std::vector<Real> *> > BoundaryCondition::_static_JxW_face;
std::vector<std::map<FEType, const std::vector<std::vector<Real> > *> > BoundaryCondition::_static_phi_face;
std::vector<std::map<FEType, const std::vector<std::vector<RealGradient> > *> > BoundaryCondition::_static_dphi_face;
std::vector<std::map<FEType, const std::vector<std::vector<RealTensor> > *> > BoundaryCondition::_static_d2phi_face;
std::vector<std::map<FEType, const std::vector<Point> *> > BoundaryCondition::_static_normals_face;

std::map<unsigned int, std::vector<unsigned int> > BoundaryCondition::_boundary_to_var_nums;
std::map<unsigned int, std::vector<unsigned int> > BoundaryCondition::_boundary_to_var_nums_nodal;
std::vector<std::vector<unsigned int> > BoundaryCondition::_nodal_bc_var_dofs;
std::vector<std::vector<std::vector<Real> > > BoundaryCondition::_var_vals_face;
std::vector<std::vector<std::vector<RealGradient> > > BoundaryCondition::_var_grads_face;
std::vector<std::vector<std::vector<RealTensor> > > BoundaryCondition::_var_seconds_face;
std::vector<std::vector<std::vector<Real> > > BoundaryCondition::_var_vals_face_nodal;
