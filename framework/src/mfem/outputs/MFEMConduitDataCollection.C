#include "MFEMConduitDataCollection.h"

registerMooseObject("MooseApp", MFEMConduitDataCollection);

InputParameters
MFEMConduitDataCollection::validParams()
{
  InputParameters params = MFEMDataCollection::validParams();
  params.addClassDescription("Output for controlling MFEMConduitDataCollection inherited data.");
  MooseEnum protocol("hdf5 json conduit_json conduit_bin", "hdf5", false);
  params.addParam<MooseEnum>("protocol",
                             protocol,
                             "Conduit relay I/O protocol to use. Options: hdf5 (default), json, "
                             "conduit_json, conduit_bin.");
  return params;
}

MFEMConduitDataCollection::MFEMConduitDataCollection(const InputParameters & parameters)
  : MFEMDataCollection(parameters),
    _conduit_dc((_file_base).c_str(), _problem_data._pmesh.get()),
    _protocol(getParam<MooseEnum>("protocol"))
{
  _conduit_dc.SetProtocol(_protocol);
  registerFields();
}
