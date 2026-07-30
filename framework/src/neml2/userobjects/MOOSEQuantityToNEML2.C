//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MOOSEQuantityToNEML2.h"
#include "Function.h"
#include "MooseVariableScalar.h"

#define registerMOOSEQuantityToNEML2(T)                                                            \
  registerMooseObject("MooseApp", MOOSE##T##ToNEML2);                                              \
  registerMooseObject("MooseApp", MOOSEOld##T##ToNEML2)

registerMOOSEQuantityToNEML2(Real);
registerMOOSEQuantityToNEML2(RankTwoTensor);
registerMOOSEQuantityToNEML2(SymmetricRankTwoTensor);
registerMOOSEQuantityToNEML2(RealVectorValue);

template <typename T, unsigned int state>
InputParameters
MOOSEQuantityToNEML2<T, state>::validParams()
{
  auto params = MOOSEToNEML2::validParams();
  params += ElementUserObject::validParams();

  params.addClassDescription(
      "Gather a MOOSE quantity of type " + demangle(typeid(T).name()) +
      " for insertion into the specified input or model parameter of a NEML2 model.");
  params.template addRequiredParam<std::string>("from_moose",
                                                "Name of the MOOSE quantity to read from");
  MooseEnum moose_type("TIME SCALAR FUNCTION VARIABLE MATERIAL", "MATERIAL");
  params.template addParam<MooseEnum>(
      "quantity_type", moose_type, "Type of the MOOSE quantity to read from.");

  // Since we use the NEML2 model to evaluate the residual AND the Jacobian at the same time, we
  // want to execute this user object only at execute_on = LINEAR (i.e. during residual evaluation).
  // The NONLINEAR exec flag below is for computing Jacobian during automatic scaling.
  ExecFlagEnum execute_options = MooseUtils::getDefaultExecFlagEnum();
  execute_options = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};
  params.set<ExecFlagEnum>("execute_on") = execute_options;

  return params;
}

template <typename T, unsigned int state>
MOOSEQuantityToNEML2<T, state>::MOOSEQuantityToNEML2(const InputParameters & params)
  : MOOSEToNEML2(params),
    ElementUserObject(params)
#ifdef NEML2_ENABLED
    ,
    _type(getParam<MooseEnum>("quantity_type").template getEnum<NEML2Utils::MOOSEIOType>()),
    _var_scalar(
        _type == NEML2Utils::MOOSEIOType::SCALAR && state == 0
            ? &this->_fe_problem.getScalarVariable(_tid, getParam<std::string>("from_moose")).sln()
            : nullptr),
    _var_scalar_old(
        _type == NEML2Utils::MOOSEIOType::SCALAR && state == 1
            ? &this->_fe_problem.getScalarVariable(_tid, getParam<std::string>("from_moose"))
                   .slnOld()
            : nullptr),
    _func(_type == NEML2Utils::MOOSEIOType::FUNCTION
              ? &this->_fe_problem.getFunction(getParam<std::string>("from_moose"), _tid)
              : nullptr),
    _mat_prop(
        _type == NEML2Utils::MOOSEIOType::MATERIAL && state == 0
            ? &this->template getMaterialPropertyByName<T>(getParam<std::string>("from_moose"))
            : nullptr),
    _mat_prop_old(
        _type == NEML2Utils::MOOSEIOType::MATERIAL && state == 1
            ? &this->template getMaterialPropertyOldByName<T>(getParam<std::string>("from_moose"))
            : nullptr),
    _var(_type == NEML2Utils::MOOSEIOType::VARIABLE && state == 0
             ? &this->_fe_problem.getStandardVariable(_tid, getParam<std::string>("from_moose"))
                    .sln()
             : nullptr),
    _var_old(_type == NEML2Utils::MOOSEIOType::VARIABLE && state == 1
                 ? &this->_fe_problem.getStandardVariable(_tid, getParam<std::string>("from_moose"))
                        .slnOld()
                 : nullptr),
    _batched(_type != NEML2Utils::MOOSEIOType::TIME && _type != NEML2Utils::MOOSEIOType::SCALAR)
#endif
{
#ifdef NEML2_ENABLED
  // A VARIABLE gatherer reads the field through getStandardVariable(...).sln()/slnOld() rather than
  // the Coupleable interface, so the variable is not otherwise part of this user object's reinit
  // set. Register it as a dependency so MOOSE refreshes its elemental solution before each
  // execute(); without this, .sln() can carry a stale (one-timestep-lagged) value depending on what
  // else in the problem happens to force the variable's reinit. Mirrors
  // ElementIntegralVariableUserObject.
  if (_type == NEML2Utils::MOOSEIOType::VARIABLE)
    addMooseVariableDependency(
        &this->_fe_problem.getStandardVariable(_tid, getParam<std::string>("from_moose")));
#endif
}

#ifdef NEML2_ENABLED
template <typename T, unsigned int state>
void
MOOSEQuantityToNEML2<T, state>::initialize()
{
  _buffer.clear();
}

template <typename T, unsigned int state>
void
MOOSEQuantityToNEML2<T, state>::execute()
{
  if (!_batched)
    return;

  for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
    _buffer.emplace_back(qpData(qp));
}

template <typename T, unsigned int state>
void
MOOSEQuantityToNEML2<T, state>::threadJoin(const UserObject & uo)
{
  if (!_batched)
    return;
  // append vectors
  const auto & m2n = static_cast<const MOOSEQuantityToNEML2<T, state> &>(uo);
  _buffer.insert(_buffer.end(), m2n._buffer.begin(), m2n._buffer.end());
}

template <typename T, unsigned int state>
at::Tensor
MOOSEQuantityToNEML2<T, state>::gatheredData() const
{
  // Unbatched scalars (time / scalar variable) are returned as 0-dim tensors; the executor
  // broadcasts them against the batched inputs before evaluating the model.
  const auto opts = at::TensorOptions().dtype(at::kDouble);
  if (!_batched)
  {
    switch (_type)
    {
      case NEML2Utils::MOOSEIOType::TIME:
        return state == 0 ? at::scalar_tensor(_t, opts) : at::scalar_tensor(_t_old, opts);
      case NEML2Utils::MOOSEIOType::SCALAR:
        return state == 0 ? at::scalar_tensor((*_var_scalar)[0], opts)
                          : at::scalar_tensor((*_var_scalar_old)[0], opts);
      case NEML2Utils::MOOSEIOType::FUNCTION:
      case NEML2Utils::MOOSEIOType::VARIABLE:
      case NEML2Utils::MOOSEIOType::MATERIAL:
        mooseError(
            "Unbatched quantity cannot be of type MATERIAL or VARIABLE. This should never happen.");
      default:
        mooseError("Invalid MOOSE quantity type. This should never happen.");
    }
  }

  return NEML2Utils::fromBlob(_buffer);
}

template <typename T, unsigned int state>
T
MOOSEQuantityToNEML2<T, state>::qpData(unsigned int qp) const
{
  mooseAssert(
      _batched,
      "elemMOOSEData should only be called for batched quantities. This should never happen.");

  // non-scalar type can only be MATERIAL
  if constexpr (!std::is_same_v<T, Real>)
    return state == 0 ? (*_mat_prop)[qp] : (*_mat_prop_old)[qp];
  else
    switch (_type)
    {
      case NEML2Utils::MOOSEIOType::TIME:
        mooseError("Batched quantity cannot be of type TIME. This should never happen.");
      case NEML2Utils::MOOSEIOType::SCALAR:
        mooseError("Batched quantity cannot be of type SCALAR. This should never happen.");
      case NEML2Utils::MOOSEIOType::FUNCTION:
        return _func->value(state == 0 ? _t : _t_old, _q_point[qp]);
      case NEML2Utils::MOOSEIOType::VARIABLE:
        return state == 0 ? (*_var)[qp] : (*_var_old)[qp];
      case NEML2Utils::MOOSEIOType::MATERIAL:
        return state == 0 ? (*_mat_prop)[qp] : (*_mat_prop_old)[qp];
      default:
        mooseError("Invalid MOOSE quantity type. This should never happen.");
    }
}
#endif

#define instantiateMOOSEQuantityToNEML2(T)                                                         \
  template class MOOSEQuantityToNEML2<T, 0>;                                                       \
  template class MOOSEQuantityToNEML2<T, 1>

instantiateMOOSEQuantityToNEML2(Real);
instantiateMOOSEQuantityToNEML2(RankTwoTensor);
instantiateMOOSEQuantityToNEML2(SymmetricRankTwoTensor);
instantiateMOOSEQuantityToNEML2(RealVectorValue);
