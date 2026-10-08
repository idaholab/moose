//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosGrayLambertSurfaceRadiationBase.h"

#include "libmesh/dense_matrix.h"
#include "libmesh/dense_vector.h"

InputParameters
KokkosGrayLambertSurfaceRadiationBase::validParams()
{
  InputParameters params = SideUserObject::validParams();
  params.addParam<Real>(
      "stefan_boltzmann_constant",
      5.670367e-8,
      "The Stefan-Boltzmann constant. Default value is in units of [W / m^2 K^4].");
  params.addRequiredCoupledVar("temperature", "The coupled temperature variable.");
  params.addRequiredParam<std::vector<FunctionName>>("emissivity",
                                                     "Emissivities for each boundary.");
  params.addParam<std::vector<BoundaryName>>(
      "fixed_temperature_boundary",
      {},
      "The list of boundary IDs from the mesh with fixed temperatures.");
  params.addParam<std::vector<FunctionName>>(
      "fixed_boundary_temperatures", {}, "The temperatures of the fixed boundary.");
  params.addParam<std::vector<BoundaryName>>(
      "adiabatic_boundary", {}, "The list of boundary IDs from the mesh that are adiabatic.");

  params.addClassDescription(
      "This object implements the exchange of heat by radiation between sidesets.");
  return params;
}

KokkosGrayLambertSurfaceRadiationBase::KokkosGrayLambertSurfaceRadiationBase(
    const InputParameters & parameters)
  : SideUserObject(parameters),
    _sigma_stefan_boltzmann(getParam<Real>("stefan_boltzmann_constant")),
    _n_sides(boundaryIDs().size()),
    _temperature(kokkosCoupledValue("temperature"))
{
  _side_type.create(_n_sides);
  _fixed_side_temperature.create(_n_sides);
  _emissivity.create(_n_sides);
  _radiosity.create(_n_sides);
  _heat_flux_density.create(_n_sides);
  _side_temperature.create(_n_sides);
  _surface_emissivity.create(_n_sides);
  _surface_irradiation.create(_n_sides);

  // set up the map from the side id to the local index & check
  // note that boundaryIDs is not in the right order anymore!
  const auto & boundary_names = getParam<std::vector<BoundaryName>>("boundary");
  const auto & eps_names = getParam<std::vector<FunctionName>>("emissivity");

  // consistency check on emissivity, must be as many entries as boundary
  if (boundary_names.size() != eps_names.size())
    paramError("emissivity", "The number of entries must match the number of boundary entries.");

  for (const auto j : index_range(boundary_names))
  {
    if (boundary_names[j] == "ANY_BOUNDARY_ID")
      paramError("boundary", "boundary must be explicitly provided.");

    const auto id = _subproblem.mesh().getBoundaryID(boundary_names[j]);
    _side_ids.push_back(id);
    _side_id_index[id] = j;
    _side_type[j] = VARIABLE_TEMPERATURE;
    _emissivity[j] = getKokkosFunctionByName(eps_names[j]);
  }

  // get the fixed boundaries of the system if any are provided
  const auto & fixed_names = getParam<std::vector<BoundaryName>>("fixed_temperature_boundary");
  const auto & fixed_temperatures =
      getParam<std::vector<FunctionName>>("fixed_boundary_temperatures");

  if (fixed_names.size() != fixed_temperatures.size())
    paramError(
        "fixed_boundary_temperatures",
        "fixed_boundary_temperatures and fixed_temperature_boundary must have the same length.");

  for (const auto i : index_range(fixed_names))
  {
    // check that fixed side ids is a subset of boundary ids and update the side type
    const auto id = _subproblem.mesh().getBoundaryID(fixed_names[i]);
    if (!_side_id_index.get().count(id))
      paramError("fixed_temperature_boundary",
                 "fixed_temperature_boundary must be a subset of boundary.");

    const auto index = _side_id_index.get()[id];
    _side_type[index] = FIXED_TEMPERATURE;
    _fixed_side_temperature[index] = getKokkosFunctionByName(fixed_temperatures[i]);
  }

  // get the adiabatic boundaries of the system if any are provided
  for (const auto & name : getParam<std::vector<BoundaryName>>("adiabatic_boundary"))
  {
    // check that adiabatic side ids is a subset of boundary ids and update the side type
    const auto id = _subproblem.mesh().getBoundaryID(name);
    if (!_side_id_index.get().count(id))
      paramError("adiabatic_boundary", "adiabatic_boundary must be a subset of boundary.");

    auto & type = _side_type[_side_id_index.get()[id]];

    // make sure that adiabatic boundaries are not already fixed boundaries
    if (type == FIXED_TEMPERATURE)
      paramError("adiabatic_boundary", "Isothermal boundary cannot also be adiabatic boundary.");

    type = ADIABATIC;
  }

  _side_id_index.copyToDevice();
  _side_type.copyToDevice();
}

void
KokkosGrayLambertSurfaceRadiationBase::initialize()
{
  // view factors are obtained here to make sure that another object had
  // time to compute them on exec initial
  _view_factors = setViewFactors();

  // areas, sigma * eps * T^4 integrals, and temperature integrals of each side
  allocateReductionBuffer(3 * _n_sides);
}

void
KokkosGrayLambertSurfaceRadiationBase::finalize()
{
  // need to do some parallel communiction here
  libmesh_call_mpi(MPI_Allreduce(MPI_IN_PLACE,
                                 _reduction_buffer.data(),
                                 _reduction_buffer.size(),
                                 libMesh::Parallel::StandardType<Real>(),
                                 libMesh::Parallel::OpFunction<Real>::sum(),
                                 comm().get()));

  // first compute averages from the totals
  std::vector<Real> beta(_n_sides);
  for (const auto j : make_range(_n_sides))
  {
    const auto area = _reduction_buffer[j];
    beta[j] = _reduction_buffer[_n_sides + j] / area;
    _side_temperature[j] = _reduction_buffer[2 * _n_sides + j] / area;
  }

  evaluateSurfaceEmissivity();

  // matrix and rhs vector for the view factor calculation
  DenseMatrix<Real> matrix(_n_sides, _n_sides);
  DenseVector<Real> rhs(_n_sides);
  DenseVector<Real> radiosity(_n_sides);
  for (const auto i : make_range(_n_sides))
  {
    rhs(i) = beta[i];
    matrix(i, i) = 1;
    for (const auto j : make_range(_n_sides))
    {
      if (_side_type[i] == ADIABATIC)
        matrix(i, j) -= _view_factors[i][j];
      else
        matrix(i, j) -= (1 - _surface_emissivity[i]) * _view_factors[i][j];
    }
  }

  // compute the radiosityes
  matrix.lu_solve(rhs, radiosity);

  // store the radiosity, temperatures and heat flux density for each surface
  for (const auto i : make_range(_n_sides))
  {
    _radiosity[i] = radiosity(i);

    // _heat_flux_density is obtained from a somewhat cumbersome relation
    // but it has the advantage that we do not divide by 1 - emissivity
    // which blows up for black bodies
    _heat_flux_density[i] = radiosity(i);
    for (const auto j : make_range(_n_sides))
      _heat_flux_density[i] -= _view_factors[i][j] * radiosity(j);

    if (_side_type[i] == ADIABATIC)
    {
      const Real e = _surface_emissivity[i];
      _side_temperature[i] = std::pow(
          (radiosity(i) + (1 - e) / e * _heat_flux_density[i]) / _sigma_stefan_boltzmann, 0.25);
    }

    // compute the surface irradiation into i from the radiosities
    _surface_irradiation[i] = 0;
    for (const auto j : make_range(_n_sides))
      _surface_irradiation[i] += _view_factors[i][j] * radiosity(j);
  }

  // the emissivity is reported at the final average temperatures, which changed for adiabatic sides
  evaluateSurfaceEmissivity();

  _radiosity.copyToDevice();
  _heat_flux_density.copyToDevice();
  _surface_irradiation.copyToDevice();
}

void
KokkosGrayLambertSurfaceRadiationBase::evaluateSurfaceEmissivity()
{
  // The emissivity functions are only available in the Kokkos execution space
  _side_temperature.copyToDevice();

  const auto emissivity = _emissivity;
  const auto side_temperature = _side_temperature;
  const auto surface_emissivity = _surface_emissivity;

  ::Kokkos::parallel_for(
      ::Kokkos::RangePolicy<ExecSpace>(0, _n_sides), KOKKOS_LAMBDA(const unsigned int i) {
        surface_emissivity[i] = emissivity[i].value(side_temperature[i], Moose::Kokkos::Real3(0));
      });
  ::Kokkos::fence();

  _surface_emissivity.copyToHost();
}
