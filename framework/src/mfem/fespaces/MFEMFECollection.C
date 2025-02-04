#include "MFEMFECollection.h"
#include "MFEMProblem.h"

registerMooseObject("PlatypusApp", MFEMFECollection);

InputParameters
MFEMFECollection::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params.registerBase("MFEMFECollection");
  params.addClassDescription(
      "Specifies a collection of shape functions which (along with a mesh) define a "
      "finite element space for MFEM variables.");
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
  //  params.addParam<int>("dim", 3, "The spatial dimension of the basis the FE shape functions.");
  params.addParam<int>("vdim",
                       0,
                       "The dimension of vectors. The default (0) corresponds to scalars in H1 and "
                       "L2 finite element collections and for a vector dimension equal to the "
                       "spatial dimension for ND and RT finite element collections. Note that 2D "
                       "vectors in 1D space are not currently supported for ND and RT elements.");
  return params;
}

MFEMFECollection::MFEMFECollection(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters),
    _fec_order(parameters.get<MooseEnum>("fec_order")),
    _fec_dim(getMFEMProblem().mesh().getMFEMParMesh().Dimension()),
    _fec_vdim(parameters.get<int>("vdim")),
    _fec_type(parameters.get<MooseEnum>("fec_type")),
    _fec_name(buildFECName()),
    _fec(buildFEC())
{
}

int
MFEMFECollection::getFESpaceVDim() const
{
  if (_fec_vdim == 0 || _fec_type == "ND" || _fec_type == "RT")
  {
    return 1;
  }
  return _fec_vdim;
}

const std::string
MFEMFECollection::buildFECName()
{
  return _fec_type + "_" + std::to_string(_fec_dim) + "D_P" + std::to_string(_fec_order);
}

const std::shared_ptr<mfem::FiniteElementCollection>
MFEMFECollection::buildFEC()
{
  if (_fec_type == "ND" && _fec_vdim != 0 && _fec_vdim != _fec_dim)
  {
    if (_fec_vdim != 3)
      mooseError("No  " + _fec_type + " finite element collection available for " +
                 std::to_string(_fec_vdim) + "D vectors in " + std::to_string(_fec_dim) +
                 "D space.");
    if (_fec_dim == 1)
    {
      return std::make_shared<mfem::ND_R1D_FECollection>(_fec_order, _fec_dim);
    }
    else
    {
      mooseAssert(_fec_dim == 2, "Unsupported spatial dimension " + std::to_string(_fec_dim));
      return std::make_shared<mfem::ND_R2D_FECollection>(_fec_order, _fec_dim);
    }
  }
  else if (_fec_type == "RT" && _fec_vdim != 0 && _fec_vdim != _fec_dim)
  {
    if (_fec_vdim != 3)
      mooseError("No  " + _fec_type + " finite element collection available for " +
                 std::to_string(_fec_vdim) + "D vectors in " + std::to_string(_fec_dim) +
                 "D space.");
    if (_fec_dim == 1)
    {
      return std::make_shared<mfem::RT_R1D_FECollection>(_fec_order, _fec_dim);
    }
    else
    {
      mooseAssert(_fec_dim == 2, "Unsupported spatial dimension " + std::to_string(_fec_dim));
      return std::make_shared<mfem::RT_R2D_FECollection>(_fec_order, _fec_dim);
    }
  }
  else
  {
    return std::shared_ptr<mfem::FiniteElementCollection>(
        mfem::FiniteElementCollection::New(_fec_name.c_str()));
  }
}
