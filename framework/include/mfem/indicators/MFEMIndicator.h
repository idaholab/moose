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

#include "MFEMGeneralUserObject.h"
#include "MFEMContainers.h"
#include "MFEMVariable.h"

/*
 * Wrapper class for mfem::ErrorEstimator objects. To keep the
 * naming consistent with MOOSE, we refer to it as an Indicator.
 */
class MFEMIndicator : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMIndicator(const InputParameters & params);

  virtual ~MFEMIndicator() = default;

  /// Create the estimator internally.
  virtual void createEstimator() = 0;

  /// Get reference to FE space using the name we store when setting up this class
  mfem::ParFiniteElementSpace & getFESpace() const { return _fespace; }

  /// Get reference to the FE space's underlying mesh
  mfem::ParMesh & getParMesh() const { return *(_fespace.GetParMesh()); }

  /// Method to fetch the error estimator after creation
  std::shared_ptr<mfem::ErrorEstimator> getEstimator() const;

protected:
  /// Name of the variable associated with the weak form that the kernel is applied to
  const VariableName & _var_name;

  /// Name of the kernel providing the error estimate
  const std::string & _kernel_name;

  /// Shared pointer to the MFEM estimator wrapped by this class
  std::shared_ptr<mfem::ErrorEstimator> _error_estimator;

  // FE space that the variable lives in
  mfem::ParFiniteElementSpace & _fespace;
};

inline std::shared_ptr<mfem::ErrorEstimator>
MFEMIndicator::getEstimator() const
{
  mooseAssert(_error_estimator,
              "Attempting to retrieve error estimator before it's been constructed");
  return _error_estimator;
}

#endif
