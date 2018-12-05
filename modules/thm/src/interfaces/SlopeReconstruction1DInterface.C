#include "SlopeReconstruction1DInterface.h"
#include "MooseObject.h"

const unsigned int SlopeReconstruction1DInterface::_n_side = 2;
const unsigned int SlopeReconstruction1DInterface::_n_sten = 3;

template <>
InputParameters
validParams<SlopeReconstruction1DInterface>()
{
  InputParameters params = emptyInputParameters();

  params.addRequiredParam<MooseEnum>(
      "scheme", FlowModel::getSlopeReconstructionMooseEnum("None"), "Slope reconstruction scheme");

  return params;
}

SlopeReconstruction1DInterface::SlopeReconstruction1DInterface(const MooseObject * moose_object)
  : _scheme(RELAP7::stringToEnum<FlowModel::ESlopeReconstructionType>(
        moose_object->parameters().get<MooseEnum>("scheme")))
{
}

std::vector<RealGradient>
SlopeReconstruction1DInterface::getElementSlopes(const Elem * elem) const
{
  // Determine flow channel direction
  const RealVectorValue dir_unnormalized = elem->node_ref(1) - elem->node_ref(0);
  const RealVectorValue dir = dir_unnormalized / dir_unnormalized.norm();

  std::vector<const Elem *> neighbors(_n_side, nullptr);
  for (unsigned int i_side = 0; i_side < _n_side; i_side++)
    neighbors[i_side] = elem->neighbor(i_side);

  // get this element's position and solution
  const Point x_elem = elem->centroid();
  const auto W_elem = computeElementPrimitiveVariables(elem);

  // get the number of slopes to be stored
  const unsigned int n_slopes = W_elem.size();

  // neighbor positions and solutions, if any
  std::vector<Point> x_neighbor(_n_side);
  std::vector<std::vector<Real>> W_neighbor(_n_side);

  // one-sided slopes on each side
  std::vector<std::vector<Real>> slopes_one_sided(_n_side, std::vector<Real>(n_slopes, 0.0));

  // flags indicating whether each side is a boundary
  std::vector<bool> side_is_boundary(_n_side, false);

  // loop over the sides to compute the one-sided slopes
  for (unsigned int i_side = 0; i_side < _n_side; i_side++)
  {
    if (neighbors[i_side] != nullptr)
    {
      x_neighbor[i_side] = neighbors[i_side]->centroid();
      W_neighbor[i_side] = computeElementPrimitiveVariables(neighbors[i_side]);

      // compute change in position along flow channel direction
      const Real dx = (x_elem - x_neighbor[i_side]) * dir;

      for (unsigned int m = 0; m < n_slopes; m++)
        slopes_one_sided[i_side][m] = (W_elem[m] - W_neighbor[i_side][m]) / dx;
    }
    else
      side_is_boundary[i_side] = true;
  }

  // determine is cell is an interior cell
  const bool is_interior_cell = (!side_is_boundary[0] && !side_is_boundary[1]);

  // vector for the (possibly limited) slopes
  std::vector<RealGradient> slopes_limited(n_slopes, RealGradient(0.0, 0.0, 0.0));

  // limit the slopes
  switch (_scheme)
  {
    // first-order, zero slope
    case FlowModel::None:
      break;

    // full reconstruction; no limitation
    case FlowModel::Full:
    {
      for (unsigned int m = 0; m < n_slopes; m++)
      {
        // For 1-D, the current element falls into 1 of 4 categories:
        // 1) interior cell
        // 2) left boundary cell (side 0 is boundary)
        // 3) right boundary cell (side 1 is boundary)
        // 4) left AND right boundary cell (there is a single cell in mesh)
        if ((!side_is_boundary[0]) && (!side_is_boundary[1]))
        {
          // compute change in position along flow channel direction
          const Real dx = (x_neighbor[0] - x_neighbor[1]) * dir;

          // use central slope
          slopes_limited[m](0) = (W_neighbor[0][m] - W_neighbor[1][m]) / dx;
        }
        else if (side_is_boundary[0])
          // side 0 is boundary; use side 1 slope
          slopes_limited[m](0) = slopes_one_sided[1][m];
        else if (side_is_boundary[1])
          // side 1 is boundary; use side 0 slope
          slopes_limited[m](0) = slopes_one_sided[0][m];
        else
          // single cell in mesh; no slope
          slopes_limited[m](0) = 0.0;
      }
    }
    break;

    // minmod limiter
    case FlowModel::Minmod:

      if (is_interior_cell)
      {
        for (unsigned int m = 0; m < n_slopes; m++)
        {
          if ((slopes_one_sided[0][m] * slopes_one_sided[1][m]) > 0.0)
          {
            if (std::abs(slopes_one_sided[0][m]) < std::abs(slopes_one_sided[1][m]))
              slopes_limited[m](0) = slopes_one_sided[0][m];
            else
              slopes_limited[m](0) = slopes_one_sided[1][m];
          }
        }
      }
      break;

    // MC (monotonized central-difference) limiter
    case FlowModel::MC:

      if (is_interior_cell)
      {
        // compute change in position along flow channel direction
        const Real dx = (x_neighbor[0] - x_neighbor[1]) * dir;

        std::vector<Real> slopes_central(n_slopes, 0.0);
        for (unsigned int m = 0; m < n_slopes; m++)
          slopes_central[m] = (W_neighbor[0][m] - W_neighbor[1][m]) / dx;

        for (unsigned int m = 0; m < n_slopes; m++)
        {
          if (slopes_central[m] > 0.0 && slopes_one_sided[0][m] > 0.0 &&
              slopes_one_sided[1][m] > 0.0)
            slopes_limited[m](0) = std::min(
                slopes_central[m], 2.0 * std::min(slopes_one_sided[0][m], slopes_one_sided[1][m]));
          else if (slopes_central[m] < 0.0 && slopes_one_sided[0][m] < 0.0 &&
                   slopes_one_sided[1][m] < 0.0)
            slopes_limited[m](0) = std::max(
                slopes_central[m], 2.0 * std::max(slopes_one_sided[0][m], slopes_one_sided[1][m]));
        }
      }
      break;

    // superbee limiter
    case FlowModel::Superbee:

      if (is_interior_cell)
      {
        for (unsigned int m = 0; m < n_slopes; m++)
        {
          Real slope1 = 0.0;
          Real slope2 = 0.0;

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
            slopes_limited[m](0) = std::max(slope1, slope2);
          else if (slope1 < 0.0 && slope2 < 0.0)
            slopes_limited[m](0) = std::min(slope1, slope2);
        }
      }
      break;

    default:
      mooseError("Unknown slope reconstruction scheme");
      break;
  }

  return slopes_limited;
}
