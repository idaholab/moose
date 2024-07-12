#include "MFEMFECollection.h"

registerMooseObject("PlatypusApp", MFEMFECollection);

InputParameters
MFEMFECollection::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params.registerBase("MFEMFECollection");

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
  params.addParam<MooseEnum>(
      "fec_type", fec_types, "Specifies the family of FE shape functions (FE space).");

  return params;
}

MFEMFECollection::MFEMFECollection(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters),
    _fec_order(parameters.get<MooseEnum>("fec_order")),
    _fec_type(parameters.get<MooseEnum>("fec_type")),
    _fec_name(buildFECName()),
    _fec(buildFEC())
{
}

const std::string
MFEMFECollection::buildFECName()
{
  return _fec_type + "_3D_P" + std::to_string(_fec_order);
}

const std::shared_ptr<mfem::FiniteElementCollection>
MFEMFECollection::buildFEC()
{
  auto * fec_ptr = mfem::FiniteElementCollection::New(_fec_name.c_str());
  return std::shared_ptr<mfem::FiniteElementCollection>(fec_ptr);
}
