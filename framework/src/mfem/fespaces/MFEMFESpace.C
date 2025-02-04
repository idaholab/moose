#include "MFEMFESpace.h"
#include "MFEMProblem.h"

registerMooseObject("PlatypusApp", MFEMFESpace);

InputParameters
MFEMFESpace::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params.registerBase("MFEMFESpace");
  params.addClassDescription("Specifies a finite element space for `MFEMVariable`s to be defined "
                             "with respect to.");
  MooseEnum ordering("NODES VDIM", "VDIM", false);
  params.addParam<MooseEnum>("ordering", ordering, "Ordering style to use for vector DoFs.");
  params += MFEMFECollection::validParams();
  return params;
}

MFEMFESpace::MFEMFESpace(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters),
    _ordering(parameters.get<MooseEnum>("ordering")),
    _fec(parameters),
    _vdim(_fec.getFESpaceVDim()),
    _fespace(buildFESpace())
{
}

const std::shared_ptr<mfem::ParFiniteElementSpace>
MFEMFESpace::buildFESpace()
{
  mfem::ParMesh & pmesh = getMFEMProblem().mesh().getMFEMParMesh();
  mfem::FiniteElementCollection & fec = *_fec.getFEC();

  return std::make_shared<mfem::ParFiniteElementSpace>(&pmesh, &fec, _vdim, _ordering);
}
