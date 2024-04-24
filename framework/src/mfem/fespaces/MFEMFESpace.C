#include "MFEMFESpace.h"

registerMooseObject("PlatypusApp", MFEMFESpace);

InputParameters
MFEMFESpace::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.registerBase("MFEMFESpace");
  MooseEnum fespace_types("H1 ND RT L2", "H1", true);
  params.addParam<MooseEnum>(
      "fespace_type",
      fespace_types,
      "Specifies the family of FE shape functions (FE space) to use for this variable.");
  MooseEnum order(
      "CONSTANT FIRST SECOND THIRD FOURTH FIFTH SIXTH SEVENTH EIGHTH NINTH TENTH ELEVENTH TWELFTH"
      "THIRTEENTH FOURTEENTH FIFTEENTH SIXTEENTH SEVENTEENTH EIGHTTEENTH NINETEENTH TWENTIETH "
      "TWENTYFIRST TWENTYSECOND TWENTYTHIRD TWENTYFOURTH TWENTYFIFTH TWENTYSIXTH TWENTYSEVENTH "
      "TWENTYEIGHTH TWENTYNINTH THIRTIETH THIRTYFIRST THIRTYSECOND THIRTYTHIRD THIRTYFOURTH "
      "THIRTYFIFTH THIRTYSIXTH THIRTYSEVENTH THIRTYEIGHTH THIRTYNINTH FORTIETH FORTYFIRST"
      "FORTYSECOND FORTYTHIRD",
      "FIRST",
      true);
  params.addParam<MooseEnum>("order",
                             order,
                             "Order of the FE shape function to use for this variable (additional"
                             "orders not listed here are allowed,"
                             "depending on the family.");
  params.addParam<int>("vdim", 1, "Dimension of degrees of freedom");
  return params;
}

MFEMFESpace::MFEMFESpace(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    order(parameters.get<MooseEnum>("order")),
    vdim(parameters.get<int>("vdim")),
    fespace_type(parameters.get<MooseEnum>("fespace_type")),
    fec_name(createFECName(fespace_type, order))
{
}

const std::string
MFEMFESpace::createFECName(const std::string & fespace_type, const int order)
{
  return fespace_type + "_3D_P" + std::to_string(order);
}

MFEMFESpace::~MFEMFESpace() {}
