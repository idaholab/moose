//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once
#include <map>
#include <set>
#include <string>
#include <memory>
#include <vector>
#include "libmesh/ignore_warnings.h"
#include <mfem.hpp>
#include "libmesh/restore_warnings.h"

namespace Moose::MFEM
{

/// Lightweight adaptor over an std::map from strings to pointer to T
template <typename T>
class NamedFieldsMap
{
public:
  using MapType = std::map<std::string, std::shared_ptr<T>>;
  using const_iterator = typename MapType::const_iterator;

  /// Default initializer.
  NamedFieldsMap() = default;

  /// Destructor.
  ~NamedFieldsMap() { DeregisterAll(); }

  /// Construct new field with name field_name and register.
  template <class FieldType, class... FieldArgs>
  void Register(const std::string & field_name, FieldArgs &&... args)
  {
    Register(field_name, std::make_shared<FieldType>(std::forward<FieldArgs>(args)...));
  }

  /// Register association between field and field_name.
  void Register(const std::string & field_name, std::shared_ptr<T> field)
  {
    CheckFieldIsRegistrable(field_name, field.get());

    Deregister(field_name);

    _field_map[field_name] = std::move(field);
  }

  /// Unregister association between a field and the field_name.
  void Deregister(const std::string & field_name) { _field_map.erase(field_name); }

  /// Predicate to check if a field is registered with name field_name.
  [[nodiscard]] inline bool Has(const std::string & field_name) const
  {
    return FindField(field_name) != end();
  }

  /// Returns a shared pointer to the field. This is guaranteed to return a non-null shared pointer.
  [[nodiscard]] inline std::shared_ptr<T> GetShared(const std::string & field_name) const
  {
    CheckFieldIsRegistered(field_name);

    auto it = FindField(field_name);

    return EnsureFieldPointerIsNonNull(it);
  }

  /// Returns a reference to a field.
  [[nodiscard]] inline T & GetRef(const std::string & field_name) const
  {
    return *GetShared(field_name);
  }

  /// Returns a non-owning pointer to the field. This is guaranteed to return a non-null pointer.
  [[nodiscard]] inline T * Get(const std::string & field_name) const
  {
    return GetShared(field_name).get();
  }

  /// Returns a non-owning pointer to the field where TDerived is a derived class of class T.
  template <typename TDerived>
  [[nodiscard]] inline TDerived * Get(const std::string & field_name) const
  {
    auto ptr = Get(field_name);

    return EnsurePointerCastIsNonNull<TDerived>(ptr);
  }

  /// Returns a vector containing all values for supplied keys.
  [[nodiscard]] std::vector<T *> Get(const std::vector<std::string> & keys) const
  {
    std::vector<T *> values;

    for (const auto & key : keys)
    {
      values.push_back(Get(key));
    }

    values.shrink_to_fit();
    return values;
  }

  /// Returns a begin const iterator to the registered fields.
  // NOLINTNEXTLINE(readability-identifier-naming)
  [[nodiscard]] inline const_iterator begin() const { return _field_map.begin(); }

  /// Returns an end const iterator to the registered fields.
  // NOLINTNEXTLINE(readability-identifier-naming)
  [[nodiscard]] inline const_iterator end() const { return _field_map.end(); }

  /// Returns the number of elements in the map
  int size() { return _field_map.size(); }

protected:
  /// Returns a const iterator to the field.
  [[nodiscard]] inline const_iterator FindField(const std::string & field_name) const
  {
    return _field_map.find(field_name);
  }

  /// Check that the field pointer is valid and the field has not already been registered.
  void CheckFieldIsRegistrable(const std::string & field_name, T * field) const
  {
    if (!field)
    {
      MFEM_ABORT("Cannot register NULL field with name '" << field_name << "'.");
    }

    CheckForDoubleRegistration(field_name, field);
  }

  /// Check for double-registration of a field. A double-registered field may
  /// result in undefined behavior.
  void CheckForDoubleRegistration(const std::string & field_name, T * field) const
  {
    if (Has(field_name) && Get(field_name) == field)
    {
      MFEM_ABORT("The field '" << field_name << "' is already registered.");
    }
  }

  /// Check that a field exists in the map.
  void CheckFieldIsRegistered(const std::string & field_name) const
  {
    if (!Has(field_name))
    {
      MFEM_ABORT("The field '" << field_name << "' has not been registered.");
    }
  }

  /// Ensure that a returned shared pointer is valid.
  inline std::shared_ptr<T> EnsureFieldPointerIsNonNull(const_iterator & iterator) const
  {
    auto owned_ptr = iterator->second;

    if (!owned_ptr)
    {
      MFEM_ABORT("The field '" << iterator->first << "' is NULL.");
    }

    return owned_ptr;
  }

  /// Ensure that a dynamic cast is successful.
  template <typename TDerived>
  inline TDerived * EnsurePointerCastIsNonNull(T * ptr) const
  {
    auto derived_ptr = dynamic_cast<TDerived *>(ptr);

    if (!derived_ptr)
    {
      MFEM_ABORT("The dynamic cast performed on the field pointer failed.");
    }

    return derived_ptr;
  }

  /// Clear all associations between names and fields.
  void DeregisterAll() { _field_map.clear(); }

private:
  MapType _field_map{};
};

/// Lightweight adaptor over a std::map relating names of GridFunctions with the name of their time
/// derivatives
class TimeDerivativeMap
{
public:
  using MapType = std::map<std::string, std::string>;
  using const_iterator = typename MapType::const_iterator;

  inline void addTimeDerivativeAssociation(const std::string & var_name,
                                           const std::string & time_derivative_var_name)
  {
    _field_map.emplace(var_name, time_derivative_var_name);
  }

  inline bool isTimeDerivative(const std::string & time_derivative_var_name) const
  {
    for (auto const & [map_var_name, map_time_derivative_var_name] : _field_map)
    {
      if (map_time_derivative_var_name == time_derivative_var_name)
        return true;
    }
    return false;
  }

  inline bool hasTimeDerivative(const std::string & var_name) const
  {
    return _field_map.count(var_name);
  }

  inline const std::string & getTimeDerivativeName(const std::string & var_name) const
  {
    auto it = _field_map.find(var_name);
    if (it != _field_map.end())
      return it->second;
    else
    {
      mooseError("No variable representing the time derivative of ", var_name, " found.");
      return null_str;
    }
  }

  inline const std::string & getTimeIntegralName(const std::string & time_derivative_var_name) const
  {
    for (auto const & [map_var_name, map_time_derivative_var_name] : _field_map)
    {
      if (map_time_derivative_var_name == time_derivative_var_name)
        return map_var_name;
    }
    mooseError(
        "No variable representing the time integral of ", time_derivative_var_name, " found.");
    return null_str;
  }

  inline static std::string createTimeDerivativeName(std::string_view name)
  {
    return std::string("d") + std::string(name) + std::string("_dt");
  }

private:
  MapType _field_map;
  const std::string null_str;
};

using FECollections = Moose::MFEM::NamedFieldsMap<mfem::FiniteElementCollection>;
using FESpaces = Moose::MFEM::NamedFieldsMap<mfem::ParFiniteElementSpace>;
using SubMeshes = Moose::MFEM::NamedFieldsMap<mfem::ParSubMesh>;

// ParComplexGridFunction does not inherit from ParGridFunction, so we need to
// use a separate NamedFieldsMap for it, but we bundle them together.
struct GridFunctions
{
  Moose::MFEM::NamedFieldsMap<mfem::ParGridFunction> real_gfs;
  Moose::MFEM::NamedFieldsMap<mfem::ParComplexGridFunction> cpx_gfs;
};

} // namespace Moose::MFEM

#endif
