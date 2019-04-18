//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseVariableBase.h"

namespace libMesh
{
template <typename>
class DenseVector;
template <typename>
class NumericVector;
class Point;
}

class Assembly;

class MooseVariableFEBase : public MooseVariableBase
{
public:
  MooseVariableFEBase(unsigned int var_num,
                      const FEType & fe_type,
                      SystemBase & sys,
                      Moose::VarKindType var_kind,
                      THREAD_ID tid);

  /**
   * Clear out the dof indices.  We do this in case this variable is not going to be prepared at
   * all...
   */
  virtual void clearDofIndices() = 0;

  /**
   * Prepare the elemental degrees of freedom
   */
  virtual void prepare() = 0;

  /**
   * Prepare the neighbor element degrees of freedom
   */
  virtual void prepareNeighbor() = 0;

  /**
   * Prepare a lower dimensional element's degrees of freedom
   */
  virtual void prepareLowerD() = 0;

  virtual void prepareAux() = 0;

  virtual void reinitNode() = 0;
  virtual void reinitAux() = 0;
  virtual void reinitAuxNeighbor() = 0;

  virtual void reinitNodes(const std::vector<dof_id_type> & nodes) = 0;
  virtual void reinitNodesNeighbor(const std::vector<dof_id_type> & nodes) = 0;

  /**
   * Is this variable nodal
   * @return true if it nodal, otherwise false
   */
  virtual bool isNodal() const = 0;

  /**
   * @returns true if this is a vector-valued element, false otherwise.
   */
  virtual bool isVector() const = 0;

  virtual dof_id_type & nodalDofIndex() = 0;
  virtual dof_id_type & nodalDofIndexNeighbor() = 0;

  /**
   * Current element this variable is evaluated at
   */
  virtual const Elem *& currentElem() const = 0;

  virtual const MooseArray<Point> & normals() const = 0;

  /**
   * The subdomains the variable is active on
   */
  virtual const std::set<SubdomainID> & activeSubdomains() const = 0;
  /**
   * Is the variable active on the subdomain?
   * @param subdomain The subdomain id in question
   * @return true if active on subdomain, false otherwise
   */
  virtual bool activeOnSubdomain(SubdomainID subdomain) const = 0;

  /**
   * Prepare the initial condition
   */
  virtual void prepareIC() = 0;

  /**
   * Compute values at interior quadrature points
   */
  virtual void computeElemValues() = 0;
  /**
   * Compute values at facial quadrature points
   */
  virtual void computeElemValuesFace() = 0;
  /**
   * Compute values at facial quadrature points for the neighbor
   */
  virtual void computeNeighborValuesFace() = 0;
  /**
   * Compute values at quadrature points for the neighbor
   */
  virtual void computeNeighborValues() = 0;
  /**
   * Compute nodal values of this variable in the neighbor
   */
  virtual void computeNodalNeighborValues() = 0;
  /**
   * Compute nodal values of this variable
   */
  virtual void computeNodalValues() = 0;
  /**
   * Set values for this variable to keep everything up to date
   */
  virtual void setDofValues(const DenseVector<Number> & value) = 0;
  /**
   * Get the value of this variable at given node
   */
  virtual Number getNodalValue(const Node & node) = 0;
  /**
   * Get the old value of this variable at given node
   */
  virtual Number getNodalValueOld(const Node & node) = 0;
  /**
   * Get the t-2 value of this variable at given node
   */
  virtual Number getNodalValueOlder(const Node & node) = 0;
  /**
   * Get the current value of this variable on an element
   * @param[in] elem   Element at which to get value
   * @param[in] idx    Local index of this variable's element DoFs
   * @return Variable value
   */
  virtual Number getElementalValue(const Elem * elem, unsigned int idx = 0) const = 0;
  /**
   * Get the old value of this variable on an element
   * @param[in] elem   Element at which to get value
   * @param[in] idx    Local index of this variable's element DoFs
   * @return Variable value
   */
  virtual Number getElementalValueOld(const Elem * elem, unsigned int idx = 0) const = 0;
  /**
   * Get the older value of this variable on an element
   * @param[in] elem   Element at which to get value
   * @param[in] idx    Local index of this variable's element DoFs
   * @return Variable value
   */
  virtual Number getElementalValueOlder(const Elem * elem, unsigned int idx = 0) const = 0;

  virtual void getDofIndices(const Elem * elem, std::vector<dof_id_type> & dof_indices) = 0;
  /**
   * Get neighbor DOF indices for currently selected element
   * @return the neighbor degree of freedom indices
   */
  virtual std::vector<dof_id_type> & dofIndicesNeighbor() = 0;

  /**
   * Get dof indices for the current lower dimensional element (this is meaningful when performing
   * mortar FEM)
   * @return the lower dimensional element's dofs
   */
  virtual const std::vector<dof_id_type> & dofIndicesLower() const = 0;

  virtual unsigned int numberOfDofsNeighbor() = 0;

  virtual void insert(NumericVector<Number> & residual) = 0;
  virtual void add(NumericVector<Number> & residual) = 0;

  /**
   * Deprecated method. Use dofValues
   */
  virtual const MooseArray<Number> & dofValue() = 0;
  /**
   * Returns dof solution on element
   */
  virtual const MooseArray<Number> & dofValues() = 0;
  /**
   * Returns old dof solution on element
   */
  virtual const MooseArray<Number> & dofValuesOld() = 0;
  /**
   * Returns older dof solution on element
   */
  virtual const MooseArray<Number> & dofValuesOlder() = 0;
  /**
   * Returns previous nl solution on element
   */
  virtual const MooseArray<Number> & dofValuesPreviousNL() = 0;
  /**
   * Returns dof solution on neighbor element
   */
  virtual const MooseArray<Number> & dofValuesNeighbor() = 0;
  /**
   * Returns old dof solution on neighbor element
   */
  virtual const MooseArray<Number> & dofValuesOldNeighbor() = 0;
  /**
   * Returns older dof solution on neighbor element
   */
  virtual const MooseArray<Number> & dofValuesOlderNeighbor() = 0;
  /**
   * Returns previous nl solution on neighbor element
   */
  virtual const MooseArray<Number> & dofValuesPreviousNLNeighbor() = 0;
  /**
   * Returns time derivative of degrees of freedom
   */
  virtual const MooseArray<Number> & dofValuesDot() = 0;
  /**
   * Returns time derivative of neighboring degrees of freedom
   */
  virtual const MooseArray<Number> & dofValuesDotNeighbor() = 0;
  /**
   * Returns second time derivative of degrees of freedom
   */
  virtual const MooseArray<Number> & dofValuesDotDot() = 0;
  /**
   * Returns second time derivative of neighboring degrees of freedom
   */
  virtual const MooseArray<Number> & dofValuesDotDotNeighbor() = 0;
  /**
   * Returns old time derivative of degrees of freedom
   */
  virtual const MooseArray<Number> & dofValuesDotOld() = 0;
  /**
   * Returns old time derivative of neighboring degrees of freedom
   */
  virtual const MooseArray<Number> & dofValuesDotOldNeighbor() = 0;
  /**
   * Returns old second time derivative of degrees of freedom
   */
  virtual const MooseArray<Number> & dofValuesDotDotOld() = 0;
  /**
   * Returns old second time derivative of neighboring degrees of freedom
   */
  virtual const MooseArray<Number> & dofValuesDotDotOldNeighbor() = 0;
  /**
   * Returns derivative of time derivative of degrees of freedom
   */
  virtual const MooseArray<Number> & dofValuesDuDotDu() = 0;
  /**
   * Returns derivative of time derivative of neighboring degrees of freedom
   */
  virtual const MooseArray<Number> & dofValuesDuDotDuNeighbor() = 0;
  /**
   * Returns derivative of second time derivative of degrees of freedom
   */
  virtual const MooseArray<Number> & dofValuesDuDotDotDu() = 0;
  /**
   * Returns derivative of second time derivative of neighboring degrees of freedom
   */
  virtual const MooseArray<Number> & dofValuesDuDotDotDuNeighbor() = 0;

  /**
   * Return phi size
   */
  virtual size_t phiSize() = 0;
  /**
   * Return phiFace size
   */
  virtual size_t phiFaceSize() = 0;
  /**
   * Return phiNeighbor size
   */
  virtual size_t phiNeighborSize() = 0;
  /**
   * Return phiFaceNeighbor size
   */
  virtual size_t phiFaceNeighborSize() = 0;
};

