//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ViewFactorBase.h"
#include "libmesh/quadrature.h"

#include <limits>

InputParameters
ViewFactorBase::validParams()
{
  InputParameters params = SideUserObject::validParams();
  params.addParam<Real>("view_factor_tol",
                        std::numeric_limits<Real>::max(),
                        "Tolerance for checking view factors. Default is to allow everything.");
  params.addParam<bool>(
      "print_view_factor_info", false, "Flag to print information about computed view factors.");
  params.addParam<bool>("normalize_view_factor",
                        true,
                        "Determines if view factors are normalized to sum to one (consistent with "
                        "their definition).");
  params.addClassDescription(
      "A base class for automatic computation of view factors between sidesets.");
  return params;
}

ViewFactorBase::ViewFactorBase(const InputParameters & parameters)
  : SideUserObject(parameters),
    _n_sides(boundaryIDs().size()),
    _areas(_n_sides),
    _view_factor_tol(getParam<Real>("view_factor_tol")),
    _normalize_view_factor(getParam<bool>("normalize_view_factor")),
    _print_view_factor_info(getParam<bool>("print_view_factor_info"))
{
  // sizing the view factor array
  _view_factors.resize(_n_sides);
  for (auto & v : _view_factors)
    v.resize(_n_sides);

  // set up the map from the side id to the local index & side name to local index
  std::vector<BoundaryName> boundary_names = getParam<std::vector<BoundaryName>>("boundary");
  for (unsigned int j = 0; j < boundary_names.size(); ++j)
    _side_name_index[boundary_names[j]] = j;
}

unsigned int
ViewFactorBase::getSideNameIndex(std::string name) const
{
  auto it = _side_name_index.find(name);
  if (it == _side_name_index.end())
    mooseError("Boundary ", name, " does not exist.");
  return it->second;
}

Real
ViewFactorBase::getViewFactor(BoundaryID from_id, BoundaryID to_id) const
{
  auto from_name = _mesh.getBoundaryName(from_id);
  auto to_name = _mesh.getBoundaryName(to_id);

  return getViewFactor(from_name, to_name);
}

Real
ViewFactorBase::getViewFactor(BoundaryName from_name, BoundaryName to_name) const
{
  auto from = _side_name_index.find(from_name);
  auto to = _side_name_index.find(to_name);
  if (from == _side_name_index.end())
    mooseError("Boundary id ",
               _mesh.getBoundaryID(from_name),
               " with name ",
               from_name,
               " not listed in boundary parameter.");

  if (to == _side_name_index.end())
    mooseError("Boundary id ",
               _mesh.getBoundaryID(to_name),
               " with name ",
               to_name,
               " not listed in boundary parameter.");

  return _view_factors[from->second][to->second];
}

void
ViewFactorBase::finalize()
{
  // do some communication before finalizing view_factors
  for (unsigned int i = 0; i < _n_sides; ++i)
    gatherSum(_view_factors[i]);

  finalizeViewFactor();
  checkAndNormalizeViewFactor();
}

void
ViewFactorBase::threadJoin(const UserObject & y)
{
  const ViewFactorBase & pps = static_cast<const ViewFactorBase &>(y);
  for (unsigned int i = 0; i < _n_sides; ++i)
  {
    for (unsigned int j = 0; j < _n_sides; ++j)
      _view_factors[i][j] += pps._view_factors[i][j];
  }
  threadJoinViewFactor(y);
}

Real
ViewFactorBase::devReciprocity(unsigned int i, unsigned int j) const
{
  return _view_factors[i][j] - _areas[j] / _areas[i] * _view_factors[j][i];
}

Real
ViewFactorBase::maxDevReciprocity() const
{
  Real v = 0;
  for (unsigned int i = 0; i < _n_sides; ++i)
    for (unsigned int j = i + 1; j < _n_sides; ++j)
    {
      Real s = std::abs(devReciprocity(i, j));
      if (s > v)
        v = s;
    }
  return v;
}

Real
ViewFactorBase::viewFactorRowSum(unsigned int i) const
{
  Real s = 0;
  for (unsigned int to = 0; to < _n_sides; ++to)
    s += _view_factors[i][to];
  return s;
}

Real
ViewFactorBase::maxDevRowSum() const
{
  Real v = 0;
  for (unsigned int i = 0; i < _n_sides; ++i)
  {
    Real s = std::abs(1 - viewFactorRowSum(i));
    if (s > v)
      v = s;
  }
  return v;
}

void
ViewFactorBase::checkAndNormalizeViewFactor()
{
  // collect statistics
  Real max_sum_deviation = maxDevRowSum();
  Real max_reciprocity_deviation = maxDevReciprocity();
  Real max_correction = std::numeric_limits<Real>::lowest();
  Real min_correction = std::numeric_limits<Real>::max();

  if (_print_view_factor_info)
    _console << "\nSum of all view factors from side i to all other faces before normalization.\n"
             << std::endl;

  // check view factors
  if (_print_view_factor_info)
    for (unsigned int from = 0; from < _n_sides; ++from)
      _console << "View factors from sideset " << boundaryNames()[from] << " sum to "
               << viewFactorRowSum(from) << std::endl;

  if (max_sum_deviation > _view_factor_tol)
    mooseError("Maximum deviation of view factor row sum is ",
               max_sum_deviation,
               " exceeding the set tolerance of ",
               _view_factor_tol);

  // normalize view factors
  if (_normalize_view_factor)
  {
    if (_print_view_factor_info)
      _console << "\nNormalizing view factors.\n" << std::endl;

    // allocate space
    DenseVector<Real> rhs(_n_sides);
    DenseVector<Real> lmults(_n_sides);
    DenseMatrix<Real> matrix(_n_sides, _n_sides);

    // equations for the Lagrange multiplier
    for (unsigned int i = 0; i < _n_sides; ++i)
    {
      // set the right hand side
      rhs(i) = 1 - viewFactorRowSum(i);

      // contribution from the delta_ii element
      matrix(i, i) = -0.5;

      // contributions from the upper diagonal
      for (unsigned int j = i + 1; j < _n_sides; ++j)
      {
        Real ar = _areas[i] / _areas[j];
        Real f = 2 * (1 + ar * ar);
        matrix(i, i) += -1 / f;
        matrix(i, j) += -1 / f * ar;
        rhs(i) += ar * ar / (1 + ar * ar) * devReciprocity(i, j);
      }

      // contributions from the lower diagonal
      for (unsigned int j = 0; j < i; ++j)
      {
        Real ar = _areas[j] / _areas[i];
        Real f = 2 * (1 + ar * ar);
        matrix(i, i) += -1 / f * ar * ar;
        matrix(i, j) += -1 / f * ar;
        rhs(i) -= ar * devReciprocity(j, i) * (1 - ar * ar / (1 + ar * ar));
      }
    }

    // solve the linear system
    matrix.lu_solve(rhs, lmults);

    // perform corrections but store view factors to temporary array
    // because we cannot modify _view_factors as it's used in this calc
    std::vector<std::vector<Real>> vf_temp = _view_factors;
    for (unsigned int i = 0; i < _n_sides; ++i)
      for (unsigned int j = 0; j < _n_sides; ++j)
      {
        Real correction;
        if (i == j)
          correction = -0.5 * lmults(i);
        else
        {
          Real ar = _areas[i] / _areas[j];
          Real f = 2 * (1 + ar * ar);
          correction = -(lmults(i) + lmults(j) * ar + 2 * ar * ar * devReciprocity(i, j)) / f;
        }

        // update the temporary view factor
        vf_temp[i][j] += correction;

        if (correction < min_correction)
          min_correction = correction;
        if (correction > max_correction)
          max_correction = correction;
      }
    _view_factors = vf_temp;
  }

  if (_print_view_factor_info)
  {
    _console << "\nFinal view factors.\n" << std::endl;
    for (unsigned int from = 0; from < _n_sides; ++from)
    {
      std::string from_name;
      for (auto pair : _side_name_index)
        if (pair.second == from)
          from_name = pair.first;
      auto from_id = _mesh.getBoundaryID(from_name);

      for (unsigned int to = 0; to < _n_sides; ++to)
      {
        std::string to_name;
        for (auto pair : _side_name_index)
          if (pair.second == to)
            to_name = pair.first;
        auto to_id = _mesh.getBoundaryID(to_name);
        _console << from_name << " (" << from_id << ") -> " << to_name << " (" << to_id
                 << ") = " << _view_factors[from][to] << std::endl;
      }
    }
  }

  // check sum of view factors and maximum deviation on reciprocity
  Real max_sum_deviation_after_normalization = maxDevRowSum();
  Real max_reciprocity_deviation_after_normalization = maxDevReciprocity();

  // print symmary
  _console << std::endl;
  _console << COLOR_CYAN << "Summary of the view factor computation"
           << "\n"
           << COLOR_DEFAULT << std::endl;
  if (_normalize_view_factor)
    _console << "Normalization is performed." << std::endl;
  else
    _console << "Normalization is skipped." << std::endl;
  _console << std::setw(60) << std::left << "Number of patches: " << _n_sides << std::endl;
  _console << std::setw(60) << std::left
           << "Max deviation sum = 1 before normalization: " << max_sum_deviation << std::endl;
  _console << std::setw(60) << std::left
           << "Max deviation from reciprocity before normalization: " << max_reciprocity_deviation
           << std::endl;
  if (_normalize_view_factor)
  {
    _console << std::setw(60) << std::left << "Maximum correction: " << max_correction << std::endl;
    _console << std::setw(60) << std::left << "Minimum correction: " << min_correction << std::endl;
    _console << std::setw(60) << "Max deviation sum = 1 after normalization: "
             << max_sum_deviation_after_normalization << std::endl;
    _console << std::setw(60) << std::left << "Max deviation from reciprocity after normalization: "
             << max_reciprocity_deviation_after_normalization << std::endl;
    _console << std::endl;
  }
}

unsigned int
ViewFactorBase::indexHelper(unsigned int i, unsigned int j) const
{
  mooseAssert(i <= j, "indexHelper requires i <= j");
  if (i == j)
    return _n_sides + i;
  unsigned int pos = 2 * _n_sides;
  for (unsigned int l = 0; l < _n_sides; ++l)
    for (unsigned int m = l + 1; m < _n_sides; ++m)
    {
      if (l == i && m == j)
        return pos;
      else
        ++pos;
    }
  mooseError("Should never get here");
  return 0;
}
