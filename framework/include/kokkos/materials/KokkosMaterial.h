//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// initQpStatefulProperties() and computeQpProperties() are intentionally hidden
// but some compilers generate ugly warnings

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#endif

#include "KokkosMaterialBase.h"
#include "KokkosDatum.h"

#include "Coupleable.h"
#include "MaterialPropertyInterface.h"

namespace Moose
{
namespace Kokkos
{

/**
 * The base class for a user to derive their own Kokkos materials.
 *
 * The user should define initQpStatefulProperties() and computeQpProperties() as inlined public
 * methods in their derived class (not virtual override). The signature of computeQpProperties()
 * expected to be defined in the derived class is as follows:
 *
 * @param qp The local quadrature point index
 * @param datum The Datum object of the current thread
 *
 * KOKKOS_FUNCTION void computeQpProperties(const unsigned int qp, Datum & datum) const;
 *
 * The signature of initQpStatefulProperties() can be found in the code below, and its definition in
 * the derived class is optional. If it is defined in the derived class, it will hide the default
 * definition in the base class.
 */
class Material : public MaterialBase, public Coupleable, public MaterialPropertyInterface
{
public:
  static InputParameters validParams();

  /**
   * Constructor
   */
  Material(const InputParameters & parameters);

  /**
   * Copy constructor for parallel dispatch
   */
  Material(const Material & object);

  /**
   * Dispatch stateful material property initialization
   */
  virtual void initStatefulProperties(unsigned int) override;
  /**
   * Dispatch material property evaluation
   */
  virtual void computeProperties() override;

  /**
   * Default methods to prevent compile errors even when these methods were not defined in the
   * derived class
   */
  ///@{
  /**
   * Initialize stateful material properties on a quadrature point
   * @param qp The local quadrature point index
   * @param datum The Datum object of the current thread
   */
  KOKKOS_FUNCTION void initQpStatefulProperties(const unsigned int /* qp */,
                                                Datum & /* datum */) const
  {
  }
  /**
   * Get the function pointer of the default initQpStatefulProperties()
   * @returns The function pointer
   */
  static auto defaultInitStateful() { return &Material::initQpStatefulProperties; }
  ///@}

  /**
   * The parallel computation entry functions called by Kokkos
   */
  ///@{
  template <typename Derived>
  KOKKOS_FUNCTION void operator()(ElementInit, const ThreadID tid, const Derived & material) const;
  template <typename Derived>
  KOKKOS_FUNCTION void operator()(SideInit, const ThreadID tid, const Derived & material) const;
  template <typename Derived>
  KOKKOS_FUNCTION void operator()(NeighborInit, const ThreadID tid, const Derived & material) const;
  template <typename Derived>
  KOKKOS_FUNCTION void
  operator()(ElementCompute, const ThreadID tid, const Derived & material) const;
  template <typename Derived>
  KOKKOS_FUNCTION void operator()(SideCompute, const ThreadID tid, const Derived & material) const;
  template <typename Derived>
  KOKKOS_FUNCTION void
  operator()(NeighborCompute, const ThreadID tid, const Derived & material) const;
  ///@}

protected:
  /**
   * Get a material property by property name for any state
   * @tparam T The property data type
   * @tparam dimension The property dimension
   * @tparam state The property state
   * @param prop_name_in The property name
   * @returns The material property
   */
  template <typename T, unsigned int dimension = 0, unsigned int state = 0>
  MaterialProperty<T, dimension> getKokkosMaterialPropertyByName(const std::string & prop_name_in);
  /**
   * Get an old material property by property name
   * @tparam T The property data type
   * @tparam dimension The property dimension
   * @param prop_name The property name
   * @returns The material property
   */
  template <typename T, unsigned int dimension = 0>
  MaterialProperty<T, dimension> getKokkosMaterialPropertyOldByName(const std::string & prop_name)
  {
    return getKokkosMaterialPropertyByName<T, dimension, 1>(prop_name);
  }
  /**
   * Get an older material property by property name
   * @tparam T The property data type
   * @tparam dimension The property dimension
   * @param prop_name The property name
   * @returns The material property
   */
  template <typename T, unsigned int dimension = 0>
  MaterialProperty<T, dimension> getKokkosMaterialPropertyOlderByName(const std::string & prop_name)
  {
    return getKokkosMaterialPropertyByName<T, dimension, 2>(prop_name);
  }
  /**
   * Get a material property for any state
   * @tparam T The property data type
   * @tparam dimension The property dimension
   * @tparam state The property state
   * @param name The property name or the parameter name containing the property name
   * @returns The material property
   */
  template <typename T, unsigned int dimension = 0, unsigned int state = 0>
  MaterialProperty<T, dimension> getKokkosMaterialProperty(const std::string & name)
  {
    return getKokkosMaterialPropertyByName<T, dimension, state>(getMaterialPropertyName(name));
  }
  /**
   * Get an old material property
   * @tparam T The property data type
   * @tparam dimension The property dimension
   * @param name The property name or the parameter name containing the property name
   * @returns The material property
   */
  template <typename T, unsigned int dimension = 0>
  MaterialProperty<T, dimension> getKokkosMaterialPropertyOld(const std::string & name)
  {
    return getKokkosMaterialPropertyByName<T, dimension, 1>(getMaterialPropertyName(name));
  }
  /**
   * Get an older material property
   * @tparam T The property data type
   * @tparam dimension The property dimension
   * @param name The property name or the parameter name containing the property name
   * @returns The material property
   */
  template <typename T, unsigned int dimension = 0>
  MaterialProperty<T, dimension> getKokkosMaterialPropertyOlder(const std::string & name)
  {
    return getKokkosMaterialPropertyByName<T, dimension, 2>(getMaterialPropertyName(name));
  }

  virtual void checkMaterialProperty(const std::string & name, const unsigned int state) override
  {
    // Avoid performing duplicate checks for triple block/face/neighbor materials
    if (boundaryRestricted() || !_bnd)
      MaterialPropertyInterface::checkMaterialProperty(name, state);
  }

  virtual bool isBoundaryMaterial() const override { return _bnd; }

  virtual const std::unordered_set<unsigned int> & getMatPropDependencies() const override
  {
    return MaterialPropertyInterface::getMatPropDependencies();
  }

  virtual const MaterialData & materialData() const override { return _material_data; }
  virtual MaterialData & materialData() override { return _material_data; }
  virtual MaterialDataType materialDataType() override { return _material_data_type; }

  /**
   * Flag whether the material is on faces
   */
  const bool _bnd;
  /**
   * Flag whether the material is on neighbor faces
   */
  const bool _neighbor;

private:
  /**
   * Dummy members unused for Kokkos materials
   */
  ///@{
  const QBase * const & _qrule;
  virtual const QBase & qRule() const override { return *_qrule; }
  ///@}
};

template <typename Derived>
KOKKOS_FUNCTION void
Material::operator()(ElementInit, const ThreadID tid, const Derived & material) const
{
  auto elem = kokkosElementID(tid);

  Datum datum(elem, libMesh::invalid_uint, kokkosAssembly(), kokkosSystems());

  for (unsigned int qp = 0; qp < datum.n_qps(); qp++)
  {
    datum.reinit();
    material.initQpStatefulProperties(qp, datum);
  }
}

template <typename Derived>
KOKKOS_FUNCTION void
Material::operator()(SideInit, const ThreadID tid, const Derived & material) const
{
  auto [elem, side] = kokkosElementSideID(tid);

  Datum datum(elem, side, kokkosAssembly(), kokkosSystems());

  for (unsigned int qp = 0; qp < datum.n_qps(); qp++)
  {
    datum.reinit();
    material.initQpStatefulProperties(qp, datum);
  }
}

template <typename Derived>
KOKKOS_FUNCTION void
Material::operator()(NeighborInit, const ThreadID tid, const Derived & material) const
{
  auto [elem, side] = kokkosElementSideID(tid);

  Datum datum(elem, side, kokkosAssembly(), kokkosSystems());

  for (unsigned int qp = 0; qp < datum.n_qps(); qp++)
  {
    datum.reinit();
    material.initQpStatefulProperties(qp, datum);
  }
}

template <typename Derived>
KOKKOS_FUNCTION void
Material::operator()(ElementCompute, const ThreadID tid, const Derived & material) const
{
  auto elem = kokkosElementID(tid);

  Datum datum(elem, libMesh::invalid_uint, kokkosAssembly(), kokkosSystems());

  for (unsigned int qp = 0; qp < datum.n_qps(); qp++)
  {
    datum.reinit();
    material.computeQpProperties(qp, datum);
  }
}

template <typename Derived>
KOKKOS_FUNCTION void
Material::operator()(SideCompute, const ThreadID tid, const Derived & material) const
{
  auto [elem, side] = kokkosElementSideID(tid);

  Datum datum(elem, side, kokkosAssembly(), kokkosSystems());

  for (unsigned int qp = 0; qp < datum.n_qps(); qp++)
  {
    datum.reinit();
    material.computeQpProperties(qp, datum);
  }
}

template <typename Derived>
KOKKOS_FUNCTION void
Material::operator()(NeighborCompute, const ThreadID tid, const Derived & material) const
{
  auto [elem, side] = kokkosElementSideID(tid);

  Datum datum(elem, side, kokkosAssembly(), kokkosSystems());

  for (unsigned int qp = 0; qp < datum.n_qps(); qp++)
  {
    datum.reinit();
    material.computeQpProperties(qp, datum);
  }
}

template <typename T, unsigned int dimension, unsigned int state>
MaterialProperty<T, dimension>
Material::getKokkosMaterialPropertyByName(const std::string & prop_name_in)
{
  MaterialBase::checkExecutionStage();

  const auto prop_name =
      _get_suffix.empty()
          ? prop_name_in
          : MooseUtils::join(std::vector<std::string>({prop_name_in, _get_suffix}), "_");

  if constexpr (state == 0)
    _requested_props.insert(prop_name);

  auto prop =
      MaterialPropertyInterface::getKokkosMaterialPropertyByName<T, dimension, state>(prop_name);

  registerPropName(prop_name, true, state);

  return prop;
}

} // namespace Kokkos
} // namespace Moose
