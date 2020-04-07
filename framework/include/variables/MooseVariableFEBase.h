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

class MooseVariableFEBase;
template <>
InputParameters validParams<MooseVariableFEBase>();

class MooseVariableFEBase : public MooseVariableBase
{
public:
  static InputParameters validParams();

  MooseVariableFEBase(const InputParameters & parameters);

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
   * Filed type of this variable
   */
  virtual Moose::VarFieldType fieldType() const = 0;

  /**
   * @returns true if this is a vector-valued element, false otherwise.
   */
  virtual bool isVector() const = 0;

  /**
   * Is this variable defined at nodes
   * @return true if it the variable is defined at nodes, otherwise false
   */
  virtual bool isNodalDefined() const = 0;

  virtual const dof_id_type & nodalDofIndex() const = 0;
  virtual const dof_id_type & nodalDofIndexNeighbor() const = 0;

  /**
   * Current element this variable is evaluated at
   */
  virtual const Elem * const & currentElem() const = 0;

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
   * compute values at quadrature points on the lower dimensional element
   */
  virtual void computeLowerDValues() = 0;
  /**
   * Compute nodal values of this variable in the neighbor
   */
  virtual void computeNodalNeighborValues() = 0;
  /**
   * Compute nodal values of this variable
   */
  virtual void computeNodalValues() = 0;

  /**
   * Get neighbor DOF indices for currently selected element
   * @return the neighbor degree of freedom indices
   */
  virtual const std::vector<dof_id_type> & dofIndicesNeighbor() const = 0;

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
   * Return phi size
   */
  virtual size_t phiSize() const = 0;
  /**
   * Return phiFace size
   */
  virtual size_t phiFaceSize() const = 0;
  /**
   * Return phiNeighbor size
   */
  virtual size_t phiNeighborSize() const = 0;
  /**
   * Return phiFaceNeighbor size
   */
  virtual size_t phiFaceNeighborSize() const = 0;
  /**
   * Return the number of shape functions on the lower dimensional element for this variable
   */
  virtual size_t phiLowerSize() const = 0;
};
