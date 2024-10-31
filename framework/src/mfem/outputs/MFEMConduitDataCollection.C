#include "MFEMConduitDataCollection.h"

registerMooseObject("PlatypusApp", MFEMConduitDataCollection);

InputParameters
MFEMConduitDataCollection::validParams()
{
  InputParameters params = MFEMDataCollection::validParams();
  params.addClassDescription("Output for controlling MFEMConduitDataCollection inherited data.");
  params.addParam<std::string>("protocol",
                               "hdf5",
                               "Conduit relay I/O protocol to use. Options: hdf5 (default), json, "
                               "conduit_json, conduit_bin");
  return params;
}

MFEMConduitDataCollection::MFEMConduitDataCollection(const InputParameters & parameters)
  : MFEMDataCollection(parameters),
    _conduit_dc((_file_base + std::string("/Run") + std::to_string(getFileNumber())).c_str(),
                _problem_data._pmesh.get()),
    _protocol(getParam<std::string>("protocol"))
{
  _conduit_dc.SetProtocol(_protocol);
  registerFields();
}
