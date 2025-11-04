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
  const MooseEnum _protocol;
};

#endif
