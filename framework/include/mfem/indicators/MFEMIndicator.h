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

  /// Create the estimator internally and return a bool to indicate if it
  /// succeeded. This base class should not be used directly, so we return
  /// false here.
  virtual void createEstimator() = 0;

  /// Get shared pointer to FE Space using the name we store when setting up this class
  virtual mfem::ParFiniteElementSpace & getFESpace() const { return _fespace; }

  mfem::ParMesh & getParMesh() const { return *(_fespace.GetParMesh()); }

  /// Method to fetch the error estimator after creation
  std::shared_ptr<mfem::ErrorEstimator> getEstimator() const;

protected:
  /// Name of (the test variable associated with) the weak form that the kernel is applied to.
  const VariableName & _var_name;

  const std::string & _kernel_name;
  std::shared_ptr<mfem::ErrorEstimator> _error_estimator;

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
