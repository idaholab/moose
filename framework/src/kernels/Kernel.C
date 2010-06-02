#include "Kernel.h"
#include "Moose.h"
#include "MooseSystem.h"
#include "ElementData.h"
#include "MaterialData.h"
#include "MaterialFactory.h"
#include "ParallelUniqueId.h"

// libMesh includes
#include "dof_map.h"
#include "dense_vector.h"
#include "numeric_vector.h"
#include "dense_subvector.h"
#include "libmesh_common.h"

template<>
InputParameters validParams<Kernel>()
{
  InputParameters params;
  params.addRequiredParam<std::string>("variable", "The name of the variable that this kernel operates on");
  params.addParam<std::vector<unsigned int> >("block", "The list of ids of the blocks (subdomain) that this kernel will be applied to");
  params.addParam<std::vector<std::string> >("coupled_to", "The list of variable names this Kernel is coupled to.");
  params.addParam<std::vector<std::string> >("coupled_as", "The list of variable names as referenced inside of this Kernel which correspond with the coupled_to names");
  return params;
}


Kernel::Kernel(std::string name, MooseSystem & moose_system, InputParameters parameters):
  MaterialPropertyInterface(moose_system._material_data),
   _name(name),
   _moose_system(moose_system),
   _element_data(moose_system._element_data),
   _tid(Moose::current_thread_id),
   _parameters(parameters),
   _mesh(*_moose_system.getMesh()),
   _var_name(parameters.get<std::string>("variable")),
   _is_aux(_moose_system._aux_system->has_variable(_var_name)),
   _var_num(_is_aux ? _moose_system._aux_system->variable_number(_var_name) : _moose_system._system->variable_number(_var_name)),
   _integrated(parameters.have_parameter<bool>("_integrated") ? parameters.get<bool>("_integrated") : true),
   _dim(_moose_system._dim),
   _t(_moose_system._t),
   _dt(_moose_system._dt),
   _dt_old(_moose_system._dt_old),
   _is_transient(_moose_system._is_transient),
   _is_eigenvalue(_moose_system._is_eigenvalue),
   _t_step(_moose_system._t_step),
   _bdf2_wei(_moose_system._bdf2_wei),
   _t_scheme(_moose_system._t_scheme),
   _u(_element_data._var_vals[_tid][_var_num]),
   _grad_u(_element_data._var_grads[_tid][_var_num]),
   _second_u(_element_data._var_seconds[_tid][_var_num]),
   _u_old(_element_data._var_vals_old[_tid][_var_num]),
   _u_older(_element_data._var_vals_older[_tid][_var_num]),
   _grad_u_old(_element_data._var_grads_old[_tid][_var_num]),
   _grad_u_older(_element_data._var_grads_older[_tid][_var_num]),
   _fe_type(_is_aux ? _moose_system._aux_dof_map->variable_type(_var_num) : _moose_system._dof_map->variable_type(_var_num)),
   _current_elem(_element_data._current_elem[_tid]),
   _material(_moose_system._element_data._material[_tid]),
   _JxW(*(_element_data._JxW[_tid])[_fe_type]),
   _phi(*(_element_data._phi[_tid])[_fe_type]),
   _test((_element_data._test[_tid])[_var_num]),
   _dphi(*(_element_data._dphi[_tid])[_fe_type]),
   _dtest(*(_element_data._dphi[_tid])[_fe_type]),
   _d2phi(*(_element_data._d2phi[_tid])[_fe_type]),
   _d2test(*(_element_data._d2phi[_tid])[_fe_type]),
   _qrule(_element_data._qrule[_tid]),
   _q_point(*(_element_data._q_point[_tid])[_fe_type]),
   _coupled_to(parameters.have_parameter<std::vector<std::string> >("coupled_to") ? parameters.get<std::vector<std::string> >("coupled_to") : std::vector<std::string>(0)),
   _coupled_as(parameters.have_parameter<std::vector<std::string> >("coupled_as") ? parameters.get<std::vector<std::string> >("coupled_as") : std::vector<std::string>(0)),
   _real_zero(_moose_system._real_zero[_tid]),
   _zero(_moose_system._zero[_tid]),
   _grad_zero(_moose_system._grad_zero[_tid]),
   _second_zero(_moose_system._second_zero[_tid]),
   _has_second_derivatives(_fe_type.family == CLOUGH || _fe_type.family == HERMITE),
   _start_time(parameters.have_parameter<Real>("start_time") ? parameters.get<Real>("start_time") : -std::numeric_limits<Real>::max()),
   _stop_time(parameters.have_parameter<Real>("stop_time") ? parameters.get<Real>("stop_time") : std::numeric_limits<Real>::max())
{
  
  // Add all of our coupled variables to the coupled_to and coupled_as vectors
  for (std::set<std::string>::const_iterator iter = _parameters.coupledVarsBegin();
       iter != _parameters.coupledVarsEnd();
       ++iter)
  {
    if (_parameters.get<std::string>(*iter) != std::string())
    {
      _coupled_as.push_back(*iter);
      _coupled_to.push_back(_parameters.get<std::string>(*iter));
    }
  }
  
  // If this variable isn't known yet... make it so
  if(std::find(_element_data._var_nums[0].begin(),_element_data._var_nums[0].end(),_var_num) == _element_data._var_nums[0].end())
    _element_data._var_nums[0].push_back(_var_num);

  // FIXME: this for statement will go into a common ancestor
  for(unsigned int i=0;i<_coupled_to.size();i++)
  {
    std::string coupled_var_name=_coupled_to[i];

    //Is it in the nonlinear system or the aux system?
    if(_moose_system._system->has_variable(coupled_var_name))
    {
      unsigned int coupled_var_num = _moose_system._system->variable_number(coupled_var_name);

      _coupled_as_to_var_num[_coupled_as[i]] = coupled_var_num;

      if(std::find(_element_data._var_nums[0].begin(),_element_data._var_nums[0].end(),coupled_var_num) == _element_data._var_nums[0].end())
        _element_data._var_nums[0].push_back(coupled_var_num);

      if(std::find(_coupled_var_nums.begin(),_coupled_var_nums.end(),coupled_var_num) == _coupled_var_nums.end())
        _coupled_var_nums.push_back(coupled_var_num);
    }
    else //Look for it in the Aux system
    {
      unsigned int coupled_var_num = _moose_system._aux_system->variable_number(coupled_var_name);

      _aux_coupled_as_to_var_num[_coupled_as[i]] = coupled_var_num;

      if(std::find(_element_data._aux_var_nums[0].begin(),_element_data._aux_var_nums[0].end(),coupled_var_num) == _element_data._aux_var_nums[0].end())
        _element_data._aux_var_nums[0].push_back(coupled_var_num);

      if(std::find(_aux_coupled_var_nums.begin(),_aux_coupled_var_nums.end(),coupled_var_num) == _aux_coupled_var_nums.end())
        _aux_coupled_var_nums.push_back(coupled_var_num);
    }  
  }
}


std::string
Kernel::name() const
{
  return _name;
}

std::string
Kernel::varName() const
{
  return _var_name;
}

const std::vector<std::string> &
Kernel::coupledTo() const
{
  return _coupled_to;
}

void
Kernel::computeResidual()
{
//  Moose::perf_log.push("computeResidual()","Kernel");
  
  DenseSubVector<Number> & var_Re = *_element_data._var_Res[_tid][_var_num];

  for (_i=0; _i<_phi.size(); _i++)
    for (_qp=0; _qp<_qrule->n_points(); _qp++)
      var_Re(_i) += _moose_system._scaling_factor[_var_num]*_JxW[_qp]*computeQpResidual();
  
//  Moose::perf_log.pop("computeResidual()","Kernel");
}

void
Kernel::computeJacobian()
{
//  Moose::perf_log.push("computeJacobian()",_name);

  DenseMatrix<Number> & var_Ke = *_element_data._var_Kes[_tid][_var_num];


  for (_i=0; _i<_phi.size(); _i++)
    for (_j=0; _j<_phi.size(); _j++)
      for (_qp=0; _qp<_qrule->n_points(); _qp++)
        var_Ke(_i,_j) += _moose_system._scaling_factor[_var_num]*_JxW[_qp]*computeQpJacobian();
  
//  Moose::perf_log.pop("computeJacobian()",_name);
}

void
Kernel::computeOffDiagJacobian(DenseMatrix<Number> & Ke, unsigned int jvar)
{
//  Moose::perf_log.push("computeOffDiagJacobian()",_name);

  for (_i=0; _i<_phi.size(); _i++)
    for (_j=0; _j<_phi.size(); _j++)
      for (_qp=0; _qp<_qrule->n_points(); _qp++)
      {
        if(jvar == _var_num)
          Ke(_i,_j) += _moose_system._scaling_factor[_var_num]*_JxW[_qp]*computeQpJacobian();
        else
          Ke(_i,_j) += _moose_system._scaling_factor[_var_num]*_JxW[_qp]*computeQpOffDiagJacobian(jvar);
      }
  
//  Moose::perf_log.pop("computeOffDiagJacobian()",_name);
}


Real
Kernel::computeIntegral()
{
//  Moose::perf_log.push("computeIntegral()",_name);

  Real sum = 0;
  
  for (_qp=0; _qp<_qrule->n_points(); _qp++)
      sum += _JxW[_qp]*computeQpIntegral();
  
//  Moose::perf_log.pop("computeIntegral()",_name);
  return sum;
}

void
Kernel::subdomainSetup()
{
}

unsigned int
Kernel::variable()
{
  return _var_num;
}

THREAD_ID
Kernel::tid()
{
  return _tid;
}

Real
Kernel::startTime()
{
  return _start_time;
}

Real
Kernel::stopTime()
{
  return _stop_time;
}

unsigned int
Kernel::modifiedAuxVarNum(unsigned int var_num)
{
  return MAX_VARS + var_num;
}

Real
Kernel::computeQpJacobian()
  {
    return 0;
  }

Real
Kernel::computeQpOffDiagJacobian(unsigned int /*jvar*/)
  {
    return 0;
  }

Real
Kernel::computeQpIntegral()
  {
    return 0;
  }

bool
Kernel::isAux(std::string name)
{
  return _aux_coupled_as_to_var_num.find(name) != _aux_coupled_as_to_var_num.end();
}

bool
Kernel::isCoupled(std::string name)
{
  bool found = std::find(_coupled_as.begin(),_coupled_as.end(),name) != _coupled_as.end();

  //See if it's an Aux variable
  if(!found)
    found = isAux(name);

  return found;
}

unsigned int
Kernel::coupled(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"Kernel "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    mooseError("");
  }

  if(!isAux(name))
    return _coupled_as_to_var_num[name];
  else
    return modifiedAuxVarNum(_aux_coupled_as_to_var_num[name]);
}

MooseArray<Real> &
Kernel::coupledVal(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"Kernel "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    mooseError("");
  }

  if(!isAux(name))
    return _element_data._var_vals[_tid][_coupled_as_to_var_num[name]];
  else
    return _element_data._aux_var_vals[_tid][_aux_coupled_as_to_var_num[name]];
}

MooseArray<RealGradient> &
Kernel::coupledGrad(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"Kernel "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    mooseError("");
  }

  if(!isAux(name))
    return _element_data._var_grads[_tid][_coupled_as_to_var_num[name]];
  else
    return _element_data._aux_var_grads[_tid][_aux_coupled_as_to_var_num[name]];
}

MooseArray<RealTensor> &
Kernel::coupledSecond(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"Kernel "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    mooseError("");
  }

  //Aux vars can't have second derivatives!
  return _element_data._var_seconds[_tid][_coupled_as_to_var_num[name]];
}

MooseArray<Real> &
Kernel::coupledValOld(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"Kernel "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    mooseError("");
  }

  if(!isAux(name))
    return _element_data._var_vals_old[_tid][_coupled_as_to_var_num[name]];
  else
    return _element_data._aux_var_vals_old[_tid][_aux_coupled_as_to_var_num[name]];
}

MooseArray<Real> &
Kernel::coupledValOlder(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"Kernel "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    mooseError("");
  }

  if(!isAux(name))
    return _element_data._var_vals_older[_tid][_coupled_as_to_var_num[name]];
  else
    return _element_data._aux_var_vals_older[_tid][_aux_coupled_as_to_var_num[name]];
}

MooseArray<RealGradient> &
Kernel::coupledGradValOld(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"Kernel "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    mooseError("");
  }
  
  return _element_data._var_grads_old[_tid][_coupled_as_to_var_num[name]];
}
