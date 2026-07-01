//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MOOSEToNEML2.h"
#include "NEML2Utils.h"
#include "DomainUserObject.h"

#include "RankTwoTensor.h"
#include "SymmetricRankTwoTensor.h"

#include <set>

/**
 * Gather a MOOSE quantity for insertion into the NEML2 model.
 *
 * In addition to gathering data in element interiors, this object can gather data on boundaries
 * and interfaces (see the 'interface_boundaries' parameter inherited from DomainUserObject). On
 * interfaces, data is gathered from both the element side and the neighboring element side.
 */
template <typename T, unsigned int state>
class MOOSEQuantityToNEML2 : public MOOSEToNEML2, public DomainUserObject
{
public:
  static InputParameters validParams();

  MOOSEQuantityToNEML2(const InputParameters & params);

#ifndef NEML2_ENABLED
  void initialize() override {}
  void executeOnElement() override {}
  void executeOnBoundary() override {}
  void executeOnInterface() override {}
  void finalize() override {}
  void threadJoin(const UserObject &) override {}
#else
  void initialize() override;
  void executeOnElement() override;
  void executeOnBoundary() override;
  void executeOnInterface() override;
  void finalize() override {}
  void threadJoin(const UserObject &) override;

  neml2::Tensor gatheredData() const override;

protected:
  using ElemSide = std::tuple<dof_id_type, unsigned int>;

  /// Where to read the per-quadrature-point data from
  enum class DataSource
  {
    Elem,        ///< element interior
    ElemSide,    ///< current element side (face)
    NeighborSide ///< neighboring element side (on internal/interface sides)
  };

  T qpData(unsigned int, DataSource) const;

  /// MOOSE quantity type to read from
  const NEML2Utils::MOOSEIOType _type;

  ///@{
  /// candidate MOOSE quantities to read data from
  const VariableValue * _var_scalar = nullptr;
  const VariableValue * _var_scalar_old = nullptr;
  const Function * _func = nullptr;
  const MaterialProperty<T> * _mat_prop = nullptr;
  const MaterialProperty<T> * _mat_prop_old = nullptr;
  const VariableValue * _var = nullptr;
  const VariableValue * _var_old = nullptr;
  ///@}

  ///@{
  /// candidate MOOSE quantities to read side/neighbor data from (for boundary/interface gathering)
  const MaterialProperty<T> * _face_mat_prop = nullptr;
  const MaterialProperty<T> * _face_mat_prop_old = nullptr;
  const MaterialProperty<T> * _neighbor_mat_prop = nullptr;
  const MaterialProperty<T> * _neighbor_mat_prop_old = nullptr;
  const VariableValue * _var_neighbor = nullptr;
  const VariableValue * _var_neighbor_old = nullptr;
  ///@}

  /// Whether the gathered data should be batched
  bool _batched = false;

  /// Intermediate data buffer, filled during the element loop
  std::vector<T> _buffer;

  /// Element-side keys already gathered in this iteration
  std::set<ElemSide> _visited_elem_sides;
#endif
};

#define defineMOOSEQuantityToNEML2(T)                                                              \
  using MOOSE##T##ToNEML2 = MOOSEQuantityToNEML2<T, 0>;                                            \
  using MOOSEOld##T##ToNEML2 = MOOSEQuantityToNEML2<T, 1>
defineMOOSEQuantityToNEML2(Real);
defineMOOSEQuantityToNEML2(RankTwoTensor);
defineMOOSEQuantityToNEML2(SymmetricRankTwoTensor);
defineMOOSEQuantityToNEML2(RealVectorValue);
#undef defineMOOSEQuantityToNEML2
