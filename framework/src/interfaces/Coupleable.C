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
#include "SystemBase.h"
#include "AuxiliarySystem.h"

#include "AuxKernel.h"
#include "ElementUserObject.h"
#include "NodalUserObject.h"

Coupleable::Coupleable(const MooseObject * moose_object, bool nodal, bool is_fv)
  : _c_parameters(moose_object->parameters()),
    _c_name(_c_parameters.get<std::string>("_object_name")),
    _c_type(_c_parameters.get<std::string>("_type")),
    _c_fe_problem(*_c_parameters.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _c_sys(_c_parameters.isParamValid("_sys") ? _c_parameters.get<SystemBase *>("_sys") : nullptr),
    _new_to_deprecated_coupled_vars(_c_parameters.getNewToDeprecatedVarMap()),
    _c_nodal(nodal),
    _c_is_implicit(_c_parameters.have_parameter<bool>("implicit")
                       ? _c_parameters.get<bool>("implicit")
                       : true),
    _c_allow_element_to_nodal_coupling(
        _c_parameters.have_parameter<bool>("_allow_nodal_to_elemental_coupling")
            ? _c_parameters.get<bool>("_allow_nodal_to_elemental_coupling")
            : false),
    _c_tid(_c_parameters.get<THREAD_ID>("_tid")),
    _zero(_c_fe_problem._zero[_c_tid]),
    _phi_zero(_c_fe_problem._phi_zero[_c_tid]),
    _ad_zero(_c_fe_problem._ad_zero[_c_tid]),
    _grad_zero(_c_fe_problem._grad_zero[_c_tid]),
    _ad_grad_zero(_c_fe_problem._ad_grad_zero[_c_tid]),
    _grad_phi_zero(_c_fe_problem._grad_phi_zero[_c_tid]),
    _second_zero(_c_fe_problem._second_zero[_c_tid]),
    _ad_second_zero(_c_fe_problem._ad_second_zero[_c_tid]),
    _second_phi_zero(_c_fe_problem._second_phi_zero[_c_tid]),
    _vector_zero(_c_fe_problem._vector_zero[_c_tid]),
    _vector_curl_zero(_c_fe_problem._vector_curl_zero[_c_tid]),
    _coupleable_neighbor(_c_parameters.have_parameter<bool>("_neighbor")
                             ? _c_parameters.get<bool>("_neighbor")
                             : false),
    _coupleable_max_qps(Moose::constMaxQpsPerElem),
    _is_fv(is_fv),
    _obj(moose_object),
    _writable_coupled_variables(libMesh::n_threads())
{
  SubProblem & problem = *_c_parameters.getCheckedPointerParam<SubProblem *>("_subproblem");
  _obj->getMooseApp().registerInterfaceObject(*this);

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
          MooseVariableFieldBase * moose_var =
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
          else if (auto * tmp_var = dynamic_cast<MooseVariableFV<Real> *>(moose_var))
          {
            // We are using a finite volume variable through add*CoupledVar as opposed to getFunctor
            // so we can be reasonably confident that the variable values will be obtained using
            // traditional pre-evaluation and quadrature point indexing
            tmp_var->requireQpComputations();
            _coupled_standard_fv_moose_vars.push_back(tmp_var);
          }
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
}

bool
Coupleable::isCoupled(const std::string & var_name_in, unsigned int i) const
{
  const auto var_name = _c_parameters.checkForRename(var_name_in);

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

bool
Coupleable::isCoupledConstant(const std::string & var_name) const
{
  return _c_parameters.hasDefaultCoupledValue(var_name);
}

unsigned int
Coupleable::coupledComponents(const std::string & var_name_in) const
{
  const auto var_name = _c_parameters.checkForRename(var_name_in);

  if (isCoupled(var_name))
  {
    mooseAssert(_coupled_vars.find(var_name) != _coupled_vars.end(),
                var_name << " must not actually be coupled!");
    return _coupled_vars.at(var_name).size();
  }
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
Coupleable::checkFuncType(const std::string var_name, VarType t, FuncAge age) const
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

  coupledCallback(var_name, age == FuncAge::Old || age == FuncAge::Older);
}

bool
Coupleable::checkVar(const std::string & var_name_in,
                     unsigned int comp,
                     unsigned int comp_bound) const
{
  const auto var_name = _c_parameters.checkForRename(var_name_in);
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

  auto vars_vector_it = _coupled_vars.find(var_name);
  if (vars_vector_it == _coupled_vars.end())
    mooseError(_c_name, ": Trying to get a coupled var ", var_name, " that doesn't exist");

  const auto & vars_vector = vars_vector_it->second;

  auto bound = comp_bound ? comp_bound : vars_vector.size();
  checkComponent(_obj, comp, bound, var_name);

  // We should know we have a variable now
  const auto * var = vars_vector[comp];
  if (!var)
    mooseError(
        _c_name,
        ": We did all our checks for the existence of a var, yet we still don't have a var!?");

  // Only perform the following checks for objects that feed into residuals/Jacobians, e.g. objects
  // that inherit from the TaggingInterface
  if (_c_parameters.have_parameter<MultiMooseEnum>("vector_tags"))
  {
    // Are we attempting to couple to a non-FV var in an FV object?
    if (!var->isFV() && _is_fv)
      mooseError("Attempting to couple non-FV variable ",
                 var->name(),
                 " into an FV object ",
                 _c_name,
                 ". This is not currently supported");
  }

  if (!(vars_vector[comp])->isNodal() && _c_nodal && !_c_allow_element_to_nodal_coupling)
    mooseError(_c_name, ": cannot couple elemental variables into nodal objects");

  return true;
}

const MooseVariableFieldBase *
Coupleable::getFEVar(const std::string & var_name, unsigned int comp) const
{
  mooseDeprecated("Coupleable::getFEVar is deprecated. Please use Coupleable::getFieldVar instead. "
                  "Note that this method could potentially return a finite volume variable");
  return getFieldVar(var_name, comp);
}

MooseVariableFieldBase *
Coupleable::getFieldVar(const std::string & var_name, unsigned int comp)
{
  return getVarHelper<MooseVariableFieldBase>(var_name, comp);
}

const MooseVariableFieldBase *
Coupleable::getFieldVar(const std::string & var_name, unsigned int comp) const
{
  return getVarHelper<MooseVariableFieldBase>(var_name, comp);
}

MooseVariable *
Coupleable::getVar(const std::string & var_name, unsigned int comp)
{
  return const_cast<MooseVariable *>(getVarHelper<MooseVariable>(var_name, comp));
}

VectorMooseVariable *
Coupleable::getVectorVar(const std::string & var_name, unsigned int comp)
{
  auto * const var =
      const_cast<VectorMooseVariable *>(getVarHelper<VectorMooseVariable>(var_name, comp));

  if (_c_nodal && var && var->feType().family != LAGRANGE_VEC)
    mooseError(_c_name, ": Only LAGRANGE_VEC vector variables are defined at nodes");

  return var;
}

ArrayMooseVariable *
Coupleable::getArrayVar(const std::string & var_name, unsigned int comp)
{
  return const_cast<ArrayMooseVariable *>(getVarHelper<ArrayMooseVariable>(var_name, comp));
}

const MooseVariable *
Coupleable::getVar(const std::string & var_name, unsigned int comp) const
{
  return getVarHelper<MooseVariable>(var_name, comp);
}

const VectorMooseVariable *
Coupleable::getVectorVar(const std::string & var_name, unsigned int comp) const
{
  const auto * const var = getVarHelper<VectorMooseVariable>(var_name, comp);

  if (_c_nodal && var && var->feType().family != LAGRANGE_VEC)
    mooseError(_c_name, ": Only LAGRANGE_VEC vector variables are defined at nodes");

  return var;
}

const ArrayMooseVariable *
Coupleable::getArrayVar(const std::string & var_name, unsigned int comp) const
{
  return getVarHelper<ArrayMooseVariable>(var_name, comp);
}

const VariableValue *
Coupleable::getDefaultValue(const std::string & var_name, unsigned int comp) const
{
  // make sure we don't access values that were not provided
  checkComponent(_obj, comp, _c_parameters.numberDefaultCoupledValues(var_name), var_name);

  auto default_value_it = _default_value.find(var_name);
  if (default_value_it == _default_value.end())
  {
    _default_value[var_name].emplace_back(std::make_unique<VariableValue>(
        _coupleable_max_qps, _c_parameters.defaultCoupledValue(var_name, 0)));
    for (unsigned int j = 1; j < _c_parameters.numberDefaultCoupledValues(var_name); ++j)
      _default_value[var_name].emplace_back(std::make_unique<VariableValue>(
          _coupleable_max_qps, _c_parameters.defaultCoupledValue(var_name, j)));
    default_value_it = _default_value.find(var_name);
  }

  return default_value_it->second[comp].get();
}

const VectorVariableValue *
Coupleable::getDefaultVectorValue(const std::string & var_name) const
{
  auto default_value_it = _default_vector_value.find(var_name);
  if (default_value_it == _default_vector_value.end())
  {
    auto value = std::make_unique<VectorVariableValue>(_coupleable_max_qps, 0);
    bool already_warned = false;
    for (unsigned int qp = 0; qp < _coupleable_max_qps; ++qp)
      for (const auto i : make_range(Moose::dim))
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

const ArrayVariableValue *
Coupleable::getDefaultArrayValue(const std::string & var_name) const
{
  auto default_value_it = _default_array_value.find(var_name);
  if (default_value_it == _default_array_value.end())
  {
    auto value = std::make_unique<ArrayVariableValue>(_coupleable_max_qps);
    for (unsigned int qp = 0; qp < _coupleable_max_qps; ++qp)
    {
      auto n = _c_parameters.numberDefaultCoupledValues(var_name);
      (*value)[qp].resize(n);
      for (unsigned int i = 0; i < n; ++i)
        (*value)[qp](i) = _c_parameters.defaultCoupledValue(var_name, i);
    }
    default_value_it =
        _default_array_value.insert(std::make_pair(var_name, std::move(value))).first;
  }

  return default_value_it->second.get();
}

template <typename T>
const T &
Coupleable::getDefaultNodalValue(const std::string & var_name, unsigned int comp) const
{
  auto && default_variable_value = getDefaultValue(var_name, comp);
  return *default_variable_value->data();
}

template <>
const RealVectorValue &
Coupleable::getDefaultNodalValue<RealVectorValue>(const std::string & var_name, unsigned int) const
{
  auto && default_variable_value = getDefaultVectorValue(var_name);
  return *default_variable_value->data();
}

template <>
const RealEigenVector &
Coupleable::getDefaultNodalValue<RealEigenVector>(const std::string & var_name, unsigned int) const
{
  auto && default_variable_value = getDefaultArrayValue(var_name);
  return *default_variable_value->data();
}

unsigned int
Coupleable::coupled(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getFieldVar(var_name, comp);
  if (!var)
  {
    mooseAssert(_optional_var_index.find(var_name) != _optional_var_index.end(),
                "optional var index for " << var_name << " does not exist!");
    // make sure we don't try to access default var ids that were not provided
    checkComponent(_obj, comp, _optional_var_index.at(var_name).size(), var_name);
    return _optional_var_index.at(var_name)[comp];
  }
  checkFuncType(var_name, VarType::Ignore, FuncAge::Curr);

  if (var->kind() == Moose::VAR_NONLINEAR &&
      // are we not an object that feeds into the nonlinear system?
      (!_c_sys || _c_sys->varKind() != Moose::VAR_NONLINEAR ||
       // are we an object that impacts the nonlinear system and this variable is within our
       // nonlinear system?
       var->sys().number() == _c_sys->number()))
    return var->number();
  else
    // Avoid registering coupling to variables outside of our system (e.g. avoid potentially
    // creating bad Jacobians)
    return std::numeric_limits<unsigned int>::max() - var->number();
}

template <>
const GenericVariableValue<false> &
Coupleable::coupledGenericValue<false>(const std::string & var_name, unsigned int comp) const
{
  return coupledValue(var_name, comp);
}

template <>
const GenericVariableValue<true> &
Coupleable::coupledGenericValue<true>(const std::string & var_name, unsigned int comp) const
{
  return adCoupledValue(var_name, comp);
}

const VariableValue &
Coupleable::coupledValue(const std::string & var_name, unsigned int comp) const
{
  const auto * const var = getVarHelper<MooseVariableField<Real>>(var_name, comp);
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

template <typename T>
const typename OutputTools<T>::VariableValue &
Coupleable::vectorTagValueHelper(const std::string & var_names,
                                 const TagID tag,
                                 const unsigned int index) const
{
  const auto * const var = getVarHelper<MooseVariableField<T>>(var_names, index);
  if (!var)
    mooseError(var_names, ": invalid variable name for coupledVectorTagValue");
  checkFuncType(var_names, VarType::Ignore, FuncAge::Curr);

  if (!_c_fe_problem.vectorTagExists(tag))
    mooseError("Attempting to couple to vector tag with ID ",
               tag,
               "in ",
               _c_name,
               ", but a vector tag with that ID does not exist");

  const_cast<Coupleable *>(this)->addFEVariableCoupleableVectorTag(tag);

  if (_c_nodal)
    return var->nodalVectorTagValue(tag);
  else
    return var->vectorTagValue(tag);
}

template <typename T>
void
Coupleable::requestStates(const std::string & var_name,
                          const TagName & tag_name,
                          const unsigned int comp)
{
  auto var =
      const_cast<MooseVariableField<T> *>(getVarHelper<MooseVariableField<T>>(var_name, comp));
  if (!var)
    mooseError(var_name, ": invalid variable name for tag coupling");

  auto & var_sys = var->sys();
  if (tag_name == Moose::OLD_SOLUTION_TAG)
    var_sys.needSolutionState(1);
  else if (tag_name == Moose::OLDER_SOLUTION_TAG)
    var_sys.needSolutionState(2);
}

template <typename T>
const typename OutputTools<T>::VariableValue &
Coupleable::vectorTagValueHelper(const std::string & var_names,
                                 const std::string & tag_param_name,
                                 const unsigned int index) const
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  const TagName tag_name = MooseUtils::toUpper(_c_parameters.get<TagName>(tag_param_name));

  const bool older_state_tag = _older_state_tags.count(tag_name);
  if (older_state_tag)
    // We may need to add solution states and create vector tags
    const_cast<Coupleable *>(this)->requestStates<T>(var_names, tag_name, index);

  if (!_c_fe_problem.vectorTagExists(tag_name))
    mooseError("Tagged vector with tag name '", tag_name, "' does not exist");

  TagID tag = _c_fe_problem.getVectorTagID(tag_name);
  return vectorTagValueHelper<T>(var_names, tag, index);
}

template <>
const GenericVariableValue<false> &
Coupleable::coupledGenericDofValue<false>(const std::string & var_name, unsigned int comp) const
{
  return coupledDofValues(var_name, comp);
}

template <>
const GenericVariableValue<true> &
Coupleable::coupledGenericDofValue<true>(const std::string & var_name, unsigned int comp) const
{
  return adCoupledDofValues(var_name, comp);
}

const VariableValue &
Coupleable::coupledValueLower(const std::string & var_name, const unsigned int comp) const
{
  const auto * var = getVar(var_name, comp);
  if (!var)
    return *getDefaultValue(var_name, comp);
  checkFuncType(var_name, VarType::Ignore, FuncAge::Curr);

  if (_coupleable_neighbor)
    mooseError(_c_name, ":coupledValueLower cannot be called in a coupleable neighbor object");

  if (_c_nodal)
    return (_c_is_implicit) ? var->dofValues() : var->dofValuesOld();
  else
    return (_c_is_implicit) ? var->slnLower() : var->slnLowerOld();
}

const VariableValue &
Coupleable::coupledVectorTagValue(const std::string & var_names,
                                  TagID tag,
                                  unsigned int index) const
{
  return vectorTagValueHelper<Real>(var_names, tag, index);
}

const VariableValue &
Coupleable::coupledVectorTagValue(const std::string & var_names,
                                  const std::string & tag_name,
                                  unsigned int index) const
{
  return vectorTagValueHelper<Real>(var_names, tag_name, index);
}

const ArrayVariableValue &
Coupleable::coupledVectorTagArrayValue(const std::string & var_names,
                                       TagID tag,
                                       unsigned int index) const
{
  return vectorTagValueHelper<RealEigenVector>(var_names, tag, index);
}

const ArrayVariableValue &
Coupleable::coupledVectorTagArrayValue(const std::string & var_names,
                                       const std::string & tag_name,
                                       unsigned int index) const
{
  return vectorTagValueHelper<RealEigenVector>(var_names, tag_name, index);
}

const VariableGradient &
Coupleable::coupledVectorTagGradient(const std::string & var_names,
                                     TagID tag,
                                     unsigned int index) const
{
  const auto * var = getVar(var_names, index);
  if (!var)
    mooseError(var_names, ": invalid variable name for coupledVectorTagGradient");
  checkFuncType(var_names, VarType::Ignore, FuncAge::Curr);

  if (!_c_fe_problem.vectorTagExists(tag))
    mooseError("Attempting to couple to vector tag with ID ",
               tag,
               "in ",
               _c_name,
               ", but a vector tag with that ID does not exist");

  const_cast<Coupleable *>(this)->addFEVariableCoupleableVectorTag(tag);

  return var->vectorTagGradient(tag);
}

const VariableGradient &
Coupleable::coupledVectorTagGradient(const std::string & var_names,
                                     const std::string & tag_name,
                                     unsigned int index) const
{
  if (!_c_parameters.isParamValid(tag_name))
    mooseError("Tag name parameter '", tag_name, "' is invalid");

  TagName tagname = _c_parameters.get<TagName>(tag_name);
  if (!_c_fe_problem.vectorTagExists(tagname))
    mooseError("Tagged vector with tag name '", tagname, "' does not exist");

  TagID tag = _c_fe_problem.getVectorTagID(tagname);
  return coupledVectorTagGradient(var_names, tag, index);
}

const ArrayVariableGradient &
Coupleable::coupledVectorTagArrayGradient(const std::string & var_names,
                                          TagID tag,
                                          unsigned int index) const
{
  const auto * var = getArrayVar(var_names, index);
  if (!var)
    mooseError(var_names, ": invalid variable name for coupledVectorTagArrayGradient");
  checkFuncType(var_names, VarType::Ignore, FuncAge::Curr);

  if (!_c_fe_problem.vectorTagExists(tag))
    mooseError("Attempting to couple to vector tag with ID ",
               tag,
               "in ",
               _c_name,
               ", but a vector tag with that ID does not exist");

  const_cast<Coupleable *>(this)->addFEVariableCoupleableVectorTag(tag);

  return var->vectorTagGradient(tag);
}

const ArrayVariableGradient &
Coupleable::coupledVectorTagArrayGradient(const std::string & var_names,
                                          const std::string & tag_name,
                                          unsigned int index) const
{
  if (!_c_parameters.isParamValid(tag_name))
    mooseError("Tag name parameter '", tag_name, "' is invalid");

  TagName tagname = _c_parameters.get<TagName>(tag_name);
  if (!_c_fe_problem.vectorTagExists(tagname))
    mooseError("Tagged vector with tag name '", tagname, "' does not exist");

  TagID tag = _c_fe_problem.getVectorTagID(tagname);
  return coupledVectorTagArrayGradient(var_names, tag, index);
}

template <typename T>
const typename OutputTools<T>::VariableValue &
Coupleable::vectorTagDofValueHelper(const std::string & var_name,
                                    const TagID tag,
                                    const unsigned int comp) const
{
  const auto * var = getVarHelper<MooseVariableField<T>>(var_name, comp);
  if (!var)
    mooseError(var_name, ": invalid variable name for coupledVectorTagDofValue");
  checkFuncType(var_name, VarType::Ignore, FuncAge::Curr);

  const_cast<Coupleable *>(this)->addFEVariableCoupleableVectorTag(tag);

  return var->vectorTagDofValue(tag);
}

template <typename T>
const typename OutputTools<T>::VariableValue &
Coupleable::vectorTagDofValueHelper(const std::string & var_name,
                                    const std::string & tag_param_name,
                                    const unsigned int comp) const
{
  if (!_c_parameters.isParamValid(tag_param_name))
    mooseError("Tag name parameter '", tag_param_name, "' is invalid");

  const TagName tag_name = MooseUtils::toUpper(_c_parameters.get<TagName>(tag_param_name));

  const bool older_state_tag = _older_state_tags.count(tag_name);
  if (older_state_tag)
    // We may need to add solution states and create vector tags
    const_cast<Coupleable *>(this)->requestStates<T>(var_name, tag_name, comp);

  if (!_c_fe_problem.vectorTagExists(tag_name))
    mooseError("Tagged vector with tag name '", tag_name, "' does not exist");

  TagID tag = _c_fe_problem.getVectorTagID(tag_name);

  return vectorTagDofValueHelper<T>(var_name, tag, comp);
}

const VariableValue &
Coupleable::coupledVectorTagDofValue(const std::string & var_name,
                                     TagID tag,
                                     unsigned int comp) const
{
  return vectorTagDofValueHelper<Real>(var_name, tag, comp);
}

const VariableValue &
Coupleable::coupledVectorTagDofValue(const std::string & var_name,
                                     const std::string & tag_name,
                                     unsigned int comp) const
{
  return vectorTagDofValueHelper<Real>(var_name, tag_name, comp);
}

const ArrayVariableValue &
Coupleable::coupledVectorTagArrayDofValue(const std::string & var_name,
                                          const std::string & tag_name) const
{
  return vectorTagDofValueHelper<RealEigenVector>(var_name, tag_name);
}

const VariableValue &
Coupleable::coupledMatrixTagValue(const std::string & var_names,
                                  TagID tag,
                                  unsigned int index) const
{
  const auto * var = getVar(var_names, index);
  if (!var)
    mooseError(var_names, ": invalid variable name for coupledMatrixTagValue");
  checkFuncType(var_names, VarType::Ignore, FuncAge::Curr);

  const_cast<Coupleable *>(this)->addFEVariableCoupleableMatrixTag(tag);

  if (_c_nodal)
    return var->nodalMatrixTagValue(tag);
  return var->matrixTagValue(tag);
}

const VariableValue &
Coupleable::coupledMatrixTagValue(const std::string & var_names,
                                  const std::string & tag_name,
                                  unsigned int index) const
{
  if (!_c_parameters.isParamValid(tag_name))
    mooseError("Tag name parameter '", tag_name, "' is invalid");

  TagName tagname = _c_parameters.get<TagName>(tag_name);
  if (!_c_fe_problem.matrixTagExists(tagname))
    mooseError("Matrix tag name '", tagname, "' does not exist");

  TagID tag = _c_fe_problem.getMatrixTagID(tagname);
  return coupledMatrixTagValue(var_names, tag, index);
}

const VectorVariableValue &
Coupleable::coupledVectorValue(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVectorVar(var_name, comp);
  if (!var)
    return *getDefaultVectorValue(var_name);
  checkFuncType(var_name, VarType::Ignore, FuncAge::Curr);

  if (!_coupleable_neighbor)
  {
    if (_c_nodal)
      return _c_is_implicit ? var->nodalValueArray() : var->nodalValueOldArray();
    else
      return _c_is_implicit ? var->sln() : var->slnOld();
  }
  else
  {
    if (_c_nodal)
      // Since this is at a node, I don't feel like there should be any "neighbor" logic
      return _c_is_implicit ? var->nodalValueArray() : var->nodalValueOldArray();
    else
      return _c_is_implicit ? var->slnNeighbor() : var->slnOldNeighbor();
  }
}

const ArrayVariableValue &
Coupleable::coupledArrayValue(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getArrayVar(var_name, comp);
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

std::vector<const ArrayVariableValue *>
Coupleable::coupledArrayValues(const std::string & var_name) const
{
  auto func = [this, &var_name](unsigned int comp) { return &coupledArrayValue(var_name, comp); };
  return coupledVectorHelper<const ArrayVariableValue *>(var_name, func);
}

MooseVariable &
Coupleable::writableVariable(const std::string & var_name, unsigned int comp)
{
  auto * var = dynamic_cast<MooseVariable *>(getVar(var_name, comp));

  const auto * aux = dynamic_cast<const AuxKernel *>(this);
  const auto * euo = dynamic_cast<const ElementUserObject *>(this);
  const auto * nuo = dynamic_cast<const NodalUserObject *>(this);

  if (!aux && !euo && !nuo)
    mooseError("writableVariable() can only be called from AuxKernels, ElementUserObjects, or "
               "NodalUserObjects. '",
               _obj->name(),
               "' is neither of those.");

  if (aux && !aux->isNodal() && var->isNodal())
    mooseError("The elemental AuxKernel '",
               _obj->name(),
               "' cannot obtain a writable reference to the nodal variable '",
               var->name(),
               "'.");
  if (euo && var->isNodal())
    mooseError("The ElementUserObject '",
               _obj->name(),
               "' cannot obtain a writable reference to the nodal variable '",
               var->name(),
               "'.");

  // make sure only one object can access a variable
  checkWritableVar(var);

  return *var;
}

VariableValue &
Coupleable::writableCoupledValue(const std::string & var_name, unsigned int comp)
{
  mooseDeprecated("Coupleable::writableCoupledValue is deprecated, please use "
                  "Coupleable::writableVariable instead. ");

  // check if the variable exists
  auto * const var = getVar(var_name, comp);
  if (!var)
    mooseError(
        "Unable to create a writable reference for '", var_name, "', is it a constant expression?");

  // is the requested variable an AuxiliaryVariable?
  if (!_c_fe_problem.getAuxiliarySystem().hasVariable(var->name()))
    mooseError(
        "'", var->name(), "' must be an auxiliary variable in Coupleable::writableCoupledValue");

  // check that the variable type (elemental/nodal) is compatible with the object type
  const auto * aux = dynamic_cast<const AuxKernel *>(this);

  if (!aux)
    mooseError("writableCoupledValue() can only be called from AuxKernels, but '",
               _obj->name(),
               "' is not an AuxKernel.");

  if (!aux->isNodal() && var->isNodal())
    mooseError("The elemental AuxKernel '",
               _obj->name(),
               "' cannot obtain a writable reference to the nodal variable '",
               var->name(),
               "'.");

  // make sure only one object can access a variable
  checkWritableVar(var);

  return const_cast<VariableValue &>(coupledValue(var_name, comp));
}

void
Coupleable::checkWritableVar(MooseVariable * var)
{
  // check block restrictions for compatibility
  const auto * br = dynamic_cast<const BlockRestrictable *>(this);
  if (!var->hasBlocks(br->blockIDs()))
    mooseError("The variable '",
               var->name(),
               "' must be defined on all blocks '",
               _obj->name(),
               "' is defined on");

  // make sure only one object can access a variable
  for (const auto & ci : _obj->getMooseApp().getInterfaceObjects<Coupleable>())
    if (ci != this && ci->_writable_coupled_variables[_c_tid].count(var))
    {
      // if both this and ci are block restrictable then we check if the block restrictions
      // are not overlapping. If they don't we permit the call.
      const auto * br_other = dynamic_cast<const BlockRestrictable *>(ci);
      if (br && br_other && br->blockRestricted() && br_other->blockRestricted() &&
          !MooseUtils::setsIntersect(br->blockIDs(), br_other->blockIDs()))
        continue;

      mooseError("'",
                 ci->_obj->name(),
                 "' already obtained a writable reference to '",
                 var->name(),
                 "'. Only one object can obtain such a reference per variable and subdomain in a "
                 "simulation.");
    }

  // var is unique across threads, so we could forego having a separate set per thread, but we
  // need quick access to the list of all variables that need to be inserted into the solution
  // vector by a given thread.
  _writable_coupled_variables[_c_tid].insert(var);
}

const VariableValue &
Coupleable::coupledValueOld(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVar(var_name, comp);
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
Coupleable::coupledValueOlder(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVar(var_name, comp);
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
Coupleable::coupledValuePreviousNL(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVar(var_name, comp);
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
Coupleable::coupledVectorValueOld(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVectorVar(var_name, comp);
  if (!var)
    return *getDefaultVectorValue(var_name);
  checkFuncType(var_name, VarType::Ignore, FuncAge::Old);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->slnOld() : var->slnOlder();
  return (_c_is_implicit) ? var->slnOldNeighbor() : var->slnOlderNeighbor();
}

const VectorVariableValue &
Coupleable::coupledVectorValueOlder(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVectorVar(var_name, comp);
  if (!var)
    return *getDefaultVectorValue(var_name);
  checkFuncType(var_name, VarType::Ignore, FuncAge::Older);

  if (!_coupleable_neighbor)
    return var->slnOlder();
  return var->slnOlderNeighbor();
}

const ArrayVariableValue &
Coupleable::coupledArrayValueOld(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getArrayVar(var_name, comp);
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
Coupleable::coupledArrayValueOlder(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getArrayVar(var_name, comp);
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
Coupleable::coupledDot(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVar(var_name, comp);
  if (!var)
  {
    _default_value_zero.resize(_coupleable_max_qps, 0);
    return _default_value_zero;
  }
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
Coupleable::coupledDotDot(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVar(var_name, comp);
  if (!var)
  {
    _default_value_zero.resize(_coupleable_max_qps, 0);
    return _default_value_zero;
  }
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
Coupleable::coupledDotOld(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVar(var_name, comp);
  if (!var)
  {
    _default_value_zero.resize(_coupleable_max_qps, 0);
    return _default_value_zero;
  }
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
Coupleable::coupledDotDotOld(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVar(var_name, comp);
  if (!var)
  {
    _default_value_zero.resize(_coupleable_max_qps, 0);
    return _default_value_zero;
  }
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
Coupleable::coupledVectorDot(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVectorVar(var_name, comp);
  if (!var)
  {
    _default_vector_value_zero.resize(_coupleable_max_qps);
    return _default_vector_value_zero;
  }
  checkFuncType(var_name, VarType::Dot, FuncAge::Curr);

  if (!_coupleable_neighbor)
    return var->uDot();
  return var->uDotNeighbor();
}

const VectorVariableValue &
Coupleable::coupledVectorDotDot(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVectorVar(var_name, comp);
  if (!var)
  {
    _default_vector_value_zero.resize(_coupleable_max_qps);
    return _default_vector_value_zero;
  }
  checkFuncType(var_name, VarType::Dot, FuncAge::Curr);

  if (!_coupleable_neighbor)
    return var->uDotDot();
  return var->uDotDotNeighbor();
}

const VectorVariableValue &
Coupleable::coupledVectorDotOld(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVectorVar(var_name, comp);
  if (!var)
  {
    _default_vector_value_zero.resize(_coupleable_max_qps);
    return _default_vector_value_zero;
  }
  checkFuncType(var_name, VarType::Dot, FuncAge::Old);

  if (!_coupleable_neighbor)
    return var->uDotOld();
  return var->uDotOldNeighbor();
}

const VectorVariableValue &
Coupleable::coupledVectorDotDotOld(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVectorVar(var_name, comp);
  if (!var)
  {
    _default_vector_value_zero.resize(_coupleable_max_qps);
    return _default_vector_value_zero;
  }
  checkFuncType(var_name, VarType::Dot, FuncAge::Old);

  if (!_coupleable_neighbor)
    return var->uDotDotOld();
  return var->uDotDotOldNeighbor();
}

const VariableValue &
Coupleable::coupledVectorDotDu(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVectorVar(var_name, comp);
  if (!var)
  {
    _default_value_zero.resize(_coupleable_max_qps, 0);
    return _default_value_zero;
  }
  checkFuncType(var_name, VarType::Dot, FuncAge::Curr);

  if (!_coupleable_neighbor)
    return var->duDotDu();
  return var->duDotDuNeighbor();
}

const VariableValue &
Coupleable::coupledVectorDotDotDu(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVectorVar(var_name, comp);
  if (!var)
  {
    _default_value_zero.resize(_coupleable_max_qps, 0);
    return _default_value_zero;
  }
  checkFuncType(var_name, VarType::Dot, FuncAge::Curr);

  if (!_coupleable_neighbor)
    return var->duDotDotDu();
  return var->duDotDotDuNeighbor();
}

const ArrayVariableValue &
Coupleable::coupledArrayDot(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getArrayVar(var_name, comp);
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
Coupleable::coupledArrayDotDot(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getArrayVar(var_name, comp);
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
Coupleable::coupledArrayDotOld(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getArrayVar(var_name, comp);
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
Coupleable::coupledArrayDotDotOld(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getArrayVar(var_name, comp);
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
Coupleable::coupledDotDu(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVar(var_name, comp);
  if (!var)
  {
    _default_value_zero.resize(_coupleable_max_qps, 0);
    return _default_value_zero;
  }
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
Coupleable::coupledDotDotDu(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVar(var_name, comp);
  if (!var)
  {
    _default_value_zero.resize(_coupleable_max_qps, 0);
    return _default_value_zero;
  }
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
Coupleable::coupledGradient(const std::string & var_name, unsigned int comp) const
{
  const auto * const var = getVarHelper<MooseVariableField<Real>>(var_name, comp);
  if (!var)
  {
    _default_gradient.resize(_coupleable_max_qps);
    return _default_gradient;
  }
  checkFuncType(var_name, VarType::Gradient, FuncAge::Curr);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->gradSln() : var->gradSlnOld();
  return (_c_is_implicit) ? var->gradSlnNeighbor() : var->gradSlnOldNeighbor();
}

const VariableGradient &
Coupleable::coupledGradientOld(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVar(var_name, comp);
  if (!var)
  {
    _default_gradient.resize(_coupleable_max_qps);
    return _default_gradient;
  }
  checkFuncType(var_name, VarType::Gradient, FuncAge::Old);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->gradSlnOld() : var->gradSlnOlder();
  return (_c_is_implicit) ? var->gradSlnOldNeighbor() : var->gradSlnOlderNeighbor();
}

const VariableGradient &
Coupleable::coupledGradientOlder(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVar(var_name, comp);
  if (!var)
  {
    _default_gradient.resize(_coupleable_max_qps);
    return _default_gradient;
  }
  checkFuncType(var_name, VarType::Gradient, FuncAge::Older);

  if (!_coupleable_neighbor)
    return var->gradSlnOlder();
  return var->gradSlnOlderNeighbor();
}

const VariableGradient &
Coupleable::coupledGradientPreviousNL(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVar(var_name, comp);
  _c_fe_problem.needsPreviousNewtonIteration(true);
  if (!var)
  {
    _default_gradient.resize(_coupleable_max_qps);
    return _default_gradient;
  }
  checkFuncType(var_name, VarType::Gradient, FuncAge::Curr);

  if (!_coupleable_neighbor)
    return var->gradSlnPreviousNL();
  return var->gradSlnPreviousNLNeighbor();
}

const VariableGradient &
Coupleable::coupledGradientDot(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVar(var_name, comp);
  if (!var)
  {
    _default_gradient.resize(_coupleable_max_qps);
    return _default_gradient;
  }
  checkFuncType(var_name, VarType::GradientDot, FuncAge::Curr);

  if (!_coupleable_neighbor)
    return var->gradSlnDot();
  return var->gradSlnNeighborDot();
}

const VariableGradient &
Coupleable::coupledGradientDotDot(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVar(var_name, comp);
  if (!var)
  {
    _default_gradient.resize(_coupleable_max_qps);
    return _default_gradient;
  }
  checkFuncType(var_name, VarType::GradientDot, FuncAge::Curr);

  if (!_coupleable_neighbor)
    return var->gradSlnDotDot();
  return var->gradSlnNeighborDotDot();
}

const VectorVariableGradient &
Coupleable::coupledVectorGradient(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVectorVar(var_name, comp);
  if (!var)
  {
    _default_vector_gradient.resize(_coupleable_max_qps);
    return _default_vector_gradient;
  }
  checkFuncType(var_name, VarType::Gradient, FuncAge::Curr);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->gradSln() : var->gradSlnOld();
  return (_c_is_implicit) ? var->gradSlnNeighbor() : var->gradSlnOldNeighbor();
}

const VectorVariableGradient &
Coupleable::coupledVectorGradientOld(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVectorVar(var_name, comp);
  if (!var)
  {
    _default_vector_gradient.resize(_coupleable_max_qps);
    return _default_vector_gradient;
  }
  checkFuncType(var_name, VarType::Gradient, FuncAge::Old);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->gradSlnOld() : var->gradSlnOlder();
  return (_c_is_implicit) ? var->gradSlnOldNeighbor() : var->gradSlnOlderNeighbor();
}

const VectorVariableGradient &
Coupleable::coupledVectorGradientOlder(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVectorVar(var_name, comp);
  if (!var)
  {
    _default_vector_gradient.resize(_coupleable_max_qps);
    return _default_vector_gradient;
  }
  checkFuncType(var_name, VarType::Gradient, FuncAge::Older);

  if (!_coupleable_neighbor)
    return var->gradSlnOlder();
  return var->gradSlnOlderNeighbor();
}

const ArrayVariableGradient &
Coupleable::coupledArrayGradient(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getArrayVar(var_name, comp);
  if (!var)
    return _default_array_gradient;
  checkFuncType(var_name, VarType::Gradient, FuncAge::Curr);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->gradSln() : var->gradSlnOld();
  return (_c_is_implicit) ? var->gradSlnNeighbor() : var->gradSlnOldNeighbor();
}

const ArrayVariableGradient &
Coupleable::coupledArrayGradientOld(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getArrayVar(var_name, comp);
  if (!var)
    return _default_array_gradient;
  checkFuncType(var_name, VarType::Gradient, FuncAge::Old);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->gradSlnOld() : var->gradSlnOlder();
  return (_c_is_implicit) ? var->gradSlnOldNeighbor() : var->gradSlnOlderNeighbor();
}

const ArrayVariableGradient &
Coupleable::coupledArrayGradientOlder(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getArrayVar(var_name, comp);
  if (!var)
    return _default_array_gradient;
  checkFuncType(var_name, VarType::Gradient, FuncAge::Older);

  if (!_coupleable_neighbor)
    return var->gradSlnOlder();
  return var->gradSlnOlderNeighbor();
}

const VectorVariableCurl &
Coupleable::coupledCurl(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVectorVar(var_name, comp);
  if (!var)
  {
    _default_vector_curl.resize(_coupleable_max_qps);
    return _default_vector_curl;
  }
  checkFuncType(var_name, VarType::Gradient, FuncAge::Curr);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->curlSln() : var->curlSlnOld();
  return (_c_is_implicit) ? var->curlSlnNeighbor() : var->curlSlnOldNeighbor();
}

const VectorVariableCurl &
Coupleable::coupledCurlOld(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVectorVar(var_name, comp);
  if (!var)
  {
    _default_vector_curl.resize(_coupleable_max_qps);
    return _default_vector_curl;
  }
  checkFuncType(var_name, VarType::Gradient, FuncAge::Old);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->curlSlnOld() : var->curlSlnOlder();
  return (_c_is_implicit) ? var->curlSlnOldNeighbor() : var->curlSlnOlderNeighbor();
}

const VectorVariableCurl &
Coupleable::coupledCurlOlder(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVectorVar(var_name, comp);
  if (!var)
  {
    _default_vector_curl.resize(_coupleable_max_qps);
    return _default_vector_curl;
  }
  checkFuncType(var_name, VarType::Gradient, FuncAge::Older);

  if (!_coupleable_neighbor)
    return var->curlSlnOlder();
  return var->curlSlnOlderNeighbor();
}

const VariableSecond &
Coupleable::coupledSecond(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVar(var_name, comp);
  if (!var)
  {
    _default_second.resize(_coupleable_max_qps);
    return _default_second;
  }
  checkFuncType(var_name, VarType::Second, FuncAge::Curr);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->secondSln() : var->secondSlnOlder();
  return (_c_is_implicit) ? var->secondSlnNeighbor() : var->secondSlnOlderNeighbor();
}

const VariableSecond &
Coupleable::coupledSecondOld(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVar(var_name, comp);
  if (!var)
  {
    _default_second.resize(_coupleable_max_qps);
    return _default_second;
  }
  checkFuncType(var_name, VarType::Second, FuncAge::Old);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->secondSlnOld() : var->secondSlnOlder();
  return (_c_is_implicit) ? var->secondSlnOldNeighbor() : var->secondSlnOlderNeighbor();
}

const VariableSecond &
Coupleable::coupledSecondOlder(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVar(var_name, comp);
  if (!var)
  {
    _default_second.resize(_coupleable_max_qps);
    return _default_second;
  }
  checkFuncType(var_name, VarType::Second, FuncAge::Older);

  if (!_coupleable_neighbor)
    return var->secondSlnOlder();
  return var->secondSlnOlderNeighbor();
}

const VariableSecond &
Coupleable::coupledSecondPreviousNL(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVar(var_name, comp);
  _c_fe_problem.needsPreviousNewtonIteration(true);
  if (!var)
  {
    _default_second.resize(_coupleable_max_qps);
    return _default_second;
  }
  checkFuncType(var_name, VarType::Second, FuncAge::Curr);

  if (!_coupleable_neighbor)
    return var->secondSlnPreviousNL();
  return var->secondSlnPreviousNLNeighbor();
}

template <typename T>
const T &
Coupleable::coupledNodalValue(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVarHelper<MooseVariableFE<T>>(var_name, comp);
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
Coupleable::coupledNodalValueOld(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVarHelper<MooseVariableFE<T>>(var_name, comp);
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
Coupleable::coupledNodalValueOlder(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVarHelper<MooseVariableFE<T>>(var_name, comp);
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
Coupleable::coupledNodalValuePreviousNL(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVarHelper<MooseVariableFE<T>>(var_name, comp);
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
Coupleable::coupledNodalDot(const std::string & var_name, unsigned int comp) const
{
  static const T zero = 0;
  const auto * var = getVarHelper<MooseVariableFE<T>>(var_name, comp);
  if (!var)
    return zero;
  checkFuncType(var_name, VarType::Dot, FuncAge::Curr);

  if (!_coupleable_neighbor)
    return var->nodalValueDot();
  mooseError("Neighbor version not implemented");
}

const VariableValue &
Coupleable::coupledNodalDotDot(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVar(var_name, comp);
  if (!var)
  {
    _default_value_zero.resize(_coupleable_max_qps, 0);
    return _default_value_zero;
  }
  checkFuncType(var_name, VarType::Dot, FuncAge::Curr);

  if (!_coupleable_neighbor)
    return var->dofValuesDotDot();
  return var->dofValuesDotDotNeighbor();
}

const VariableValue &
Coupleable::coupledNodalDotOld(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVar(var_name, comp);
  if (!var)
  {
    _default_value_zero.resize(_coupleable_max_qps, 0);
    return _default_value_zero;
  }
  checkFuncType(var_name, VarType::Dot, FuncAge::Old);

  if (!_coupleable_neighbor)
    return var->dofValuesDotOld();
  return var->dofValuesDotOldNeighbor();
}

const VariableValue &
Coupleable::coupledNodalDotDotOld(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVar(var_name, comp);
  if (!var)
  {
    _default_value_zero.resize(_coupleable_max_qps, 0);
    return _default_value_zero;
  }
  checkFuncType(var_name, VarType::Dot, FuncAge::Old);

  if (!_coupleable_neighbor)
    return var->dofValuesDotDotOld();
  return var->dofValuesDotDotOldNeighbor();
}

const VariableValue &
Coupleable::coupledDofValues(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVarHelper<MooseVariableField<Real>>(var_name, comp);
  if (!var)
    return *getDefaultValue(var_name, comp);
  checkFuncType(var_name, VarType::Ignore, FuncAge::Curr);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->dofValues() : var->dofValuesOld();
  return (_c_is_implicit) ? var->dofValuesNeighbor() : var->dofValuesOldNeighbor();
}

std::vector<const VariableValue *>
Coupleable::coupledAllDofValues(const std::string & var_name) const
{
  auto func = [this, &var_name](unsigned int comp) { return &coupledDofValues(var_name, comp); };
  return coupledVectorHelper<const VariableValue *>(var_name, func);
}

const VariableValue &
Coupleable::coupledDofValuesOld(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVar(var_name, comp);
  if (!var)
    return *getDefaultValue(var_name, comp);
  checkFuncType(var_name, VarType::Ignore, FuncAge::Old);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->dofValuesOld() : var->dofValuesOlder();
  return (_c_is_implicit) ? var->dofValuesOldNeighbor() : var->dofValuesOlderNeighbor();
}

std::vector<const VariableValue *>
Coupleable::coupledAllDofValuesOld(const std::string & var_name) const
{
  auto func = [this, &var_name](unsigned int comp) { return &coupledDofValuesOld(var_name, comp); };
  return coupledVectorHelper<const VariableValue *>(var_name, func);
}

const VariableValue &
Coupleable::coupledDofValuesOlder(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVar(var_name, comp);
  if (!var)
    return *getDefaultValue(var_name, comp);
  checkFuncType(var_name, VarType::Ignore, FuncAge::Older);

  if (!_coupleable_neighbor)
    return var->dofValuesOlder();
  return var->dofValuesOlderNeighbor();
}

std::vector<const VariableValue *>
Coupleable::coupledAllDofValuesOlder(const std::string & var_name) const
{
  auto func = [this, &var_name](unsigned int comp)
  { return &coupledDofValuesOlder(var_name, comp); };
  return coupledVectorHelper<const VariableValue *>(var_name, func);
}

const ArrayVariableValue &
Coupleable::coupledArrayDofValues(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getArrayVar(var_name, comp);
  if (!var)
    return *getDefaultArrayValue(var_name);
  checkFuncType(var_name, VarType::Ignore, FuncAge::Curr);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->dofValues() : var->dofValuesOld();
  return (_c_is_implicit) ? var->dofValuesNeighbor() : var->dofValuesOldNeighbor();
}

const ADVariableValue &
Coupleable::adCoupledDofValues(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVarHelper<MooseVariableField<Real>>(var_name, comp);

  if (!var)
    return *getADDefaultValue(var_name);
  checkFuncType(var_name, VarType::Ignore, FuncAge::Curr);

  if (!_c_is_implicit)
    mooseError("Not implemented");

  if (!_coupleable_neighbor)
    return var->adDofValues();
  return var->adDofValuesNeighbor();
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

template <typename T>
const typename Moose::ADType<T>::type &
Coupleable::adCoupledNodalValue(const std::string & var_name, unsigned int comp) const
{
  static const typename Moose::ADType<T>::type zero = 0;
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

  const auto * var = getVarHelper<MooseVariableFE<T>>(var_name, comp);

  return var->adNodalValue();
}

const ADVariableValue &
Coupleable::adCoupledValue(const std::string & var_name, unsigned int comp) const
{
  const auto * const var = getVarHelper<MooseVariableField<Real>>(var_name, comp);

  if (!var)
    return *getADDefaultValue(var_name);
  checkFuncType(var_name, VarType::Ignore, FuncAge::Curr);

  if (!_c_is_implicit)
    mooseError("Not implemented");

  if (_c_nodal)
    return var->adDofValues();

  if (!_coupleable_neighbor)
    return var->adSln();
  return var->adSlnNeighbor();
}

const ADVariableValue &
Coupleable::adCoupledLowerValue(const std::string & var_name, unsigned int comp) const
{
  auto var = getVarHelper<MooseVariableFE<Real>>(var_name, comp);

  if (!var)
    return *getADDefaultValue(var_name);
  checkFuncType(var_name, VarType::Ignore, FuncAge::Curr);

  if (!_c_is_implicit)
    mooseError("adCoupledLowerValue cannot be called in a coupleable neighbor object");

  if (_c_nodal)
    return var->adDofValues();
  else
    return var->adSlnLower();
}

const ADVariableGradient &
Coupleable::adCoupledGradient(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVarHelper<MooseVariableField<Real>>(var_name, comp);

  if (!var)
    return getADDefaultGradient();
  checkFuncType(var_name, VarType::Gradient, FuncAge::Curr);

  if (!_c_is_implicit)
    mooseError("Not implemented");

  if (!_coupleable_neighbor)
    return var->adGradSln();
  return var->adGradSlnNeighbor();
}

const ADVariableGradient &
Coupleable::adCoupledGradientDot(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVarHelper<MooseVariableField<Real>>(var_name, comp);

  if (!var)
    return getADDefaultGradient();
  checkFuncType(var_name, VarType::GradientDot, FuncAge::Curr);

  if (!_c_is_implicit)
    mooseError("Not implemented");

  if (!_coupleable_neighbor)
    return var->adGradSlnDot();
  return var->adGradSlnNeighborDot();
}

const ADVariableSecond &
Coupleable::adCoupledSecond(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVarHelper<MooseVariableField<Real>>(var_name, comp);

  if (!var)
    return getADDefaultSecond();
  checkFuncType(var_name, VarType::Second, FuncAge::Curr);

  if (!_c_is_implicit)
    mooseError("Not implemented");

  if (!_coupleable_neighbor)
    return var->adSecondSln();
  else
    return var->adSecondSlnNeighbor();
}

const ADVectorVariableSecond &
adCoupledVectorSecond(const std::string & /*var_name*/, unsigned int /*comp = 0*/)
{
  mooseError("Automatic differentiation using second derivatives of vector variables is not "
             "implemented.");
}

const ADVariableValue &
Coupleable::adCoupledDot(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVarHelper<MooseVariableField<Real>>(var_name, comp);

  if (!var)
    return *getADDefaultValue(var_name);
  checkFuncType(var_name, VarType::Dot, FuncAge::Curr);

  if (_c_nodal)
    mooseError("Not implemented");

  if (!_coupleable_neighbor)
    return var->adUDot();
  return var->adUDotNeighbor();
}

const ADVariableValue &
Coupleable::adCoupledDotDot(const std::string & var_name, unsigned int comp) const
{
  const auto * const var = getVarHelper<MooseVariableField<Real>>(var_name, comp);

  if (!var)
    return *getADDefaultValue(var_name);
  checkFuncType(var_name, VarType::Dot, FuncAge::Curr);

  if (_c_nodal)
    mooseError("Not implemented");

  if (!_coupleable_neighbor)
    return var->adUDotDot();
  return var->adUDotDotNeighbor();
}

const ADVectorVariableValue &
Coupleable::adCoupledVectorDot(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVectorVar(var_name, comp);
  if (!var)
    return *getADDefaultVectorValue(var_name);
  checkFuncType(var_name, VarType::Dot, FuncAge::Curr);

  if (_c_nodal)
    mooseError("Not implemented");

  if (!_coupleable_neighbor)
    return var->adUDot();
  return var->adUDotNeighbor();
}

const ADVectorVariableValue &
Coupleable::adCoupledVectorValue(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVectorVar(var_name, comp);
  if (!var)
    return *getADDefaultVectorValue(var_name);
  checkFuncType(var_name, VarType::Ignore, FuncAge::Curr);

  if (_c_nodal)
    mooseError("Not implemented");
  if (!_c_is_implicit)
    mooseError("Not implemented");

  if (!_coupleable_neighbor)
    return var->adSln();
  return var->adSlnNeighbor();
}

const ADVectorVariableGradient &
Coupleable::adCoupledVectorGradient(const std::string & var_name, unsigned int comp) const
{
  const auto * var = getVectorVar(var_name, comp);
  if (!var)
    return getADDefaultVectorGradient();
  checkFuncType(var_name, VarType::Gradient, FuncAge::Curr);

  if (!_c_is_implicit)
    mooseError("Not implemented");

  if (!_coupleable_neighbor)
    return var->adGradSln();
  return var->adGradSlnNeighbor();
}

const ADVariableValue *
Coupleable::getADDefaultValue(const std::string & var_name) const
{
  auto default_value_it = _ad_default_value.find(var_name);
  if (default_value_it == _ad_default_value.end())
  {
    auto value = std::make_unique<ADVariableValue>(_coupleable_max_qps,
                                                   _c_parameters.defaultCoupledValue(var_name));
    default_value_it = _ad_default_value.insert(std::make_pair(var_name, std::move(value))).first;
  }

  return default_value_it->second.get();
}

const ADVectorVariableValue *
Coupleable::getADDefaultVectorValue(const std::string & var_name) const
{
  auto default_value_it = _ad_default_vector_value.find(var_name);
  if (default_value_it == _ad_default_vector_value.end())
  {
    RealVectorValue default_vec;
    for (unsigned int i = 0; i < _c_parameters.numberDefaultCoupledValues(var_name); ++i)
      default_vec(i) = _c_parameters.defaultCoupledValue(var_name, i);
    auto value = std::make_unique<ADVectorVariableValue>(_coupleable_max_qps, default_vec);
    default_value_it =
        _ad_default_vector_value.insert(std::make_pair(var_name, std::move(value))).first;
  }

  return default_value_it->second.get();
}

const ADVariableGradient &
Coupleable::getADDefaultGradient() const
{
  _ad_default_gradient.resize(_coupleable_max_qps);
  return _ad_default_gradient;
}

const ADVectorVariableGradient &
Coupleable::getADDefaultVectorGradient() const
{
  _ad_default_vector_gradient.resize(_coupleable_max_qps);
  return _ad_default_vector_gradient;
}

const ADVariableSecond &
Coupleable::getADDefaultSecond() const
{
  _ad_default_second.resize(_coupleable_max_qps);
  return _ad_default_second;
}

const ADVariableValue &
Coupleable::adZeroValue() const
{
  mooseDeprecated("Method adZeroValue() is deprecated. Use '_ad_zero' instead.");
  return _ad_zero;
}

const ADVariableGradient &
Coupleable::adZeroGradient() const
{
  mooseDeprecated("Method adZeroGradient() is deprecated. Use '_ad_grad_zero' instead.");
  return _ad_grad_zero;
}

const ADVariableSecond &
Coupleable::adZeroSecond() const
{
  mooseDeprecated("Method adZeroSecond() is deprecated. Use '_ad_second_zero' instead.");
  return _ad_second_zero;
}

template <>
const GenericVariableValue<false> &
Coupleable::genericZeroValue<false>()
{
  return _zero;
}

template <>
const GenericVariableValue<true> &
Coupleable::genericZeroValue<true>()
{
  return _ad_zero;
}

template <>
const GenericVariableGradient<false> &
Coupleable::genericZeroGradient<false>()
{
  return _grad_zero;
}

template <>
const GenericVariableGradient<true> &
Coupleable::genericZeroGradient<true>()
{
  return _ad_grad_zero;
}

template <>
const GenericVariableSecond<false> &
Coupleable::genericZeroSecond<false>()
{
  return _second_zero;
}

template <>
const GenericVariableSecond<true> &
Coupleable::genericZeroSecond<true>()
{
  return _ad_second_zero;
}

template <>
const GenericVariableGradient<false> &
Coupleable::coupledGenericGradient<false>(const std::string & var_name, unsigned int comp) const
{
  return coupledGradient(var_name, comp);
}

template <>
const GenericVariableGradient<true> &
Coupleable::coupledGenericGradient<true>(const std::string & var_name, unsigned int comp) const
{
  return adCoupledGradient(var_name, comp);
}

std::vector<unsigned int>
Coupleable::coupledIndices(const std::string & var_name) const
{
  auto func = [this, &var_name](unsigned int comp) { return coupled(var_name, comp); };
  return coupledVectorHelper<unsigned int>(var_name, func);
}

VariableName
Coupleable::coupledName(const std::string & var_name, unsigned int comp) const
{
  if (getVar(var_name, comp))
    return getVar(var_name, comp)->name();
  // Detect if we are in the case where a constant was passed in lieu of a variable
  else if (isCoupledConstant(var_name))
    mooseError(_c_name,
               ": a variable name was queried but a constant was passed for parameter '",
               var_name,
               "Either pass a true variable or contact a developer to shield the call to "
               "'coupledName' with 'isCoupledConstant'");
  else
    mooseError(
        _c_name, ": Variable '", var_name, "' does not exist, yet its coupled name is requested");
}

std::vector<VariableName>
Coupleable::coupledNames(const std::string & var_name) const
{
  auto func = [this, &var_name](unsigned int comp) { return coupledName(var_name, comp); };
  return coupledVectorHelper<VariableName>(var_name, func);
}

std::vector<const VariableValue *>
Coupleable::coupledValues(const std::string & var_name) const
{
  auto func = [this, &var_name](unsigned int comp) { return &coupledValue(var_name, comp); };
  return coupledVectorHelper<const VariableValue *>(var_name, func);
}

template <>
std::vector<const GenericVariableValue<false> *>
Coupleable::coupledGenericValues<false>(const std::string & var_name) const
{
  return coupledValues(var_name);
}

template <>
std::vector<const GenericVariableValue<true> *>
Coupleable::coupledGenericValues<true>(const std::string & var_name) const
{
  return adCoupledValues(var_name);
}

std::vector<const ADVariableValue *>
Coupleable::adCoupledValues(const std::string & var_name) const
{
  auto func = [this, &var_name](unsigned int comp) { return &adCoupledValue(var_name, comp); };
  return coupledVectorHelper<const ADVariableValue *>(var_name, func);
}

std::vector<const ADVectorVariableValue *>
Coupleable::adCoupledVectorValues(const std::string & var_name) const
{
  auto func = [this, &var_name](unsigned int comp)
  { return &adCoupledVectorValue(var_name, comp); };
  return coupledVectorHelper<const ADVectorVariableValue *>(var_name, func);
}

std::vector<const VariableValue *>
Coupleable::coupledVectorTagValues(const std::string & var_names, TagID tag) const
{
  auto func = [this, &var_names, &tag](unsigned int comp)
  { return &coupledVectorTagValue(var_names, tag, comp); };
  return coupledVectorHelper<const VariableValue *>(var_names, func);
}

std::vector<const VariableValue *>
Coupleable::coupledVectorTagValues(const std::string & var_names,
                                   const std::string & tag_name) const
{
  if (!_c_parameters.isParamValid(tag_name))
    mooseError("Tag name parameter '", tag_name, "' is invalid");

  TagName tagname = _c_parameters.get<TagName>(tag_name);
  if (!_c_fe_problem.vectorTagExists(tagname))
    mooseError("Tagged vector with tag name '", tagname, "' does not exist");

  TagID tag = _c_fe_problem.getVectorTagID(tagname);
  return coupledVectorTagValues(var_names, tag);
}

std::vector<const ArrayVariableValue *>
Coupleable::coupledVectorTagArrayValues(const std::string & var_names, TagID tag) const
{
  auto func = [this, &var_names, &tag](unsigned int index)
  { return &coupledVectorTagArrayValue(var_names, tag, index); };
  return coupledVectorHelper<const ArrayVariableValue *>(var_names, func);
}

std::vector<const ArrayVariableValue *>
Coupleable::coupledVectorTagArrayValues(const std::string & var_names,
                                        const std::string & tag_name) const
{
  if (!_c_parameters.isParamValid(tag_name))
    mooseError("Tag name parameter '", tag_name, "' is invalid");

  TagName tagname = _c_parameters.get<TagName>(tag_name);
  if (!_c_fe_problem.vectorTagExists(tagname))
    mooseError("Tagged vector with tag name '", tagname, "' does not exist");

  TagID tag = _c_fe_problem.getVectorTagID(tagname);
  return coupledVectorTagArrayValues(var_names, tag);
}

std::vector<const VariableGradient *>
Coupleable::coupledVectorTagGradients(const std::string & var_names, TagID tag) const
{
  auto func = [this, &var_names, &tag](unsigned int index)
  { return &coupledVectorTagGradient(var_names, tag, index); };
  return coupledVectorHelper<const VariableGradient *>(var_names, func);
}

std::vector<const VariableGradient *>
Coupleable::coupledVectorTagGradients(const std::string & var_names,
                                      const std::string & tag_name) const
{
  if (!_c_parameters.isParamValid(tag_name))
    mooseError("Tag name parameter '", tag_name, "' is invalid");

  TagName tagname = _c_parameters.get<TagName>(tag_name);
  if (!_c_fe_problem.vectorTagExists(tagname))
    mooseError("Tagged vector with tag name '", tagname, "' does not exist");

  TagID tag = _c_fe_problem.getVectorTagID(tagname);
  return coupledVectorTagGradients(var_names, tag);
}

std::vector<const ArrayVariableGradient *>
Coupleable::coupledVectorTagArrayGradients(const std::string & var_names, TagID tag) const
{
  auto func = [this, &var_names, &tag](unsigned int index)
  { return &coupledVectorTagArrayGradient(var_names, tag, index); };
  return coupledVectorHelper<const ArrayVariableGradient *>(var_names, func);
}

std::vector<const ArrayVariableGradient *>
Coupleable::coupledVectorTagArrayGradients(const std::string & var_names,
                                           const std::string & tag_name) const
{
  if (!_c_parameters.isParamValid(tag_name))
    mooseError("Tag name parameter '", tag_name, "' is invalid");

  TagName tagname = _c_parameters.get<TagName>(tag_name);
  if (!_c_fe_problem.vectorTagExists(tagname))
    mooseError("Tagged vector with tag name '", tagname, "' does not exist");

  TagID tag = _c_fe_problem.getVectorTagID(tagname);
  return coupledVectorTagArrayGradients(var_names, tag);
}

std::vector<const VariableValue *>
Coupleable::coupledVectorTagDofValues(const std::string & var_names, TagID tag) const
{
  auto func = [this, &var_names, &tag](unsigned int comp)
  { return &coupledVectorTagDofValue(var_names, tag, comp); };
  return coupledVectorHelper<const VariableValue *>(var_names, func);
}

std::vector<const VariableValue *>
Coupleable::coupledVectorTagDofValues(const std::string & var_names,
                                      const std::string & tag_name) const
{
  if (!_c_parameters.isParamValid(tag_name))
    mooseError("Tag name parameter '", tag_name, "' is invalid");

  TagName tagname = _c_parameters.get<TagName>(tag_name);
  if (!_c_fe_problem.vectorTagExists(tagname))
    mooseError("Tagged vector with tag name '", tagname, "' does not exist");

  TagID tag = _c_fe_problem.getVectorTagID(tagname);
  return coupledVectorTagDofValues(var_names, tag);
}

std::vector<const VariableValue *>
Coupleable::coupledMatrixTagValues(const std::string & var_names, TagID tag) const
{
  auto func = [this, &var_names, &tag](unsigned int comp)
  { return &coupledMatrixTagValue(var_names, tag, comp); };
  return coupledVectorHelper<const VariableValue *>(var_names, func);
}

std::vector<const VariableValue *>
Coupleable::coupledMatrixTagValues(const std::string & var_names,
                                   const std::string & tag_name) const
{
  if (!_c_parameters.isParamValid(tag_name))
    mooseError("Tag name parameter '", tag_name, "' is invalid");

  TagName tagname = _c_parameters.get<TagName>(tag_name);
  if (!_c_fe_problem.matrixTagExists(tagname))
    mooseError("Matrix tag name '", tagname, "' does not exist");

  TagID tag = _c_fe_problem.getMatrixTagID(tagname);
  return coupledMatrixTagValues(var_names, tag);
}

std::vector<const VariableValue *>
Coupleable::coupledValuesOld(const std::string & var_name) const
{
  auto func = [this, &var_name](unsigned int comp) { return &coupledValueOld(var_name, comp); };
  return coupledVectorHelper<const VariableValue *>(var_name, func);
}

std::vector<const VariableGradient *>
Coupleable::coupledGradients(const std::string & var_name) const
{
  auto func = [this, &var_name](unsigned int comp) { return &coupledGradient(var_name, comp); };
  return coupledVectorHelper<const VariableGradient *>(var_name, func);
}

template <>
std::vector<const GenericVariableGradient<false> *>
Coupleable::coupledGenericGradients<false>(const std::string & var_name) const
{
  return coupledGradients(var_name);
}

template <>
std::vector<const GenericVariableGradient<true> *>
Coupleable::coupledGenericGradients<true>(const std::string & var_name) const
{
  auto func = [this, &var_name](unsigned int comp) { return &adCoupledGradient(var_name, comp); };
  return coupledVectorHelper<const GenericVariableGradient<true> *>(var_name, func);
}

std::vector<const ADVariableGradient *>
Coupleable::adCoupledGradients(const std::string & var_name) const
{
  auto func = [this, &var_name](unsigned int comp) { return &adCoupledGradient(var_name, comp); };
  return coupledVectorHelper<const ADVariableGradient *>(var_name, func);
}

std::vector<const VariableGradient *>
Coupleable::coupledGradientsOld(const std::string & var_name) const
{
  auto func = [this, &var_name](unsigned int comp) { return &coupledGradientOld(var_name, comp); };
  return coupledVectorHelper<const VariableGradient *>(var_name, func);
}

std::vector<const VariableValue *>
Coupleable::coupledDots(const std::string & var_name) const
{
  auto func = [this, &var_name](unsigned int comp) { return &coupledDot(var_name, comp); };
  return coupledVectorHelper<const VariableValue *>(var_name, func);
}

std::vector<const ADVariableValue *>
Coupleable::adCoupledDots(const std::string & var_name) const
{
  auto func = [this, &var_name](unsigned int comp) { return &adCoupledDot(var_name, comp); };
  return coupledVectorHelper<const ADVariableValue *>(var_name, func);
}

// Explicit instantiations

template const Real & Coupleable::getDefaultNodalValue<Real>(const std::string & var_name,
                                                             unsigned int comp) const;

template const Real & Coupleable::coupledNodalValue<Real>(const std::string & var_name,
                                                          unsigned int comp) const;
template const ADReal & Coupleable::adCoupledNodalValue<Real>(const std::string & var_name,
                                                              unsigned int comp) const;
template const ADRealVectorValue &
Coupleable::adCoupledNodalValue<RealVectorValue>(const std::string & var_name,
                                                 unsigned int comp) const;

template const RealVectorValue &
Coupleable::coupledNodalValue<RealVectorValue>(const std::string & var_name,
                                               unsigned int comp) const;
template const Real & Coupleable::coupledNodalValueOld<Real>(const std::string & var_name,
                                                             unsigned int comp) const;
template const RealVectorValue &
Coupleable::coupledNodalValueOld<RealVectorValue>(const std::string & var_name,
                                                  unsigned int comp) const;
template const Real & Coupleable::coupledNodalValueOlder<Real>(const std::string & var_name,
                                                               unsigned int comp) const;
template const RealVectorValue &
Coupleable::coupledNodalValueOlder<RealVectorValue>(const std::string & var_name,
                                                    unsigned int comp) const;
template const Real & Coupleable::coupledNodalValuePreviousNL<Real>(const std::string & var_name,
                                                                    unsigned int comp) const;
template const RealVectorValue &
Coupleable::coupledNodalValuePreviousNL<RealVectorValue>(const std::string & var_name,
                                                         unsigned int comp) const;
template const Real & Coupleable::coupledNodalDot<Real>(const std::string & var_name,
                                                        unsigned int comp) const;
template const RealVectorValue &
Coupleable::coupledNodalDot<RealVectorValue>(const std::string & var_name, unsigned int comp) const;
