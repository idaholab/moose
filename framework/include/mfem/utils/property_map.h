#pragma once
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

#include "MooseException.h"

#include "mfem.hpp"

namespace platypus
{

/**
 * Class to manage MFEM coefficient objects representing material
 * properties. In particular, it will handle the complexity of
 * piecewise coefficients being built up from multiple materials.
 */
template <class T, class Tpw>
class PropertyMap
{
public:
  void addProperty(const std::string & name, std::unique_ptr<T> && coeff)
  {
    const auto [_, inserted] = this->_properties.emplace(name, std::move(coeff));
    if (!inserted)
    {
      throw MooseException("Property with name '" + name +
                           "' already present in PropertyMap object");
    }
  }

  // Note: If you attempt to overwrite an existing block then an exception will be thrown and data
  // for that property will be left in an undefined state.
  void addPiecewiseBlocks(const std::string & name,
                          std::shared_ptr<T> coeff,
                          const std::vector<std::string> & blocks)
  {
    // Initialise property with empty coefficients, if it does not already exist
    if (!this->hasCoefficient(name))
    {
      this->_properties.insert({name, this->emptyPWData(coeff)});
    }
    PWData * data = std::get_if<PWData>(&this->_properties[name]);
    // Throw an exception if the data is not piecewise
    if (!data)
    {
      throw MooseException("Property with name '" + name +
                           "' already present for all blocks in PropertyMap");
    }
    auto & [pw_coeff, coeff_map] = *data;

    for (const auto & block : blocks)
    {
      if (coeff_map.count(block) > 0)
      {
        throw MooseException("Property with name '" + name + "' already assigned to block " +
                             block + " in PropertyMap object");
      }
      coeff_map[block] = coeff;
      pw_coeff.UpdateCoefficient(std::stoi(block), *coeff);
    }
  }

  T & getCoefficient(const std::string & name)
  {
    auto & coeff = this->_properties.at(name);
    try
    {
      return *std::get<std::unique_ptr<T>>(coeff);
    }
    catch (std::bad_variant_access)
    {
      return std::get<0>(std::get<PWData>(coeff));
    }
  }

  bool hasCoefficient(const std::string & name) { return this->_properties.count(name) > 0; }

private:
  using PWData = std::tuple<Tpw, std::map<const std::string, std::shared_ptr<T>>>;
  std::map<const std::string, std::variant<std::unique_ptr<T>, PWData>> _properties;

  PWData emptyPWData(std::shared_ptr<T> coeff) { return PWData(); }
};

using ScalarMap = PropertyMap<mfem::Coefficient, mfem::PWCoefficient>;
using VectorMap = PropertyMap<mfem::VectorCoefficient, mfem::PWVectorCoefficient>;
using MatrixMap = PropertyMap<mfem::MatrixCoefficient, mfem::PWMatrixCoefficient>;

template <>
VectorMap::PWData
VectorMap::emptyPWData(std::shared_ptr<mfem::VectorCoefficient> coeff)
{
  return std::make_tuple(mfem::PWVectorCoefficient(coeff->GetVDim()),
                         std::map<const std::string, std::shared_ptr<mfem::VectorCoefficient>>());
}

template <>
MatrixMap::PWData
MatrixMap::emptyPWData(std::shared_ptr<mfem::MatrixCoefficient> coeff)
{
  return std::make_tuple(mfem::PWMatrixCoefficient(coeff->GetHeight(), coeff->GetWidth()),
                         std::map<const std::string, std::shared_ptr<mfem::MatrixCoefficient>>());
}
} // namespace platypus
