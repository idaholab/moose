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
  InputParameters params = GeneralUserObject::validParams();
  params.registerBase("MFEMCoefficient");
  return params;
}

MFEMCoefficient::MFEMCoefficient(const InputParameters & parameters) : GeneralUserObject(parameters)
{
}

MFEMCoefficient::~MFEMCoefficient() {}
