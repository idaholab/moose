//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"
#include "MooseEnum.h"
#include "MooseTypes.h"
#include "InputParameters.h"
#include "THMEnums.h"

#include "libmesh/elem.h"
#include "libmesh/vector_value.h"
#include "libmesh/point.h"

/**
 * Interface class for 1-D slope reconstruction
 *
 * This class provides interfaces for generating slopes for an arbitrary set
 * of variables. A number of reconstruction options are provided, including a
 * number of TVD slope limiters.
 */
template <bool is_ad>
class SlopeReconstruction1DInterface
{
public:
  SlopeReconstruction1DInterface(const MooseObject * moose_object);

  /// Slope reconstruction type
  enum ESlopeReconstructionType
  {
    None,    ///< No reconstruction; Godunov scheme
    Full,    ///< Full reconstruction; no limitation
    Minmod,  ///< Minmod slope limiter
    MC,      ///< Monotonized Central-Difference slope limiter
    Superbee ///< Superbee slope limiter
  };
  /// Map of slope reconstruction type string to enum
  static const std::map<std::string, ESlopeReconstructionType> _slope_reconstruction_type_to_enum;

  /**
   * Gets a MooseEnum for slope reconstruction type
   *
   * @param[in] name   default value
   * @returns MooseEnum for slope reconstruction type
   */
  static MooseEnum getSlopeReconstructionMooseEnum(const std::string & name = "");

protected:
  /**
   * Gets the primitive solution vector and position of neighbor(s)
   *
   * @param[in] elem   Element for which to get slopes
   * @param[in] W_neighbor   Primitive solution vector for each neighbor
   * @param[in] x_neighbor   Position for each neighbor
   */
  void getNeighborPrimitiveVariables(const Elem * elem,
                                     std::vector<std::vector<GenericReal<is_ad>>> & W_neighbor,
                                     std::vector<Point> & x_neighbor) const;

  /**
   * Gets limited slopes for the primitive variables in the 1-D direction
   *
   * @param[in] elem   Element for which to get slopes
   *
   * @returns Vector of slopes for the element in the 1-D direction
   */
  std::vector<GenericReal<is_ad>> getElementSlopes(const Elem * elem) const;

  /**
   * Gets limited slopes for the primitive variables in the 1-D direction for boundary element
   *
   * @param[in] W_elem       Primitive solution for element
   * @param[in] x_elem       Position for element
   * @param[in] dir          Direction for element
   * @param[in] W_neighbor   Primitive solution vector for each neighbor
   * @param[in] x_neighbor   Position for each neighbor
   * @param[in] W_boundary   Primitive solution vector for boundary
   *
   * @returns Vector of slopes for the element in the 1-D direction
   */
  std::vector<GenericReal<is_ad>>
  getBoundaryElementSlopes(const std::vector<GenericReal<is_ad>> & W_elem,
                           const Point & x_elem,
                           const RealVectorValue & dir,
                           std::vector<std::vector<GenericReal<is_ad>>> W_neighbor,
                           std::vector<Point> x_neighbor,
                           const std::vector<GenericReal<is_ad>> & W_boundary) const;

  /**
   * Gets limited slopes for the primitive variables in the 1-D direction
   *
   * @param[in] W_elem       Primitive solution for element
   * @param[in] x_elem       Position for element
   * @param[in] dir          Direction for element
   * @param[in] W_neighbor   Primitive solution vector for each neighbor
   * @param[in] x_neighbor   Position for each neighbor
   *
   * @returns Vector of slopes for the element in the 1-D direction
   */
  std::vector<GenericReal<is_ad>>
  getElementSlopes(const std::vector<GenericReal<is_ad>> & W_elem,
                   const Point & x_elem,
                   const RealVectorValue & dir,
                   const std::vector<std::vector<GenericReal<is_ad>>> & W_neighbor,
                   const std::vector<Point> & x_neighbor) const;

  /**
   * Computes the cell-average primitive variable values for an element
   *
   * @param[in] elem   Element for which to get values
   *
   * @returns Vector of values on element
   */
  virtual std::vector<GenericReal<is_ad>>
  computeElementPrimitiveVariables(const Elem * elem) const = 0;

  /// MooseObject this interface is extending
  const MooseObject * const _moose_object;

  /// Slope reconstruction scheme
  const ESlopeReconstructionType _scheme;

public:
  static InputParameters validParams();

protected:
  /// Number of sides
  static const unsigned int _n_side;
  /// Number of elemental values in stencil for computing slopes
  static const unsigned int _n_sten;
};

namespace THM
{
template <>
SlopeReconstruction1DInterface<true>::ESlopeReconstructionType
stringToEnum<SlopeReconstruction1DInterface<true>::ESlopeReconstructionType>(const std::string & s);

template <>
SlopeReconstruction1DInterface<false>::ESlopeReconstructionType
stringToEnum<SlopeReconstruction1DInterface<false>::ESlopeReconstructionType>(
    const std::string & s);
}

template <bool is_ad>
const std::map<std::string,
               typename SlopeReconstruction1DInterface<is_ad>::ESlopeReconstructionType>
    SlopeReconstruction1DInterface<is_ad>::_slope_reconstruction_type_to_enum{
        {"NONE", None}, {"FULL", Full}, {"MINMOD", Minmod}, {"MC", MC}, {"SUPERBEE", Superbee}};

template <bool is_ad>
MooseEnum
SlopeReconstruction1DInterface<is_ad>::getSlopeReconstructionMooseEnum(const std::string & name)
{
  return THM::getMooseEnum<SlopeReconstruction1DInterface<is_ad>::ESlopeReconstructionType>(
      name, _slope_reconstruction_type_to_enum);
}

template <bool is_ad>
const unsigned int SlopeReconstruction1DInterface<is_ad>::_n_side = 2;

template <bool is_ad>
const unsigned int SlopeReconstruction1DInterface<is_ad>::_n_sten = 3;

template <bool is_ad>
InputParameters
SlopeReconstruction1DInterface<is_ad>::validParams()
{
  InputParameters params = emptyInputParameters();

  params.addRequiredParam<MooseEnum>(
      "scheme",
      SlopeReconstruction1DInterface::getSlopeReconstructionMooseEnum("None"),
      "Slope reconstruction scheme");

  return params;
}

template <bool is_ad>
SlopeReconstruction1DInterface<is_ad>::SlopeReconstruction1DInterface(
    const MooseObject * moose_object)
  : _moose_object(moose_object),
    _scheme(THM::stringToEnum<ESlopeReconstructionType>(
        moose_object->parameters().get<MooseEnum>("scheme")))
{
}

template <bool is_ad>
void
SlopeReconstruction1DInterface<is_ad>::getNeighborPrimitiveVariables(
    const Elem * elem,
    std::vector<std::vector<GenericReal<is_ad>>> & W_neighbor,
    std::vector<Point> & x_neighbor) const
{
  W_neighbor.clear();
  x_neighbor.clear();
  for (unsigned int i_side = 0; i_side < _n_side; i_side++)
  {
    auto neighbor = elem->neighbor_ptr(i_side);
    if (neighbor && (neighbor->processor_id() == _moose_object->processor_id()))
    {
      x_neighbor.push_back(neighbor->vertex_average());
      W_neighbor.push_back(computeElementPrimitiveVariables(neighbor));
    }
  }
}

template <bool is_ad>
std::vector<GenericReal<is_ad>>
SlopeReconstruction1DInterface<is_ad>::getElementSlopes(const Elem * elem) const
{
  mooseAssert(elem, "The supplied element is a nullptr.");

  const auto W_elem = computeElementPrimitiveVariables(elem);
  const Point x_elem = elem->vertex_average();
  const RealVectorValue dir = (elem->node_ref(1) - elem->node_ref(0)).unit();

  std::vector<Point> x_neighbor;
  std::vector<std::vector<GenericReal<is_ad>>> W_neighbor;
  getNeighborPrimitiveVariables(elem, W_neighbor, x_neighbor);

  return getElementSlopes(W_elem, x_elem, dir, W_neighbor, x_neighbor);
}

template <bool is_ad>
std::vector<GenericReal<is_ad>>
SlopeReconstruction1DInterface<is_ad>::getBoundaryElementSlopes(
    const std::vector<GenericReal<is_ad>> & W_elem,
    const Point & x_elem,
    const RealVectorValue & dir,
    std::vector<std::vector<GenericReal<is_ad>>> W_neighbor,
    std::vector<Point> x_neighbor,
    const std::vector<GenericReal<is_ad>> & W_boundary) const
{
  if (W_neighbor.size() == 1)
  {
    W_neighbor.push_back(W_boundary);

    // The boundary point will be assumed to be the same distance away as neighbor
    const Point dx = x_elem - x_neighbor[0];
    const Point x_boundary = x_elem + dx;
    x_neighbor.push_back(x_boundary);
  }

  return getElementSlopes(W_elem, x_elem, dir, W_neighbor, x_neighbor);
}

template <bool is_ad>
std::vector<GenericReal<is_ad>>
SlopeReconstruction1DInterface<is_ad>::getElementSlopes(
    const std::vector<GenericReal<is_ad>> & W_elem,
    const Point & x_elem,
    const RealVectorValue & dir,
    const std::vector<std::vector<GenericReal<is_ad>>> & W_neighbor,
    const std::vector<Point> & x_neighbor) const
{
  mooseAssert(x_neighbor.size() == W_neighbor.size(),
              "Neighbor positions size must equal neighbor solutions size.");

  // get the number of slopes to be stored
  const unsigned int n_slopes = W_elem.size();

  // compute one-sided slope(s)
  std::vector<std::vector<GenericReal<is_ad>>> slopes_one_sided;
  for (unsigned int i = 0; i < W_neighbor.size(); i++)
  {
    const Real dx = (x_elem - x_neighbor[i]) * dir;

    std::vector<GenericReal<is_ad>> slopes(n_slopes, 0.0);
    for (unsigned int m = 0; m < n_slopes; m++)
      slopes[m] = (W_elem[m] - W_neighbor[i][m]) / dx;

    slopes_one_sided.push_back(slopes);
  }

  // Fill in any missing one-sided slopes and compute central slope
  std::vector<GenericReal<is_ad>> slopes_central(n_slopes, 0.0);
  if (W_neighbor.size() == 2)
  {
    const Real dx = (x_neighbor[0] - x_neighbor[1]) * dir;
    for (unsigned int m = 0; m < n_slopes; m++)
      slopes_central[m] = (W_neighbor[0][m] - W_neighbor[1][m]) / dx;
  }
  else if (W_neighbor.size() == 1)
  {
    slopes_one_sided.push_back(slopes_one_sided[0]);
    slopes_central = slopes_one_sided[0];
  }
  else // only one element; use zero slopes
  {
    slopes_one_sided.push_back(slopes_central);
    slopes_one_sided.push_back(slopes_central);
  }

  // vector for the (possibly limited) slopes
  std::vector<GenericReal<is_ad>> slopes_limited(n_slopes, 0.0);

  // limit the slopes
  switch (_scheme)
  {
    // first-order, zero slope
    case None:
      break;

    // full reconstruction; no limitation
    case Full:

      slopes_limited = slopes_central;
      break;

    // minmod limiter
    case Minmod:

      for (unsigned int m = 0; m < n_slopes; m++)
      {
        if ((slopes_one_sided[0][m] * slopes_one_sided[1][m]) > 0.0)
        {
          if (std::abs(slopes_one_sided[0][m]) < std::abs(slopes_one_sided[1][m]))
            slopes_limited[m] = slopes_one_sided[0][m];
          else
            slopes_limited[m] = slopes_one_sided[1][m];
        }
      }
      break;

    // MC (monotonized central-difference) limiter
    case MC:

      for (unsigned int m = 0; m < n_slopes; m++)
      {
        if (slopes_central[m] > 0.0 && slopes_one_sided[0][m] > 0.0 && slopes_one_sided[1][m] > 0.0)
          slopes_limited[m] = std::min(
              slopes_central[m], 2.0 * std::min(slopes_one_sided[0][m], slopes_one_sided[1][m]));
        else if (slopes_central[m] < 0.0 && slopes_one_sided[0][m] < 0.0 &&
                 slopes_one_sided[1][m] < 0.0)
          slopes_limited[m] = std::max(
              slopes_central[m], 2.0 * std::max(slopes_one_sided[0][m], slopes_one_sided[1][m]));
      }
      break;

    // superbee limiter
    case Superbee:

      for (unsigned int m = 0; m < n_slopes; m++)
      {
        GenericReal<is_ad> slope1 = 0.0;
        GenericReal<is_ad> slope2 = 0.0;

        // calculate slope1 with minmod
        if (slopes_one_sided[1][m] > 0.0 && slopes_one_sided[0][m] > 0.0)
          slope1 = std::min(slopes_one_sided[1][m], 2.0 * slopes_one_sided[0][m]);
        else if (slopes_one_sided[1][m] < 0.0 && slopes_one_sided[0][m] < 0.0)
          slope1 = std::max(slopes_one_sided[1][m], 2.0 * slopes_one_sided[0][m]);

        // calculate slope2 with minmod
        if (slopes_one_sided[1][m] > 0.0 && slopes_one_sided[0][m] > 0.0)
          slope2 = std::min(2.0 * slopes_one_sided[1][m], slopes_one_sided[0][m]);
        else if (slopes_one_sided[1][m] < 0.0 && slopes_one_sided[0][m] < 0.0)
          slope2 = std::max(2.0 * slopes_one_sided[1][m], slopes_one_sided[0][m]);

        // calculate slope with maxmod
        if (slope1 > 0.0 && slope2 > 0.0)
          slopes_limited[m] = std::max(slope1, slope2);
        else if (slope1 < 0.0 && slope2 < 0.0)
          slopes_limited[m] = std::min(slope1, slope2);
      }
      break;

    default:
      mooseError("Unknown slope reconstruction scheme");
      break;
  }
  return slopes_limited;
}
