#ifdef MFEM_ENABLED

#pragma once
#include "MFEMDataCollection.h"

/**
 * Class for output information saved in MFEM VisItDataCollections
 */
class MFEMVisItDataCollection : public MFEMDataCollection
{
public:
  static InputParameters validParams();

  MFEMVisItDataCollection(const InputParameters & parameters);

  virtual mfem::DataCollection & getDataCollection() override { return _visit_dc; }

protected:
  mfem::VisItDataCollection _visit_dc;
  const unsigned int _refinements;
};

#endif
