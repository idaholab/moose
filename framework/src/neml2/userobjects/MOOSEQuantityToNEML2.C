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
  params += DomainUserObject::validParams();

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
    DomainUserObject(params)
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
    // Side/neighbor data is only gathered for interface setups (interface_boundaries set). For
    // volume-only setups these are not requested, so the object behaves like a plain element
    // gatherer and does not pull in (possibly remote) neighbor material/variable data.
    _face_mat_prop(_type == NEML2Utils::MOOSEIOType::MATERIAL && state == 0 &&
                           !_interface_bnd_ids.empty()
                       ? &this->template getMaterialPropertyByName<T>(
                             getParam<std::string>("from_moose"), this->_face_material_data, 0)
                       : nullptr),
    _face_mat_prop_old(_type == NEML2Utils::MOOSEIOType::MATERIAL && state == 1 &&
                               !_interface_bnd_ids.empty()
                           ? &this->template getMaterialPropertyByName<T>(
                                 getParam<std::string>("from_moose"), this->_face_material_data, 1)
                           : nullptr),
    _neighbor_mat_prop(_type == NEML2Utils::MOOSEIOType::MATERIAL && state == 0 &&
                               !_interface_bnd_ids.empty()
                           ? &this->template getNeighborMaterialPropertyByName<T>(
                                 getParam<std::string>("from_moose"), 0)
                           : nullptr),
    _neighbor_mat_prop_old(_type == NEML2Utils::MOOSEIOType::MATERIAL && state == 1 &&
                                   !_interface_bnd_ids.empty()
                               ? &this->template getNeighborMaterialPropertyByName<T>(
                                     getParam<std::string>("from_moose"), 1)
                               : nullptr),
    _var_neighbor(
        _type == NEML2Utils::MOOSEIOType::VARIABLE && state == 0 && !_interface_bnd_ids.empty()
            ? &this->_fe_problem.getStandardVariable(_tid, getParam<std::string>("from_moose"))
                   .slnNeighbor()
            : nullptr),
    _var_neighbor_old(
        _type == NEML2Utils::MOOSEIOType::VARIABLE && state == 1 && !_interface_bnd_ids.empty()
            ? &this->_fe_problem.getStandardVariable(_tid, getParam<std::string>("from_moose"))
                   .slnOldNeighbor()
            : nullptr),
    _batched(_type != NEML2Utils::MOOSEIOType::TIME && _type != NEML2Utils::MOOSEIOType::SCALAR)
#endif
{
}

#ifdef NEML2_ENABLED
template <typename T, unsigned int state>
void
MOOSEQuantityToNEML2<T, state>::initialize()
{
  _buffer.clear();
  _visited_elem_sides.clear();
}

template <typename T, unsigned int state>
void
MOOSEQuantityToNEML2<T, state>::executeOnElement()
{
  if (!_batched)
    return;

  for (unsigned int qp = 0; qp < qRule().n_points(); qp++)
    _buffer.emplace_back(qpData(qp, DataSource::Elem));
}

template <typename T, unsigned int state>
void
MOOSEQuantityToNEML2<T, state>::executeOnBoundary()
{
  // Only interface setups gather side data; volume-only setups gather in executeOnElement only
  if (!_batched || _interface_bnd_ids.empty())
    return;

  // Interface sides are handled by executeOnInterface (which also sees the neighbor)
  if (_current_elem->neighbor_ptr(_current_side))
    return;

  const auto elem_side = ElemSide(_current_elem->id(), _current_side);
  if (_visited_elem_sides.insert(elem_side).second)
    for (unsigned int qp = 0; qp < qRule().n_points(); qp++)
      _buffer.emplace_back(qpData(qp, DataSource::ElemSide));
}

template <typename T, unsigned int state>
void
MOOSEQuantityToNEML2<T, state>::executeOnInterface()
{
  if (!_batched)
    return;

  const auto elem_side = ElemSide(_current_elem->id(), _current_side);
  if (_visited_elem_sides.insert(elem_side).second)
    for (unsigned int qp = 0; qp < qRule().n_points(); qp++)
      _buffer.emplace_back(qpData(qp, DataSource::ElemSide));

  const auto * neighbor_elem = _current_elem->neighbor_ptr(_current_side);

  if (neighbor_elem)
  {
    const auto neighbor_side = neighbor_elem->which_neighbor_am_i(_current_elem);
    const auto neighbor_elem_side = ElemSide(neighbor_elem->id(), neighbor_side);
    if (_visited_elem_sides.insert(neighbor_elem_side).second)
      for (unsigned int qp = 0; qp < qRule().n_points(); qp++)
        _buffer.emplace_back(qpData(qp, DataSource::NeighborSide));
  }
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
  _visited_elem_sides.insert(m2n._visited_elem_sides.begin(), m2n._visited_elem_sides.end());
}

template <typename T, unsigned int state>
neml2::Tensor
MOOSEQuantityToNEML2<T, state>::gatheredData() const
{
  if (!_batched)
  {
    switch (_type)
    {
      case NEML2Utils::MOOSEIOType::TIME:
        return state == 0 ? neml2::Scalar::full(_t, neml2::kFloat64)
                          : neml2::Scalar::full(_t_old, neml2::kFloat64);
      case NEML2Utils::MOOSEIOType::SCALAR:
        return state == 0 ? neml2::Scalar::full((*_var_scalar)[0], neml2::kFloat64)
                          : neml2::Scalar::full((*_var_scalar_old)[0], neml2::kFloat64);
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
MOOSEQuantityToNEML2<T, state>::qpData(unsigned int qp, DataSource source) const
{
  mooseAssert(
      _batched,
      "qpData should only be called for batched quantities. This should never happen.");

  // Pick the material property to read from for the requested data source
  auto matProp = [&]() -> const MaterialProperty<T> &
  {
    switch (source)
    {
      case DataSource::Elem:
        return state == 0 ? *_mat_prop : *_mat_prop_old;
      case DataSource::ElemSide:
        return state == 0 ? *_face_mat_prop : *_face_mat_prop_old;
      case DataSource::NeighborSide:
        return state == 0 ? *_neighbor_mat_prop : *_neighbor_mat_prop_old;
    }
    mooseError("Invalid data source. This should never happen.");
  };

  // non-scalar type can only be MATERIAL
  if constexpr (!std::is_same_v<T, Real>)
    return matProp()[qp];
  else
    switch (_type)
    {
      case NEML2Utils::MOOSEIOType::TIME:
        mooseError("Batched quantity cannot be of type TIME. This should never happen.");
      case NEML2Utils::MOOSEIOType::SCALAR:
        mooseError("Batched quantity cannot be of type SCALAR. This should never happen.");
      case NEML2Utils::MOOSEIOType::FUNCTION:
        // The current quadrature points (qPoints()) already reflect element-interior or face
        // points; on a neighbor side the face points are physically the same.
        return _func->value(state == 0 ? _t : _t_old, qPoints()[qp]);
      case NEML2Utils::MOOSEIOType::VARIABLE:
        if (source == DataSource::NeighborSide)
          return state == 0 ? (*_var_neighbor)[qp] : (*_var_neighbor_old)[qp];
        // Elem and ElemSide both read the element solution, which is refilled on face reinit
        return state == 0 ? (*_var)[qp] : (*_var_old)[qp];
      case NEML2Utils::MOOSEIOType::MATERIAL:
        return matProp()[qp];
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
