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
    _ad_second_zero(_c_fe_problem._ad_second_zero[_c_tid]),
    _second_phi_zero(_c_fe_problem._second_phi_zero[_c_tid]),
    _vector_zero(_c_fe_problem._vector_zero[_c_tid]),
    _vector_curl_zero(_c_fe_problem._vector_curl_zero[_c_tid]),
    _coupleable_neighbor(_c_parameters.have_parameter<bool>("_neighbor")
                             ? _c_parameters.get<bool>("_neighbor")
                             : false),
    _coupleable_max_qps(_c_fe_problem.getMaxQps()),
    _obj(moose_object)
{
  SubProblem & problem = *_c_parameters.getCheckedPointerParam<SubProblem *>("_subproblem");

  unsigned int optional_var_index_counter = 0;

  // Coupling
  for (auto iter = _c_parameters.coupledVarsBegin(); iter != _c_parameters.coupledVarsEnd(); ++iter)
  {
    std::string name = *iter;

    std::vector<std::string> vars = _c_parameters.getVecMooseType(name);
    if (vars.size() > 0)
    {
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
          else if (auto * tmp_var = dynamic_cast<ArrayMooseVariable *>(moose_var))
            _coupled_array_moose_vars.push_back(tmp_var);
          else
            _obj->paramError(name, "provided c++ type for variable parameter is not supported");
        }
        else if (problem.hasScalarVariable(coupled_var_name))
        {
          MooseVariableScalar * moose_scalar_var =
              &problem.getScalarVariable(_c_tid, coupled_var_name);
          _c_coupled_scalar_vars[name].push_back(moose_scalar_var);
        }
        else
          _obj->paramError(name, "coupled variable '", coupled_var_name, "' was not found");
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
  _ad_default_vector_gradient.resize(_coupleable_max_qps);
}

bool
Coupleable::isCoupled(const std::string & var_name, unsigned int i)
{
  auto it = _coupled_vars.find(var_name);
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
checkComponent(const MooseObject * obj,
               unsigned int comp,
               unsigned int bound,
               const std::string & var_name)
{
  if (bound > 0 && comp >= bound)
    obj->paramError(
        var_name, "component ", comp, " is out of range for this variable (max ", bound - 1, ")");
}

// calls to this must go *after* get[bla]Var calls and (checking for nullptr
// return).  Because checkFuncType calls coupledCallback which should only be
// called if the variables was actually coupled.
void
Coupleable::checkFuncType(const std::string var_name, VarType t, FuncAge age)
{
  if (t == VarType::Gradient && _c_nodal)
    mooseError(_c_name, ": nodal variables do not have gradients");

  if (age == FuncAge::Old || age == FuncAge::Older || t == VarType::GradientDot ||
      t == VarType::Dot)
    validateExecutionerType(var_name, "coupled[Vector][Gradient/Dot]Old[er]");
  if (age == FuncAge::Older && !_c_is_implicit)
    mooseError("object '",
               _c_name,
               "' uses older variable values that are unavailable with explicit schemes");

  // this goes at end after all error checks
  coupledCallback(var_name, age == FuncAge::Old || age == FuncAge::Older);
}

bool
Coupleable::checkVar(const std::string & var_name, unsigned int comp, unsigned int comp_bound)
{
  auto it = _c_coupled_scalar_vars.find(var_name);
  if (it != _c_coupled_scalar_vars.end())
  {
    std::string cvars;
    for (auto jt : it->second)
      cvars += " " + jt->name();

    _obj->paramError(var_name,
                     "cannot couple '",
                     var_name,
                     "' to a scalar variable (",
                     cvars,
                     ") where field variable is expected");
  }

  if (!isCoupled(var_name, comp))
    return false; // return false since variable is *not* coupled

  auto bound = comp_bound ? comp_bound : _coupled_vars[var_name].size();
  checkComponent(_obj, comp, bound, var_name);

  if (!(_coupled_vars[var_name][comp])->isNodal() && _c_nodal)
    mooseError(_c_name, ": cannot couple elemental variables into nodal objects");

  return true;
}

MooseVariableFEBase *
Coupleable::getFEVar(const std::string & var_name, unsigned int comp)
{
  if (!checkVar(var_name, comp, 0))
    return nullptr;
  return _coupled_vars[var_name][comp];
}

template <typename T>
MooseVariableFE<T> *
Coupleable::getVarHelper(const std::string & var_name, unsigned int comp)
{
  if (!checkVar(var_name, comp, 0))
    return nullptr;

  if (auto * coupled_var = dynamic_cast<MooseVariableFE<T> *>(_coupled_vars[var_name][comp]))
    return coupled_var;
  else
  {
    for (auto & var : _coupled_standard_moose_vars)
      if (var->name() == var_name)
        mooseError("The named variable is a standard variable, try a "
                   "'coupled[Value/Gradient/Dot/etc]...' function instead");
    for (auto & var : _coupled_vector_moose_vars)
      if (var->name() == var_name)
        mooseError("The named variable is a vector variable, try a "
                   "'coupledVector[Value/Gradient/Dot/etc]...' function instead");
    for (auto & var : _coupled_array_moose_vars)
      if (var->name() == var_name)
        mooseError("The named variable is an array variable, try a "
                   "'coupledArray[Value/Gradient/Dot/etc]...' function instead");
    mooseError(
        "Variable '", var_name, "' is of a different C++ type than you tried to fetch it as.");
  }
}

MooseVariable *
Coupleable::getVar(const std::string & var_name, unsigned int comp)
{
  return getVarHelper<Real>(var_name, comp);
}

VectorMooseVariable *
Coupleable::getVectorVar(const std::string & var_name, unsigned int comp)
{
  if (_c_nodal)
    mooseError("Nodal object '",
               _c_name,
               "' uses vector variables which are not required to be continuous. Don't use vector "
               "variables"
               "with nodal compute objects.");
  return getVarHelper<RealVectorValue>(var_name, comp);
}

ArrayMooseVariable *
Coupleable::getArrayVar(const std::string & var_name, unsigned int comp)
{
  return getVarHelper<RealEigenVector>(var_name, comp);
}

VariableValue *
Coupleable::getDefaultValue(const std::string & var_name, unsigned int comp)
{
  // make sure we don't access values that were not provided
  checkComponent(_obj, comp, _c_parameters.numberDefaultCoupledValues(var_name), var_name);

  auto default_value_it = _default_value.find(var_name);
  if (default_value_it == _default_value.end())
  {
    _default_value[var_name].emplace_back(libmesh_make_unique<VariableValue>(
        _coupleable_max_qps, _c_parameters.defaultCoupledValue(var_name, 0)));
    for (unsigned int j = 1; j < _c_parameters.numberDefaultCoupledValues(var_name); ++j)
      _default_value[var_name].emplace_back(libmesh_make_unique<VariableValue>(
          _coupleable_max_qps, _c_parameters.defaultCoupledValue(var_name, j)));
    default_value_it = _default_value.find(var_name);
  }

  return default_value_it->second[comp].get();
}

VectorVariableValue *
Coupleable::getDefaultVectorValue(const std::string & var_name)
{
  auto default_value_it = _default_vector_value.find(var_name);
  if (default_value_it == _default_vector_value.end())
  {
    auto value = libmesh_make_unique<VectorVariableValue>(_coupleable_max_qps, 0);
    bool already_warned = false;
    for (unsigned int qp = 0; qp < _coupleable_max_qps; ++qp)
      for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
      {
        try
        {
          (*value)[qp](i) = _c_parameters.defaultCoupledValue(var_name, i);
        }
        catch (const std::out_of_range &)
        {
          if (!already_warned)
            mooseWarning(
                "You supplied less than 3 arguments for the default vector value for variable ",
                var_name,
                ". Did you accidently leave something off? We are going to assign 0s, assuming "
                "this "
                "was intentional.");
          already_warned = true;
          (*value)[qp](i) = 0;
        }
      }
    default_value_it =
        _default_vector_value.insert(std::make_pair(var_name, std::move(value))).first;
  }

  return default_value_it->second.get();
}

ArrayVariableValue *
Coupleable::getDefaultArrayValue(const std::string & var_name)
{
  std::map<std::string, ArrayVariableValue *>::iterator default_value_it =
      _default_array_value.find(var_name);
  if (default_value_it == _default_array_value.end())
  {
    ArrayVariableValue * value = new ArrayVariableValue(_coupleable_max_qps);
    for (unsigned int qp = 0; qp < _coupleable_max_qps; ++qp)
    {
      auto n = _c_parameters.numberDefaultCoupledValues(var_name);
      (*value)[qp].resize(n);
      for (unsigned int i = 0; i < n; ++i)
        (*value)[qp](i) = _c_parameters.defaultCoupledValue(var_name, i);
    }
    default_value_it = _default_array_value.insert(std::make_pair(var_name, value)).first;
  }

  return default_value_it->second;
}

template <typename T>
const T &
Coupleable::getDefaultNodalValue(const std::string & var_name, unsigned int comp)
{
  auto && default_variable_value = getDefaultValue(var_name, comp);
  return *default_variable_value->data();
}

template <>
const RealVectorValue &
Coupleable::getDefaultNodalValue<RealVectorValue>(const std::string & var_name, unsigned int)
{
  auto && default_variable_value = getDefaultVectorValue(var_name);
  return *default_variable_value->data();
}

template <>
const RealEigenVector &
Coupleable::getDefaultNodalValue<RealEigenVector>(const std::string & var_name, unsigned int)
{
  auto && default_variable_value = getDefaultArrayValue(var_name);
  return *default_variable_value->data();
}

unsigned int
Coupleable::coupled(const std::string & var_name, unsigned int comp)
{
  MooseVariableFEBase * var = getFEVar(var_name, comp);
  if (!var)
  {
    // make sure we don't try to access default var ids that were not provided
    checkComponent(_obj, comp, _optional_var_index[var_name].size(), var_name);
    return _optional_var_index[var_name][comp];
  }
  checkFuncType(var_name, VarType::Ignore, FuncAge::Curr);

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

const VariableValue &
Coupleable::coupledValue(const std::string & var_name, unsigned int comp)
{
  MooseVariable * var = getVar(var_name, comp);
  if (!var)
    return *getDefaultValue(var_name, comp);
  checkFuncType(var_name, VarType::Ignore, FuncAge::Curr);

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
  MooseVariable * var = getVar(var_name, comp);
  if (!var)
    mooseError(var_name, ": invalid variable name for coupledVectorTagValue");
  checkFuncType(var_name, VarType::Ignore, FuncAge::Curr);

  addFEVariableCoupleableVectorTag(tag);

  if (_c_nodal)
    return var->nodalVectorTagValue(tag);
  else
    return var->vectorTagValue(tag);
}

const VariableValue &
Coupleable::coupledMatrixTagValue(const std::string & var_name, TagID tag, unsigned int comp)
{
  MooseVariable * var = getVar(var_name, comp);
  if (!var)
    mooseError(var_name, ": invalid variable name for coupledMatrixTagValue");
  checkFuncType(var_name, VarType::Ignore, FuncAge::Curr);

  addFEVariableCoupleableMatrixTag(tag);

  if (_c_nodal)
    return var->nodalMatrixTagValue(tag);
  return var->matrixTagValue(tag);
}

const VectorVariableValue &
Coupleable::coupledVectorValue(const std::string & var_name, unsigned int comp)
{
  VectorMooseVariable * var = getVectorVar(var_name, comp);
  if (!var)
    return *getDefaultVectorValue(var_name);
  checkFuncType(var_name, VarType::Ignore, FuncAge::Curr);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->sln() : var->slnOld();
  return (_c_is_implicit) ? var->slnNeighbor() : var->slnOldNeighbor();
}

const ArrayVariableValue &
Coupleable::coupledArrayValue(const std::string & var_name, unsigned int comp)
{
  ArrayMooseVariable * var = getArrayVar(var_name, comp);
  if (!var)
    return *getDefaultArrayValue(var_name);
  checkFuncType(var_name, VarType::Ignore, FuncAge::Curr);

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      return (_c_is_implicit) ? var->dofValues() : var->dofValuesOld();
    return (_c_is_implicit) ? var->sln() : var->slnOld();
  }
  else
  {
    if (_c_nodal)
      return (_c_is_implicit) ? var->dofValuesNeighbor() : var->dofValuesOldNeighbor();
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
  MooseVariable * var = getVar(var_name, comp);
  if (!var)
    return *getDefaultValue(var_name, comp);
  checkFuncType(var_name, VarType::Ignore, FuncAge::Old);

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      return (_c_is_implicit) ? var->dofValuesOld() : var->dofValuesOlder();
    return (_c_is_implicit) ? var->slnOld() : var->slnOlder();
  }
  else
  {
    if (_c_nodal)
      return (_c_is_implicit) ? var->dofValuesOldNeighbor() : var->dofValuesOlderNeighbor();
    return (_c_is_implicit) ? var->slnOldNeighbor() : var->slnOlderNeighbor();
  }
}

const VariableValue &
Coupleable::coupledValueOlder(const std::string & var_name, unsigned int comp)
{
  MooseVariable * var = getVar(var_name, comp);
  if (!var)
    return *getDefaultValue(var_name, comp);
  checkFuncType(var_name, VarType::Ignore, FuncAge::Older);

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      return var->dofValuesOlder();
    return var->slnOlder();
  }
  else
  {
    if (_c_nodal)
      return var->dofValuesOlderNeighbor();
    return var->slnOlderNeighbor();
  }
}

const VariableValue &
Coupleable::coupledValuePreviousNL(const std::string & var_name, unsigned int comp)
{
  MooseVariable * var = getVar(var_name, comp);
  if (!var)
    return *getDefaultValue(var_name, comp);
  checkFuncType(var_name, VarType::Ignore, FuncAge::Curr);

  _c_fe_problem.needsPreviousNewtonIteration(true);
  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      return var->dofValuesPreviousNL();
    return var->slnPreviousNL();
  }
  else
  {
    if (_c_nodal)
      return var->dofValuesPreviousNLNeighbor();
    return var->slnPreviousNLNeighbor();
  }
}

const VectorVariableValue &
Coupleable::coupledVectorValueOld(const std::string & var_name, unsigned int comp)
{
  VectorMooseVariable * var = getVectorVar(var_name, comp);
  if (!var)
    return *getDefaultVectorValue(var_name);
  checkFuncType(var_name, VarType::Ignore, FuncAge::Old);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->slnOld() : var->slnOlder();
  return (_c_is_implicit) ? var->slnOldNeighbor() : var->slnOlderNeighbor();
}

const VectorVariableValue &
Coupleable::coupledVectorValueOlder(const std::string & var_name, unsigned int comp)
{
  VectorMooseVariable * var = getVectorVar(var_name, comp);
  if (!var)
    return *getDefaultVectorValue(var_name);
  checkFuncType(var_name, VarType::Ignore, FuncAge::Older);

  if (!_coupleable_neighbor)
    return var->slnOlder();
  return var->slnOlderNeighbor();
}

const ArrayVariableValue &
Coupleable::coupledArrayValueOld(const std::string & var_name, unsigned int comp)
{
  ArrayMooseVariable * var = getArrayVar(var_name, comp);
  if (!var)
    return *getDefaultArrayValue(var_name);
  checkFuncType(var_name, VarType::Ignore, FuncAge::Old);

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      return (_c_is_implicit) ? var->dofValuesOld() : var->dofValuesOlder();
    return (_c_is_implicit) ? var->slnOld() : var->slnOlder();
  }
  else
  {
    if (_c_nodal)
      return (_c_is_implicit) ? var->dofValuesOldNeighbor() : var->dofValuesOlderNeighbor();
    return (_c_is_implicit) ? var->slnOldNeighbor() : var->slnOlderNeighbor();
  }
}

const ArrayVariableValue &
Coupleable::coupledArrayValueOlder(const std::string & var_name, unsigned int comp)
{
  ArrayMooseVariable * var = getArrayVar(var_name, comp);
  if (!var)
    return *getDefaultArrayValue(var_name);
  checkFuncType(var_name, VarType::Ignore, FuncAge::Older);

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      return var->dofValuesOlder();
    return var->slnOlder();
  }
  else
  {
    if (_c_nodal)
      return var->dofValuesOlderNeighbor();
    return var->slnOlderNeighbor();
  }
}

const VariableValue &
Coupleable::coupledDot(const std::string & var_name, unsigned int comp)
{
  MooseVariable * var = getVar(var_name, comp);
  if (!var)
    return _default_value_zero;
  checkFuncType(var_name, VarType::Dot, FuncAge::Curr);

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      return var->dofValuesDot();
    return var->uDot();
  }
  else
  {
    if (_c_nodal)
      return var->dofValuesDotNeighbor();
    return var->uDotNeighbor();
  }
}

const VariableValue &
Coupleable::coupledDotResidual(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_value_zero;

  validateExecutionerType(var_name, "coupledDotResidual");
  MooseVariable * var = getVar(var_name, comp);
  if (var == NULL)
    mooseError("Call corresponding vector variable method");

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      return var->dofValuesDotResidual();
    else
      return var->uDotResidual();
  }
  else
  {
    if (_c_nodal)
      return var->dofValuesDotNeighborResidual();
    else
      return var->uDotNeighborResidual();
  }
}

const VariableValue &
Coupleable::coupledDotDot(const std::string & var_name, unsigned int comp)
{
  MooseVariable * var = getVar(var_name, comp);
  if (!var)
    return _default_value_zero;
  checkFuncType(var_name, VarType::Dot, FuncAge::Curr);

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      return var->dofValuesDotDot();
    return var->uDotDot();
  }
  else
  {
    if (_c_nodal)
      return var->dofValuesDotDotNeighbor();
    return var->uDotDotNeighbor();
  }
}

const VariableValue &
Coupleable::coupledDotDotResidual(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_value_zero;

  validateExecutionerType(var_name, "coupledDotDotResidual");
  MooseVariable * var = getVar(var_name, comp);
  if (var == NULL)
    mooseError("Call corresponding vector variable method");

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      return var->dofValuesDotDotResidual();
    else
      return var->uDotDotResidual();
  }
  else
  {
    if (_c_nodal)
      return var->dofValuesDotDotNeighborResidual();
    else
      return var->uDotDotNeighborResidual();
  }
}

const VariableValue &
Coupleable::coupledDotOld(const std::string & var_name, unsigned int comp)
{
  MooseVariable * var = getVar(var_name, comp);
  if (!var)
    return _default_value_zero;
  checkFuncType(var_name, VarType::Dot, FuncAge::Old);

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      return var->dofValuesDotOld();
    return var->uDotOld();
  }
  else
  {
    if (_c_nodal)
      return var->dofValuesDotOldNeighbor();
    return var->uDotOldNeighbor();
  }
}

const VariableValue &
Coupleable::coupledDotDotOld(const std::string & var_name, unsigned int comp)
{
  MooseVariable * var = getVar(var_name, comp);
  if (!var)
    return _default_value_zero;
  checkFuncType(var_name, VarType::Dot, FuncAge::Old);

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      return var->dofValuesDotDotOld();
    return var->uDotDotOld();
  }
  else
  {
    if (_c_nodal)
      return var->dofValuesDotDotOldNeighbor();
    return var->uDotDotOldNeighbor();
  }
}

const VectorVariableValue &
Coupleable::coupledVectorDot(const std::string & var_name, unsigned int comp)
{
  VectorMooseVariable * var = getVectorVar(var_name, comp);
  if (!var)
    return _default_vector_value_zero;
  checkFuncType(var_name, VarType::Dot, FuncAge::Curr);

  if (!_coupleable_neighbor)
    return var->uDot();
  return var->uDotNeighbor();
}

const VectorVariableValue &
Coupleable::coupledVectorDotResidual(const std::string & var_name, unsigned int comp)
{
  VectorMooseVariable * var = getVectorVar(var_name, comp);
  if (!var)
    return _default_vector_value_zero;
  checkFuncType(var_name, VarType::Dot, FuncAge::Curr);

  if (!_coupleable_neighbor)
    return var->uDotResidual();
  return var->uDotNeighborResidual();
}

const VectorVariableValue &
Coupleable::coupledVectorDotDot(const std::string & var_name, unsigned int comp)
{
  VectorMooseVariable * var = getVectorVar(var_name, comp);
  if (!var)
    return _default_vector_value_zero;
  checkFuncType(var_name, VarType::Dot, FuncAge::Curr);

  if (!_coupleable_neighbor)
    return var->uDotDot();
  return var->uDotDotNeighbor();
}

const VectorVariableValue &
Coupleable::coupledVectorDotDotResidual(const std::string & var_name, unsigned int comp)
{
  VectorMooseVariable * var = getVectorVar(var_name, comp);
  if (!var)
    return _default_vector_value_zero;
  checkFuncType(var_name, VarType::Dot, FuncAge::Curr);

  if (!_coupleable_neighbor)
    return var->uDotDotResidual();
  return var->uDotDotNeighborResidual();
}

const VectorVariableValue &
Coupleable::coupledVectorDotOld(const std::string & var_name, unsigned int comp)
{
  VectorMooseVariable * var = getVectorVar(var_name, comp);
  if (!var)
    return _default_vector_value_zero;
  checkFuncType(var_name, VarType::Dot, FuncAge::Old);

  if (!_coupleable_neighbor)
    return var->uDotOld();
  return var->uDotOldNeighbor();
}

const VectorVariableValue &
Coupleable::coupledVectorDotDotOld(const std::string & var_name, unsigned int comp)
{
  VectorMooseVariable * var = getVectorVar(var_name, comp);
  if (!var)
    return _default_vector_value_zero;
  checkFuncType(var_name, VarType::Dot, FuncAge::Old);

  if (!_coupleable_neighbor)
    return var->uDotDotOld();
  return var->uDotDotOldNeighbor();
}

const VariableValue &
Coupleable::coupledVectorDotDu(const std::string & var_name, unsigned int comp)
{
  VectorMooseVariable * var = getVectorVar(var_name, comp);
  if (!var)
    return _default_value_zero;
  checkFuncType(var_name, VarType::Dot, FuncAge::Curr);

  if (!_coupleable_neighbor)
    return var->duDotDu();
  return var->duDotDuNeighbor();
}

const VariableValue &
Coupleable::coupledVectorDotDotDu(const std::string & var_name, unsigned int comp)
{
  VectorMooseVariable * var = getVectorVar(var_name, comp);
  if (!var)
    return _default_value_zero;
  checkFuncType(var_name, VarType::Dot, FuncAge::Curr);

  if (!_coupleable_neighbor)
    return var->duDotDotDu();
  return var->duDotDotDuNeighbor();
}

const ArrayVariableValue &
Coupleable::coupledArrayDot(const std::string & var_name, unsigned int comp)
{
  ArrayMooseVariable * var = getArrayVar(var_name, comp);
  if (!var)
    return _default_array_value_zero;
  checkFuncType(var_name, VarType::Dot, FuncAge::Curr);

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      return var->dofValuesDot();
    return var->uDot();
  }
  else
  {
    if (_c_nodal)
      return var->dofValuesDotNeighbor();
    return var->uDotNeighbor();
  }
}

const ArrayVariableValue &
Coupleable::coupledArrayDotResidual(const std::string & var_name, unsigned int comp)
{
  ArrayMooseVariable * var = getArrayVar(var_name, comp);
  if (!var)
    return _default_array_value_zero;
  checkFuncType(var_name, VarType::Dot, FuncAge::Curr);

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      return var->dofValuesDotResidual();
    return var->uDotResidual();
  }
  else
  {
    if (_c_nodal)
      return var->dofValuesDotNeighborResidual();
    return var->uDotNeighborResidual();
  }
}

const ArrayVariableValue &
Coupleable::coupledArrayDotDot(const std::string & var_name, unsigned int comp)
{
  ArrayMooseVariable * var = getArrayVar(var_name, comp);
  if (!var)
    return _default_array_value_zero;
  checkFuncType(var_name, VarType::Dot, FuncAge::Curr);

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      return var->dofValuesDotDot();
    return var->uDotDot();
  }
  else
  {
    if (_c_nodal)
      return var->dofValuesDotDotNeighbor();
    return var->uDotDotNeighbor();
  }
}

const ArrayVariableValue &
Coupleable::coupledArrayDotDotResidual(const std::string & var_name, unsigned int comp)
{
  ArrayMooseVariable * var = getArrayVar(var_name, comp);
  if (!var)
    return _default_array_value_zero;
  checkFuncType(var_name, VarType::Dot, FuncAge::Curr);

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      return var->dofValuesDotDotResidual();
    return var->uDotDotResidual();
  }
  else
  {
    if (_c_nodal)
      return var->dofValuesDotDotNeighborResidual();
    return var->uDotDotNeighborResidual();
  }
}

const ArrayVariableValue &
Coupleable::coupledArrayDotOld(const std::string & var_name, unsigned int comp)
{
  ArrayMooseVariable * var = getArrayVar(var_name, comp);
  if (!var)
    return _default_array_value_zero;
  checkFuncType(var_name, VarType::Dot, FuncAge::Old);

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      return var->dofValuesDotOld();
    return var->uDotOld();
  }
  else
  {
    if (_c_nodal)
      return var->dofValuesDotOldNeighbor();
    return var->uDotOldNeighbor();
  }
}

const ArrayVariableValue &
Coupleable::coupledArrayDotDotOld(const std::string & var_name, unsigned int comp)
{
  ArrayMooseVariable * var = getArrayVar(var_name, comp);
  if (!var)
    return _default_array_value_zero;
  checkFuncType(var_name, VarType::Dot, FuncAge::Old);

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      return var->dofValuesDotDotOld();
    return var->uDotDotOld();
  }
  else
  {
    if (_c_nodal)
      return var->dofValuesDotDotOldNeighbor();
    return var->uDotDotOldNeighbor();
  }
}

const VariableValue &
Coupleable::coupledDotDu(const std::string & var_name, unsigned int comp)
{
  MooseVariable * var = getVar(var_name, comp);
  if (!var)
    return _default_value_zero;
  checkFuncType(var_name, VarType::Dot, FuncAge::Curr);

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      return var->dofValuesDuDotDu();
    return var->duDotDu();
  }
  else
  {
    if (_c_nodal)
      return var->dofValuesDuDotDuNeighbor();
    return var->duDotDuNeighbor();
  }
}

const VariableValue &
Coupleable::coupledDotDotDu(const std::string & var_name, unsigned int comp)
{
  MooseVariable * var = getVar(var_name, comp);
  if (!var)
    return _default_value_zero;
  checkFuncType(var_name, VarType::Dot, FuncAge::Curr);

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      return var->dofValuesDuDotDotDu();
    return var->duDotDotDu();
  }
  else
  {
    if (_c_nodal)
      return var->dofValuesDuDotDotDuNeighbor();
    return var->duDotDotDuNeighbor();
  }
}

const VariableGradient &
Coupleable::coupledGradient(const std::string & var_name, unsigned int comp)
{
  MooseVariable * var = getVar(var_name, comp);
  if (!var)
    return _default_gradient;
  checkFuncType(var_name, VarType::Gradient, FuncAge::Curr);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->gradSln() : var->gradSlnOld();
  return (_c_is_implicit) ? var->gradSlnNeighbor() : var->gradSlnOldNeighbor();
}

const VariableGradient &
Coupleable::coupledGradientOld(const std::string & var_name, unsigned int comp)
{
  MooseVariable * var = getVar(var_name, comp);
  if (!var)
    return _default_gradient;
  checkFuncType(var_name, VarType::Gradient, FuncAge::Old);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->gradSlnOld() : var->gradSlnOlder();
  return (_c_is_implicit) ? var->gradSlnOldNeighbor() : var->gradSlnOlderNeighbor();
}

const VariableGradient &
Coupleable::coupledGradientOlder(const std::string & var_name, unsigned int comp)
{
  MooseVariable * var = getVar(var_name, comp);
  if (!var)
    return _default_gradient;
  checkFuncType(var_name, VarType::Gradient, FuncAge::Older);

  if (!_coupleable_neighbor)
    return var->gradSlnOlder();
  return var->gradSlnOlderNeighbor();
}

const VariableGradient &
Coupleable::coupledGradientPreviousNL(const std::string & var_name, unsigned int comp)
{
  MooseVariable * var = getVar(var_name, comp);
  _c_fe_problem.needsPreviousNewtonIteration(true);
  if (!var)
    return _default_gradient;
  checkFuncType(var_name, VarType::Gradient, FuncAge::Curr);

  if (!_coupleable_neighbor)
    return var->gradSlnPreviousNL();
  return var->gradSlnPreviousNLNeighbor();
}

const VariableGradient &
Coupleable::coupledGradientDot(const std::string & var_name, unsigned int comp)
{
  MooseVariable * var = getVar(var_name, comp);
  if (!var)
    return _default_gradient;
  checkFuncType(var_name, VarType::GradientDot, FuncAge::Curr);

  if (!_coupleable_neighbor)
    return var->gradSlnDot();
  return var->gradSlnNeighborDot();
}

const VariableGradient &
Coupleable::coupledGradientDotDot(const std::string & var_name, unsigned int comp)
{
  MooseVariable * var = getVar(var_name, comp);
  if (!var)
    return _default_gradient;
  checkFuncType(var_name, VarType::GradientDot, FuncAge::Curr);

  if (!_coupleable_neighbor)
    return var->gradSlnDotDot();
  return var->gradSlnNeighborDotDot();
}

const VectorVariableGradient &
Coupleable::coupledVectorGradient(const std::string & var_name, unsigned int comp)
{
  VectorMooseVariable * var = getVectorVar(var_name, comp);
  if (!var)
    return _default_vector_gradient;
  checkFuncType(var_name, VarType::Gradient, FuncAge::Curr);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->gradSln() : var->gradSlnOld();
  return (_c_is_implicit) ? var->gradSlnNeighbor() : var->gradSlnOldNeighbor();
}

const VectorVariableGradient &
Coupleable::coupledVectorGradientOld(const std::string & var_name, unsigned int comp)
{
  VectorMooseVariable * var = getVectorVar(var_name, comp);
  if (!var)
    return _default_vector_gradient;
  checkFuncType(var_name, VarType::Gradient, FuncAge::Old);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->gradSlnOld() : var->gradSlnOlder();
  return (_c_is_implicit) ? var->gradSlnOldNeighbor() : var->gradSlnOlderNeighbor();
}

const VectorVariableGradient &
Coupleable::coupledVectorGradientOlder(const std::string & var_name, unsigned int comp)
{
  VectorMooseVariable * var = getVectorVar(var_name, comp);
  if (!var)
    return _default_vector_gradient;
  checkFuncType(var_name, VarType::Gradient, FuncAge::Older);

  if (!_coupleable_neighbor)
    return var->gradSlnOlder();
  return var->gradSlnOlderNeighbor();
}

const ArrayVariableGradient &
Coupleable::coupledArrayGradient(const std::string & var_name, unsigned int comp)
{
  ArrayMooseVariable * var = getArrayVar(var_name, comp);
  if (!var)
    return _default_array_gradient;
  checkFuncType(var_name, VarType::Gradient, FuncAge::Curr);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->gradSln() : var->gradSlnOld();
  return (_c_is_implicit) ? var->gradSlnNeighbor() : var->gradSlnOldNeighbor();
}

const ArrayVariableGradient &
Coupleable::coupledArrayGradientOld(const std::string & var_name, unsigned int comp)
{
  ArrayMooseVariable * var = getArrayVar(var_name, comp);
  if (!var)
    return _default_array_gradient;
  checkFuncType(var_name, VarType::Gradient, FuncAge::Old);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->gradSlnOld() : var->gradSlnOlder();
  return (_c_is_implicit) ? var->gradSlnOldNeighbor() : var->gradSlnOlderNeighbor();
}

const ArrayVariableGradient &
Coupleable::coupledArrayGradientOlder(const std::string & var_name, unsigned int comp)
{
  ArrayMooseVariable * var = getArrayVar(var_name, comp);
  if (!var)
    return _default_array_gradient;
  checkFuncType(var_name, VarType::Gradient, FuncAge::Older);

  if (!_coupleable_neighbor)
    return var->gradSlnOlder();
  return var->gradSlnOlderNeighbor();
}

const VectorVariableCurl &
Coupleable::coupledCurl(const std::string & var_name, unsigned int comp)
{
  VectorMooseVariable * var = getVectorVar(var_name, comp);
  if (!var)
    return _default_vector_curl;
  checkFuncType(var_name, VarType::Gradient, FuncAge::Curr);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->curlSln() : var->curlSlnOld();
  return (_c_is_implicit) ? var->curlSlnNeighbor() : var->curlSlnOldNeighbor();
}

const VectorVariableCurl &
Coupleable::coupledCurlOld(const std::string & var_name, unsigned int comp)
{
  VectorMooseVariable * var = getVectorVar(var_name, comp);
  if (!var)
    return _default_vector_curl;
  checkFuncType(var_name, VarType::Gradient, FuncAge::Old);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->curlSlnOld() : var->curlSlnOlder();
  return (_c_is_implicit) ? var->curlSlnOldNeighbor() : var->curlSlnOlderNeighbor();
}

const VectorVariableCurl &
Coupleable::coupledCurlOlder(const std::string & var_name, unsigned int comp)
{
  VectorMooseVariable * var = getVectorVar(var_name, comp);
  if (!var)
    return _default_vector_curl;
  checkFuncType(var_name, VarType::Gradient, FuncAge::Older);

  if (!_coupleable_neighbor)
    return var->curlSlnOlder();
  return var->curlSlnOlderNeighbor();
}

const VariableSecond &
Coupleable::coupledSecond(const std::string & var_name, unsigned int comp)
{
  MooseVariable * var = getVar(var_name, comp);
  if (!var)
    return _default_second;
  checkFuncType(var_name, VarType::Gradient, FuncAge::Curr);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->secondSln() : var->secondSlnOlder();
  return (_c_is_implicit) ? var->secondSlnNeighbor() : var->secondSlnOlderNeighbor();
}

const VariableSecond &
Coupleable::coupledSecondOld(const std::string & var_name, unsigned int comp)
{
  MooseVariable * var = getVar(var_name, comp);
  if (!var)
    return _default_second;
  checkFuncType(var_name, VarType::Gradient, FuncAge::Old);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->secondSlnOld() : var->secondSlnOlder();
  return (_c_is_implicit) ? var->secondSlnOldNeighbor() : var->secondSlnOlderNeighbor();
}

const VariableSecond &
Coupleable::coupledSecondOlder(const std::string & var_name, unsigned int comp)
{
  MooseVariable * var = getVar(var_name, comp);
  if (!var)
    return _default_second;
  checkFuncType(var_name, VarType::Gradient, FuncAge::Older);

  if (!_coupleable_neighbor)
    return var->secondSlnOlder();
  return var->secondSlnOlderNeighbor();
}

const VariableSecond &
Coupleable::coupledSecondPreviousNL(const std::string & var_name, unsigned int comp)
{
  MooseVariable * var = getVar(var_name, comp);
  _c_fe_problem.needsPreviousNewtonIteration(true);
  if (!var)
    return _default_second;
  checkFuncType(var_name, VarType::Gradient, FuncAge::Curr);

  if (!_coupleable_neighbor)
    return var->secondSlnPreviousNL();
  return var->secondSlnPreviousNLNeighbor();
}

template <typename T>
const T &
Coupleable::coupledNodalValue(const std::string & var_name, unsigned int comp)
{
  MooseVariableFE<T> * var = getVarHelper<T>(var_name, comp);
  if (!var)
    return getDefaultNodalValue<T>(var_name, comp);
  checkFuncType(var_name, VarType::Ignore, FuncAge::Curr);

  if (!var->isNodal())
    mooseError(_c_name,
               ": Trying to get nodal values of variable '",
               var->name(),
               "', but it is not nodal.");

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->nodalValue() : var->nodalValueOld();
  return (_c_is_implicit) ? var->nodalValueNeighbor() : var->nodalValueOldNeighbor();
}

template <typename T>
const T &
Coupleable::coupledNodalValueOld(const std::string & var_name, unsigned int comp)
{
  MooseVariableFE<T> * var = getVarHelper<T>(var_name, comp);
  if (!var)
    return getDefaultNodalValue<T>(var_name, comp);
  checkFuncType(var_name, VarType::Ignore, FuncAge::Old);

  if (!var->isNodal())
    mooseError(_c_name,
               ": Trying to get old nodal values of variable '",
               var->name(),
               "', but it is not nodal.");

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->nodalValueOld() : var->nodalValueOlder();
  return (_c_is_implicit) ? var->nodalValueOldNeighbor() : var->nodalValueOlderNeighbor();
}

template <typename T>
const T &
Coupleable::coupledNodalValueOlder(const std::string & var_name, unsigned int comp)
{
  MooseVariableFE<T> * var = getVarHelper<T>(var_name, comp);
  if (!var)
    return getDefaultNodalValue<T>(var_name, comp);
  checkFuncType(var_name, VarType::Ignore, FuncAge::Older);

  if (!var->isNodal())
    mooseError(_c_name,
               ": Trying to get older nodal values of variable '",
               var->name(),
               "', but it is not nodal.");

  if (!_coupleable_neighbor)
    return var->nodalValueOlder();
  return var->nodalValueOlderNeighbor();
}

template <typename T>
const T &
Coupleable::coupledNodalValuePreviousNL(const std::string & var_name, unsigned int comp)
{
  MooseVariableFE<T> * var = getVarHelper<T>(var_name, comp);
  if (!var)
    return getDefaultNodalValue<T>(var_name, comp);
  checkFuncType(var_name, VarType::Ignore, FuncAge::Curr);

  _c_fe_problem.needsPreviousNewtonIteration(true);

  if (!_coupleable_neighbor)
    return var->nodalValuePreviousNL();
  return var->nodalValuePreviousNLNeighbor();
}

template <typename T>
const T &
Coupleable::coupledNodalDot(const std::string & var_name, unsigned int comp)
{
  static const T zero = 0;
  MooseVariableFE<T> * var = getVarHelper<T>(var_name, comp);
  if (!var)
    return zero;
  checkFuncType(var_name, VarType::Dot, FuncAge::Curr);

  if (!_coupleable_neighbor)
    return var->nodalValueDot();
  mooseError("Neighbor version not implemented");
}

template <typename T>
const T &
Coupleable::coupledNodalDotResidual(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  static const T zero = 0;
  if (!isCoupled(var_name)) // Return default 0
    return zero;

  validateExecutionerType(var_name, "coupledNodalDotResidual");
  coupledCallback(var_name, false);
  MooseVariableFE<T> * var = getVarHelper<T>(var_name, comp);
  if (var == NULL)
    mooseError("Call corresponding vector variable method");

  if (!_coupleable_neighbor)
    return var->nodalValueDotResidual();
  else
    mooseError("Neighbor version not implemented");
}

const VariableValue &
Coupleable::coupledNodalDotDot(const std::string & var_name, unsigned int comp)
{
  MooseVariable * var = getVar(var_name, comp);
  if (!var)
    return _default_value_zero;
  checkFuncType(var_name, VarType::Dot, FuncAge::Curr);

  if (!_coupleable_neighbor)
    return var->dofValuesDotDot();
  return var->dofValuesDotDotNeighbor();
}

const VariableValue &
Coupleable::coupledNodalDotDotResidual(const std::string & var_name, unsigned int comp)
{
  checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_value_zero;

  validateExecutionerType(var_name, "coupledNodalDotDotResidual");
  coupledCallback(var_name, false);
  MooseVariable * var = getVar(var_name, comp);
  if (var == NULL)
    mooseError("Call corresponding vector variable method");

  if (!_coupleable_neighbor)
    return var->dofValuesDotDotResidual();
  else
    return var->dofValuesDotDotNeighborResidual();
}

const VariableValue &
Coupleable::coupledNodalDotOld(const std::string & var_name, unsigned int comp)
{
  MooseVariable * var = getVar(var_name, comp);
  if (!var)
    return _default_value_zero;
  checkFuncType(var_name, VarType::Dot, FuncAge::Old);

  if (!_coupleable_neighbor)
    return var->dofValuesDotOld();
  return var->dofValuesDotOldNeighbor();
}

const VariableValue &
Coupleable::coupledNodalDotDotOld(const std::string & var_name, unsigned int comp)
{
  MooseVariable * var = getVar(var_name, comp);
  if (!var)
    return _default_value_zero;
  checkFuncType(var_name, VarType::Dot, FuncAge::Old);

  if (!_coupleable_neighbor)
    return var->dofValuesDotDotOld();
  return var->dofValuesDotDotOldNeighbor();
}

const VariableValue &
Coupleable::coupledDofValues(const std::string & var_name, unsigned int comp)
{
  MooseVariable * var = getVar(var_name, comp);
  if (!var)
    return *getDefaultValue(var_name, comp);
  checkFuncType(var_name, VarType::Ignore, FuncAge::Curr);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->dofValues() : var->dofValuesOld();
  return (_c_is_implicit) ? var->dofValuesNeighbor() : var->dofValuesOldNeighbor();
}

const VariableValue &
Coupleable::coupledDofValuesOld(const std::string & var_name, unsigned int comp)
{
  MooseVariable * var = getVar(var_name, comp);
  if (!var)
    return *getDefaultValue(var_name, comp);
  checkFuncType(var_name, VarType::Ignore, FuncAge::Old);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->dofValuesOld() : var->dofValuesOlder();
  return (_c_is_implicit) ? var->dofValuesOldNeighbor() : var->dofValuesOlderNeighbor();
}

const VariableValue &
Coupleable::coupledDofValuesOlder(const std::string & var_name, unsigned int comp)
{
  MooseVariable * var = getVar(var_name, comp);
  if (!var)
    return *getDefaultValue(var_name, comp);
  checkFuncType(var_name, VarType::Ignore, FuncAge::Older);

  if (!_coupleable_neighbor)
    return var->dofValuesOlder();
  return var->dofValuesOlderNeighbor();
}

void
Coupleable::validateExecutionerType(const std::string & name, const std::string & fn_name) const
{
  if (!_c_fe_problem.isTransient())
    mooseError(_c_name,
               ": Calling \"",
               fn_name,
               "\" on variable \"",
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
VectorVariableValue *
Coupleable::getADDefaultVectorValue<RESIDUAL>(const std::string & var_name)
{
  return getDefaultVectorValue(var_name);
}

template <>
VariableGradient &
Coupleable::getADDefaultGradient<RESIDUAL>()
{
  return _default_gradient;
}

template <>
VectorVariableGradient &
Coupleable::getADDefaultVectorGradient<RESIDUAL>()
{
  return _default_vector_gradient;
}

template <>
VariableSecond &
Coupleable::getADDefaultSecond<RESIDUAL>()
{
  return _default_second;
}

template <>
const VariableValue &
Coupleable::adZeroValueTemplate<RESIDUAL>()
{
  return _zero;
}

template <>
const VariableGradient &
Coupleable::adZeroGradientTemplate<RESIDUAL>()
{
  return _grad_zero;
}

template <>
const VariableSecond &
Coupleable::adZeroSecondTemplate<RESIDUAL>()
{
  return _second_zero;
}

template <typename T, ComputeStage compute_stage>
const typename Moose::ValueType<T, compute_stage>::type &
Coupleable::adCoupledNodalValueTemplate(const std::string & var_name, unsigned int comp)
{
  static const typename Moose::ValueType<T, compute_stage>::type zero = 0;
  if (!isCoupled(var_name))
    return zero;

  if (!_c_nodal)
    mooseError("The adCoupledNodalValue method should only be called for nodal computing objects");
  if (_coupleable_neighbor)
    mooseError(
        "The adCoupledNodalValue method shouldn't be called for neighbor computing objects. I "
        "don't even know what that would mean, although maybe someone could explain it to me.");
  if (!_c_is_implicit)
    mooseError("If you're going to use an explicit scheme, then use coupledNodalValue instead of "
               "adCoupledNodalValue");

  coupledCallback(var_name, false);
  MooseVariableFE<T> * var = getVarHelper<T>(var_name, comp);

  return var->template adNodalValue<compute_stage>();
}

// Explicit instantiations

template const Real & Coupleable::getDefaultNodalValue<Real>(const std::string & var_name,
                                                             unsigned int comp);

template MooseVariableFE<Real> * Coupleable::getVarHelper<Real>(const std::string & var_name,
                                                                unsigned int comp);

template MooseVariableFE<RealVectorValue> *
Coupleable::getVarHelper<RealVectorValue>(const std::string & var_name, unsigned int comp);

template const Real & Coupleable::coupledNodalValue<Real>(const std::string & var_name,
                                                          unsigned int comp);
template const RealVectorValue &
Coupleable::coupledNodalValue<RealVectorValue>(const std::string & var_name, unsigned int comp);
template const Real & Coupleable::coupledNodalValueOld<Real>(const std::string & var_name,
                                                             unsigned int comp);
template const RealVectorValue &
Coupleable::coupledNodalValueOld<RealVectorValue>(const std::string & var_name, unsigned int comp);
template const Real & Coupleable::coupledNodalValueOlder<Real>(const std::string & var_name,
                                                               unsigned int comp);
template const RealVectorValue &
Coupleable::coupledNodalValueOlder<RealVectorValue>(const std::string & var_name,
                                                    unsigned int comp);
template const Real & Coupleable::coupledNodalValuePreviousNL<Real>(const std::string & var_name,
                                                                    unsigned int comp);
template const RealVectorValue &
Coupleable::coupledNodalValuePreviousNL<RealVectorValue>(const std::string & var_name,
                                                         unsigned int comp);
template const Real & Coupleable::coupledNodalDot<Real>(const std::string & var_name,
                                                        unsigned int comp);
template const Real & Coupleable::coupledNodalDotResidual<Real>(const std::string & var_name,
                                                                unsigned int comp);
template const RealVectorValue &
Coupleable::coupledNodalDot<RealVectorValue>(const std::string & var_name, unsigned int comp);

template const RealVectorValue &
Coupleable::coupledNodalDotResidual<RealVectorValue>(const std::string & var_name,
                                                     unsigned int comp);

template const Real &
Coupleable::adCoupledNodalValueTemplate<Real, RESIDUAL>(const std::string & var_name,
                                                        unsigned int comp);
template const RealVectorValue &
Coupleable::adCoupledNodalValueTemplate<RealVectorValue, RESIDUAL>(const std::string & var_name,
                                                                   unsigned int comp);
template const DualReal &
Coupleable::adCoupledNodalValueTemplate<Real, JACOBIAN>(const std::string & var_name,
                                                        unsigned int comp);
template const libMesh::VectorValue<DualReal> &
Coupleable::adCoupledNodalValueTemplate<RealVectorValue, JACOBIAN>(const std::string & var_name,
                                                                   unsigned int comp);
