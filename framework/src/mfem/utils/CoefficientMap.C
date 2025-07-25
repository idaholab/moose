//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED
#include "CoefficientMap.h"

namespace Moose::MFEM
{
template <>
VectorMap::PWData
VectorMap::emptyPWData(std::shared_ptr<mfem::VectorCoefficient> coeff)
{
  return std::make_tuple(this->make<mfem::PWVectorCoefficient>(coeff->GetVDim()),
                         std::map<const std::string, std::shared_ptr<mfem::VectorCoefficient>>());
}

template <>
MatrixMap::PWData
MatrixMap::emptyPWData(std::shared_ptr<mfem::MatrixCoefficient> coeff)
{
  return std::make_tuple(
      this->make<mfem::PWMatrixCoefficient>(coeff->GetHeight(), coeff->GetWidth()),
      std::map<const std::string, std::shared_ptr<mfem::MatrixCoefficient>>());
}

template <>
void
VectorMap::checkPWData(std::shared_ptr<mfem::VectorCoefficient> coeff,
                       std::shared_ptr<mfem::PWVectorCoefficient> existing_pw,
                       const std::string & name)
{
  const int new_dim = coeff->GetVDim(), old_dim = existing_pw->GetVDim();
  if (new_dim != old_dim)
    mooseError("Trying to assign vector of dimension " + std::to_string(new_dim) +
               " to property '" + name + "' with dimension dimension " + std::to_string(old_dim));
}

template <>
void
MatrixMap::checkPWData(std::shared_ptr<mfem::MatrixCoefficient> coeff,
                       std::shared_ptr<mfem::PWMatrixCoefficient> existing_pw,
                       const std::string & name)
{
  const int new_height = coeff->GetHeight(), new_width = coeff->GetWidth(),
            old_height = existing_pw->GetHeight(), old_width = existing_pw->GetWidth();
  if (new_height != old_height || new_width != old_width)
    mooseError("Trying to assign matrix with dimensions (" + std::to_string(new_height) + ", " +
               std::to_string(new_width) + ") to property '" + name + "' with dimensions (" +
               std::to_string(old_height) + ", " + std::to_string(old_width) + ")");
}
}

#endif
