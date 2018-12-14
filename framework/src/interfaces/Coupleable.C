//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Coupleable.h"
#include "Problem.h"
#include "SubProblem.h"
#include "FEProblem.h"
#include "MooseVariableScalar.h"
#include "MooseVariableFE.h"
#include "InputParameters.h"
#include "MooseObject.h"

Coupleable::Coupleable(const MooseObject * moose_object, bool nodal)
  : _c_parameters(moose_object->parameters()),
    _c_name(_c_parameters.get<std::string>("_object_name")),
    _c_fe_problem(*_c_parameters.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _c_nodal(nodal),
    _c_is_implicit(_c_parameters.have_parameter<bool>("implicit")
                       ? _c_parameters.get<bool>("implicit")
                       : true),
    _c_tid(_c_parameters.get<THREAD_ID>("_tid")),
    _zero(_c_fe_problem._zero[_c_tid]),
    _ad_zero(_c_fe_problem._ad_zero[_c_tid]),
    _grad_zero(_c_fe_problem._grad_zero[_c_tid]),
    _ad_grad_zero(_c_fe_problem._ad_grad_zero[_c_tid]),
    _second_zero(_c_fe_problem._second_zero[_c_tid]),
    _second_phi_zero(_c_fe_problem._second_phi_zero[_c_tid]),
    _vector_zero(_c_fe_problem._vector_zero[_c_tid]),
    _vector_curl_zero(_c_fe_problem._vector_curl_zero[_c_tid]),
    _coupleable_neighbor(_c_parameters.have_parameter<bool>("_neighbor")
                             ? _c_parameters.get<bool>("_neighbor")
                             : false),
    _coupleable_max_qps(_c_fe_problem.getMaxQps())
{
  SubProblem & problem = *_c_parameters.getCheckedPointerParam<SubProblem *>("_subproblem");

  unsigned int optional_var_index_counter = 0;
  // Coupling
  for (std::set<std::string>::const_iterator iter = _c_parameters.coupledVarsBegin();
       iter != _c_parameters.coupledVarsEnd();
       ++iter)
  {
    std::string name = *iter;
    if (_c_parameters.getVecMooseType(name) != std::vector<std::string>())
    {
      std::vector<std::string> vars = _c_parameters.getVecMooseType(*iter);
      for (const auto & coupled_var_name : vars)
      {
        if (problem.hasVariable(coupled_var_name))
        {
          MooseVariableFEBase * moose_var =
              &problem.getVariable(_c_tid,
                                   coupled_var_name,
                                   Moose::VarKindType::VAR_ANY,
                                   Moose::VarFieldType::VAR_FIELD_ANY);
          _coupled_vars[name].push_back(moose_var);
          _coupled_moose_vars.push_back(moose_var);
          if (auto * tmp_var = dynamic_cast<MooseVariable *>(moose_var))
            _coupled_standard_moose_vars.push_back(tmp_var);
          else if (auto * tmp_var = dynamic_cast<VectorMooseVariable *>(moose_var))
            _coupled_vector_moose_vars.push_back(tmp_var);
          else
            mooseError("Unknown variable type!");
        }
        else if (problem.hasScalarVariable(coupled_var_name))
        {
          MooseVariableScalar * moose_scalar_var =
              &problem.getScalarVariable(_c_tid, coupled_var_name);
          _c_coupled_scalar_vars[name].push_back(moose_scalar_var);
        }
        else
          mooseError(_c_name, ": Coupled variable '", coupled_var_name, "' was not found");
      }
    }
    else // This means it was optional coupling.  Let's assign a unique id to this variable
    {
      _optional_var_index[name].assign(_c_parameters.numberDefaultCoupledValues(name), 0);
      for (unsigned int j = 0; j < _optional_var_index[name].size(); ++j)
        _optional_var_index[name][j] =
            std::numeric_limits<unsigned int>::max() - optional_var_index_counter;
      ++optional_var_index_counter;
    }
  }

  _default_value_zero.resize(_coupleable_max_qps, 0);
  _default_gradient.resize(_coupleable_max_qps);
  _default_second.resize(_coupleable_max_qps);
  _default_vector_value_zero.resize(_coupleable_max_qps);
  _default_vector_gradient.resize(_coupleable_max_qps);
  _default_vector_curl.resize(_coupleable_max_qps);
  _ad_default_gradient.resize(_coupleable_max_qps);
  _ad_default_second.resize(_coupleable_max_qps);
}

Coupleable::~Coupleable()
{
  for (auto & it : _default_value)
    for (auto itt : it.second)
    {
      itt->release();
      delete itt;
    }
  for (auto & it : _ad_default_value)
  {
    it.second->release();
    delete it.second;
  }

  _default_value_zero.release();
  _default_gradient.release();
  _default_second.release();
  _default_vector_value_zero.release();
  _default_vector_gradient.release();
  _default_vector_curl.release();
  _ad_default_gradient.release();
  _ad_default_second.release();
}

void
Coupleable::coupledCallback(const std::string & /*var_name*/, bool /*is_old*/)
{
}

bool
Coupleable::isCoupled(const std::string & var_name, unsigned int i)
{
  std::map<std::string, std::vector<MooseVariableFEBase *>>::iterator it =
      _coupled_vars.find(var_name);
  if (it != _coupled_vars.end())
    return (i < it->second.size());
  else
  {
    // Make sure the user originally requested this value in the InputParameter syntax
    if (!_c_parameters.hasCoupledValue(var_name))
      mooseError(_c_name,
                 ": The coupled variable \"",
                 var_name,
                 "\" was never added to this objects's "
                 "InputParameters, please double-check your "
                 "spelling");

    return false;
  }
}

unsigned int
Coupleable::coupledComponents(const std::string & var_name)
{
  if (isCoupled(var_name))
    return _coupled_vars[var_name].size();
  else
  {
    if (_c_parameters.hasDefaultCoupledValue(var_name))
      return _c_parameters.numberDefaultCoupledValues(var_name);
    else
      return 0;
  }
}

void
Coupleable::checkVar(const std::string & var_name)
{
  auto it = _c_coupled_scalar_vars.find(var_name);
  if (it != _c_coupled_scalar_vars.end())
  {
    std::string cvars;
    for (auto jt : it->second)
      cvars += " " + jt->name();
    mooseError(_c_name,
               ": Trying to couple a scalar variable where field variable is expected, '",
               var_name,
               " =",
               cvars,
               "'");
  }
  // NOTE: non-existent variables are handled in the constructor
}

MooseVariableFEBase *
Coupleable::getFEVar(const std::string & var_name, unsigned int comp)
{
  if (comp < _coupled_vars[var_name].size())
  {
    // Error check - don't couple elemental to nodal
    if (!(_coupled_vars[var_name][comp])->isNodal() && _c_nodal)
      mooseError(_c_name, ": You cannot couple an elemental variable to a nodal variable");
    return _coupled_vars[var_name][comp];
  }
  else
    mooseError(_c_name, ": Trying to get a non-existent component of variable '", var_name, "'");
}

MooseVariable *
Coupleable::getVar(const std::string & var_name, unsigned int comp)
{
  if (comp < _coupled_vars[var_name].size())
  {
    // Error check - don't couple elemental to nodal
    if (!(_coupled_vars[var_name][comp])->isNodal() && _c_nodal)
      mooseError(_c_name, ": You cannot couple an elemental variable to a nodal variable");
    if (auto * coupled_var = dynamic_cast<MooseVariable *>(_coupled_vars[var_name][comp]))
      return coupled_var;
    else
      mooseError("Variable of wrong type");
  }
  else
    mooseError(_c_name, ": Trying to get a non-existent component of variable '", var_name, "'");
}

VectorMooseVariable *
Coupleable::getVectorVar(const std::string & var_name, unsigned int comp)
{
  if (comp < _coupled_vars[var_name].size())
  {
    // Error check - don't couple elemental to nodal
    if (!(_coupled_vars[var_name][comp])->isNodal() && _c_nodal)
      mooseError(_c_name, ": You cannot couple an elemental variable to a nodal variable");
    if (auto * coupled_var = dynamic_cast<VectorMooseVariable *>(_coupled_vars[var_name][comp]))
      return coupled_var;
    else
      mooseError("Variable of wrong type");
  }
  else
    mooseError(_c_name, ": Trying to get a non-existent component of variable '", var_name, "'");
}

unsigned int
Coupleable::coupled(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);

  if (!isCoupled(var_name))
  {
    // make sure we don't try to access default var ids that were not provided
    if (comp + 1 > _optional_var_index[var_name].size())
      mooseError(_c_name,
                 ": Requested component ",
                 comp,
                 " of coupled value ",
                 var_name,
                 " is out of range.");
    return _optional_var_index[var_name][comp];
  }

  MooseVariableFEBase * var = getFEVar(var_name, comp);
  switch (var->kind())
  {
    case Moose::VAR_NONLINEAR:
      return var->number();
    case Moose::VAR_AUXILIARY:
      return std::numeric_limits<unsigned int>::max() - var->number();
    default:
      mooseError(_c_name, ": Unknown variable kind. Corrupted binary?");
  }
}

VariableValue *
Coupleable::getDefaultValue(const std::string & var_name, unsigned int comp)
{
  // make sure we don't access values that were not provided
  if (comp + 1 > _c_parameters.numberDefaultCoupledValues(var_name))
    mooseError(_c_name,
               ": Requested component ",
               comp,
               " of coupled value ",
               var_name,
               " is out of range.");

  std::map<std::string, std::vector<VariableValue *>>::iterator default_value_it =
      _default_value.find(var_name);
  if (default_value_it == _default_value.end())
  {
    _default_value[var_name] = {
        new VariableValue(_coupleable_max_qps, _c_parameters.defaultCoupledValue(var_name, 0))};
    for (unsigned int j = 1; j < _c_parameters.numberDefaultCoupledValues(var_name); ++j)
      _default_value[var_name].push_back(
          new VariableValue(_coupleable_max_qps, _c_parameters.defaultCoupledValue(var_name, j)));
    default_value_it = _default_value.find(var_name);
  }

  return default_value_it->second[comp];
}

VectorVariableValue *
Coupleable::getVectorDefaultValue(const std::string & var_name)
{
  std::map<std::string, VectorVariableValue *>::iterator default_value_it =
      _default_vector_value.find(var_name);
  if (default_value_it == _default_vector_value.end())
  {
    VectorVariableValue * value = new VectorVariableValue(_coupleable_max_qps, 0);
    default_value_it = _default_vector_value.insert(std::make_pair(var_name, value)).first;
  }

  return default_value_it->second;
}

const VariableValue &
Coupleable::coupledValue(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name))
    return *getDefaultValue(var_name, comp);

  coupledCallback(var_name, false);
  MooseVariable * var = getVar(var_name, comp);
  if (var == NULL)
    mooseError("Call coupledVectorValue for coupled vector variables");

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      return (_c_is_implicit) ? var->dofValues() : var->dofValuesOld();
    else
      return (_c_is_implicit) ? var->sln() : var->slnOld();
  }
  else
  {
    if (_c_nodal)
      return (_c_is_implicit) ? var->dofValuesNeighbor() : var->dofValuesOldNeighbor();
    else
      return (_c_is_implicit) ? var->slnNeighbor() : var->slnOldNeighbor();
  }
}

const VariableValue &
Coupleable::coupledVectorTagValue(const std::string & var_name, TagID tag, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name))
    mooseError(var_name, ": invalid variable name for coupledVectorTagValue");

  addFEVariableCoupleableVectorTag(tag);

  coupledCallback(var_name, false);
  MooseVariable * var = getVar(var_name, comp);
  if (var == NULL)
    mooseError("Call coupledVectorValue for coupled vector variables");

  if (_c_nodal)
    return var->nodalVectorTagValue(tag);
  else
    return var->vectorTagValue(tag);
}

const VariableValue &
Coupleable::coupledMatrixTagValue(const std::string & var_name, TagID tag, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name))
    mooseError(var_name, ": invalid variable name for coupledMatrixTagValue");

  addFEVariableCoupleableMatrixTag(tag);

  coupledCallback(var_name, false);
  MooseVariable * var = getVar(var_name, comp);
  if (var == NULL)
    mooseError("Call coupledVectorValue for coupled vector variables");

  if (_c_nodal)
    return var->nodalMatrixTagValue(tag);
  else
    return var->matrixTagValue(tag);
}

const VectorVariableValue &
Coupleable::coupledVectorValue(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name))
    return *getVectorDefaultValue(var_name);

  coupledCallback(var_name, false);
  VectorMooseVariable * var = getVectorVar(var_name, comp);
  if (var == NULL)
    mooseError("Call coupledValue for coupled regular variables");

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      mooseError("Vector variables are not required to be continuous and so should not be used "
                 "with nodal compute objects");
    else
      return (_c_is_implicit) ? var->sln() : var->slnOld();
  }
  else
  {
    if (_c_nodal)
      mooseError("Vector variables are not required to be continuous and so should not be used "
                 "with nodal compute objects");
    else
      return (_c_is_implicit) ? var->slnNeighbor() : var->slnOldNeighbor();
  }
}

VariableValue &
Coupleable::writableCoupledValue(const std::string & var_name, unsigned int comp)
{
  return const_cast<VariableValue &>(coupledValue(var_name, comp));
}

const VariableValue &
Coupleable::coupledValueOld(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name))
    return *getDefaultValue(var_name, comp);

  validateExecutionerType(var_name, "coupledValueOld");
  coupledCallback(var_name, true);
  MooseVariable * var = getVar(var_name, comp);
  if (var == NULL)
    mooseError("Call coupledVectorValueOld for coupled vector variables");

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      return (_c_is_implicit) ? var->dofValuesOld() : var->dofValuesOlder();
    else
      return (_c_is_implicit) ? var->slnOld() : var->slnOlder();
  }
  else
  {
    if (_c_nodal)
      return (_c_is_implicit) ? var->dofValuesOldNeighbor() : var->dofValuesOlderNeighbor();
    else
      return (_c_is_implicit) ? var->slnOldNeighbor() : var->slnOlderNeighbor();
  }
}

const VariableValue &
Coupleable::coupledValueOlder(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name))
    return *getDefaultValue(var_name, comp);

  validateExecutionerType(var_name, "coupledValueOlder");
  coupledCallback(var_name, true);
  MooseVariable * var = getVar(var_name, comp);
  if (var == NULL)
    mooseError("Call coupledVectorValueOlder for coupled vector variables");

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
    {
      if (_c_is_implicit)
        return var->dofValuesOlder();
      else
        mooseError(_c_name, ": Older values not available for explicit schemes");
    }
    else
    {
      if (_c_is_implicit)
        return var->slnOlder();
      else
        mooseError(_c_name, ": Older values not available for explicit schemes");
    }
  }
  else
  {
    if (_c_nodal)
    {
      if (_c_is_implicit)
        return var->dofValuesOlderNeighbor();
      else
        mooseError(_c_name, ": Older values not available for explicit schemes");
    }
    else
    {
      if (_c_is_implicit)
        return var->slnOlderNeighbor();
      else
        mooseError(_c_name, ": Older values not available for explicit schemes");
    }
  }
}

const VariableValue &
Coupleable::coupledValuePreviousNL(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name))
    return *getDefaultValue(var_name, comp);

  _c_fe_problem.needsPreviousNewtonIteration(true);
  coupledCallback(var_name, true);
  MooseVariable * var = getVar(var_name, comp);
  if (var == NULL)
    mooseError("Call corresponding vector variable method");

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      return var->dofValuesPreviousNL();
    else
      return var->slnPreviousNL();
  }
  else
  {
    if (_c_nodal)
      return var->dofValuesPreviousNLNeighbor();
    else
      return var->slnPreviousNLNeighbor();
  }
}

const VectorVariableValue &
Coupleable::coupledVectorValueOld(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name))
    return *getVectorDefaultValue(var_name);

  validateExecutionerType(var_name, "coupledVectorValueOld");
  coupledCallback(var_name, true);
  VectorMooseVariable * var = getVectorVar(var_name, comp);
  if (var == NULL)
    mooseError("Call coupledValueOld for coupled scalar field variables");

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      mooseError("Vector variables are not required to be continuous and so should not be used "
                 "with nodal compute objects");
    else
      return (_c_is_implicit) ? var->slnOld() : var->slnOlder();
  }
  else
  {
    if (_c_nodal)
      mooseError("Vector variables are not required to be continuous and so should not be used "
                 "with nodal compute objects");
    else
      return (_c_is_implicit) ? var->slnOldNeighbor() : var->slnOlderNeighbor();
  }
}

const VectorVariableValue &
Coupleable::coupledVectorValueOlder(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name))
    return *getVectorDefaultValue(var_name);

  validateExecutionerType(var_name, "coupledVectorValueOlder");
  coupledCallback(var_name, true);
  VectorMooseVariable * var = getVectorVar(var_name, comp);
  if (var == NULL)
    mooseError("Call coupledValueOlder for coupled scalar field variables");

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      mooseError("Vector variables are not required to be continuous and so should not be used "
                 "with nodal compute objects");
    else
    {
      if (_c_is_implicit)
        return var->slnOlder();
      else
        mooseError("Older values not available for explicit schemes");
    }
  }
  else
  {
    if (_c_nodal)
      mooseError("Vector variables are not required to be continuous and so should not be used "
                 "with nodal compute objects");
    else
    {
      if (_c_is_implicit)
        return var->slnOlderNeighbor();
      else
        mooseError("Older values not available for explicit schemes");
    }
  }
}

const VariableValue &
Coupleable::coupledDot(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_value_zero;

  validateExecutionerType(var_name, "coupledDot");
  MooseVariable * var = getVar(var_name, comp);
  if (var == NULL)
    mooseError("Call corresponding vector variable method");

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      return var->dofValuesDot();
    else
      return var->uDot();
  }
  else
  {
    if (_c_nodal)
      return var->dofValuesDotNeighbor();
    else
      return var->uDotNeighbor();
  }
}

const VariableValue &
Coupleable::coupledDotDot(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_value_zero;

  validateExecutionerType(var_name, "coupledDotDot");
  MooseVariable * var = getVar(var_name, comp);
  if (var == NULL)
    mooseError("Call corresponding vector variable method");

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      return var->dofValuesDotDot();
    else
      return var->uDotDot();
  }
  else
  {
    if (_c_nodal)
      return var->dofValuesDotDotNeighbor();
    else
      return var->uDotDotNeighbor();
  }
}

const VariableValue &
Coupleable::coupledDotOld(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_value_zero;

  validateExecutionerType(var_name, "coupledDotOld");
  MooseVariable * var = getVar(var_name, comp);
  if (var == NULL)
    mooseError("Call corresponding vector variable method");

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      return var->dofValuesDotOld();
    else
      return var->uDotOld();
  }
  else
  {
    if (_c_nodal)
      return var->dofValuesDotOldNeighbor();
    else
      return var->uDotOldNeighbor();
  }
}

const VariableValue &
Coupleable::coupledDotDotOld(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_value_zero;

  validateExecutionerType(var_name, "coupledDotDotOld");
  MooseVariable * var = getVar(var_name, comp);
  if (var == NULL)
    mooseError("Call corresponding vector variable method");

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      return var->dofValuesDotDotOld();
    else
      return var->uDotDotOld();
  }
  else
  {
    if (_c_nodal)
      return var->dofValuesDotDotOldNeighbor();
    else
      return var->uDotDotOldNeighbor();
  }
}

const VectorVariableValue &
Coupleable::coupledVectorDot(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_vector_value_zero;

  validateExecutionerType(var_name, "coupledVectorDot");
  VectorMooseVariable * var = getVectorVar(var_name, comp);
  if (var == NULL)
    mooseError("Call corresponding standard variable method");

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      mooseError("Vector variables are not required to be continuous and so should not be used "
                 "with nodal compute objects");
    else
      return var->uDot();
  }
  else
  {
    if (_c_nodal)
      mooseError("Vector variables are not required to be continuous and so should not be used "
                 "with nodal compute objects");
    else
      return var->uDotNeighbor();
  }
}

const VectorVariableValue &
Coupleable::coupledVectorDotDot(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_vector_value_zero;

  validateExecutionerType(var_name, "coupledVectorDotDot");
  VectorMooseVariable * var = getVectorVar(var_name, comp);
  if (var == NULL)
    mooseError("Call corresponding standard variable method");

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      mooseError("Vector variables are not required to be continuous and so should not be used "
                 "with nodal compute objects");
    else
      return var->uDotDot();
  }
  else
  {
    if (_c_nodal)
      mooseError("Vector variables are not required to be continuous and so should not be used "
                 "with nodal compute objects");
    else
      return var->uDotDotNeighbor();
  }
}

const VectorVariableValue &
Coupleable::coupledVectorDotOld(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_vector_value_zero;

  validateExecutionerType(var_name, "coupledVectorDotOld");
  VectorMooseVariable * var = getVectorVar(var_name, comp);
  if (var == NULL)
    mooseError("Call corresponding standard variable method");

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      mooseError("Vector variables are not required to be continuous and so should not be used "
                 "with nodal compute objects");
    else
      return var->uDotOld();
  }
  else
  {
    if (_c_nodal)
      mooseError("Vector variables are not required to be continuous and so should not be used "
                 "with nodal compute objects");
    else
      return var->uDotOldNeighbor();
  }
}

const VectorVariableValue &
Coupleable::coupledVectorDotDotOld(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_vector_value_zero;

  validateExecutionerType(var_name, "coupledVectorDotDotOld");
  VectorMooseVariable * var = getVectorVar(var_name, comp);
  if (var == NULL)
    mooseError("Call corresponding standard variable method");

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      mooseError("Vector variables are not required to be continuous and so should not be used "
                 "with nodal compute objects");
    else
      return var->uDotDotOld();
  }
  else
  {
    if (_c_nodal)
      mooseError("Vector variables are not required to be continuous and so should not be used "
                 "with nodal compute objects");
    else
      return var->uDotDotOldNeighbor();
  }
}

const VariableValue &
Coupleable::coupledDotDu(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_value_zero;

  validateExecutionerType(var_name, "coupledDotDu");
  MooseVariable * var = getVar(var_name, comp);
  if (var == NULL)
    mooseError("Call corresponding vector variable method");

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      return var->dofValuesDuDotDu();
    else
      return var->duDotDu();
  }
  else
  {
    if (_c_nodal)
      return var->dofValuesDuDotDuNeighbor();
    else
      return var->duDotDuNeighbor();
  }
}

const VariableValue &
Coupleable::coupledDotDotDu(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_value_zero;

  validateExecutionerType(var_name, "coupledDotDotDu");
  MooseVariable * var = getVar(var_name, comp);
  if (var == NULL)
    mooseError("Call corresponding vector variable method");

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      return var->dofValuesDuDotDotDu();
    else
      return var->duDotDotDu();
  }
  else
  {
    if (_c_nodal)
      return var->dofValuesDuDotDotDuNeighbor();
    else
      return var->duDotDotDu();
  }
}

const VariableGradient &
Coupleable::coupledGradient(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_gradient;

  coupledCallback(var_name, false);
  if (_c_nodal)
    mooseError(_c_name, ": Nodal variables do not have gradients");

  MooseVariable * var = getVar(var_name, comp);
  if (var == NULL)
    mooseError("Call corresponding vector variable method");

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->gradSln() : var->gradSlnOld();
  else
    return (_c_is_implicit) ? var->gradSlnNeighbor() : var->gradSlnOldNeighbor();
}

const VariableGradient &
Coupleable::coupledGradientOld(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_gradient;

  coupledCallback(var_name, true);
  if (_c_nodal)
    mooseError(_c_name, ": Nodal compute objects do not support gradients");

  validateExecutionerType(var_name, "coupledGradientOld");
  MooseVariable * var = getVar(var_name, comp);
  if (var == NULL)
    mooseError("Call corresponding vector variable method");

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->gradSlnOld() : var->gradSlnOlder();
  else
    return (_c_is_implicit) ? var->gradSlnOldNeighbor() : var->gradSlnOlderNeighbor();
}

const VariableGradient &
Coupleable::coupledGradientOlder(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_gradient;

  coupledCallback(var_name, true);
  if (_c_nodal)
    mooseError(_c_name, ": Nodal compute objects do not support gradients");

  validateExecutionerType(var_name, "coupledGradientOlder");
  MooseVariable * var = getVar(var_name, comp);
  if (var == NULL)
    mooseError("Call corresponding vector variable method");

  if (_c_is_implicit)
  {
    if (!_coupleable_neighbor)
      return var->gradSlnOlder();
    else
      return var->gradSlnOlderNeighbor();
  }
  else
    mooseError(_c_name, ": Older values not available for explicit schemes");
}

const VariableGradient &
Coupleable::coupledGradientPreviousNL(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_gradient;

  _c_fe_problem.needsPreviousNewtonIteration(true);
  coupledCallback(var_name, true);
  if (_c_nodal)
    mooseError(_c_name, ": Nodal compute objects do not support gradients");

  MooseVariable * var = getVar(var_name, comp);
  if (var == NULL)
    mooseError("Call corresponding vector variable method");

  if (!_coupleable_neighbor)
    return var->gradSlnPreviousNL();
  else
    return var->gradSlnPreviousNLNeighbor();
}

const VariableGradient &
Coupleable::coupledGradientDot(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_gradient;

  coupledCallback(var_name, false);
  if (_c_nodal)
    mooseError(_c_name, ": Nodal variables do not have gradients");

  MooseVariable * var = getVar(var_name, comp);
  if (var == NULL)
    mooseError("Call corresponding vector variable method");

  if (!_coupleable_neighbor)
    return var->gradSlnDot();
  else
    return var->gradSlnNeighborDot();
}

const VariableGradient &
Coupleable::coupledGradientDotDot(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_gradient;

  coupledCallback(var_name, false);
  if (_c_nodal)
    mooseError(_c_name, ": Nodal variables do not have gradients");

  MooseVariable * var = getVar(var_name, comp);
  if (var == NULL)
    mooseError("Call corresponding vector variable method");

  if (!_coupleable_neighbor)
    return var->gradSlnDotDot();
  else
    return var->gradSlnNeighborDotDot();
}

const VectorVariableGradient &
Coupleable::coupledVectorGradient(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_vector_gradient;

  coupledCallback(var_name, false);
  if (_c_nodal)
    mooseError(_c_name, ": Gradients are non-sensical with nodal compute objects");

  VectorMooseVariable * var = getVectorVar(var_name, comp);
  if (var == NULL)
    mooseError("Call corresponding standard variable method");

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->gradSln() : var->gradSlnOld();
  else
    return (_c_is_implicit) ? var->gradSlnNeighbor() : var->gradSlnOldNeighbor();
}

const VectorVariableGradient &
Coupleable::coupledVectorGradientOld(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_vector_gradient;

  coupledCallback(var_name, true);
  if (_c_nodal)
    mooseError(_c_name, ": Gradients are non-sensical with nodal compute objects");

  validateExecutionerType(var_name, "coupledGradientOld");
  VectorMooseVariable * var = getVectorVar(var_name, comp);
  if (var == NULL)
    mooseError("Call corresponding standard variable method");

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->gradSlnOld() : var->gradSlnOlder();
  else
    return (_c_is_implicit) ? var->gradSlnOldNeighbor() : var->gradSlnOlderNeighbor();
}

const VectorVariableGradient &
Coupleable::coupledVectorGradientOlder(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_vector_gradient;

  coupledCallback(var_name, true);
  if (_c_nodal)
    mooseError(_c_name, ": Gradients are non-sensical with nodal compute objects");

  validateExecutionerType(var_name, "coupledGradientOlder");
  VectorMooseVariable * var = getVectorVar(var_name, comp);
  if (var == NULL)
    mooseError("Call corresponding standard variable method");

  if (_c_is_implicit)
  {
    if (!_coupleable_neighbor)
      return var->gradSlnOlder();
    else
      return var->gradSlnOlderNeighbor();
  }
  else
    mooseError(_c_name, ": Older values not available for explicit schemes");
}

const VectorVariableCurl &
Coupleable::coupledCurl(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name)) // Return default 0
    return _default_vector_curl;

  coupledCallback(var_name, false);
  if (_c_nodal)
    mooseError("Nodal variables do not have curls");

  VectorMooseVariable * var = getVectorVar(var_name, comp);
  if (var == NULL)
    mooseError("Call corresponding scalar field variable method");

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->curlSln() : var->curlSlnOld();
  else
    return (_c_is_implicit) ? var->curlSlnNeighbor() : var->curlSlnOldNeighbor();
}

const VectorVariableCurl &
Coupleable::coupledCurlOld(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name)) // Return default 0
    return _default_vector_curl;

  coupledCallback(var_name, true);
  if (_c_nodal)
    mooseError("Nodal variables do not have curls");

  validateExecutionerType(var_name, "coupledCurlOld");
  VectorMooseVariable * var = getVectorVar(var_name, comp);
  if (var == NULL)
    mooseError("Call corresponding scalar field variable method");

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->curlSlnOld() : var->curlSlnOlder();
  else
    return (_c_is_implicit) ? var->curlSlnOldNeighbor() : var->curlSlnOlderNeighbor();
}

const VectorVariableCurl &
Coupleable::coupledCurlOlder(const std::string & var_name, unsigned int comp)
{
  if (!isCoupled(var_name)) // Return default 0
    return _default_vector_curl;

  coupledCallback(var_name, true);
  if (_c_nodal)
    mooseError("Nodal variables do not have curls");

  validateExecutionerType(var_name, "coupledCurlOlder");
  VectorMooseVariable * var = getVectorVar(var_name, comp);
  if (var == NULL)
    mooseError("Call corresponding scalar field variable method");

  if (_c_is_implicit)
  {
    if (!_coupleable_neighbor)
      return var->curlSlnOlder();
    else
      return var->curlSlnOlderNeighbor();
  }
  else
    mooseError("Older values not available for explicit schemes");
}

const VariableSecond &
Coupleable::coupledSecond(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_second;

  coupledCallback(var_name, false);
  if (_c_nodal)
    mooseError(_c_name, ": Nodal variables do not have second derivatives");

  MooseVariable * var = getVar(var_name, comp);
  if (var == NULL)
    mooseError("Call corresponding vector variable method");

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->secondSln() : var->secondSlnOlder();
  else
    return (_c_is_implicit) ? var->secondSlnNeighbor() : var->secondSlnOlderNeighbor();
}

const VariableSecond &
Coupleable::coupledSecondOld(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_second;

  coupledCallback(var_name, true);
  if (_c_nodal)
    mooseError(_c_name, ": Nodal variables do not have second derivatives");

  validateExecutionerType(var_name, "coupledSecondOld");
  MooseVariable * var = getVar(var_name, comp);
  if (var == NULL)
    mooseError("Call corresponding vector variable method");
  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->secondSlnOld() : var->secondSlnOlder();
  else
    return (_c_is_implicit) ? var->secondSlnOldNeighbor() : var->secondSlnOlderNeighbor();
}

const VariableSecond &
Coupleable::coupledSecondOlder(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_second;

  coupledCallback(var_name, true);
  if (_c_nodal)
    mooseError(_c_name, ": Nodal variables do not have second derivatives");

  validateExecutionerType(var_name, "coupledSecondOlder");
  MooseVariable * var = getVar(var_name, comp);
  if (var == NULL)
    mooseError("Call corresponding vector variable method");
  if (_c_is_implicit)
  {
    if (!_coupleable_neighbor)
      return var->secondSlnOlder();
    else
      return var->secondSlnOlderNeighbor();
  }
  else
    mooseError(_c_name, ": Older values not available for explicit schemes");
}

const VariableSecond &
Coupleable::coupledSecondPreviousNL(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_second;

  _c_fe_problem.needsPreviousNewtonIteration(true);
  coupledCallback(var_name, true);
  if (_c_nodal)
    mooseError(_c_name, ": Nodal variables do not have second derivatives");

  MooseVariable * var = getVar(var_name, comp);
  if (var == NULL)
    mooseError("Call corresponding vector variable method");
  if (!_coupleable_neighbor)
    return var->secondSlnPreviousNL();
  else
    return var->secondSlnPreviousNLNeighbor();
}

const VariableValue &
Coupleable::coupledNodalValue(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name))
    return *getDefaultValue(var_name, comp);

  coupledCallback(var_name, false);
  MooseVariable * var = getVar(var_name, comp);
  if (var == NULL)
    mooseError("Call corresponding vector variable method");
  if (!var->isNodal())
    mooseError(_c_name,
               ": Trying to get nodal values of variable '",
               var->name(),
               "', but it is not nodal.");

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->dofValues() : var->dofValuesOld();
  else
    return (_c_is_implicit) ? var->dofValuesNeighbor() : var->dofValuesOldNeighbor();
}

const VariableValue &
Coupleable::coupledNodalValueOld(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name))
    return *getDefaultValue(var_name, comp);

  validateExecutionerType(var_name, "coupledNodalValueOld");
  coupledCallback(var_name, true);
  MooseVariable * var = getVar(var_name, comp);
  if (var == NULL)
    mooseError("Call corresponding vector variable method");
  if (!var->isNodal())
    mooseError(_c_name,
               ": Trying to get old nodal values of variable '",
               var->name(),
               "', but it is not nodal.");

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->dofValuesOld() : var->dofValuesOlder();
  else
    return (_c_is_implicit) ? var->dofValuesOldNeighbor() : var->dofValuesOlderNeighbor();
}

const VariableValue &
Coupleable::coupledNodalValueOlder(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name))
    return *getDefaultValue(var_name, comp);

  validateExecutionerType(var_name, "coupledNodalValueOlder");
  coupledCallback(var_name, true);
  MooseVariable * var = getVar(var_name, comp);
  if (var == NULL)
    mooseError("Call corresponding vector variable method");
  if (!var->isNodal())
    mooseError(_c_name,
               ": Trying to get older nodal values of variable '",
               var->name(),
               "', but it is not nodal.");
  if (_c_is_implicit)
  {
    if (!_coupleable_neighbor)
      return var->dofValuesOlder();
    else
      return var->dofValuesOlderNeighbor();
  }
  else
    mooseError(_c_name, ": Older values not available for explicit schemes");
}

const VariableValue &
Coupleable::coupledNodalValuePreviousNL(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name))
    return *getDefaultValue(var_name, comp);

  _c_fe_problem.needsPreviousNewtonIteration(true);
  coupledCallback(var_name, true);
  MooseVariable * var = getVar(var_name, comp);
  if (var == NULL)
    mooseError("Call corresponding vector variable method");

  if (!_coupleable_neighbor)
    return var->dofValuesPreviousNL();
  else
    return var->dofValuesPreviousNLNeighbor();
}

const VariableValue &
Coupleable::coupledNodalDot(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_value_zero;

  validateExecutionerType(var_name, "coupledNodalDot");
  coupledCallback(var_name, false);
  MooseVariable * var = getVar(var_name, comp);
  if (var == NULL)
    mooseError("Call corresponding vector variable method");

  if (!_coupleable_neighbor)
    return var->dofValuesDot();
  else
    return var->dofValuesDotNeighbor();
}

const VariableValue &
Coupleable::coupledNodalDotDot(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_value_zero;

  validateExecutionerType(var_name, "coupledNodalDotDot");
  coupledCallback(var_name, false);
  MooseVariable * var = getVar(var_name, comp);
  if (var == NULL)
    mooseError("Call corresponding vector variable method");

  if (!_coupleable_neighbor)
    return var->dofValuesDotDot();
  else
    return var->dofValuesDotDotNeighbor();
}

const VariableValue &
Coupleable::coupledNodalDotOld(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_value_zero;

  validateExecutionerType(var_name, "coupledNodalDotOld");
  coupledCallback(var_name, false);
  MooseVariable * var = getVar(var_name, comp);
  if (var == NULL)
    mooseError("Call corresponding vector variable method");

  if (!_coupleable_neighbor)
    return var->dofValuesDotOld();
  else
    return var->dofValuesDotOldNeighbor();
}

const VariableValue &
Coupleable::coupledNodalDotDotOld(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_value_zero;

  validateExecutionerType(var_name, "coupledNodalDotDotOld");
  coupledCallback(var_name, false);
  MooseVariable * var = getVar(var_name, comp);
  if (var == NULL)
    mooseError("Call corresponding vector variable method");

  if (!_coupleable_neighbor)
    return var->dofValuesDotDotOld();
  else
    return var->dofValuesDotDotOldNeighbor();
}
const DenseVector<Number> &
Coupleable::coupledSolutionDoFs(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  // default coupling is not available for elemental solutions
  if (!isCoupled(var_name))
    mooseError(_c_name, ": invalid variable name for coupledSolutionDoFs");

  if (_c_nodal)
    mooseError(_c_name, ": nodal objects should not call coupledSolutionDoFs");

  coupledCallback(var_name, false);
  MooseVariableFEBase * var = getFEVar(var_name, comp);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->solutionDoFs() : var->solutionDoFsOld();
  else
    return (_c_is_implicit) ? var->solutionDoFsNeighbor() : var->solutionDoFsOldNeighbor();
}

const DenseVector<Number> &
Coupleable::coupledSolutionDoFsOld(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  // default coupling is not available for elemental solutions
  if (!isCoupled(var_name))
    mooseError(_c_name, ": invalid variable name for coupledSolutionDoFsOld");

  if (_c_nodal)
    mooseError(_c_name, ": nodal objects should not call coupledSolutionDoFsOld");

  validateExecutionerType(var_name, "coupledSolutionDoFsOld");
  coupledCallback(var_name, true);
  MooseVariableFEBase * var = getFEVar(var_name, comp);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->solutionDoFsOld() : var->solutionDoFsOlder();
  else
    return (_c_is_implicit) ? var->solutionDoFsOldNeighbor() : var->solutionDoFsOlderNeighbor();
}

const DenseVector<Number> &
Coupleable::coupledSolutionDoFsOlder(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  // default coupling is not available for elemental solutions
  if (!isCoupled(var_name))
    mooseError(_c_name, ": invalid variable name for coupledSolutionDoFsOlder");

  if (_c_nodal)
    mooseError(_c_name, ": nodal objects should not call coupledSolutionDoFsOlder");

  validateExecutionerType(var_name, "coupledSolutionDoFsOlder");
  coupledCallback(var_name, true);
  MooseVariableFEBase * var = getFEVar(var_name, comp);
  if (_c_is_implicit)
  {
    if (!_coupleable_neighbor)
      return var->solutionDoFsOlder();
    else
      return var->solutionDoFsOlderNeighbor();
  }
  else
    mooseError(_c_name, ": Older values not available for explicit schemes");
}

void
Coupleable::validateExecutionerType(const std::string & name, const std::string & fn_name) const
{
  if (!_c_fe_problem.isTransient())
    mooseError(_c_name,
               ": Calling '",
               fn_name,
               "' on variable \"",
               name,
               "\" when using a \"Steady\" executioner is not allowed. This value is available "
               "only in transient simulations.");
}

template <>
VariableValue *
Coupleable::getADDefaultValue<RESIDUAL>(const std::string & var_name)
{
  return getDefaultValue(var_name, 0);
}

template <>
VariableGradient &
Coupleable::getADDefaultGradient<RESIDUAL>()
{
  return _default_gradient;
}

template <>
const typename VariableValueType<JACOBIAN>::type &
Coupleable::adZeroTemplate<JACOBIAN>()
{
  return _ad_zero;
}

template <>
const typename VariableGradientType<JACOBIAN>::type &
Coupleable::adGradZeroTemplate<JACOBIAN>()
{
  return _ad_grad_zero;
}
