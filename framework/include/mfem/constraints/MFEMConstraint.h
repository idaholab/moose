//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "MFEMObject.h"
#include "MFEMBlockRestrictable.h"

/**
 * Base class for objects constraining the value an MFEM variable may take within
 * one or more mesh subdomains, given by 'block'. It holds everything that does not
 * depend on whether the variable is real or complex; the two families of
 * constraint derive from it separately and each declares its own ApplyConstraint
 * taking the grid function type it acts on:
 *
 * - MFEMEssentialConstraint        (mfem::ParGridFunction)
 * - MFEMComplexEssentialConstraint (mfem::ParComplexGridFunction)
 *
 * This mirrors how MFEMBoundaryCondition relates to MFEMEssentialBC and
 * MFEMComplexEssentialBC.
 */
class MFEMConstraint : public MFEMObject, public MFEMBlockRestrictable
{
public:
  static InputParameters validParams();

  MFEMConstraint(const InputParameters & parameters);
  virtual ~MFEMConstraint() = default;

  /// Name of the trial variable (gridfunction) the constraint acts on.
  virtual const std::string & getTrialVariableName() const { return _trial_var_name; }

  /// Reports an error if this constraint shares a subdomain with @p other, a
  /// constraint on the same variable. Restricting the constraints on a variable to
  /// disjoint sets of subdomains makes the dofs a tree-cotree gauge selects independent of
  /// the order the constraints are applied in, since the gauge seeds its spanning
  /// forest with every dof of the subdomains it excludes.
  void checkDisjointSubdomains(MFEMConstraint & other);

protected:
  /// True dofs of @p pfes the constraint fixes: those of the constrained subdomains,
  /// less the subdomain boundary unless 'constrain_subdomain_boundary' is set (see
  /// Moose::MFEM::subdomainDofs). They depend only on the mesh and the space, so
  /// they are cached and only recomputed when the space is updated.
  const mfem::Array<int> & getSubdomainTrueDofs(mfem::ParFiniteElementSpace & pfes);
  /// L-dof marker of the dofs of @p pfes the constraint fixes, matching
  /// getSubdomainTrueDofs() and cached alongside it.
  const mfem::Array<int> & getSubdomainDofMarker(mfem::ParFiniteElementSpace & pfes);

  /// Name of the trial variable the constraint is applied to.
  const VariableName & _trial_var_name;

private:
  /// Rebuilds the cached constrained dofs if @p pfes was updated since they were built.
  void updateSubdomainDofs(mfem::ParFiniteElementSpace & pfes);

  /// Whether the dofs on the boundary of the constrained subdomains are constrained.
  const bool _constrain_subdomain_boundary;
  /// Constrained true dofs of the most recent updateSubdomainDofs() call.
  mfem::Array<int> _subdomain_tdofs;
  /// L-dof marker matching _subdomain_tdofs.
  mfem::Array<int> _subdomain_dof_marker;
  /// FE space sequence number _subdomain_tdofs was built for; -1 marks "never built".
  long _subdomain_tdofs_sequence = -1;
};

#endif
