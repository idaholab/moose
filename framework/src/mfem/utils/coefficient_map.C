#ifdef MFEM_ENABLED
#include "coefficient_map.h"

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
  {
    throw MooseException("Trying to assign vector of dimension " + std::to_string(new_dim) +
                         " to property '" + name + "' with dimension dimension " +
                         std::to_string(old_dim));
  }
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
  {
    throw MooseException("Trying to assign matrix with dimensions (" + std::to_string(new_height) +
                         ", " + std::to_string(new_width) + ") to property '" + name +
                         "' with dimensions (" + std::to_string(old_height) + ", " +
                         std::to_string(old_width) + ")");
  }
}
}

#endif
