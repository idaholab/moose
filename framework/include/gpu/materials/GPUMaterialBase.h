//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUTypes.h"
#include "GPUMaterialPropertyValue.h"

// initQpStatefulProperties() and computeQpProperties() are intentionally hidden
// but some compilers generate ugly warnings

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#endif

#include "MaterialBase.h"

namespace Moose
{
namespace Kokkos
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

  // Unused for Kokkos materials because all elements are computed in parallel
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
                                                       const std::vector<unsigned int> dims = {})
  {
    std::string prop_name = name;
    if (_pars.have_parameter<MaterialPropertyName>(name))
      prop_name = _pars.get<MaterialPropertyName>(name);

    return declareKokkosPropertyByName<T, dimension>(prop_name, dims);
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
                              const std::vector<unsigned int> dims = {});

  /**
   * Get the material data type
   * @returns The material data type
   */
  virtual Moose::MaterialDataType materialDataType() = 0;

  /**
   * Get the number of elements this material operates on for element material property evaluation
   * @returns The number of elements
   */
  KOKKOS_FUNCTION auto numElements() const { return _element_ids.size(); }
  /**
   * Get the number of sides this material is operating on for face material property evaluation
   * @returns The number of sides
   */
  KOKKOS_FUNCTION auto numElementSides() const { return _element_side_ids.size(); }
  /**
   * Get the element ID for a thread
   * @param tid The thread ID
   * @returns The element ID
   */
  KOKKOS_FUNCTION auto elementID(size_t tid) const { return _element_ids[tid]; }
  /**
   * Get the element ID - side index pair for a thread
   * @param tid The thread ID
   * @returns The element ID - side index pair
   */
  KOKKOS_FUNCTION auto elementSideID(size_t tid) const { return _element_side_ids[tid]; }

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
  /**
   * Element IDs this material operates on for element material property evaluation
   */
  Array<dof_id_type> _element_ids;
  /**
   * Element ID - side index pairs this material operates on for face material property evaluation
   */
  Array<Pair<dof_id_type, unsigned int>> _element_side_ids;
};

template <typename T, unsigned int dimension>
MaterialProperty<T, dimension>
MaterialBase::declareKokkosPropertyByName(const std::string & prop_name,
                                          const std::vector<unsigned int> dims)
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
      prop_name_modified, dims, this, isBoundaryMaterial());

  registerPropName(prop_name_modified, false, 0);

  return prop;
}

} // namespace Kokkos
} // namespace Moose
