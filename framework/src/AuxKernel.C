#include "AuxKernel.h"

//libmesh includes
#include "numeric_vector.h"

AuxKernel::AuxKernel(std::string name,
                     Parameters parameters,
                     std::string var_name,
                     std::vector<std::string> coupled_to,
                     std::vector<std::string> coupled_as)
  :Kernel(name, parameters, var_name, false, coupled_to, coupled_as),
   _u_nodal(_aux_var_vals_nodal[_var_num]),
   _u_old_nodal(_aux_var_vals_old_nodal[_var_num]),
   _u_older_nodal(_aux_var_vals_older_nodal[_var_num])
  {}

void
AuxKernel::init()
{
  _nonlinear_old_soln = _system->old_local_solution.get();
  _nonlinear_older_soln = _system->older_local_solution.get();
  
  _aux_soln = _aux_system->solution.get();
  _aux_old_soln = _aux_system->old_local_solution.get();
  _aux_older_soln = _aux_system->older_local_solution.get();
}

void
AuxKernel::reinit(const NumericVector<Number>& soln, const Node & node)
{
  unsigned int nonlinear_system_number = _system->number();
  unsigned int aux_system_number = _aux_system->number();

  //Non Aux vars first
  for(unsigned int i=0; i<_var_nums.size(); i++)
  {
    unsigned int var_num = _var_nums[i];
    
    //The zero is the component... that works fine for lagrange FE types.
    unsigned int dof_number = node.dof_number(nonlinear_system_number, var_num, 0);

    _var_vals_nodal[var_num] = soln(dof_number);
    _var_vals_old_nodal[var_num] = (*_nonlinear_old_soln)(dof_number);
    _var_vals_older_nodal[var_num] = (*_nonlinear_older_soln)(dof_number);
  }

  const NumericVector<Number>& aux_soln = *_aux_system->solution;
  const NumericVector<Number>& aux_old_soln = *_aux_system->old_local_solution;
  const NumericVector<Number>& aux_older_soln = *_aux_system->older_local_solution;

  //Now Aux vars
  for(unsigned int i=0; i<_aux_var_nums.size(); i++)
  {
    unsigned int var_num = _aux_var_nums[i];
    
    //The zero is the component... that works fine for lagrange FE types.
    unsigned int dof_number = node.dof_number(aux_system_number, var_num, 0);

    _aux_var_dofs[var_num] = dof_number;
    _aux_var_vals_nodal[var_num] = (*_aux_soln)(dof_number);
    _aux_var_vals_old_nodal[var_num] = (*_aux_old_soln)(dof_number);
    _aux_var_vals_older_nodal[var_num] = (*_aux_older_soln)(dof_number);
  }
}

void
AuxKernel::computeAndStore()
{
  _aux_soln->set(_aux_var_dofs[_var_num], computeValue());
}

Real &
AuxKernel::coupledValAux(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"AuxKernel "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    libmesh_error();
  }

  if(!isAux(name))
    return _var_vals_nodal[_coupled_as_to_var_num[name]];
  else
    return _aux_var_vals_nodal[_aux_coupled_as_to_var_num[name]];
}


Real &
AuxKernel::coupledValOldAux(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"AuxKernel "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    libmesh_error();
  }

  if(!isAux(name))
    return _var_vals_old_nodal[_coupled_as_to_var_num[name]];
  else
    return _aux_var_vals_old_nodal[_aux_coupled_as_to_var_num[name]];
}


Real &
AuxKernel::coupledValOlderAux(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"AuxKernel "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    libmesh_error();
  }

  if(!isAux(name))
    return _var_vals_older_nodal[_coupled_as_to_var_num[name]];
  else
    return _aux_var_vals_older_nodal[_aux_coupled_as_to_var_num[name]];
}


const NumericVector<Number> * AuxKernel::_nonlinear_old_soln;
const NumericVector<Number> * AuxKernel::_nonlinear_older_soln;
NumericVector<Number> * AuxKernel::_aux_soln;
const NumericVector<Number> * AuxKernel::_aux_old_soln;
const NumericVector<Number> * AuxKernel::_aux_older_soln;

std::map<unsigned int, Real > AuxKernel::_var_vals_nodal;
std::map<unsigned int, Real > AuxKernel::_var_vals_old_nodal;
std::map<unsigned int, Real > AuxKernel::_var_vals_older_nodal;

std::map<unsigned int, unsigned int> AuxKernel::_aux_var_dofs;
std::map<unsigned int, Real > AuxKernel::_aux_var_vals_nodal;
std::map<unsigned int, Real > AuxKernel::_aux_var_vals_old_nodal;
std::map<unsigned int, Real > AuxKernel::_aux_var_vals_older_nodal;
