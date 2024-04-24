#include "MFEMMaterial.h"

registerMooseObject("PlatypusApp", MFEMMaterial);

InputParameters
MFEMMaterial::validParams()
{
  InputParameters params = GeneralUserObject::validParams();

  params.set<std::string>("_moose_base") = "MaterialBase";
  params.addPrivateParam<bool>("_neighbor", false);
  params.addPrivateParam<bool>("_interface", false);

  params.addParam<std::vector<SubdomainName>>(
      "block", "The list of blocks (ids or names) that this object will be applied");
  return params;
}

MFEMMaterial::MFEMMaterial(const InputParameters & parameters)
  : GeneralUserObject(parameters), blocks(getParam<std::vector<SubdomainName>>("block"))
{
}

MFEMMaterial::~MFEMMaterial() {}
