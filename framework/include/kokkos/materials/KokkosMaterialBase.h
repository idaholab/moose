//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosTypes.h"
#include "KokkosMaterialPropertyValue.h"
#include "KokkosDispatcher.h"

#include "MaterialBase.h"

namespace Moose::Kokkos
{

/**
 * The base class for Kokkos materials
 */
class MaterialBase : public ::MaterialBase,
                     public MeshHolder,
                     public AssemblyHolder,
                     public SystemHolder
{
public:
  static InputParameters validParams();

  /**
   * Constructor
   */
  MaterialBase(const InputParameters & parameters);
  /**
   * Copy constructor for parallel dispatch
   */
  MaterialBase(const MaterialBase & object);

  /**
   * Setup block and boundary restrictions
   */
  virtual void initialSetup() override;

  // Unused for Kokkos materials because all subdomains are computed in parallel
  virtual void subdomainSetup() override final {}

  /**
   * Kokkos function tags
   */
  ///@{
  struct ElementInit
  {
  };
  struct SideInit
  {
  };
  struct NeighborInit
  {
  };
  struct ElementCompute
  {
  };
  struct SideCompute
  {
  };
  struct NeighborCompute
  {
  };
  ///@}

protected:
  /**
   * Declare a material property
   * @tparam T The property data type
   * @tparam dimension The property dimension
   * @param name The property name or the parameter name containing the property name
   * @param dims The vector containing the size of each dimension
   * @returns The material property
   */
  template <typename T, unsigned int dimension = 0>
  MaterialProperty<T, dimension> declareKokkosProperty(const std::string & name,
                                                       const std::vector<unsigned int> & dims = {})
  {
    std::string prop_name = name;
    if (_pars.have_parameter<MaterialPropertyName>(name))
      prop_name = _pars.get<MaterialPropertyName>(name);

    return declareKokkosPropertyByName<T, dimension>(prop_name, dims);
  }
  /**
   * Declare a lazy material property
   * @tparam T The property data type
   * @tparam dimension The property dimension
   * @param name The property name or the parameter name containing the property name
   * @param dims The vector containing the size of each dimension
   * @returns The material property
   */
  template <typename T, unsigned int dimension = 0>
  MaterialProperty<T, dimension>
  declareKokkosLazyProperty(const std::string & name, const std::vector<unsigned int> & dims = {})
  {
    std::string prop_name = name;
    if (_pars.have_parameter<MaterialPropertyName>(name))
      prop_name = _pars.get<MaterialPropertyName>(name);

    return declareKokkosLazyPropertyByName<T, dimension>(prop_name, dims);
  }
  /**
   * Declare a material property by property name
   * @tparam T The property data type
   * @tparam dimension The property dimension
   * @param prop_name The property name
   * @param dims The vector containing the size of each dimension
   * @returns The material property
   */
  template <typename T, unsigned int dimension = 0>
  MaterialProperty<T, dimension>
  declareKokkosPropertyByName(const std::string & prop_name,
                              const std::vector<unsigned int> & dims = {})
  {
    return declareKokkosPropertyInternal<T, dimension>(prop_name, dims, false);
  }
  /**
   * Declare a lazy material property by property name
   * The lazy property is only allocated when any object requests it
   * @tparam T The property data type
   * @tparam dimension The property dimension
   * @param prop_name The property name
   * @param dims The vector containing the size of each dimension
   * @returns The material property
   */
  template <typename T, unsigned int dimension = 0>
  MaterialProperty<T, dimension>
  declareKokkosLazyPropertyByName(const std::string & prop_name,
                                  const std::vector<unsigned int> & dims = {})
  {
    return declareKokkosPropertyInternal<T, dimension>(prop_name, dims, true);
  }

  /**
   * Get the number of elements this material operates on for element material property evaluation
   * @returns The number of elements
   */
  KOKKOS_FUNCTION dof_id_type numKokkosElements() const { return _element_ids.size(); }
  /**
   * Get the number of sides this material is operating on for face material property evaluation
   * @returns The number of sides
   */
  KOKKOS_FUNCTION dof_id_type numKokkosElementSides() const { return _element_side_ids.size(); }
  /**
   * Get the contiguous element ID for a thread
   * @param tid The thread ID
   * @returns The contiguous element ID
   */
  KOKKOS_FUNCTION ContiguousElementID kokkosElementID(ThreadID tid) const
  {
    return _element_ids[tid];
  }
  /**
   * Get the contiguous element ID - side index pair for a thread
   * @param tid The thread ID
   * @returns The contiguous element ID - side index pair
   */
  KOKKOS_FUNCTION auto kokkosElementSideID(ThreadID tid) const { return _element_side_ids[tid]; }

  /**
   * Kokkos functor dispatchers
   */
  ///@{
  std::unique_ptr<DispatcherBase> _init_dispatcher;
  std::unique_ptr<DispatcherBase> _compute_dispatcher;
  ///@}

  /**
   * TODO: Move to TransientInterface
   */
  ///@{
  /**
   * Time
   */
  Scalar<Real> _t;
  /**
   * Old time
   */
  Scalar<const Real> _t_old;
  /**
   * The number of the time step
   */
  Scalar<int> _t_step;
  /**
   * Time step size
   */
  Scalar<Real> _dt;
  /**
   * Size of the old time step
   */
  Scalar<Real> _dt_old;
  ///@}

private:
  // Unused for Kokkos materials because they are hidden by Kokkos functions
  virtual void initQpStatefulProperties() override final {}
  virtual void computeQpProperties() override final {}

  /**
   * Internal method for declaring a material property
   * @tparam T The property data type
   * @tparam dimension The property dimension
   * @param prop_name The property name
   * @param dims The vector containing the size of each dimension
   * @param lazy Whether the property is a lazy property
   */
  template <typename T, unsigned int dimension>
  MaterialProperty<T, dimension> declareKokkosPropertyInternal(
      const std::string & prop_name, const std::vector<unsigned int> & dims, const bool lazy);

  /**
   * Contiguous element IDs this material operates on for element material property evaluation
   */
  Array<ContiguousElementID> _element_ids;
  /**
   * Contiguous element ID - side index pairs this material operates on for face material property
   * evaluation
   */
  Array<Pair<ContiguousElementID, unsigned int>> _element_side_ids;
};

template <typename T, unsigned int dimension>
MaterialProperty<T, dimension>
MaterialBase::declareKokkosPropertyInternal(const std::string & prop_name,
                                            const std::vector<unsigned int> & dims,
                                            const bool lazy)
{
  static_assert(dimension <= 4, "Up to four-dimensional Kokkos material properties are allowed.");

  if (dims.size() != dimension)
    mooseError("The declared Kokkos material property '",
               prop_name,
               "'\nhas a different dimension (",
               dimension,
               ") with the provided dimension (",
               dims.size(),
               ").");

  const auto prop_name_modified =
      _declare_suffix.empty()
          ? prop_name
          : MooseUtils::join(std::vector<std::string>({prop_name, _declare_suffix}), "_");

  auto prop = materialData().declareKokkosProperty<T, dimension>(
      prop_name_modified, dims, this, isBoundaryMaterial(), lazy);

  registerPropName(prop_name_modified, false, 0);

  return prop;
}

} // namespace Moose::Kokkos
