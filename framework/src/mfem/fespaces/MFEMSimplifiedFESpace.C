//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MFEMSimplifiedFESpace.h"
#include "MFEMProblem.h"

InputParameters
MFEMSimplifiedFESpace::validParams()
{
  InputParameters params = MFEMFESpace::validParams();
  params.addClassDescription("Base class for the simplified interfaces to build MFEM finite "
                             "element spaces. It provides the common parameters.");
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
  return params;
}

MFEMSimplifiedFESpace::MFEMSimplifiedFESpace(const InputParameters & parameters)
  : MFEMFESpace(parameters), _fec_order(parameters.get<MooseEnum>("fec_order"))
{
}

int
MFEMSimplifiedFESpace::getProblemDim() const
{
  return getMFEMProblem().mesh().getMFEMParMesh().Dimension();
}

#endif
