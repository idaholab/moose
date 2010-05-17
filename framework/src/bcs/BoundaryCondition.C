//MOOSE includes
#include "Moose.h"
#include "MooseSystem.h"
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

BoundaryCondition::BoundaryCondition(std::string name, MooseSystem & moose_system, InputParameters parameters) :
   _name(name),
   _moose_system(moose_system),
   _tid(Moose::current_thread_id),
   _parameters(parameters),
   _mesh(*_moose_system.getMesh()),
   _var_name(parameters.get<std::string>("variable")),
   _is_aux(_moose_system._aux_system->has_variable(_var_name)),
   _var_num(_is_aux ? _moose_system._aux_system->variable_number(_var_name) : _moose_system._system->variable_number(_var_name)),
   _fe_type(_is_aux ? _moose_system._aux_dof_map->variable_type(_var_num) : _moose_system._dof_map->variable_type(_var_num)),
   _integrated(parameters.have_parameter<bool>("_integrated") ? parameters.get<bool>("_integrated") : true),
   _dim(_moose_system._dim),
   _t(_moose_system._t),
   _dt(_moose_system._dt),
   _dt_old(_moose_system._dt_old),
   _is_transient(_moose_system._is_transient),
   _current_elem(_moose_system._current_elem[_tid]),
   _material(_moose_system._material[_tid]),
   _boundary_id(parameters.get<unsigned int>("_boundary_id")),
   _side_elem(NULL),
   _JxW_face(*moose_system._JxW_face[_tid][_fe_type]),
   _phi_face(*moose_system._phi_face[_tid][_fe_type]),
   _dphi_face(*moose_system._dphi_face[_tid][_fe_type]),
   _d2phi_face(*moose_system._d2phi_face[_tid][_fe_type]),
   _normals_face(*moose_system._normals_face[_tid][_fe_type]),
   _qface(moose_system._qface[_tid]),
   _q_point_face(*moose_system._q_point_face[_tid][_fe_type]),
   _coupled_to(parameters.have_parameter<std::vector<std::string> >("coupled_to") ? parameters.get<std::vector<std::string> >("coupled_to") : std::vector<std::string>(0)),
   _coupled_as(parameters.have_parameter<std::vector<std::string> >("coupled_as") ? parameters.get<std::vector<std::string> >("coupled_as") : std::vector<std::string>(0)),
   _real_zero(_moose_system._real_zero[_tid]),
   _zero(_moose_system._zero[_tid]),
   _grad_zero(_moose_system._grad_zero[_tid]),
   _second_zero(_moose_system._second_zero[_tid]),
   _start_time(parameters.have_parameter<Real>("start_time") ? parameters.get<Real>("start_time") : -std::numeric_limits<Real>::max()),
   _stop_time(parameters.have_parameter<Real>("stop_time") ? parameters.get<Real>("stop_time") : std::numeric_limits<Real>::max()),
   _current_side(moose_system._current_side[_tid]),
   _current_node(moose_system._current_node[_tid]),
   _current_residual(moose_system._current_residual[_tid]),
   _u_face(_integrated ? moose_system._var_vals_face[_tid][_var_num] : moose_system._var_vals_face_nodal[_tid][_var_num]),
   _grad_u_face(_integrated ? moose_system._var_grads_face[_tid][_var_num] : moose_system._grad_zero[_tid]),
   _second_u_face(_integrated ? moose_system._var_seconds_face[_tid][_var_num] : moose_system._second_zero[_tid])
{
  // FIXME: this for statement will go into a common ancestor
  for(unsigned int i=0;i<_coupled_to.size();i++)
  {
    std::string coupled_var_name=_coupled_to[i];

    //Is it in the nonlinear system or the aux system?
    if(_moose_system._system->has_variable(coupled_var_name))
    {
      unsigned int coupled_var_num = _moose_system._system->variable_number(coupled_var_name);

      _coupled_as_to_var_num[_coupled_as[i]] = coupled_var_num;

      if(std::find(_moose_system._var_nums.begin(),_moose_system._var_nums.end(),coupled_var_num) == _moose_system._var_nums.end())
        _moose_system._var_nums.push_back(coupled_var_num);

      if(std::find(_coupled_var_nums.begin(),_coupled_var_nums.end(),coupled_var_num) == _coupled_var_nums.end())
        _coupled_var_nums.push_back(coupled_var_num);
    }
    else //Look for it in the Aux system
    {
      unsigned int coupled_var_num = _moose_system._aux_system->variable_number(coupled_var_name);

      _aux_coupled_as_to_var_num[_coupled_as[i]] = coupled_var_num;

      if(std::find(_moose_system._aux_var_nums.begin(),_moose_system._aux_var_nums.end(),coupled_var_num) == _moose_system._aux_var_nums.end())
        _moose_system._aux_var_nums.push_back(coupled_var_num);

      if(std::find(_aux_coupled_var_nums.begin(),_aux_coupled_var_nums.end(),coupled_var_num) == _aux_coupled_var_nums.end())
        _aux_coupled_var_nums.push_back(coupled_var_num);
    }
  }

  if(_integrated)
    addVarNums(moose_system._boundary_to_var_nums[_boundary_id]);
  else
    addVarNums(moose_system._boundary_to_var_nums_nodal[_boundary_id]);
}

void BoundaryCondition::addVarNums(std::vector<unsigned int> & var_nums)
{
  // If this variable isn't known yet... make it so
  if(std::find(var_nums.begin(), var_nums.end(), _var_num) == var_nums.end())
    var_nums.push_back(_var_num);

  for(unsigned int i=0;i<_coupled_to.size();i++)
  {
    std::string coupled_var_name=_coupled_to[i];

    unsigned int coupled_var_num = _moose_system._system->variable_number(coupled_var_name);

    if(std::find(var_nums.begin(),var_nums.end(),coupled_var_num) == var_nums.end())
      var_nums.push_back(coupled_var_num);
  }
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

  DenseSubVector<Number> & var_Re = *_moose_system._var_Res[_tid][_var_num];

  if(_integrated)
    for (_qp=0; _qp<_qface->n_points(); _qp++)
      for (_i=0; _i<_phi_face.size(); _i++)
        var_Re(_i) += _moose_system._scaling_factor[_var_num]*_JxW_face[_qp]*computeQpResidual();
  else
  {
    //Use _qp to keep things standard at the leaf level
    //_qp is really looping over nodes right now.
    for(_qp=0; _qp<_current_elem->n_nodes(); _qp++)
      if(_current_elem->is_node_on_side(_qp,_current_side))
	var_Re(_qp) = _moose_system._scaling_factor[_var_num]*computeQpResidual();
  }

//  Moose::perf_log.pop("computeResidual()","BoundaryCondition");
}

void
BoundaryCondition::computeJacobian()
{
//  Moose::perf_log.push("computeJacobian()","BoundaryCondition");

  DenseMatrix<Number> & var_Ke = *_moose_system._var_Kes[_tid][_var_num];

  if(_integrated)
    for (_qp=0; _qp<_qface->n_points(); _qp++)
      for (_i=0; _i<_phi_face.size(); _i++)
        for (_j=0; _j<_phi_face.size(); _j++)
          var_Ke(_i,_j) += _moose_system._scaling_factor[_var_num]*_JxW_face[_qp]*computeQpJacobian();
  else
  {
    for(_i=0; _i<_phi_face.size(); _i++)
    {
      if(_current_elem->is_node_on_side(_i,_current_side))
      {
        //Zero out the row and put 1 on the diagonal
        for(_j=0; _j<_phi_face.size(); _j++)
          var_Ke(_i,_j) = 0;
        
        var_Ke(_i,_i) = _moose_system._scaling_factor[_var_num]*1;
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
  _current_residual->set(_moose_system._nodal_bc_var_dofs[_tid][_var_num], _moose_system._scaling_factor[_var_num]*computeQpResidual());
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

unsigned int
BoundaryCondition::variable()
{
  return _var_num;
}

THREAD_ID
BoundaryCondition::tid()
{
  return _tid;
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
    return _moose_system._var_vals_face[_tid][_coupled_as_to_var_num[name]];

  return _moose_system._var_vals_face_nodal[_tid][_coupled_as_to_var_num[name]];
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
    return _moose_system._var_grads_face[_tid][_coupled_as_to_var_num[name]];

  mooseError("");
}

bool
BoundaryCondition::isCoupled(std::string name)
{
  bool found = std::find(_coupled_as.begin(),_coupled_as.end(),name) != _coupled_as.end();

  //See if it's an Aux variable
  if(!found)
    found = isAux(name);

  return found;
}

unsigned int
BoundaryCondition::coupled(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"Kernel "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    mooseError("");
  }

  if(!isAux(name))
    return _coupled_as_to_var_num[name];
  else
    return Kernel::modifiedAuxVarNum(_aux_coupled_as_to_var_num[name]);
}

Real
BoundaryCondition::computeQpJacobian()
{
  return 0;
}

Real
BoundaryCondition::computeQpOffDiagJacobian(unsigned int jvar)
{
  return 0;
}

Real
BoundaryCondition::computeQpIntegral()
{
  return 0;
}

bool
BoundaryCondition::isAux(std::string name)
{
  return _aux_coupled_as_to_var_num.find(name) != _aux_coupled_as_to_var_num.end();
}
