#include "AuxKernel.h"

//local includes
#include "Moose.h"
#include "MooseSystem.h"
#include "ElementData.h"

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

AuxKernel::AuxKernel(std::string name, MooseSystem & moose_system, InputParameters parameters) :
 MaterialPropertyInterface(moose_system._material_data),
   _name(name),
   _moose_system(moose_system),
   _element_data(moose_system._element_data),
   _aux_data(moose_system._aux_data),
   _tid(Moose::current_thread_id),
   _parameters(parameters),
   _var_name(parameters.get<std::string>("variable")),
   _is_aux(_moose_system._aux_system->has_variable(_var_name)),
   _var_num(_is_aux ? _moose_system._aux_system->variable_number(_var_name) : _moose_system._system->variable_number(_var_name)),
   _coupled_to(parameters.have_parameter<std::vector<std::string> >("coupled_to") ? parameters.get<std::vector<std::string> >("coupled_to") : std::vector<std::string>(0)),
   _coupled_as(parameters.have_parameter<std::vector<std::string> >("coupled_as") ? parameters.get<std::vector<std::string> >("coupled_as") : std::vector<std::string>(0)),
   _fe_type(_is_aux ? _moose_system._aux_dof_map->variable_type(_var_num) : _moose_system._dof_map->variable_type(_var_num)),
   _material(_moose_system._element_data._material[_tid]),
   _qrule(_element_data._qrule[_tid]),
   _t(_moose_system._t),
   _dt(_moose_system._dt),
   _dt_old(_moose_system._dt_old),
   _t_step(_moose_system._t_step),
   _bdf2_wei(_moose_system._bdf2_wei),
   _nodal(_fe_type.family == LAGRANGE),
   _u_aux(_nodal ? moose_system._aux_data._aux_var_vals_nodal[_tid][_var_num] : moose_system._aux_data._aux_var_vals_element[_tid][_var_num]),
   _u_old_aux(_nodal ? moose_system._aux_data._aux_var_vals_old_nodal[_tid][_var_num] : moose_system._aux_data._aux_var_vals_old_element[_tid][_var_num]),
   _u_older_aux(_nodal ? moose_system._aux_data._aux_var_vals_older_nodal[_tid][_var_num] : moose_system._aux_data._aux_var_vals_older_element[_tid][_var_num]),
   _current_node(moose_system._face_data._current_node[_tid])
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
  if(std::find(_element_data._aux_var_nums[0].begin(),_element_data._aux_var_nums[0].end(),_var_num) == _element_data._aux_var_nums[0].end())
    _element_data._aux_var_nums[0].push_back(_var_num);

  if(_nodal)
    _moose_system._aux_data._nodal_var_nums.push_back(_var_num);
  else
    _moose_system._aux_data._element_var_nums.push_back(_var_num);

  // FIXME: this for statement will go into a common ancestor (it is copied from Kernel.C)
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

void
AuxKernel::computeAndStore(THREAD_ID tid)
{
  _moose_system._aux_data._aux_soln->set(_moose_system._aux_var_dofs[tid][_var_num], computeValue());
}

bool
AuxKernel::isNodal()
{
  return _nodal;
}

void
AuxKernel::subdomainSetup()
{
}

unsigned int
AuxKernel::variable()
{
  return _var_num;
}

std::string
AuxKernel::name() const
{
  return _name;
}

std::string
AuxKernel::varName() const
{
  return _var_name;
}

const std::vector<std::string> &
AuxKernel::coupledTo() const
{
  return _coupled_to;
}

Real
AuxKernel::computeQpResidual()
{
  return 0;
}

bool
AuxKernel::isAux(std::string name)
{
  return _aux_coupled_as_to_var_num.find(name) != _aux_coupled_as_to_var_num.end();
}

bool
AuxKernel::isCoupled(std::string name)
{
  bool found = std::find(_coupled_as.begin(),_coupled_as.end(),name) != _coupled_as.end();

  //See if it's an Aux variable
  if(!found)
    found = isAux(name);

  return found;
}

unsigned int
AuxKernel::coupled(std::string name)
{
  if(!isCoupled(name))
    mooseError("\nAuxKernel _" + _name + "_ was not provided with a variable coupled_as " + name + "\n\n");

  if(!isAux(name))
    return _coupled_as_to_var_num[name];
  else
    return Kernel::modifiedAuxVarNum(_aux_coupled_as_to_var_num[name]);
}

MooseArray<Real> &
AuxKernel::coupledVal(std::string name)
{
  if(!isCoupled(name))
    mooseError("\nAuxKernel _" + _name + "_ was not provided with a variable coupled_as " + name + "\n\n");

  if(!isAux(name))
    return _element_data._var_vals[_tid][_coupled_as_to_var_num[name]];
  else
    return _element_data._aux_var_vals[_tid][_aux_coupled_as_to_var_num[name]];
}

MooseArray<RealGradient> &
AuxKernel::coupledGrad(std::string name)
{
  if(!isCoupled(name))
    mooseError("\nAuxKernel _" + _name + "_ was not provided with a variable coupled_as " + name + "\n\n");

  if(!isAux(name))
    return _element_data._var_grads[_tid][_coupled_as_to_var_num[name]];
  else
    return _element_data._aux_var_grads[_tid][_aux_coupled_as_to_var_num[name]];
}

MooseArray<RealTensor> &
AuxKernel::coupledSecond(std::string name)
{
  if(!isCoupled(name))
    mooseError("\nAuxKernel _" + _name + "_ was not provided with a variable coupled_as " + name + "\n\n");

  //Aux vars can't have second derivatives!
  return _element_data._var_seconds[_tid][_coupled_as_to_var_num[name]];
}

MooseArray<Real> &
AuxKernel::coupledValOld(std::string name)
{
  if(!isCoupled(name))
    mooseError("\nAuxKernel _" + _name + "_ was not provided with a variable coupled_as " + name + "\n\n");

  if(!isAux(name))
    return _element_data._var_vals_old[_tid][_coupled_as_to_var_num[name]];
  else
    return _element_data._aux_var_vals_old[_tid][_aux_coupled_as_to_var_num[name]];
}

MooseArray<Real> &
AuxKernel::coupledValOlder(std::string name)
{
  if(!isCoupled(name))
    mooseError("\nAuxKernel _" + _name + "_ was not provided with a variable coupled_as " + name + "\n\n");

  if(!isAux(name))
    return _element_data._var_vals_older[_tid][_coupled_as_to_var_num[name]];
  else
    return _element_data._aux_var_vals_older[_tid][_aux_coupled_as_to_var_num[name]];
}

MooseArray<RealGradient> &
AuxKernel::coupledGradValOld(std::string name)
{
  if(!isCoupled(name))
    mooseError("\nAuxKernel _" + _name + "_ was not provided with a variable coupled_as " + name + "\n\n");

  return _element_data._var_grads_old[_tid][_coupled_as_to_var_num[name]];
}

Real &
AuxKernel::coupledValAux(std::string name)
{
  if(!isCoupled(name))
    mooseError("\nAuxKernel _" + _name + "_ was not provided with a variable coupled_as " + name + "\n\n");

  if(_nodal)
  {
    if(!isAux(name))
      return _moose_system._aux_data._var_vals_nodal[_tid][_coupled_as_to_var_num[name]];
    else
      return _moose_system._aux_data._aux_var_vals_nodal[_tid][_aux_coupled_as_to_var_num[name]];
  }
  else
  {
    if(!isAux(name))
      return _moose_system._aux_data._var_vals_element[_tid][_coupled_as_to_var_num[name]];
    else
      return _moose_system._aux_data._aux_var_vals_element[_tid][_aux_coupled_as_to_var_num[name]];
  }
}


Real &
AuxKernel::coupledValOldAux(std::string name)
{
  if(!isCoupled(name))
    mooseError("\nAuxKernel _" + _name + "_ was not provided with a variable coupled_as " + name + "\n\n");

  if(_nodal)
  {
    if(!isAux(name))
      return _moose_system._aux_data._var_vals_old_nodal[_tid][_coupled_as_to_var_num[name]];
    else
      return _moose_system._aux_data._aux_var_vals_old_nodal[_tid][_aux_coupled_as_to_var_num[name]];
  }
  else
  {
    if(!isAux(name))
      return _moose_system._aux_data._var_vals_old_element[_tid][_coupled_as_to_var_num[name]];
    else
      return _moose_system._aux_data._aux_var_vals_old_element[_tid][_aux_coupled_as_to_var_num[name]];
  }
}


Real &
AuxKernel::coupledValOlderAux(std::string name)
{
  if(!isCoupled(name))
    mooseError("\nKernel _" + _name + "_ was not provided with a variable coupled_as " + name + "\n\n");

  if(_nodal)
  {
    if(!isAux(name))
      return _moose_system._aux_data._var_vals_older_nodal[_tid][_coupled_as_to_var_num[name]];
    else
      return _moose_system._aux_data._aux_var_vals_older_nodal[_tid][_aux_coupled_as_to_var_num[name]];
  }
  else
  {
    if(!isAux(name))
      return _moose_system._aux_data._var_vals_older_element[_tid][_coupled_as_to_var_num[name]];
    else
      return _moose_system._aux_data._aux_var_vals_older_element[_tid][_aux_coupled_as_to_var_num[name]];
  }  
}


RealGradient &
AuxKernel::coupledGradAux(std::string name)
{
  if(!isCoupled(name))
    mooseError("\nAuxKernel _" + _name + "_ was not provided with a variable coupled_as " + name + "\n\n");

  if(_nodal)
    mooseError("Gradient can not be recovered with nodal AuxKernel _" + _name + "_ with a variable coupled_as " + name + "\n\n");
  else
  {
    if(!isAux(name))
      return _moose_system._aux_data._var_grads_element[_tid][_coupled_as_to_var_num[name]];
    else
      return _moose_system._aux_data._aux_var_grads_element[_tid][_aux_coupled_as_to_var_num[name]];
  }
}

RealGradient &
AuxKernel::coupledGradOldAux(std::string name)
{
  if(!isCoupled(name))
    mooseError("\nAuxKernel _" + _name + "_ was not provided with a variable coupled_as " + name + "\n\n");

  if(_nodal)
    mooseError("Old Gradient can not be recovered with nodal AuxKernel _" + _name + "_ with a variable coupled_as " + name + "\n\n");
  else
  {
    if(!isAux(name))
      return _moose_system._aux_data._var_grads_old_element[_tid][_coupled_as_to_var_num[name]];
    else
      return _moose_system._aux_data._aux_var_grads_old_element[_tid][_aux_coupled_as_to_var_num[name]];
  }
}

RealGradient &
AuxKernel::coupledGradOlderAux(std::string name)
{
  if(!isCoupled(name))
    mooseError("\nAuxKernel _" + _name + "_ was not provided with a variable coupled_as " + name + "\n\n");

  if(_nodal)
    mooseError("Older Gradient can not be recovered with nodal AuxKernel _" + _name + "_ with a variable coupled_as " + name + "\n\n");    
  else
  {
    if(!isAux(name))
      return _moose_system._aux_data._var_grads_older_element[_tid][_coupled_as_to_var_num[name]];
    else
      return _moose_system._aux_data._aux_var_grads_older_element[_tid][_aux_coupled_as_to_var_num[name]];
  }
}
