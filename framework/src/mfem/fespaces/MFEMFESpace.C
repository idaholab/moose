#include "MFEMFESpace.h"
#include "MFEMProblem.h"

registerMooseObject("PlatypusApp", MFEMFESpace);

InputParameters
MFEMFESpace::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params.registerBase("MFEMFESpace");
  params.addClassDescription(
      "Specifies a finite element space for `MFEMVariable`s to be defined "
      "with respect to. This involves pairing a collection of shape functions with a mesh.");
  MooseEnum fec_order(
      "CONSTANT FIRST SECOND THIRD FOURTH FIFTH SIXTH SEVENTH EIGHTH NINTH TENTH ELEVENTH TWELFTH"
      "THIRTEENTH FOURTEENTH FIFTEENTH SIXTEENTH SEVENTEENTH EIGHTTEENTH NINETEENTH TWENTIETH "
      "TWENTYFIRST TWENTYSECOND TWENTYTHIRD TWENTYFOURTH TWENTYFIFTH TWENTYSIXTH TWENTYSEVENTH "
      "TWENTYEIGHTH TWENTYNINTH THIRTIETH THIRTYFIRST THIRTYSECOND THIRTYTHIRD THIRTYFOURTH "
      "THIRTYFIFTH THIRTYSIXTH THIRTYSEVENTH THIRTYEIGHTH THIRTYNINTH FORTIETH FORTYFIRST"
      "FORTYSECOND FORTYTHIRD",
      "FIRST",
      true);
  params.addParam<MooseEnum>("fec_order", fec_order, "Order of the FE shape function to use.");
  MooseEnum fec_types("H1 ND RT L2", "H1", true);
  params.addParam<MooseEnum>("fec_type", fec_types, "Specifies the family of FE shape functions.");
  MooseEnum ordering("NODES VDIM", "VDIM", false);
  params.addParam<MooseEnum>("ordering", ordering, "Ordering style to use for vector DoFs.");
  params.addParam<int>("vdim",
                       0,
                       "The dimension of vectors. The default (0) corresponds to scalars in H1 and "
                       "L2 finite element collections and for a vector dimension equal to the "
                       "spatial dimension for ND and RT finite element collections. Note that 2D "
                       "vectors in 1D space are not currently supported for ND and RT elements.");
  return params;
}

MFEMFESpace::MFEMFESpace(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters),
    _fec_order(parameters.get<MooseEnum>("fec_order")),
    _fec_type(parameters.get<MooseEnum>("fec_type")),
    _ordering(parameters.get<MooseEnum>("ordering")),
    _vec_components(parameters.get<int>("vdim")),
    _vdim(_vec_components == 0 || _fec_type == "ND" || _fec_type == "RT" ? 1 : _vec_components),
    _problem_dim(getMFEMProblem().mesh().getMFEMParMesh().Dimension()),
    _fec(buildFEC()),
    _fespace(buildFESpace())
{
}

const std::string
MFEMFESpace::buildFECName() const
{
  return _fec_type + "_" + std::to_string(_problem_dim) + "D_P" + std::to_string(_fec_order);
}

const std::shared_ptr<mfem::FiniteElementCollection>
MFEMFESpace::buildFEC() const
{
  if (_fec_type == "ND" && _vec_components != 0 && _vec_components != _problem_dim)
  {
    if (_vec_components != 3)
      mooseError("No  " + _fec_type + " finite element collection available for " +
                 std::to_string(_vec_components) + "D vectors in " + std::to_string(_problem_dim) +
                 "D space.");
    if (_problem_dim == 1)
    {
      return std::make_shared<mfem::ND_R1D_FECollection>(_fec_order, _problem_dim);
    }
    else
    {
      mooseAssert(dim == 2, "Unsupported spatial dimension " + std::to_string(_problem_dim));
      return std::make_shared<mfem::ND_R2D_FECollection>(_fec_order, _problem_dim);
    }
  }
  else if (_fec_type == "RT" && _vec_components != 0 && _vec_components != _problem_dim)
  {
    if (_vec_components != 3)
      mooseError("No  " + _fec_type + " finite element collection available for " +
                 std::to_string(_vec_components) + "D vectors in " + std::to_string(_problem_dim) +
                 "D space.");
    if (_problem_dim == 1)
    {
      return std::make_shared<mfem::RT_R1D_FECollection>(_fec_order, _problem_dim);
    }
    else
    {
      mooseAssert(_problem_dim == 2,
                  "Unsupported spatial dimension " + std::to_string(_problem_dim));
      return std::make_shared<mfem::RT_R2D_FECollection>(_fec_order, _problem_dim);
    }
  }
  else
  {
    return std::shared_ptr<mfem::FiniteElementCollection>(
        mfem::FiniteElementCollection::New(buildFECName().c_str()));
  }
}

const std::shared_ptr<mfem::ParFiniteElementSpace>
MFEMFESpace::buildFESpace()
{
  mfem::ParMesh & pmesh = getMFEMProblem().mesh().getMFEMParMesh();

  return std::make_shared<mfem::ParFiniteElementSpace>(&pmesh, _fec.get(), _vdim, _ordering);
}
