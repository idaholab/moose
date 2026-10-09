//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosSideUserObject.h"
#include "KokkosFunction.h"
#include "KokkosMap.h"

/**
 * Kokkos user object computing the heat flux on a set of surfaces in radiative heat transfer with
 * each other. The derived class provides the view factors through setViewFactors().
 *
 * Consumers access the per-boundary results through the registered virtual hooks with
 * getVirtualKokkosUserObject<KokkosGrayLambertSurfaceRadiationBase>(). The hooks are defined by
 * each concrete derived class, which forwards them to the inherited implementations, so that the
 * registration can verify the concrete class implements the interface.
 */
class KokkosGrayLambertSurfaceRadiationBase : public Moose::Kokkos::SideUserObject
{
public:
  static InputParameters validParams();

  KokkosGrayLambertSurfaceRadiationBase(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void finalize() override;

  bool checkVariableBoundaryIntegrity() const override { return false; }

  /// Define enum for boundary type
  enum RAD_BND_TYPE
  {
    VARIABLE_TEMPERATURE = 0,
    FIXED_TEMPERATURE = 4,
    ADIABATIC = 8
  };

  template <typename Derived>
  KOKKOS_FUNCTION void reduce(Datum & datum, Real * result) const;
  template <typename Derived>
  KOKKOS_FUNCTION void join(Real * result, const Real * source) const;
  template <typename Derived>
  KOKKOS_FUNCTION void init(Real * result) const;

  ///@{ device interface of this user object, defined by the concrete derived classes
  KOKKOS_FUNCTION Real getSurfaceIrradiation(BoundaryID id) const;
  KOKKOS_FUNCTION Real getSurfaceHeatFluxDensity(BoundaryID id) const;
  KOKKOS_FUNCTION Real getSurfaceTemperature(BoundaryID id) const;
  KOKKOS_FUNCTION Real getSurfaceRadiosity(BoundaryID id) const;
  KOKKOS_FUNCTION Real getSurfaceEmissivity(BoundaryID id) const;
  ///@}

  /// Whether a boundary participates in the radiative exchange
  KOKKOS_FUNCTION bool hasSurface(BoundaryID id) const { return _side_id_index.exists(id); }

  /**
   * Evaluate the emissivity of every surface at its average temperature into
   * _surface_emissivity. Public because it launches a device lambda.
   */
  void evaluateSurfaceEmissivity();

protected:
  /// a purely virtual function that defines where view factors come from
  virtual std::vector<std::vector<Real>> setViewFactors() = 0;

  ///@{ implementations of the device interface
  KOKKOS_FUNCTION Real surfaceIrradiation(BoundaryID id) const;
  KOKKOS_FUNCTION Real surfaceHeatFluxDensity(BoundaryID id) const;
  KOKKOS_FUNCTION Real surfaceTemperature(BoundaryID id) const;
  KOKKOS_FUNCTION Real surfaceRadiosity(BoundaryID id) const;
  KOKKOS_FUNCTION Real surfaceEmissivity(BoundaryID id) const;
  ///@}

  /// Stefan-Boltzmann constant
  const Real _sigma_stefan_boltzmann;
  /// number of active boundary ids
  const unsigned int _n_sides;
  /// the coupled temperature variable
  const Moose::Kokkos::VariableValue _temperature;
  /// the boundary IDs ordered by the participating side indices
  std::vector<BoundaryID> _side_ids;
  /// side id to index map, side ids can have holes or be out of order
  Moose::Kokkos::Map<BoundaryID, unsigned int> _side_id_index;
  /// the type of the side, allows lookup index -> type
  Moose::Kokkos::Array<unsigned int> _side_type;
  /// fixed temperature functions indexed by participating side index, empty for other sides
  Moose::Kokkos::Array<Moose::Kokkos::Function> _fixed_side_temperature;
  /// emissivity functions indexed by participating side index
  Moose::Kokkos::Array<Moose::Kokkos::Function> _emissivity;
  /// the radiosity of each surface
  Moose::Kokkos::Array<Real> _radiosity;
  /// the heat flux density qdot
  Moose::Kokkos::Array<Real> _heat_flux_density;
  /// the average temperature: this could be important for adiabatic walls
  Moose::Kokkos::Array<Real> _side_temperature;
  /// the emissivity evaluated at the average temperature of each surface
  Moose::Kokkos::Array<Real> _surface_emissivity;
  /// the irradiation into each surface
  Moose::Kokkos::Array<Real> _surface_irradiation;
  /// the view factors which are set by setViewFactors by derived classes
  std::vector<std::vector<Real>> _view_factors;
};

registerVirtualKokkosUserObjectBase(KokkosGrayLambertSurfaceRadiationBase,
                                    getSurfaceIrradiation,
                                    getSurfaceHeatFluxDensity,
                                    getSurfaceTemperature,
                                    getSurfaceRadiosity,
                                    getSurfaceEmissivity);

KOKKOS_FUNCTION inline Real
KokkosGrayLambertSurfaceRadiationBase::surfaceIrradiation(BoundaryID id) const
{
  return _surface_irradiation[_side_id_index[id]];
}

KOKKOS_FUNCTION inline Real
KokkosGrayLambertSurfaceRadiationBase::surfaceHeatFluxDensity(BoundaryID id) const
{
  return _heat_flux_density[_side_id_index[id]];
}

KOKKOS_FUNCTION inline Real
KokkosGrayLambertSurfaceRadiationBase::surfaceTemperature(BoundaryID id) const
{
  return _side_temperature[_side_id_index[id]];
}

KOKKOS_FUNCTION inline Real
KokkosGrayLambertSurfaceRadiationBase::surfaceRadiosity(BoundaryID id) const
{
  return _radiosity[_side_id_index[id]];
}

KOKKOS_FUNCTION inline Real
KokkosGrayLambertSurfaceRadiationBase::surfaceEmissivity(BoundaryID id) const
{
  return _surface_emissivity[_side_id_index[id]];
}

template <typename Derived>
KOKKOS_FUNCTION void
KokkosGrayLambertSurfaceRadiationBase::reduce(Datum & datum, Real * result) const
{
  // The reduction buffer holds areas, sigma * eps * T^4 integrals, and temperature integrals of
  // every participating side in this order. A side contributes to every participating boundary
  // it belongs to.
  const auto & mesh = datum.mesh();
  const auto elem = datum.elemID();
  const auto side = datum.side();

  for (unsigned int b = 0; b < mesh.getNumSideBoundaryIDs(elem, side); ++b)
  {
    const auto id = mesh.getSideBoundaryID(elem, side, b);
    if (!_side_id_index.exists(id))
      continue;

    const auto index = _side_id_index[id];
    const auto type = _side_type[index];

    for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
    {
      const auto JxW = datum.JxW(qp);

      result[index] += JxW;

      if (type == ADIABATIC)
        continue;

      const Real temp = type == VARIABLE_TEMPERATURE
                            ? _temperature(datum, qp)
                            : _fixed_side_temperature[index].value(_t, datum.q_point(qp));

      result[_n_sides + index] += JxW * _sigma_stefan_boltzmann *
                                  _emissivity[index].value(temp, Moose::Kokkos::Real3(0)) * temp *
                                  temp * temp * temp;
      result[2 * _n_sides + index] += JxW * temp;
    }
  }
}

template <typename Derived>
KOKKOS_FUNCTION void
KokkosGrayLambertSurfaceRadiationBase::join(Real * result, const Real * source) const
{
  for (unsigned int i = 0; i < 3 * _n_sides; ++i)
    result[i] += source[i];
}

template <typename Derived>
KOKKOS_FUNCTION void
KokkosGrayLambertSurfaceRadiationBase::init(Real * result) const
{
  for (unsigned int i = 0; i < 3 * _n_sides; ++i)
    result[i] = 0;
}
