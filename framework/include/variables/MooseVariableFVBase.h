//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseVariableFEBase.h"

namespace libMesh
{
template <typename>
class NumericVector;
}

class FaceInfo;

class MooseVariableFVBase;

template <>
InputParameters validParams<MooseVariableFVBase>();

class MooseVariableFVBase : public MooseVariableFEBase
{
public:
  static InputParameters validParams();

  MooseVariableFVBase(const InputParameters & parameters);

  virtual void prepare() override final
  {
    // mooseError("prepare not supported by MooseVariableFVBase");
  }
  virtual void prepareNeighbor() override final
  {
    // mooseError("prepareNeighbor not supported by MooseVariableFVBase");
  }
  virtual void prepareAux() override final
  {
    // mooseError("prepareAux not supported by MooseVariableFVBase");
  }
  virtual void reinitNode() override final
  {
    // mooseError("reinitNode not supported by MooseVariableFVBase");
  }
  virtual void reinitNodes(const std::vector<dof_id_type> & /*nodes*/) override final
  {
    // mooseError("reinitNodes not supported by MooseVariableFVBase");
  }
  virtual void reinitNodesNeighbor(const std::vector<dof_id_type> & /*nodes*/) override final
  {
    // mooseError("reinitNodesNeighbor not supported by MooseVariableFVBase");
  }
  virtual void reinitAux() override final
  {
    // mooseError("reinitAux not supported by MooseVariableFVBase");
  }
  virtual void reinitAuxNeighbor() override final
  {
    // mooseError("reinitAuxNeighbor not supported by MooseVariableFVBase");
  }
  virtual void prepareLowerD() override final
  {
    // mooseError("prepareLowerD not supported by MooseVariableFVBase");
  }
  virtual const dof_id_type & nodalDofIndex() const override final
  {
    mooseError("nodalDofIndex not supported by MooseVariableFVBase");
  }
  virtual const dof_id_type & nodalDofIndexNeighbor() const override final
  {
    mooseError("nodalDofIndexNeighbor not supported by MooseVariableFVBase");
  }
  virtual size_t phiSize() const override final
  {
    mooseError("phiSize not supported by MooseVariableFVBase");
  }
  virtual size_t phiFaceSize() const override final
  {
    mooseError("phiFaceSize not supported by MooseVariableFVBase");
  }
  virtual size_t phiNeighborSize() const override final
  {
    mooseError("phiNeighborSize not supported by MooseVariableFVBase");
  }
  virtual size_t phiFaceNeighborSize() const override final
  {
    mooseError("phiFaceNeighborSize not supported by MooseVariableFVBase");
  }
  virtual size_t phiLowerSize() const override final
  {
    mooseError("phiLowerSize not supported by MooseVariableFVBase");
  }

  virtual void computeElemValuesFace() override final
  {
    // mooseError("computeElemValuesFace not supported by MooseVariableFVBase");
  }
  virtual void computeNeighborValuesFace() override final
  {
    // mooseError("computeNeighborValuesFace not supported by MooseVariableFVBase");
  }
  virtual void computeNeighborValues() override final
  {
    // mooseError("computeNeighborValues not supported by MooseVariableFVBase");
  }
  virtual void computeLowerDValues() override final
  {
    // mooseError("computeLowerDValues not supported by MooseVariableFVBase");
  }
  virtual void computeNodalNeighborValues() override final
  {
    // mooseError("computeNodalNeighborValues not supported by MooseVariableFVBase");
  }
  virtual void computeNodalValues() override final
  {
    // mooseError("computeNodalValues not supported by MooseVariableFVBase");
  }
  virtual const std::vector<dof_id_type> & dofIndicesLower() const override final
  {
    mooseError("dofIndicesLower not supported by MooseVariableFVBase");
  }
  virtual unsigned int numberOfDofsNeighbor() override final
  {
    mooseError("numberOfDofsNeighbor not supported by MooseVariableFVBase");
  }

  virtual bool isNodal() const override final { return false; }

  virtual bool isNodalDefined() const override final { return false; }

  /// Current neighbor element this variable is being evaluated at in a
  /// face/flux loop - this is nullptr for an elemental loop.
  virtual const Elem * const & currentNeighbor() const = 0;

  /// Compute values at face quadrature points for the element+neighbor (both
  /// sides of the face).
  virtual void computeFaceValues(const FaceInfo & fi) = 0;
};
