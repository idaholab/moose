#ifdef MFEM_ENABLED

#pragma once

#include "MFEMGeneralUserObject.h"
#include "MFEMContainers.h"

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
  const std::string & getTestVariableName() const { return _test_var_name; }

  virtual mfem::ErrorEstimator * createEstimator() { return nullptr; }

  std::shared_ptr<MFEMEstimator> setUp(std::string estimator_type, std::string estimator_name, InputParameters estimator_params);

protected:
  // Name of (the test variable associated with) the weak form that the kernel is applied to.
  std::string                           _test_var_name;
  std::string                           _kernel_name;
  std::string                           _fe_space_name;
  std::shared_ptr<mfem::ErrorEstimator> _error_estimator;
};

#endif
