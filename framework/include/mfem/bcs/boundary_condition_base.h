#pragma once
#include <fstream>
#include <iostream>
#include <memory>

#include "mesh_extras.hpp"

namespace platypus
{

class BoundaryCondition
{
public:
  BoundaryCondition(std::string name_, mfem::Array<int> bdr_attributes_);
  mfem::Array<int> GetMarkers(mfem::Mesh & mesh);

  std::string _name;
  mfem::Array<int> _bdr_attributes;
  mfem::Array<int> _markers;

  virtual void ApplyBC(mfem::LinearForm & b) {}
};

} // namespace platypus
