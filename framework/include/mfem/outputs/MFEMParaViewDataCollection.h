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

#include "MFEMDataCollection.h"

/**
 * Class for output information saved in MFEM ParaViewDataCollections
 */
class MFEMParaViewDataCollection : public MFEMDataCollection
{
public:
  static InputParameters validParams();

  MFEMParaViewDataCollection(const InputParameters & parameters);

  virtual mfem::DataCollection & getDataCollection() override { return _pv_dc; }

  /// Register user-specified scalar coefficients to the DataCollection
  void
  registerScalarCoefficients(std::vector<MFEMScalarCoefficientName> & scalar_coefficient_names);
  /// Register user-specified vector coefficients to the DataCollection
  void
  registerVectorCoefficients(std::vector<MFEMVectorCoefficientName> & vector_coefficient_names);

protected:
  mfem::ParaViewDataCollection _pv_dc;
  std::vector<MFEMScalarCoefficientName> _scalar_coefficient_names;
  std::vector<MFEMVectorCoefficientName> _vector_coefficient_names;
  const bool _high_order_output;
  const unsigned int _refinements;
  const mfem::VTKFormat _vtk_format;
};

#endif
