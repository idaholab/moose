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

protected:
  mfem::ParaViewDataCollection _pv_dc;
  bool _high_order_output;
  unsigned int _refinements;
  const mfem::VTKFormat _vtk_format;
};
