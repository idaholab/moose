#include "AuxKernel.h"

//local includes
#include "Moose.h"
#include "MooseSystem.h"
#include "DofData.h"
#include "ElementData.h"

//libmesh includes
#include "numeric_vector.h"
#include "dof_map.h"

template<>
InputParameters validParams<AuxKernel>()
{
  InputParameters params = validParams<PDEBase>();
  params.addRequiredParam<std::string>("variable", "The name of the variable that this object applies to");
  // For use on the boundary only
  params.addParam<std::vector<unsigned int> >("boundary", "The list of variable names this Material is coupled to.");
  return params;
}

AuxKernel::AuxKernel(std::string name, MooseSystem & moose_system, InputParameters parameters) :
  PDEBase(name, moose_system, parameters, *moose_system._element_data[parameters.get<THREAD_ID>("_tid")]),
  MaterialPropertyInterface(moose_system._material_data[_tid]),
   _element_data(*moose_system._element_data[_tid]),
   _dof_data(moose_system._dof_data[_tid]),
   _aux_data(*moose_system._aux_data[_tid]),
   _nodal(_fe_type.family == LAGRANGE),
   _u(_nodal ? _aux_data._aux_var_vals_nodal[_var_num] : _aux_data._aux_var_vals_element[_var_num]),
   _u_old(_nodal ? _aux_data._aux_var_vals_old_nodal[_var_num] : _aux_data._aux_var_vals_old_element[_var_num]),
   _u_older(_nodal ? _aux_data._aux_var_vals_older_nodal[_var_num] : _aux_data._aux_var_vals_older_element[_var_num]),
   _current_node(moose_system._face_data[_tid]->_current_node)
{
  // Only ever one quadrature point
  _qp = 0;
  
  // If this variable isn't known yet... make it so
  _element_data._aux_var_nums[0].insert(_var_num);

  if(_nodal)
    _aux_data._nodal_var_nums.insert(_var_num);
  else
    _aux_data._element_var_nums.insert(_var_num);

  for(unsigned int i=0;i<_coupled_to.size();i++)
  {
    std::string coupled_var_name=_coupled_to[i];

    //Is it in the nonlinear system or the aux system?
    if(_moose_system.hasVariable(coupled_var_name))
    {
      unsigned int coupled_var_num = _moose_system._system->variable_number(coupled_var_name);
      _element_data._var_nums[0].insert(coupled_var_num);
    }
    //Look for it in the Aux system
    else if (_moose_system.hasAuxVariable(coupled_var_name))
    {
      unsigned int coupled_var_num = _moose_system._aux_system->variable_number(coupled_var_name);
      _element_data._aux_var_nums[0].insert(coupled_var_num);
    }
    else
      mooseError("Coupled variable '" + coupled_var_name + "' not found.");
  }
}

void
AuxKernel::computeAndStore()
{
  _aux_data._aux_soln->set(_dof_data._aux_var_dofs[_var_num], computeValue());
}

bool
AuxKernel::isNodal()
{
  return _nodal;
}

MooseArray<Real> &
AuxKernel::coupledValue(std::string name, int i)
{
  if(!isCoupled(name))
    mooseError("\nAuxKernel _" + _name + "_ was not provided with a variable coupled_as " + name + "\n\n");

  if(_nodal)
  {
    if(!isAux(name))
    {
      unsigned int temp = _coupled_vars[name][i]._num;
      temp ++;
      return _aux_data._var_vals_nodal[_coupled_vars[name][i]._num];
    }
    
    else
      return _aux_data._aux_var_vals_nodal[_coupled_aux_vars[name][i]._num];
  }
  else
  {
    if(!isAux(name))
      return _aux_data._var_vals_element[_coupled_vars[name][i]._num];
    else
      return _aux_data._aux_var_vals_element[_coupled_aux_vars[name][i]._num];
  }
}


MooseArray<Real> &
AuxKernel::coupledValueOld(std::string name, int i)
{
  if(!isCoupled(name))
    mooseError("\nAuxKernel _" + _name + "_ was not provided with a variable coupled_as " + name + "\n\n");

  if(_nodal)
  {
    if(!isAux(name))
      return _aux_data._var_vals_old_nodal[_coupled_vars[name][i]._num];
    else
      return _aux_data._aux_var_vals_old_nodal[_coupled_aux_vars[name][i]._num];
  }
  else
  {
    if(!isAux(name))
      return _aux_data._var_vals_old_element[_coupled_vars[name][i]._num];
    else
      return _aux_data._aux_var_vals_old_element[_coupled_aux_vars[name][i]._num];
  }
}


MooseArray<Real> &
AuxKernel::coupledValueOlder(std::string name, int i)
{
  if(!isCoupled(name))
    mooseError("\nKernel _" + _name + "_ was not provided with a variable coupled_as " + name + "\n\n");

  if(_nodal)
  {
    if(!isAux(name))
      return _aux_data._var_vals_older_nodal[_coupled_vars[name][i]._num];
    else
      return _aux_data._aux_var_vals_older_nodal[_coupled_aux_vars[name][i]._num];
  }
  else
  {
    if(!isAux(name))
      return _aux_data._var_vals_older_element[_coupled_vars[name][i]._num];
    else
      return _aux_data._aux_var_vals_older_element[_coupled_aux_vars[name][i]._num];
  }  
}


MooseArray<RealGradient> &
AuxKernel::coupledGradient(std::string name, int i)
{
  if(!isCoupled(name))
    mooseError("\nAuxKernel _" + _name + "_ was not provided with a variable coupled_as " + name + "\n\n");

  if(_nodal)
    mooseError("Gradient can not be recovered with nodal AuxKernel _" + _name + "_ with a variable coupled_as " + name + "\n\n");
  else
  {
    if(!isAux(name))
      return _aux_data._var_grads_element[_coupled_vars[name][i]._num];
    else
      return _aux_data._aux_var_grads_element[_coupled_aux_vars[name][i]._num];
  }
}

MooseArray<RealGradient> &
AuxKernel::coupledGradientOld(std::string name, int i)
{
  if(!isCoupled(name))
    mooseError("\nAuxKernel _" + _name + "_ was not provided with a variable coupled_as " + name + "\n\n");

  if(_nodal)
    mooseError("Old Gradient can not be recovered with nodal AuxKernel _" + _name + "_ with a variable coupled_as " + name + "\n\n");
  else
  {
    if(!isAux(name))
      return _aux_data._var_grads_old_element[_coupled_vars[name][i]._num];
    else
      return _aux_data._aux_var_grads_old_element[_coupled_aux_vars[name][i]._num];
  }
}

MooseArray<RealGradient> &
AuxKernel::coupledGradientOlder(std::string name, int i)
{
  if(!isCoupled(name))
    mooseError("\nAuxKernel _" + _name + "_ was not provided with a variable coupled_as " + name + "\n\n");

  if(_nodal)
    mooseError("Older Gradient can not be recovered with nodal AuxKernel _" + _name + "_ with a variable coupled_as " + name + "\n\n");    
  else
  {
    if(!isAux(name))
      return _aux_data._var_grads_older_element[_coupled_vars[name][i]._num];
    else
      return _aux_data._aux_var_grads_older_element[_coupled_aux_vars[name][i]._num];
  }
}
