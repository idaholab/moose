//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "ElementUserObject.h"
#include "SerialAccess.h"

#include <unordered_map>

class ProjectedStatefulMaterialNodalPatchRecoveryBase : public ElementUserObject
{
public:
  static InputParameters validParams();

  ProjectedStatefulMaterialNodalPatchRecoveryBase(const InputParameters & parameters)
    : ElementUserObject(parameters)
  {
  }

  virtual Real nodalPatchRecovery(const Point & p,
                                  const std::vector<dof_id_type> & elem_ids,
                                  std::size_t component) const = 0;
};

template <typename T, bool is_ad>
class ProjectedStatefulMaterialNodalPatchRecoveryTempl
  : public ProjectedStatefulMaterialNodalPatchRecoveryBase
{
public:
  static InputParameters validParams();

  ProjectedStatefulMaterialNodalPatchRecoveryTempl(const InputParameters & parameters);

  /**
   * Solve the least-squares problem. Use the fitted coefficients to calculate the value at the
   * requested point.
   *
   * @param p           Point at which to compute the fitted value
   * @param elem_ids    Ids of the elements in the patch
   * @param component   Index of the component to compute the fitted value of
   * @return The fitted value
   */
  virtual Real nodalPatchRecovery(const Point & p,
                                  const std::vector<dof_id_type> & elem_ids,
                                  std::size_t component) const override;

  virtual void initialSetup() override;

  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject &) override;
  virtual void finalize() override;

private:
  /**
   * Compute the P vector at a given point
   * i.e. given dim = 2, order = 2, polynomial P has the following terms:
   * 1
   * x
   * y
   * x^2
   * xy
   * y^2
   *
   * @param q_point point at which to evaluate the polynomial basis
   */
  RealEigenVector evaluateBasisFunctions(const Point & q_point) const;

  /// data type stored for each element
  typedef std::pair<RealEigenMatrix, std::vector<RealEigenVector>> ElementData;

  /// current quadrature point
  unsigned int _qp;

  /// number of scalar components in the recovered type
  std::size_t _n_components;

  /// The polynomial order, default is variable order
  const unsigned int _patch_polynomial_order;

  /// The multi-index table
  const std::vector<std::vector<unsigned int>> _multi_index;

  /// Number of basis functions
  const unsigned int _q;

  /// stored property
  const GenericMaterialProperty<T, is_ad> & _prop;

  /// The element-level A matrix and the element-level b vectors for each component
  std::unordered_map<dof_id_type, ElementData> _abs;

  /// Current subdomain
  const SubdomainID & _current_subdomain_id;

  /// list of required materials that need to be explicitly initialized at step zero
  std::unordered_map<SubdomainID, std::vector<MaterialBase *>> _required_materials;
};

typedef ProjectedStatefulMaterialNodalPatchRecoveryTempl<Real, false>
    ProjectedStatefulMaterialNodalPatchRecoveryReal;
typedef ProjectedStatefulMaterialNodalPatchRecoveryTempl<Real, true>
    ADProjectedStatefulMaterialNodalPatchRecoveryReal;
typedef ProjectedStatefulMaterialNodalPatchRecoveryTempl<RealVectorValue, false>
    ProjectedStatefulMaterialNodalPatchRecoveryRealVectorValue;
typedef ProjectedStatefulMaterialNodalPatchRecoveryTempl<RealVectorValue, true>
    ADProjectedStatefulMaterialNodalPatchRecoveryRealVectorValue;
typedef ProjectedStatefulMaterialNodalPatchRecoveryTempl<RankTwoTensor, false>
    ProjectedStatefulMaterialNodalPatchRecoveryRankTwoTensor;
typedef ProjectedStatefulMaterialNodalPatchRecoveryTempl<RankTwoTensor, true>
    ADProjectedStatefulMaterialNodalPatchRecoveryRankTwoTensor;
typedef ProjectedStatefulMaterialNodalPatchRecoveryTempl<RankFourTensor, false>
    ProjectedStatefulMaterialNodalPatchRecoveryRankFourTensor;
typedef ProjectedStatefulMaterialNodalPatchRecoveryTempl<RankFourTensor, true>
    ADProjectedStatefulMaterialNodalPatchRecoveryRankFourTensor;

// Prevent implicit instantiation in other translation units where these classes are used
extern template class ProjectedStatefulMaterialNodalPatchRecoveryTempl<Real, false>;
extern template class ProjectedStatefulMaterialNodalPatchRecoveryTempl<Real, true>;
extern template class ProjectedStatefulMaterialNodalPatchRecoveryTempl<RealVectorValue, false>;
extern template class ProjectedStatefulMaterialNodalPatchRecoveryTempl<RealVectorValue, true>;
extern template class ProjectedStatefulMaterialNodalPatchRecoveryTempl<RankTwoTensor, false>;
extern template class ProjectedStatefulMaterialNodalPatchRecoveryTempl<RankTwoTensor, true>;
extern template class ProjectedStatefulMaterialNodalPatchRecoveryTempl<RankFourTensor, false>;
extern template class ProjectedStatefulMaterialNodalPatchRecoveryTempl<RankFourTensor, true>;
