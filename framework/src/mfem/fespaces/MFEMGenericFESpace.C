#ifdef MFEM_ENABLED

#include "MFEMGenericFESpace.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMGenericFESpace);

InputParameters
MFEMGenericFESpace::validParams()
{
  InputParameters params = MFEMFESpace::validParams();
  params.addClassDescription("Class for creating arbitrary MFEM finite element spaces. It requires "
                             "the user to have some knowledge of how MFEM works.");
  params.addRequiredParam<std::string>("fec_name",
                                       "The name of the finite element collection to use for this "
                                       "space. See MFEM documentation for details.");
  params.addParam<int>("vdim", 1, "The number of degrees of freedom per basis function.");
  return params;
}

MFEMGenericFESpace::MFEMGenericFESpace(const InputParameters & parameters)
  : MFEMFESpace(parameters),
    _fec_name(parameters.get<std::string>("fec_name")),
    _vdim(parameters.get<int>("vdim"))
{
}

std::string
MFEMGenericFESpace::getFECName() const
{
  return _fec_name;
}

int
MFEMGenericFESpace::getVDim() const
{
  return _vdim;
}

bool
MFEMGenericFESpace::isScalar() const
{
  return !isVector();
}

bool
MFEMGenericFESpace::isVector() const
{
  std::string fec = getFECName();
  if (!strncmp(fec.c_str(), "RT", 2) || !strncmp(fec.c_str(), "ND", 2))
  {
    return true;
  }
  // FIXME: This assumes ALL other possible FEC types be scalar. Is that correct?
  return getVDim() > 1;
}

#endif
