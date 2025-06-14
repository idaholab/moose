//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

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
    _conduit_dc((_file_base).c_str(), &_pmesh),
    _protocol(getParam<MooseEnum>("protocol"))
{
  _conduit_dc.SetProtocol(_protocol);
  registerFields();
}

#endif
