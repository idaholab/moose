#pragma once
#include "FileOutput.h"
#include "mfem.hpp"

/**
 * Class for output information saved in MFEM DataCollections
 */
class MFEMDataCollection : public FileOutput
{
public:
  static InputParameters validParams();

  MFEMDataCollection(const InputParameters & parameters);

  virtual std::shared_ptr<mfem::DataCollection>
  createDataCollection(const std::string & collection_name) const;

protected:
  void output() override {}
};
