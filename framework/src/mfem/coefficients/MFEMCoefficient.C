#include "MFEMCoefficient.h"

registerMooseObject("PlatypusApp", MFEMCoefficient);

libMesh::Point
PointFromMFEMVector(const mfem::Vector & vec)
{
  return libMesh::Point(vec.Elem(0), vec.Elem(1), vec.Elem(2));
}

InputParameters
MFEMCoefficient::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params.addClassDescription(
      "Base class for defining mfem::Coefficient objects to add to an MFEMProblem.");
  params.registerBase("MFEMCoefficient");
  return params;
}

MFEMCoefficient::MFEMCoefficient(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters)
{
}

MFEMCoefficient::~MFEMCoefficient() {}
