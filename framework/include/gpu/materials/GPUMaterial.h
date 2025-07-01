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

template <typename Derived>
class Material : public MaterialBase, public Coupleable, public MaterialPropertyInterface
{
public:
  static InputParameters validParams()
  {
    InputParameters params = MaterialBase::validParams();
    params += MaterialPropertyInterface::validParams();
    params.addParamNamesToGroup("use_displaced_mesh", "Advanced");
    return params;
  }

  // Constructor
  Material(const InputParameters & parameters)
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

  // Copy constructor
  Material(const Material & object)
    : MaterialBase(object),
      Coupleable(&object, false),
      MaterialPropertyInterface(&object, object.blockIDs(), object.boundaryIDs()),
      _bnd(object._bnd),
      _neighbor(object._neighbor),
      _default_init(object._default_init),
      _qrule(object._qrule)
  {
  }

  virtual bool isBoundaryMaterial() const override { return _bnd; }

  virtual const std::unordered_set<unsigned int> & getMatPropDependencies() const override
  {
    return MaterialPropertyInterface::getMatPropDependencies();
  }

  // Dispatch material property calculation to GPU
  virtual void initStatefulProperties(unsigned int /* n_points */) override
  {
    if (_default_init)
      return;

    if (!_bnd && !_neighbor)
    {
      setVariableDependency();

      ::Kokkos::parallel_for(
          ::Kokkos::RangePolicy<ElementInit, ::Kokkos::IndexType<size_t>>(0, numElements()),
          *static_cast<Derived *>(this));

      setCacheFlags();
    }
    else if (_bnd && !_neighbor)
      ::Kokkos::parallel_for(
          ::Kokkos::RangePolicy<SideInit, ::Kokkos::IndexType<size_t>>(0, numElementSides()),
          *static_cast<Derived *>(this));
    else
      ::Kokkos::parallel_for(
          ::Kokkos::RangePolicy<NeighborInit, ::Kokkos::IndexType<size_t>>(0, numElementSides()),
          *static_cast<Derived *>(this));

    ::Kokkos::fence();
  }
  virtual void computeProperties() override
  {
    if (!_bnd && !_neighbor)
    {
      setVariableDependency();

      ::Kokkos::parallel_for(
          ::Kokkos::RangePolicy<ElementCompute, ::Kokkos::IndexType<size_t>>(0, numElements()),
          *static_cast<Derived *>(this));

      setCacheFlags();
    }
    else if (_bnd && !_neighbor)
      ::Kokkos::parallel_for(
          ::Kokkos::RangePolicy<SideCompute, ::Kokkos::IndexType<size_t>>(0, numElementSides()),
          *static_cast<Derived *>(this));
    else
      ::Kokkos::parallel_for(
          ::Kokkos::RangePolicy<NeighborCompute, ::Kokkos::IndexType<size_t>>(0, numElementSides()),
          *static_cast<Derived *>(this));

    ::Kokkos::fence();
  }

  // Empty methods to prevent compile errors even when these methods were not hidden by the derived
  // class
  KOKKOS_FUNCTION void initQpStatefulProperties(const unsigned int /* qp */,
                                                Datum & /* datum */) const
  {
  }

  // Overloaded operators called by Kokkos::parallel_for
  KOKKOS_FUNCTION void operator()(ElementInit, const size_t tid) const
  {
    auto material = static_cast<const Derived *>(this);
    auto elem = elementID(tid);

    Datum datum(elem, kokkosAssembly(), kokkosSystems());

    for (unsigned int qp = 0; qp < datum.n_qps(); qp++)
    {
      datum.reinit(qp);

      material->initQpStatefulProperties(qp, datum);
    }
  }
  KOKKOS_FUNCTION void operator()(SideInit, const size_t tid) const
  {
    auto material = static_cast<const Derived *>(this);
    auto elem = elementSideID(tid);

    Datum datum(elem.first, elem.second, kokkosAssembly(), kokkosSystems());

    for (unsigned int qp = 0; qp < datum.n_qps(); qp++)
    {
      datum.reinit(qp);

      material->initQpStatefulProperties(qp, datum);
    }
  }
  KOKKOS_FUNCTION void operator()(NeighborInit, const size_t tid) const
  {
    auto material = static_cast<const Derived *>(this);
    auto elem = elementSideID(tid);

    Datum datum(elem.first, elem.second, kokkosAssembly(), kokkosSystems());

    if (datum.hasNeighbor())
      for (unsigned int qp = 0; qp < datum.n_qps(); qp++)
      {
        datum.reinit(qp);

        material->initQpStatefulProperties(qp, datum);
      }
  }
  KOKKOS_FUNCTION void operator()(ElementCompute, const size_t tid) const
  {
    auto material = static_cast<const Derived *>(this);
    auto elem = elementID(tid);

    Datum datum(elem, kokkosAssembly(), kokkosSystems());

    for (unsigned int qp = 0; qp < datum.n_qps(); qp++)
    {
      datum.reinit(qp);

      material->computeQpProperties(qp, datum);
    }
  }
  KOKKOS_FUNCTION void operator()(SideCompute, const size_t tid) const
  {
    auto material = static_cast<const Derived *>(this);
    auto elem = elementSideID(tid);

    Datum datum(elem.first, elem.second, kokkosAssembly(), kokkosSystems());

    for (unsigned int qp = 0; qp < datum.n_qps(); qp++)
    {
      datum.reinit(qp);

      material->computeQpProperties(qp, datum);
    }
  }
  KOKKOS_FUNCTION void operator()(NeighborCompute, const size_t tid) const
  {
    auto material = static_cast<const Derived *>(this);
    auto elem = elementSideID(tid);

    Datum datum(elem.first, elem.second, kokkosAssembly(), kokkosSystems());

    if (datum.hasNeighbor())
      for (unsigned int qp = 0; qp < datum.n_qps(); qp++)
      {
        datum.reinit(qp);

        material->computeQpProperties(qp, datum);
      }
  }

protected:
  template <typename T, unsigned int dimension, unsigned int state>
  MaterialProperty<T, dimension>
  getKokkosGenericMaterialPropertyByName(const std::string & prop_name_in)
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
  template <typename T, unsigned int dimension = 0>
  MaterialProperty<T, dimension> getKokkosMaterialPropertyByName(const std::string & prop_name)
  {
    return getKokkosGenericMaterialPropertyByName<T, dimension, 0>(prop_name);
  }
  template <typename T, unsigned int dimension = 0>
  MaterialProperty<T, dimension> getKokkosMaterialPropertyOldByName(const std::string & prop_name)
  {
    return getKokkosGenericMaterialPropertyByName<T, dimension, 1>(prop_name);
  }
  template <typename T, unsigned int dimension = 0>
  MaterialProperty<T, dimension> getKokkosMaterialPropertyOlderByName(const std::string & prop_name)
  {
    return getKokkosGenericMaterialPropertyByName<T, dimension, 2>(prop_name);
  }

  template <typename T, unsigned int dimension, unsigned int state>
  MaterialProperty<T, dimension> getKokkosGenericMaterialProperty(const std::string & name)
  {
    return getKokkosGenericMaterialPropertyByName<T, dimension, state>(
        getMaterialPropertyName(name));
  }
  template <typename T, unsigned int dimension = 0>
  MaterialProperty<T, dimension> getKokkosMaterialProperty(const std::string & name)
  {
    return getKokkosGenericMaterialPropertyByName<T, dimension, 0>(getMaterialPropertyName(name));
  }
  template <typename T, unsigned int dimension = 0>
  MaterialProperty<T, dimension> getKokkosMaterialPropertyOld(const std::string & name)
  {
    return getKokkosGenericMaterialPropertyByName<T, dimension, 1>(getMaterialPropertyName(name));
  }
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

protected:
  const bool _bnd;
  const bool _neighbor;

  virtual const MaterialData & materialData() const override { return _material_data; }
  virtual MaterialData & materialData() override { return _material_data; }
  virtual Moose::MaterialDataType materialDataType() override { return _material_data_type; }

private:
  // Whether default initQpStatefulProperties is used
  const bool _default_init;

private:
  // Dummy members that should not be accessed by derived classes
  const QBase * const & _qrule;
  virtual const QBase & qRule() const override { return *_qrule; }
};

} // namespace Kokkos
} // namespace Moose
