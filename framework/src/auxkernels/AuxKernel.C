#include "AuxKernel.h"

//local includes
#include "Moose.h"
#include "MooseSystem.h"

//libmesh includes
#include "numeric_vector.h"
#include "dof_map.h"

template<>
InputParameters validParams<AuxKernel>()
{
  InputParameters params;
  params.addRequiredParam<std::string>("variable", "The name of the variable that this boundary condition applies to");
  params.addParam<std::vector<std::string> >("coupled_to", "The list of variable names this Material is coupled to.");
  params.addParam<std::vector<std::string> >("coupled_as", "The list of variable names as referenced inside of this Material which correspond with the coupled_as names");

  // For use on the boundary only
  params.addParam<std::vector<unsigned int> >("boundary", "The list of variable names this Material is coupled to.");
  return params;
}

AuxKernel::AuxKernel(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :Kernel(name, moose_system, parameters),
   _nodal(_fe_type.family == LAGRANGE),
   _u_aux(_nodal ? moose_system._aux_var_vals_nodal[_tid][_var_num] : moose_system._aux_var_vals_element[_tid][_var_num]),
   _u_old_aux(_nodal ? moose_system._aux_var_vals_old_nodal[_tid][_var_num] : moose_system._aux_var_vals_old_element[_tid][_var_num]),
   _u_older_aux(_nodal ? moose_system._aux_var_vals_older_nodal[_tid][_var_num] : moose_system._aux_var_vals_older_element[_tid][_var_num]),
   _current_node(moose_system._current_node[_tid])
{
  if(_nodal)
    moose_system._nodal_var_nums.push_back(_var_num);
  else
    moose_system._element_var_nums.push_back(_var_num);
}

void
AuxKernel::computeAndStore(THREAD_ID tid)
{
  _moose_system._aux_soln->set(_moose_system._aux_var_dofs[tid][_var_num], computeValue());
}

bool
AuxKernel::isNodal()
{
  return _nodal;
}

Real
AuxKernel::computeQpResidual()
  {
    return 0;
  }

Real &
AuxKernel::coupledValAux(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"AuxKernel "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    mooseError("");
  }

  if(_nodal)
  {
    if(!isAux(name))
      return _moose_system._var_vals_nodal[_tid][_coupled_as_to_var_num[name]];
    else
      return _moose_system._aux_var_vals_nodal[_tid][_aux_coupled_as_to_var_num[name]];
  }
  else
  {
    if(!isAux(name))
      return _moose_system._var_vals_element[_tid][_coupled_as_to_var_num[name]];
    else
      return _moose_system._aux_var_vals_element[_tid][_aux_coupled_as_to_var_num[name]];
  }
}


Real &
AuxKernel::coupledValOldAux(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"AuxKernel "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    mooseError("");
  }

  if(_nodal)
  {
    if(!isAux(name))
      return _moose_system._var_vals_old_nodal[_tid][_coupled_as_to_var_num[name]];
    else
      return _moose_system._aux_var_vals_old_nodal[_tid][_aux_coupled_as_to_var_num[name]];
  }
  else
  {
    if(!isAux(name))
      return _moose_system._var_vals_old_element[_tid][_coupled_as_to_var_num[name]];
    else
      return _moose_system._aux_var_vals_old_element[_tid][_aux_coupled_as_to_var_num[name]];
  }
}


Real &
AuxKernel::coupledValOlderAux(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"AuxKernel "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    mooseError("");
  }

  if(_nodal)
  {
    if(!isAux(name))
      return _moose_system._var_vals_older_nodal[_tid][_coupled_as_to_var_num[name]];
    else
      return _moose_system._aux_var_vals_older_nodal[_tid][_aux_coupled_as_to_var_num[name]];
  }
  else
  {
    if(!isAux(name))
      return _moose_system._var_vals_older_element[_tid][_coupled_as_to_var_num[name]];
    else
      return _moose_system._aux_var_vals_older_element[_tid][_aux_coupled_as_to_var_num[name]];
  }  
}


RealGradient &
AuxKernel::coupledGradAux(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"AuxKernel "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    mooseError("");
  }

  if(_nodal)
  {
    std::cerr<<std::endl<<"Gradient can not be recovered with nodal AuxKernel "<<_name<<" with a variable coupled_as "<<name<<std::endl<<std::endl;
    mooseError("");
  }
  else
  {
    if(!isAux(name))
      return _moose_system._var_grads_element[_tid][_coupled_as_to_var_num[name]];
    else
      return _moose_system._aux_var_grads_element[_tid][_aux_coupled_as_to_var_num[name]];
  }
}

RealGradient &
AuxKernel::coupledGradOldAux(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"AuxKernel "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    mooseError("");
  }

  if(_nodal)
  {
    std::cerr<<std::endl<<"Old Gradient can not be recovered with nodal AuxKernel "<<_name<<" with a variable coupled_as "<<name<<std::endl<<std::endl;
    mooseError("");
  }
  else
  {
    if(!isAux(name))
      return _moose_system._var_grads_old_element[_tid][_coupled_as_to_var_num[name]];
    else
      return _moose_system._aux_var_grads_old_element[_tid][_aux_coupled_as_to_var_num[name]];
  }
}

RealGradient &
AuxKernel::coupledGradOlderAux(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"AuxKernel "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    mooseError("");
  }

  if(_nodal)
  {
    std::cerr<<std::endl<<"Older Gradient can not be recovered with nodal AuxKernel "<<_name<<" with a variable coupled_as "<<name<<std::endl<<std::endl;
    mooseError("");
  }
  else
  {
    if(!isAux(name))
      return _moose_system._var_grads_older_element[_tid][_coupled_as_to_var_num[name]];
    else
      return _moose_system._aux_var_grads_older_element[_tid][_aux_coupled_as_to_var_num[name]];
  }
}

