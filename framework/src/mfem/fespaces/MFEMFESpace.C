#ifdef MFEM_ENABLED

#include "MFEMFESpace.h"
#include "MFEMProblem.h"

InputParameters
MFEMFESpace::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params.registerBase("MFEMFESpace");
  MooseEnum ordering("NODES VDIM", "VDIM", false);
  params.addParam<MooseEnum>("ordering", ordering, "Ordering style to use for vector DoFs.");
  params.addParam<int>("vdim", 1, "The number of degrees of freedom per basis function.");
  return params;
}

MFEMFESpace::MFEMFESpace(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters), _ordering(parameters.get<MooseEnum>("ordering"))
{
}

void
MFEMFESpace::buildFEC(const std::string & fec_name) const
{
  auto name = fec_name.c_str();
  // Handle a few cases that were not supported by mfem::FiniteElementCollection::New as of v4.7
  if (!strncmp(name, "RT_R1D", 6))
  {
    _fec = std::make_shared<mfem::RT_R1D_FECollection>(atoi(name + 11), atoi(name + 7));
  }
  else if (!strncmp(name, "RT_R2D", 6))
  {
    _fec = std::make_shared<mfem::RT_R2D_FECollection>(atoi(name + 11), atoi(name + 7));
  }
  else if (!strncmp(name, "ND_R1D", 6))
  {
    _fec = std::make_shared<mfem::ND_R1D_FECollection>(atoi(name + 11), atoi(name + 7));
  }
  else if (!strncmp(name, "ND_R2D", 6))
  {
    _fec = std::make_shared<mfem::ND_R2D_FECollection>(atoi(name + 11), atoi(name + 7));
  }
  else
  {
    _fec = std::shared_ptr<mfem::FiniteElementCollection>(mfem::FiniteElementCollection::New(name));
  }
}

void
MFEMFESpace::buildFESpace(const int vdim) const
{
  mfem::ParMesh & pmesh = getMFEMProblem().mesh().getMFEMParMesh();

  _fespace = std::make_shared<mfem::ParFiniteElementSpace>(&pmesh, getFEC().get(), vdim, _ordering);
}

#endif
