#ifdef MFEM_ENABLED

#pragma once

#include "MFEMGeneralUserObject.h"
#include "MFEMContainers.h"
#include "MFEMVariable.h"

/*
Class to construct an MFEM estimator to apply to the equation system.
*/
class MFEMEstimator : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();

  // also need reference to mfem problem here
  MFEMEstimator(const InputParameters & params);

  virtual ~MFEMEstimator() = default;

  // Get name of the test variable labelling the weak form this kernel is added to
  const std::string & getTestVariableName() const { return _variable_name; }

  // Create the estimator internally and return a bool to indicate if it
  // succeeded. This base class should not be used directly, so we return
  // false here.
  virtual bool createEstimator() { return false; }

  // Get shared pointer to FE Space using the name we store when setting up this class
  virtual std::shared_ptr<mfem::ParFiniteElementSpace> getFESpace() const;

  // Method to fetch the error estimator after creation
  std::shared_ptr<mfem::ErrorEstimator>
  getEstimator() const { return _error_estimator; }

protected:
  // Name of (the test variable associated with) the weak form that the kernel is applied to.
  std::string _variable_name;

  // reference to the above variable (for fetching the the FESpace from)
  const MFEMVariable & _variable;

  std::string _kernel_name;
  std::shared_ptr<mfem::ErrorEstimator> _error_estimator;
};

#endif
