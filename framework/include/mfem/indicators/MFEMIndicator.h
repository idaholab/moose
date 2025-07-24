#ifdef MFEM_ENABLED

#pragma once

#include "MFEMGeneralUserObject.h"
#include "MFEMContainers.h"
#include "MFEMVariable.h"

/*
Class to construct an MFEM estimator to apply to the equation system.
*/
class MFEMIndicator : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMIndicator(const InputParameters & params);

  virtual ~MFEMIndicator() = default;

  /// Get name of the test variable labelling the weak form this kernel is added to
  const std::string & getTestVariableName() const { return _variable_name; }

  /// Create the estimator internally and return a bool to indicate if it
  /// succeeded. This base class should not be used directly, so we return
  /// false here.
  virtual bool createEstimator() { return false; }

  /// Get shared pointer to FE Space using the name we store when setting up this class
  virtual std::shared_ptr<mfem::ParFiniteElementSpace> getFESpace() const;

  /// Method to fetch the error estimator after creation
  std::shared_ptr<mfem::ErrorEstimator> getEstimator() const;

protected:
  /// Name of (the test variable associated with) the weak form that the kernel is applied to.
  std::string _variable_name;

  /// Reference to the the variable we refine on. Use this refernce for fetching a reference
  /// to the relevant FE space
  const MFEMVariable & _variable;

  std::string _kernel_name;
  std::shared_ptr<mfem::ErrorEstimator> _error_estimator;
};

inline std::shared_ptr<mfem::ErrorEstimator>
MFEMIndicator::getEstimator() const
{
  mooseAssert(_error_estimator,
              "Attempting to retrieve error estimator before it's been constructed");
  return _error_estimator;
}

#endif
