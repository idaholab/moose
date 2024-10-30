#include "MFEMFESpace.h"
#include "MFEMProblem.h"

registerMooseObject("PlatypusApp", MFEMFESpace);

InputParameters
MFEMFESpace::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params.registerBase("MFEMFESpace");
  params.addClassDescription("Specifies a finite element space for MFEMVariables to be defined "
                             "with respect to.");
  params.addParam<int>("vdim", 1, "Dimension of degrees of freedom");
  MooseEnum ordering("NODES VDIM", "VDIM", false);
  params.addParam<MooseEnum>("ordering", ordering, "Ordering style to use for vector DoFs.");
  params += MFEMFECollection::validParams();
  return params;
}

MFEMFESpace::MFEMFESpace(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters),
    _vdim(parameters.get<int>("vdim")),
    _ordering(parameters.get<MooseEnum>("ordering")),
    _fec(parameters),
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
