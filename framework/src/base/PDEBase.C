/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "PDEBase.h"
#include "MooseSystem.h"

#include "quadrature_gauss.h"

template<>
InputParameters validParams<PDEBase>()
{
  InputParameters params = validParams<MooseObject>();
  params.addPrivateParam<bool>("_integrated", true);
  params.addPrivateParam<bool>("need_old_newton", false);

  // FIXME: should go into parent class
  params.addParam<Real>("start_time", -std::numeric_limits<Real>::max(), "The time that this kernel will be active after.");
  params.addParam<Real>("stop_time", std::numeric_limits<Real>::max(), "The time after which this kernel will no longer be active.");
  return params;
}

PDEBase::PDEBase(const std::string & name, MooseSystem &moose_system, InputParameters parameters, QuadraturePointData &data) :
  MooseObject(name, moose_system, parameters),
  PostprocessorInterface(moose_system._postprocessor_data[_tid]),
  FunctionInterface(moose_system._functions[_tid], parameters),
  _mesh(_use_displaced_mesh ? *_moose_system.getDisplacedMesh() : *_moose_system.getMesh()),
  _current_elem(_moose_system._dof_data[_tid]._current_elem),
  _var_name(parameters.get<std::string>("variable")),
  _is_aux(moose_system.hasAuxVariable(_var_name)),
  _var_num(_is_aux ? moose_system.getAuxVariableNumber(_var_name) : moose_system.getVariableNumber(_var_name)),
  _fe_type(_is_aux ? moose_system._aux_dof_map->variable_type(_var_num) : moose_system._dof_map->variable_type(_var_num)),
  _fe(data._fe[_fe_type]),
  _integrated(parameters.get<bool>("_integrated")),
  _dim(_moose_system._dim),
  _data(data),
  _qrule(data._qrule),
  _q_point(_use_displaced_mesh ? *data._q_point_displaced[_fe_type] : *data._q_point[_fe_type]),
  _JxW(_use_displaced_mesh ? *(data._JxW_displaced)[_fe_type] : *(data._JxW)[_fe_type]),
  _phi(*(data._phi)[_fe_type]),
  _grad_phi(*data._grad_phi[_fe_type]),
  _second_phi(*data._second_phi[_fe_type]),
  _real_zero(moose_system._real_zero[_tid]),
  _zero(moose_system._zero[_tid]),
  _grad_zero(moose_system._grad_zero[_tid]),
  _second_zero(moose_system._second_zero[_tid]),
  _t(moose_system._t),
  _dt(moose_system._dt),
  _dt_old(moose_system._dt_old),
  _is_transient(moose_system._is_transient),
  _is_eigenvalue(moose_system._is_eigenvalue),
  _t_step(moose_system._t_step),
  _bdf2_wei(moose_system._bdf2_wei),
  _t_scheme(moose_system._t_scheme),
  _start_time(parameters.get<Real>("start_time")),
  _stop_time(parameters.get<Real>("stop_time"))
{
  // Coupling
  for (std::set<std::string>::const_iterator iter = parameters.coupledVarsBegin();
       iter != parameters.coupledVarsEnd();
       ++iter)
  {
    std::string name = *iter;
    if (parameters.get<std::vector<std::string> >(*iter) != std::vector<std::string>())
    {
      std::vector<std::string> vars = parameters.get<std::vector<std::string> >(*iter);
      _all_coupled_var[name].resize(vars.size());
      for (unsigned int i = 0; i < vars.size(); i++) {
        std::string coupled_var_name = vars[i];
        _coupled_to.push_back(coupled_var_name);

        _all_coupled_var[name][i]._name = coupled_var_name;
        if (moose_system.hasVariable(coupled_var_name))
        {
          unsigned int var_num = moose_system.getVariableNumber(coupled_var_name);
          _all_coupled_var[name][i]._num = var_num;
          _coupled_vars[name].push_back(Variable(coupled_var_name, var_num));
        }
        else if (moose_system.hasAuxVariable(coupled_var_name))
        {
          unsigned int var_num = moose_system.getAuxVariableNumber(coupled_var_name);
          _all_coupled_var[name][i]._num = var_num;
          _coupled_aux_vars[name].push_back(Variable(coupled_var_name, var_num));
        }
        else
          mooseError("Coupled variable '" + coupled_var_name + "' was not found\n");
      }
    }
  }

}

PDEBase::~PDEBase()
{
}


// Integrable

unsigned int
PDEBase::variable()
{
  return _var_num;
}

std::string
PDEBase::varName() const
{
  return _var_name;
}

bool
PDEBase::isIntegrated()
{
  return _integrated;
}

void
PDEBase::subdomainSetup()
{
}

Real
PDEBase::computeIntegral()
{
//  Moose::perf_log.push("computeIntegral()",_name);

  Real sum = 0;

  for (_qp=0; _qp<_qrule->n_points(); _qp++)
      sum += _JxW[_qp]*computeQpIntegral();

//  Moose::perf_log.pop("computeIntegral()",_name);
  return sum;
}

Real
PDEBase::computeQpIntegral()
{
  return 0;
}

// Transient

Real
PDEBase::startTime()
{
  return _start_time;
}

Real
PDEBase::stopTime()
{
  return _stop_time;
}

// Coupling

const std::vector<std::string> &
PDEBase::coupledTo() const
{
  return _coupled_to;
}

bool
PDEBase::isCoupled(const std::string & varname, unsigned int i)
{
  std::map<std::string, std::vector<Variable> >::iterator it = _all_coupled_var.find(varname);
  if (it != _all_coupled_var.end())
    return (i < it->second.size());
  else
    return false;
}

bool
PDEBase::isAux(const std::string & varname, unsigned int i)
{
  std::map<std::string, std::vector<Variable> >::iterator it = _coupled_aux_vars.find(varname);
  if (it != _coupled_aux_vars.end())
    return (i < it->second.size());
  else
    return false;
}

unsigned int
PDEBase::coupled(const std::string & varname, unsigned int i)
{
  if(!isCoupled(varname, i))
    mooseError("\nObject " + name() + " was not provided with a coupled variable " + varname + "\n\n");

  if(!isAux(varname))
    return _coupled_vars[varname][i]._num;
  else
    return _moose_system.modifiedAuxVarNum(_coupled_aux_vars[varname][i]._num);
}

unsigned int
PDEBase::coupledComponents(const std::string & varname)
{
  return _all_coupled_var[varname].size();
}


VariableValue &
PDEBase::coupledValue(const std::string & varname, unsigned int i)
{
  if(!isCoupled(varname))
    mooseError("\nObject " + name() + " was not provided with a coupled variable " + varname + "\n\n");

  if(!isAux(varname))
    return _data._var_vals[_coupled_vars[varname][i]._num];
  else
    return _data._aux_var_vals[_coupled_aux_vars[varname][i]._num];
}

VariableGradient &
PDEBase::coupledGradient(const std::string & varname, unsigned int i)
{
  if(!isCoupled(varname))
    mooseError("\nObject " + name() + " was not provided with a coupled variable " + varname + "\n\n");

  if(!isAux(varname))
    return _data._var_grads[_coupled_vars[varname][i]._num];
  else
    return _data._aux_var_grads[_coupled_aux_vars[varname][i]._num];
}

VariableSecond &
PDEBase::coupledSecond(const std::string & varname, unsigned int i)
{
  if(!isCoupled(varname))
    mooseError("\nObject " + name() + " was not provided with a coupled variable " + varname + "\n\n");

  //Aux vars can't have second derivatives!
  return _data._var_seconds[_coupled_vars[varname][i]._num];
}

VariableValue &
PDEBase::coupledValueOld(const std::string & varname, unsigned int i)
{
  if(!isCoupled(varname))
    mooseError("\nObject " + name() + " was not provided with a coupled variable " + varname + "\n\n");

  if(!isAux(varname))
    return _data._var_vals_old[_coupled_vars[varname][i]._num];
  else
    return _data._aux_var_vals_old[_coupled_aux_vars[varname][i]._num];
}

VariableValue &
PDEBase::coupledValueOlder(const std::string & varname, unsigned int i)
{
  if(!isCoupled(varname))
    mooseError("\nObject " + name() + " was not provided with a coupled variable " + varname + "\n\n");

  if(!isAux(varname))
    return _data._var_vals_older[_coupled_vars[varname][i]._num];
  else
    return _data._aux_var_vals_older[_coupled_aux_vars[varname][i]._num];
}

VariableGradient &
PDEBase::coupledGradientOld(const std::string & varname, unsigned int i)
{
  if(!isCoupled(varname))
    mooseError("\nObject " + name() + " was not provided with a coupled variable " + varname + "\n\n");

  return _data._var_grads_old[_coupled_vars[varname][i]._num];
}
