#ifdef MFEM_ENABLED

#pragma once

#include "MFEMEstimator.h"
#include "MFEMProblemData.h"

class MFEMZienkiewiczZhuEstimator : public MFEMEstimator
{
public:
  // Need reference to problem here as well!!
  MFEMZienkiewiczZhuEstimator(const InputParameters & parameters);
  virtual ~MFEMZienkiewiczZhuEstimator() = default;

  static InputParameters validParams();

  virtual mfem::ErrorEstimator * createEstimator() override;

protected:
  std::unique_ptr<mfem::H1_FECollection>        _smooth_flux_fec;
  std::unique_ptr<mfem::L2_FECollection>        _flux_fec;
  std::unique_ptr<mfem::ParFiniteElementSpace>  _smooth_flux_fes;
  std::unique_ptr<mfem::ParFiniteElementSpace>  _flux_fes;
};

#endif
