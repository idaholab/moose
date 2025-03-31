#ifdef MFEM_ENABLED

#pragma once
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

#include "MooseException.h"
#include "MooseError.h"

#include "libmesh/ignore_warnings.h"
#include <mfem.hpp>
#include "libmesh/restore_warnings.h"

#include "TrackedObjectFactory.h"

namespace Moose::MFEM
{

/**
 * Class to manage MFEM coefficient objects representing material
 * properties. It can build up piecewise coefficients representing
 * properties defined across multiple materials.
 */
template <class T, class Tpw>
class CoefficientMap
{
public:
  CoefficientMap(TrackedObjectFactory<T> & factory) : _manager(factory) {}

  void addCoefficient(const std::string & name, std::shared_ptr<T> coeff)
  {
    mooseAssert(std::find(this->_manager.cbegin(), this->_manager.cend(), coeff) !=
                    this->_manager.cend(),
                "Coefficient object was not created by the appropriate coefficient manager.");

    const auto [_, inserted] = this->_properties.emplace(name, std::move(coeff));
    if (!inserted)
    {
      throw MooseException("Coefficient with name '" + name +
                           "' already present in CoefficientMap object");
    }
  }

  // Note: If you attempt to overwrite an existing block then an exception will be thrown and data
  // for that property will be left in an undefined state.
  void addPiecewiseBlocks(const std::string & name,
                          std::shared_ptr<T> coeff,
                          const std::vector<std::string> & blocks)
  {
    // If list of blocks is empty then treat as a global coefficient
    if (blocks.size() == 0)
    {
      this->addCoefficient(name, coeff);
      return;
    }

    // Initialise property with empty coefficients, if it does not already exist
    if (!this->hasCoefficient(name))
    {
      this->_properties.insert({name, this->emptyPWData(coeff)});
    }
    PWData * data = std::get_if<PWData>(&this->_properties[name]);
    // Throw an exception if the data is not piecewise
    if (!data)
    {
      throw MooseException("Global coefficient with name '" + name +
                           "' already present in CoefficientMap");
    }
    mooseAssert(std::find(this->_manager.cbegin(), this->_manager.cend(), coeff) !=
                    this->_manager.cend(),
                "Coefficient object was not created by the appropriate coefficient manager.");
    auto & [pw_coeff, coeff_map] = *data;
    this->checkPWData(coeff, pw_coeff, name);

    for (const auto & block : blocks)
    {
      if (coeff_map.count(block) > 0)
      {
        throw MooseException("Property with name '" + name + "' already assigned to block " +
                             block + " in CoefficientMap object");
      }
      coeff_map[block] = coeff;
      pw_coeff->UpdateCoefficient(std::stoi(block), *coeff);
    }
  }

  T & getCoefficient(const std::string & name)
  {
    try
    {
      auto & coeff = this->_properties.at(name);
      try
      {
        return *std::get<std::shared_ptr<T>>(coeff);
      }
      catch (std::bad_variant_access &)
      {
        return *std::get<0>(std::get<PWData>(coeff));
      }
    }
    catch (std::out_of_range &)
    {
      throw MooseException("Coefficient with name '" + name + "' has not been declared.");
    }
  }

  std::shared_ptr<T> getCoefficientPtr(const std::string & name)
  {
    try
    {
      auto & coeff = this->_properties.at(name);
      try
      {
        return std::get<std::shared_ptr<T>>(coeff);
      }
      catch (std::bad_variant_access)
      {
        throw MooseException("Property with name '" + name + "' is piecewise.");
      }
    }
    catch (std::out_of_range)
    {
      throw MooseException("Property with name '" + name + "' has not been declared.");
    }
  }

  bool hasCoefficient(const std::string & name) const { return this->_properties.count(name) > 0; }

  bool propertyDefinedOnBlock(const std::string & name, const std::string & block) const
  {
    if (!this->hasCoefficient(name))
      return false;
    auto & coeff = this->_properties.at(name);
    if (std::holds_alternative<std::shared_ptr<T>>(coeff))
      return true;
    auto block_map = std::get<1>(std::get<PWData>(coeff));
    return block_map.count(block) > 0;
  }

private:
  using PWData = std::tuple<std::shared_ptr<Tpw>, std::map<const std::string, std::shared_ptr<T>>>;
  std::map<const std::string, std::variant<std::shared_ptr<T>, PWData>> _properties;
  TrackedObjectFactory<T> & _manager;

  PWData emptyPWData(std::shared_ptr<T> /*coeff*/)
  {
    return std::make_tuple(this->_manager.template make<Tpw>(),
                           std::map<const std::string, std::shared_ptr<T>>());
  }
  void checkPWData(std::shared_ptr<T> /*coeff*/,
                   std::shared_ptr<Tpw> /* existing_pw*/,
                   const std::string & /*name*/)
  {
  }
};

using ScalarMap = CoefficientMap<mfem::Coefficient, mfem::PWCoefficient>;
using VectorMap = CoefficientMap<mfem::VectorCoefficient, mfem::PWVectorCoefficient>;
using MatrixMap = CoefficientMap<mfem::MatrixCoefficient, mfem::PWMatrixCoefficient>;

template <>
VectorMap::PWData VectorMap::emptyPWData(std::shared_ptr<mfem::VectorCoefficient> coeff);

template <>
MatrixMap::PWData MatrixMap::emptyPWData(std::shared_ptr<mfem::MatrixCoefficient> coeff);

template <>
void VectorMap::checkPWData(std::shared_ptr<mfem::VectorCoefficient> coeff,
                            std::shared_ptr<mfem::PWVectorCoefficient> existing_pw,
                            const std::string & name);

template <>
void MatrixMap::checkPWData(std::shared_ptr<mfem::MatrixCoefficient> coeff,
                            std::shared_ptr<mfem::PWMatrixCoefficient> existing_pw,
                            const std::string & name);
} // namespace Moose::MFEM

#endif
