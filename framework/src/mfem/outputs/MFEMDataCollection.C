#include "MFEMDataCollection.h"

registerMooseObject("PlatypusApp", MFEMDataCollection);

InputParameters
MFEMDataCollection::validParams()
{
  InputParameters params = FileOutput::validParams();
  params.addClassDescription("Output for controlling MFEMDataCollection inherited data.");
  return params;
}

MFEMDataCollection::MFEMDataCollection(const InputParameters & parameters) : FileOutput(parameters)
{
}

std::shared_ptr<mfem::DataCollection>
MFEMDataCollection::createDataCollection(const std::string & collection_name) const
{
  return std::make_shared<mfem::DataCollection>(collection_name);
}