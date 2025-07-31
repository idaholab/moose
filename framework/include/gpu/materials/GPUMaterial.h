//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUMaterialBase.h"
#include "GPUDatum.h"

#include "Coupleable.h"
#include "MaterialPropertyInterface.h"

namespace Moose
{
namespace Kokkos
{

/**
 * The base class for a user to derive their own Kokkos materials.
 *
 * The polymorphic design of the original MOOSE is reproduced statically by leveraging the Curiously
 * Recurring Template Pattern (CRTP), a programming idiom that involves a class template inheriting
 * from a template instantiation of itself. When the user derives their Kokkos object from this
 * class, the inheritance structure will look like:
 *
 * class UserMaterial final : public Moose::Kokkos::Material<UserMaterial>
 *
 * It is important to note that the template argument should point to the last derived class.
 * Therefore, if the user wants to define a derived class that can be further inherited, the derived
 * class should be a class template as well. Otherwise, it is recommended to mark the derived class
 * as final to prevent its inheritence by mistake.
 *
 * The user is expected to define initQpStatefulProperties() and computeQpProperties() as inlined
 * public methods in their derived class (not virtual override). The signature of
 * computeQpProperties() expected to be defined in the derived class is as follows:
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
template <typename Derived>
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
  ///@}

  /**
   * The parallel computation entry functions called by Kokkos
   */
  ///@{
  KOKKOS_FUNCTION void operator()(ElementInit, const dof_id_type tid) const;
  KOKKOS_FUNCTION void operator()(SideInit, const dof_id_type tid) const;
  KOKKOS_FUNCTION void operator()(NeighborInit, const dof_id_type tid) const;
  KOKKOS_FUNCTION void operator()(ElementCompute, const dof_id_type tid) const;
  KOKKOS_FUNCTION void operator()(SideCompute, const dof_id_type tid) const;
  KOKKOS_FUNCTION void operator()(NeighborCompute, const dof_id_type tid) const;
  ///@}

protected:
  /**
   * Get a material property by property name
   * @tparam T The property data type
   * @tparam dimension The property dimension
   * @tparam state The property state
   * @param prop_name_in The property name
   * @returns The material property
   */
  template <typename T, unsigned int dimension, unsigned int state>
  MaterialProperty<T, dimension>
  getKokkosGenericMaterialPropertyByName(const std::string & prop_name_in);
  /**
   * Get a current material property by property name
   * @tparam T The property data type
   * @tparam dimension The property dimension
   * @param prop_name The property name
   * @returns The material property
   */
  template <typename T, unsigned int dimension = 0>
  MaterialProperty<T, dimension> getKokkosMaterialPropertyByName(const std::string & prop_name)
  {
    return getKokkosGenericMaterialPropertyByName<T, dimension, 0>(prop_name);
  }
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
    return getKokkosGenericMaterialPropertyByName<T, dimension, 1>(prop_name);
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
    return getKokkosGenericMaterialPropertyByName<T, dimension, 2>(prop_name);
  }
  /**
   * Get a material property
   * @tparam T The property data type
   * @tparam dimension The property dimension
   * @tparam state The property state
   * @param name The property name or the parameter name containing the property name
   * @returns The material property
   */
  template <typename T, unsigned int dimension, unsigned int state>
  MaterialProperty<T, dimension> getKokkosGenericMaterialProperty(const std::string & name)
  {
    return getKokkosGenericMaterialPropertyByName<T, dimension, state>(
        getMaterialPropertyName(name));
  }
  /**
   * Get a current material property
   * @tparam T The property data type
   * @tparam dimension The property dimension
   * @param name The property name or the parameter name containing the property name
   * @returns The material property
   */
  template <typename T, unsigned int dimension = 0>
  MaterialProperty<T, dimension> getKokkosMaterialProperty(const std::string & name)
  {
    return getKokkosGenericMaterialPropertyByName<T, dimension, 0>(getMaterialPropertyName(name));
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
    return getKokkosGenericMaterialPropertyByName<T, dimension, 1>(getMaterialPropertyName(name));
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
    return getKokkosGenericMaterialPropertyByName<T, dimension, 2>(getMaterialPropertyName(name));
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
  virtual Moose::MaterialDataType materialDataType() override { return _material_data_type; }

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
   * Flag whether initQpStatefulProperties() was not defined in the derived class
   */
  const bool _default_init;

  /**
   * Dummy members unused for Kokkos materials
   */
  ///@{
  const QBase * const & _qrule;
  virtual const QBase & qRule() const override { return *_qrule; }
  ///@}
};

template <typename Derived>
InputParameters
Material<Derived>::validParams()
{
  InputParameters params = MaterialBase::validParams();
  params += MaterialPropertyInterface::validParams();
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");
  return params;
}

template <typename Derived>
Material<Derived>::Material(const InputParameters & parameters)
  : MaterialBase(parameters),
    Coupleable(this, false),
    MaterialPropertyInterface(this, blockIDs(), boundaryIDs()),
    _bnd(_material_data_type != Moose::BLOCK_MATERIAL_DATA),
    _neighbor(_material_data_type == Moose::NEIGHBOR_MATERIAL_DATA),
    _default_init(&Derived::initQpStatefulProperties == &Material::initQpStatefulProperties),
    _qrule(_bnd ? (_neighbor ? _subproblem.assembly(_tid, 0).qRuleNeighbor()
                             : _subproblem.assembly(_tid, 0).qRuleFace())
                : _subproblem.assembly(_tid, 0).qRule())
{
  for (auto coupled_var : getCoupledMooseVars())
    addMooseVariableDependency(coupled_var);
}

template <typename Derived>
Material<Derived>::Material(const Material & object)
  : MaterialBase(object),
    Coupleable(&object, false),
    MaterialPropertyInterface(&object, object.blockIDs(), object.boundaryIDs()),
    _bnd(object._bnd),
    _neighbor(object._neighbor),
    _default_init(object._default_init),
    _qrule(object._qrule)
{
}

template <typename Derived>
void
Material<Derived>::initStatefulProperties(unsigned int)
{
  if (_default_init)
    return;

  if (!_bnd && !_neighbor)
    ::Kokkos::parallel_for(
        ::Kokkos::RangePolicy<ElementInit, ExecSpace, ::Kokkos::IndexType<dof_id_type>>(
            0, numKokkosElements()),
        *static_cast<Derived *>(this));
  else if (_bnd && !_neighbor)
    ::Kokkos::parallel_for(
        ::Kokkos::RangePolicy<SideInit, ExecSpace, ::Kokkos::IndexType<dof_id_type>>(
            0, numKokkosElementSides()),
        *static_cast<Derived *>(this));
  else
    ::Kokkos::parallel_for(
        ::Kokkos::RangePolicy<NeighborInit, ExecSpace, ::Kokkos::IndexType<dof_id_type>>(
            0, numKokkosElementSides()),
        *static_cast<Derived *>(this));

  ::Kokkos::fence();
}

template <typename Derived>
void
Material<Derived>::computeProperties()
{
  if (!_bnd && !_neighbor)
    ::Kokkos::parallel_for(
        ::Kokkos::RangePolicy<ElementCompute, ExecSpace, ::Kokkos::IndexType<dof_id_type>>(
            0, numKokkosElements()),
        *static_cast<Derived *>(this));
  else if (_bnd && !_neighbor)
    ::Kokkos::parallel_for(
        ::Kokkos::RangePolicy<SideCompute, ExecSpace, ::Kokkos::IndexType<dof_id_type>>(
            0, numKokkosElementSides()),
        *static_cast<Derived *>(this));
  else
    ::Kokkos::parallel_for(
        ::Kokkos::RangePolicy<NeighborCompute, ExecSpace, ::Kokkos::IndexType<dof_id_type>>(
            0, numKokkosElementSides()),
        *static_cast<Derived *>(this));

  ::Kokkos::fence();
}

template <typename Derived>
KOKKOS_FUNCTION void
Material<Derived>::operator()(ElementInit, const dof_id_type tid) const
{
  auto material = static_cast<const Derived *>(this);
  auto elem = kokkosElementID(tid);

  Datum datum(elem, kokkosAssembly(), kokkosSystems());

  for (unsigned int qp = 0; qp < datum.n_qps(); qp++)
  {
    datum.reinit();
    material->initQpStatefulProperties(qp, datum);
  }
}

template <typename Derived>
KOKKOS_FUNCTION void
Material<Derived>::operator()(SideInit, const dof_id_type tid) const
{
  auto material = static_cast<const Derived *>(this);
  auto [elem, side] = kokkosElementSideID(tid);

  Datum datum(elem, side, kokkosAssembly(), kokkosSystems());

  for (unsigned int qp = 0; qp < datum.n_qps(); qp++)
  {
    datum.reinit();
    material->initQpStatefulProperties(qp, datum);
  }
}

template <typename Derived>
KOKKOS_FUNCTION void
Material<Derived>::operator()(NeighborInit, const dof_id_type tid) const
{
  auto material = static_cast<const Derived *>(this);
  auto [elem, side] = kokkosElementSideID(tid);

  Datum datum(elem, side, kokkosAssembly(), kokkosSystems());

  for (unsigned int qp = 0; qp < datum.n_qps(); qp++)
  {
    datum.reinit();
    material->initQpStatefulProperties(qp, datum);
  }
}

template <typename Derived>
KOKKOS_FUNCTION void
Material<Derived>::operator()(ElementCompute, const dof_id_type tid) const
{
  auto material = static_cast<const Derived *>(this);
  auto elem = kokkosElementID(tid);

  Datum datum(elem, kokkosAssembly(), kokkosSystems());

  for (unsigned int qp = 0; qp < datum.n_qps(); qp++)
  {
    datum.reinit();
    material->computeQpProperties(qp, datum);
  }
}

template <typename Derived>
KOKKOS_FUNCTION void
Material<Derived>::operator()(SideCompute, const dof_id_type tid) const
{
  auto material = static_cast<const Derived *>(this);
  auto [elem, side] = kokkosElementSideID(tid);

  Datum datum(elem, side, kokkosAssembly(), kokkosSystems());

  for (unsigned int qp = 0; qp < datum.n_qps(); qp++)
  {
    datum.reinit();
    material->computeQpProperties(qp, datum);
  }
}

template <typename Derived>
KOKKOS_FUNCTION void
Material<Derived>::operator()(NeighborCompute, const dof_id_type tid) const
{
  auto material = static_cast<const Derived *>(this);
  auto [elem, side] = kokkosElementSideID(tid);

  Datum datum(elem, side, kokkosAssembly(), kokkosSystems());

  for (unsigned int qp = 0; qp < datum.n_qps(); qp++)
  {
    datum.reinit();
    material->computeQpProperties(qp, datum);
  }
}

template <typename Derived>
template <typename T, unsigned int dimension, unsigned int state>
MaterialProperty<T, dimension>
Material<Derived>::getKokkosGenericMaterialPropertyByName(const std::string & prop_name_in)
{
  MaterialBase::checkExecutionStage();

  const auto prop_name =
      _get_suffix.empty()
          ? prop_name_in
          : MooseUtils::join(std::vector<std::string>({prop_name_in, _get_suffix}), "_");

  if constexpr (state == 0)
    _requested_props.insert(prop_name);

  auto prop =
      MaterialPropertyInterface::getKokkosGenericMaterialPropertyByName<T, dimension, state>(
          prop_name);

  registerPropName(prop_name, true, state);

  return prop;
}

} // namespace Kokkos
} // namespace Moose
