#pragma once
#include "MFEMDataCollection.h"

/**
 * Class for output information saved in MFEM ConduitDataCollections
 */
class MFEMConduitDataCollection : public MFEMDataCollection
{
public:
  static InputParameters validParams();

  MFEMConduitDataCollection(const InputParameters & parameters);

  virtual mfem::DataCollection & getDataCollection() override { return _conduit_dc; }

protected:
  mfem::ConduitDataCollection _conduit_dc;
  MooseEnum _protocol;
};
