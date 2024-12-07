//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseArray.h"
#include "MooseTypes.h"
#include "MeshChangedInterface.h"
#include "MooseVariableDataBase.h"
#include "TheWarehouse.h"

#include "libmesh/tensor_tools.h"
#include "libmesh/vector_value.h"
#include "libmesh/tensor_value.h"
#include "libmesh/type_n_tensor.h"
#include "libmesh/fe_type.h"
#include "libmesh/dof_map.h"
#include "libmesh/enum_fe_family.h"
#include "SubProblem.h"

#include <functional>
#include <vector>

class FaceInfo;
class SystemBase;
class TimeIntegrator;
class Assembly;

template <typename>
class MooseVariableFV;

namespace libMesh
{
class QBase;
}

namespace Moose
{
template <typename T>
void
initDofIndices(T & data, const Elem & elem)
{
  if (data._prev_elem != &elem)
  {
    data._dof_map.dof_indices(&elem, data._dof_indices, data._var_num);
    data._prev_elem = &elem;
  }
}
}

namespace
{
template <typename T, typename T2>
void
assignForAllQps(const T & value, T2 & array, const unsigned int nqp)
{
  for (const auto qp : make_range(nqp))
    array[qp] = value;
}
}

template <typename OutputType>
class MooseVariableDataFV : public MooseVariableDataBase<OutputType>, public MeshChangedInterface
{
public:
  // type for gradient, second and divergence of template class OutputType
  typedef typename libMesh::TensorTools::IncrementRank<OutputType>::type OutputGradient;
  typedef typename libMesh::TensorTools::IncrementRank<OutputGradient>::type OutputSecond;
  typedef typename libMesh::TensorTools::DecrementRank<OutputType>::type OutputDivergence;

  // shortcut for types storing values on quadrature points
  typedef MooseArray<OutputType> FieldVariableValue;
  typedef MooseArray<OutputGradient> FieldVariableGradient;
  typedef MooseArray<OutputSecond> FieldVariableSecond;
  typedef MooseArray<OutputType> FieldVariableCurl;
  typedef MooseArray<OutputDivergence> FieldVariableDivergence;

  // shape function type for the template class OutputType
  typedef typename Moose::ShapeType<OutputType>::type OutputShape;

  // type for gradient, second and divergence of shape functions of template class OutputType
  typedef typename libMesh::TensorTools::IncrementRank<OutputShape>::type OutputShapeGradient;
  typedef typename libMesh::TensorTools::IncrementRank<OutputShapeGradient>::type OutputShapeSecond;
  typedef typename libMesh::TensorTools::DecrementRank<OutputShape>::type OutputShapeDivergence;

  // DoF value type for the template class OutputType
  typedef typename Moose::DOFType<OutputType>::type OutputData;
  typedef MooseArray<OutputData> DoFValue;

  MooseVariableDataFV(const MooseVariableFV<OutputType> & var,
                      SystemBase & sys,
                      THREAD_ID tid,
                      Moose::ElementType element_type,
                      const Elem * const & elem);

  bool isNodal() const override { return false; }
  bool hasDoFsOnNodes() const override { return false; }
  libMesh::FEContinuity getContinuity() const override { return libMesh::DISCONTINUOUS; }

  /**
   * Returns whether this data structure needs automatic differentiation calculations
   */
  bool needsAD() const { return _need_ad; }

  /**
   * Set the geometry type before calculating variables values
   * @param gm_type The type type of geometry; either Volume or Face
   */
  void setGeometry(Moose::GeometryType gm_type);

  //////////////// Heavy lifting computational routines //////////////////////////////

  /**
   * compute the variable values
   */
  void computeValuesFace(const FaceInfo & fi);

  /**
   * compute the variable values
   */
  void computeValues();

  /**
   * compute AD things
   */
  void computeAD(const unsigned int num_dofs, const unsigned int nqp);

  ///////////////////////////// Shape functions /////////////////////////////////////

  /**
   * The current element
   */
  const Elem * const & currentElem() const { return _elem; }

  /**
   * prepare the initial condition
   */
  void prepareIC();

  //////////////////////////////////// Solution getters /////////////////////////////////////

  /**
   * Local solution value
   */
  const FieldVariableValue & sln(Moose::SolutionState state) const;

  /**
   * Local time derivative of solution gradient getter
   */
  const FieldVariableGradient & gradSlnDot() const;

  /**
   * Local second time derivative of solution gradient getter
   */
  const FieldVariableGradient & gradSlnDotDot() const;

  /**
   * Local solution second spatial derivative getter
   * @param state The state of the simulation: current, old, older, previous nl
   */
  const FieldVariableSecond & secondSln(Moose::SolutionState state) const;

  /**
   * Local solution curl getter
   * @param state The state of the simulation: current, old, older
   */
  const FieldVariableCurl & curlSln(Moose::SolutionState state) const;

  const ADTemplateVariableValue<OutputType> & adSln() const
  {
    _need_ad = _need_ad_u = true;
    return _ad_u;
  }

  const ADTemplateVariableGradient<OutputType> & adGradSln() const
  {
    _need_ad = _need_ad_grad_u = true;
    return _ad_grad_u;
  }

  const ADTemplateVariableGradient<OutputType> & adGradSlnDot() const
  {
    mooseError("Gradient of time derivative not yet implemented for FV");
  }

  const ADTemplateVariableSecond<OutputType> & adSecondSln() const
  {
    _need_ad = _need_ad_second_u = true;
    return _ad_second_u;
  }

  const ADTemplateVariableValue<OutputType> & adUDot() const;

  const ADTemplateVariableValue<OutputType> & adUDotDot() const;

  const FieldVariableValue & uDot() const;

  const FieldVariableValue & uDotDot() const;

  const FieldVariableValue & uDotOld() const;

  const FieldVariableValue & uDotDotOld() const;

  const VariableValue & duDotDu() const
  {
    _need_du_dot_du = true;
    return _du_dot_du;
  }

  const VariableValue & duDotDotDu() const
  {
    _need_du_dotdot_du = true;
    return _du_dotdot_du;
  }

  /**
   * Set local DOF values and evaluate the values on quadrature points
   */
  void setDofValues(const DenseVector<OutputData> & values);

  ///@{
  /**
   * dof value setters
   */
  void setDofValue(const OutputData & value, unsigned int index);
  ///@}

  OutputData
  getElementalValue(const Elem * elem, Moose::SolutionState state, unsigned int idx = 0) const;

  ///////////////////////////// dof indices ///////////////////////////////////////////////

  void getDofIndices(const Elem * elem, std::vector<dof_id_type> & dof_indices) const;
  const std::vector<dof_id_type> & dofIndices() const;
  unsigned int numberOfDofs() const;
  void clearDofIndices()
  {
    _dof_indices.clear();
    _prev_elem = nullptr;
  }

  /////////////////////////// DoF value getters /////////////////////////////////////

  const DoFValue & dofValuesDot() const;
  const DoFValue & dofValuesDotOld() const;
  const DoFValue & dofValuesDotDot() const;
  const DoFValue & dofValuesDotDotOld() const;
  const MooseArray<libMesh::Number> & dofValuesDuDotDu() const;
  const MooseArray<libMesh::Number> & dofValuesDuDotDotDu() const;

  /**
   * Return the AD dof values
   */
  const MooseArray<ADReal> & adDofValues() const;

  /**
   * Return the AD dof time derivatives
   */
  const MooseArray<ADReal> & adDofValuesDot() const;

  /////////////////////////////// Increment stuff ///////////////////////////////////////

  /**
   * Increment getter
   * @return The increment
   */
  const FieldVariableValue & increment() const { return _increment; }

  /**
   * Compute and store incremental change in solution at QPs based on increment_vec
   */
  void computeIncrementAtQps(const libMesh::NumericVector<libMesh::Number> & increment_vec);

  /// checks if a Dirichlet BC exists on this face
  bool hasDirichletBC() const { return _has_dirichlet_bc; }

  void meshChanged() override;

protected:
  virtual const MooseVariableFV<OutputType> & var() const override { return _var; }

private:
  void initializeSolnVars();

  /**
   * Helper methods for assigning nodal values from their corresponding solution values (dof
   * values as they're referred to here in this class). These methods are only truly meaningful
   * for nodal basis families
   */
  void fetchADDoFValues();

  /**
   * Helper method that tells us whether it's safe to compute _ad_u_dot
   */
  bool safeToComputeADUDot() const;

  /// A const reference to the owning MooseVariableFV object
  const MooseVariableFV<OutputType> & _var;

  const libMesh::FEType & _fe_type;

  const unsigned int _var_num;

  const Assembly & _assembly;

  /// The element type this object is storing data for. This is either Element, Neighbor, or Lower
  Moose::ElementType _element_type;

  /// Continuity type of the variable
  libMesh::FEContinuity _continuity;

  /// Increment in the variable used in dampers
  FieldVariableValue _increment;

  /// A zero AD variable
  const ADReal _ad_zero;

  /// SolutionState second_u flags
  mutable bool _need_second;
  mutable bool _need_second_old;
  mutable bool _need_second_older;
  mutable bool _need_second_previous_nl;

  /// curl flags
  mutable bool _need_curl;
  mutable bool _need_curl_old;
  mutable bool _need_curl_older;

  /// AD flags
  mutable bool _need_ad;
  mutable bool _need_ad_u;
  mutable bool _need_ad_u_dot;
  mutable bool _need_ad_u_dotdot;
  mutable bool _need_ad_grad_u;
  mutable bool _need_ad_grad_u_dot;
  mutable bool _need_ad_second_u;

  /// grad_u dots
  FieldVariableGradient _grad_u_dot;
  FieldVariableGradient _grad_u_dotdot;

  /// second_u
  FieldVariableSecond _second_u;
  FieldVariableSecond _second_u_old;
  FieldVariableSecond _second_u_older;
  FieldVariableSecond _second_u_previous_nl;

  /// curl_u
  FieldVariableCurl _curl_u;
  FieldVariableCurl _curl_u_old;
  FieldVariableCurl _curl_u_older;

  /// AD u
  ADTemplateVariableValue<OutputShape> _ad_u;
  ADTemplateVariableGradient<OutputShape> _ad_grad_u;
  ADTemplateVariableSecond<OutputShape> _ad_second_u;
  MooseArray<ADReal> _ad_dof_values;
  MooseArray<ADReal> _ad_dofs_dot;
  MooseArray<ADReal> _ad_dofs_dotdot;
  ADTemplateVariableValue<OutputShape> _ad_u_dot;
  ADTemplateVariableValue<OutputShape> _ad_u_dotdot;
  ADTemplateVariableGradient<OutputShape> _ad_grad_u_dot;

  // time derivatives

  /// u_dot (time derivative)
  FieldVariableValue _u_dot;

  /// u_dotdot (second time derivative)
  FieldVariableValue _u_dotdot, _u_dotdot_bak;

  /// u_dot_old (time derivative)
  FieldVariableValue _u_dot_old, _u_dot_old_bak;

  /// u_dotdot_old (second time derivative)
  FieldVariableValue _u_dotdot_old, _u_dotdot_old_bak;

  /// derivative of u_dot wrt u
  VariableValue _du_dot_du;

  /// derivative of u_dotdot wrt u
  VariableValue _du_dotdot_du, _du_dotdot_du_bak;

  /// Pointer to time integrator
  const TimeIntegrator * const _time_integrator;

  /// The current elem. This has to be a reference because the current elem will be constantly
  /// changing. If we initialized this to point to one elem, then in the next calculation we would
  /// be pointing to the wrong place!
  const Elem * const & _elem;
  /// used to keep track of when dof indices are out of date
  mutable const Elem * _prev_elem = nullptr;

  const std::vector<dof_id_type> & initDofIndices();

  /// if this variable has a dirichlet bc defined on a particular face
  bool _has_dirichlet_bc;

  /// Whether this variable is being calculated on a displaced system
  const bool _displaced;

  /// The quadrature rule
  const libMesh::QBase * _qrule;

  /// A dummy ADReal variable
  ADReal _ad_real_dummy = 0;

  /// Cached warehouse query for FVElementalKernels
  TheWarehouse::QueryCache<> _fv_elemental_kernel_query_cache;
  /// Cached warehouse query for FVFluxKernels
  TheWarehouse::QueryCache<> _fv_flux_kernel_query_cache;

  using MooseVariableDataBase<OutputType>::_sys;
  using MooseVariableDataBase<OutputType>::_subproblem;
  using MooseVariableDataBase<OutputType>::_need_vector_tag_dof_u;
  using MooseVariableDataBase<OutputType>::_need_matrix_tag_dof_u;
  using MooseVariableDataBase<OutputType>::_vector_tags_dof_u;
  using MooseVariableDataBase<OutputType>::_matrix_tags_dof_u;
  using MooseVariableDataBase<OutputType>::_vector_tag_u;
  using MooseVariableDataBase<OutputType>::_need_vector_tag_u;
  using MooseVariableDataBase<OutputType>::_vector_tag_grad;
  using MooseVariableDataBase<OutputType>::_need_vector_tag_grad;
  using MooseVariableDataBase<OutputType>::_matrix_tag_u;
  using MooseVariableDataBase<OutputType>::_need_matrix_tag_u;
  using MooseVariableDataBase<OutputType>::_dof_indices;
  using MooseVariableDataBase<OutputType>::_has_dof_values;
  using MooseVariableDataBase<OutputType>::fetchDoFValues;
  using MooseVariableDataBase<OutputType>::assignNodalValue;
  using MooseVariableDataBase<OutputType>::zeroSizeDofValues;
  using MooseVariableDataBase<OutputType>::_solution_tag;
  using MooseVariableDataBase<OutputType>::_old_solution_tag;
  using MooseVariableDataBase<OutputType>::_older_solution_tag;
  using MooseVariableDataBase<OutputType>::_previous_nl_solution_tag;
  using MooseVariableDataBase<OutputType>::_dof_map;
  using MooseVariableDataBase<OutputType>::_need_u_dot;
  using MooseVariableDataBase<OutputType>::_need_u_dotdot;
  using MooseVariableDataBase<OutputType>::_need_u_dot_old;
  using MooseVariableDataBase<OutputType>::_need_u_dotdot_old;
  using MooseVariableDataBase<OutputType>::_need_du_dot_du;
  using MooseVariableDataBase<OutputType>::_need_du_dotdot_du;
  using MooseVariableDataBase<OutputType>::_need_grad_dot;
  using MooseVariableDataBase<OutputType>::_need_grad_dotdot;
  using MooseVariableDataBase<OutputType>::_need_dof_values_dot;
  using MooseVariableDataBase<OutputType>::_need_dof_values_dotdot;
  using MooseVariableDataBase<OutputType>::_need_dof_values_dot_old;
  using MooseVariableDataBase<OutputType>::_need_dof_values_dotdot_old;
  using MooseVariableDataBase<OutputType>::_need_dof_du_dot_du;
  using MooseVariableDataBase<OutputType>::_need_dof_du_dotdot_du;
  using MooseVariableDataBase<OutputType>::_dof_values_dot;
  using MooseVariableDataBase<OutputType>::_dof_values_dotdot;
  using MooseVariableDataBase<OutputType>::_dof_values_dot_old;
  using MooseVariableDataBase<OutputType>::_dof_values_dotdot_old;
  using MooseVariableDataBase<OutputType>::_dof_du_dot_du;
  using MooseVariableDataBase<OutputType>::_dof_du_dotdot_du;
  using MooseVariableDataBase<OutputType>::_tid;
  using MooseVariableDataBase<OutputType>::_nodal_value_dot;
  using MooseVariableDataBase<OutputType>::_nodal_value_dotdot;
  using MooseVariableDataBase<OutputType>::_nodal_value_dot_old;
  using MooseVariableDataBase<OutputType>::_nodal_value_dotdot_old;
  using MooseVariableDataBase<OutputType>::_required_vector_tags;

  friend void Moose::initDofIndices<>(MooseVariableDataFV<OutputType> &, const Elem &);
};

/////////////////////// General template definitions //////////////////////////////////////

template <typename OutputType>
const MooseArray<ADReal> &
MooseVariableDataFV<OutputType>::adDofValues() const
{
  _need_ad = true;
  return _ad_dof_values;
}

template <typename OutputType>
const MooseArray<ADReal> &
MooseVariableDataFV<OutputType>::adDofValuesDot() const
{
  _need_ad = _need_ad_u_dot = true;
  return _ad_dofs_dot;
}

template <typename OutputType>
inline bool
MooseVariableDataFV<OutputType>::safeToComputeADUDot() const
{
  // If we don't have a time integrator then we have no way to calculate _ad_u_dot because we rely
  // on calls to TimeIntegrator::computeADTimeDerivatives. Another potential situation where
  // _ad_u_dot computation is potentially troublesome is if we are an auxiliary variable which uses
  // the auxiliary system copy of the time integrator. Some derived time integrator classes do setup
  // in their solve() method, and that solve() method only happens for the nonlinear system copy of
  // the time integrator.
  return _time_integrator && (_var.kind() == Moose::VAR_SOLVER);
}

template <typename OutputType>
inline const ADTemplateVariableValue<OutputType> &
MooseVariableDataFV<OutputType>::adUDot() const
{
  _need_ad = _need_ad_u_dot = true;

  if (!safeToComputeADUDot())
    // We will just copy the value of _u_dot into _ad_u_dot
    _need_u_dot = true;

  return _ad_u_dot;
}

template <typename OutputType>
const ADTemplateVariableValue<OutputType> &
MooseVariableDataFV<OutputType>::adUDotDot() const
{
  // Generally speaking, we need u dot information when computing u dot dot
  adUDot();

  _need_ad = _need_ad_u_dotdot = true;

  if (!safeToComputeADUDot())
    // We will just copy the value of _u_dotdot into _ad_u_dotdot
    _need_u_dotdot = true;

  return _ad_u_dotdot;
}

template <typename OutputType>
const std::vector<dof_id_type> &
MooseVariableDataFV<OutputType>::dofIndices() const
{
  return const_cast<MooseVariableDataFV<OutputType> *>(this)->initDofIndices();
}

template <typename OutputType>
unsigned int
MooseVariableDataFV<OutputType>::numberOfDofs() const
{
  return dofIndices().size();
}
