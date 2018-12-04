#include "RDG3EqnMaterial.h"
#include "RELAP7Indices3Eqn.h"
#include "MooseMesh.h"

#include "libmesh/quadrature.h"

registerMooseObject("RELAP7App", RDG3EqnMaterial);

const unsigned int RDG3EqnMaterial::_n_side = 2;
const unsigned int RDG3EqnMaterial::_n_sten = 3;

template <>
InputParameters
validParams<RDG3EqnMaterial>()
{
  InputParameters params = validParams<Material>();

  params.addClassDescription(
      "Reconstructed solution values for the 1-D, 1-phase, variable-area Euler equations");

  params.addParam<MooseEnum>(
      "scheme", FlowModel::getSlopeReconstructionMooseEnum("None"), "Slope reconstruction scheme");

  params.addRequiredCoupledVar("A_elem", "Cross-sectional area, elemental");
  params.addRequiredCoupledVar("A_linear", "Cross-sectional area, linear");
  params.addRequiredCoupledVar("rhoA", "Conserved variable: rho*A");
  params.addRequiredCoupledVar("rhouA", "Conserved variable: rho*u*A");
  params.addRequiredCoupledVar("rhoEA", "Conserved variable: rho*E*A");

  params.addRequiredParam<MaterialPropertyName>("direction",
                                                "Flow channel direction material property name");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name of fluid properties user object");

  return params;
}

RDG3EqnMaterial::RDG3EqnMaterial(const InputParameters & parameters)
  : Material(parameters),

    _scheme(
        RELAP7::stringToEnum<FlowModel::ESlopeReconstructionType>(getParam<MooseEnum>("scheme"))),

    _A_avg(coupledValue("A_elem")),
    _A_linear(coupledValue("A_linear")),
    _rhoA_avg(coupledValue("rhoA")),
    _rhouA_avg(coupledValue("rhouA")),
    _rhoEA_avg(coupledValue("rhoEA")),

    _A_var(getVar("A_elem", 0)),
    _rhoA_var(getVar("rhoA", 0)),
    _rhouA_var(getVar("rhouA", 0)),
    _rhoEA_var(getVar("rhoEA", 0)),

    _dir(getMaterialProperty<RealVectorValue>("direction")),

    _rhoA(declareProperty<Real>("rhoA")),
    _rhouA(declareProperty<Real>("rhouA")),
    _rhoEA(declareProperty<Real>("rhoEA")),

    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
}

void
RDG3EqnMaterial::computeQpProperties()
{
  // Get the limited slopes of the primitive variables: {p, u, T}.
  // See the documentation for getElementSlope(); the first entry of the
  // returned gradient values is the slope along the 1-D direction; the other
  // entries are not used.
  const auto slopes = getElementSlopes(_current_elem);
  const Real p_slope = slopes[RELAP73Eqn::SLOPE_PRESSURE](0);
  const Real vel_slope = slopes[RELAP73Eqn::SLOPE_VELOCITY](0);
  const Real T_slope = slopes[RELAP73Eqn::SLOPE_TEMPERATURE](0);

  // compute primitive variables from the cell-average solution
  const Real rho_avg = _rhoA_avg[_qp] / _A_avg[_qp];
  const Real vel_avg = _rhouA_avg[_qp] / _rhoA_avg[_qp];
  const Real v_avg = 1.0 / rho_avg;
  const Real e_avg = _rhoEA_avg[_qp] / _rhoA_avg[_qp] - 0.5 * vel_avg * vel_avg;
  const Real p_avg = _fp.p_from_v_e(v_avg, e_avg);
  const Real T_avg = _fp.T_from_v_e(v_avg, e_avg);

  // apply slopes to primitive variables
  const Real delta_x = (_q_point[_qp] - _current_elem->centroid()) * _dir[_qp];
  const Real p = p_avg + p_slope * delta_x;
  const Real vel = vel_avg + vel_slope * delta_x;
  const Real T = T_avg + T_slope * delta_x;

  // compute reconstructed conserved variables
  const Real rho = _fp.rho_from_p_T(p, T);
  const Real e = _fp.e_from_p_rho(p, rho);
  const Real E = e + 0.5 * vel * vel;

  _rhoA[_qp] = rho * _A_linear[_qp];
  _rhouA[_qp] = _rhoA[_qp] * vel;
  _rhoEA[_qp] = _rhoA[_qp] * E;
}

std::vector<RealGradient>
RDG3EqnMaterial::getElementSlopes(const Elem * elem) const
{
  // Determine flow channel direction
  const RealVectorValue dir_unnormalized = elem->node_ref(1) - elem->node_ref(0);
  const RealVectorValue dir = dir_unnormalized / dir_unnormalized.norm();

  // vector for the slopes of the primitive variables
  std::vector<RealGradient> slope_limited(RELAP73Eqn::N_EQ, RealGradient(0.0, 0.0, 0.0));

  // array to store center points of this cell and its neighbor cells
  std::vector<Point> x(_n_sten);

  // the first always stores the current cell
  x[0] = elem->centroid();

  // array for the cell-average values in the current cell and its neighbors
  std::vector<std::vector<Real>> ucell(_n_sten, std::vector<Real>(RELAP73Eqn::N_EQ, 0.0));

  // compute primitive variable values for cell; first index in stencil is for
  // current cell
  computeElementPrimitiveVariables(elem, 0, ucell);

  // Store base slopes: the first element is the central slope, and the others
  // are 1-sided slopes.
  //
  // central slope:
  //                  u_{i+1} - u_{i-1}
  //                  -----------------
  //                  x_{i+1} - x_{i-1}
  //
  // left-side slope:
  //                  u_{i} - u_{i-1}
  //                  ---------------
  //                  x_{i} - x_{i-1}
  //
  // right-side slope:
  //                  u_{i+1} - u_{i}
  //                  ---------------
  //                  x_{i+1} - x_{i}
  //
  std::vector<std::vector<Real>> slope(_n_sten, std::vector<Real>(RELAP73Eqn::N_EQ, 0.0));

  // flag to indicate that cell is internal
  bool is_internal_cell = true;

  // loop over the sides to compute the one-sided slopes
  for (unsigned int i_side = 0; i_side < _n_side; i_side++)
  {
    // index of the neighbor element
    const unsigned int i_neighbor = i_side + 1;

    // if the current element is an internal cell
    if (elem->neighbor(i_side) != nullptr)
    {
      const Elem * neighbor = elem->neighbor(i_side);

      x[i_neighbor] = neighbor->centroid();

      // compute primitive variable values for cell
      computeElementPrimitiveVariables(neighbor, i_neighbor, ucell);

      // compute change in position along flow channel direction
      const Real dx = (x[0] - x[i_neighbor]) * dir;

      // calculate the one-sided slopes of primitive variables
      for (unsigned int m = 0; m < RELAP73Eqn::N_EQ; m++)
        slope[i_neighbor][m] = (ucell[0][m] - ucell[i_neighbor][m]) / dx;
    }
    // keep zero slopes if current element is a boundary cell
    else
    {
      // flag that this is a boundary cell
      is_internal_cell = false;
    }
  }

  // compute the current element slopes with the chosen slope reconstruction scheme

  switch (_scheme)
  {
    // first-order, zero slope
    case FlowModel::None:
      break;

    // minmod limiter
    case FlowModel::Minmod:

      if (is_internal_cell)
      {
        for (unsigned int m = 0; m < RELAP73Eqn::N_EQ; m++)
        {
          if ((slope[1][m] * slope[2][m]) > 0.0)
          {
            if (std::abs(slope[1][m]) < std::abs(slope[2][m]))
              slope_limited[m](0) = slope[1][m];
            else
              slope_limited[m](0) = slope[2][m];
          }
        }
      }
      break;

    // MC (monotonized central-difference) limiter
    case FlowModel::MC:

      if (is_internal_cell)
      {
        for (unsigned int m = 0; m < RELAP73Eqn::N_EQ; m++)
        {
          const Real dx = (x[1] - x[2]) * dir;
          slope[0][m] = (ucell[1][m] - ucell[2][m]) / dx;
        }

        for (unsigned int m = 0; m < RELAP73Eqn::N_EQ; m++)
        {
          if (slope[0][m] > 0.0 && slope[1][m] > 0.0 && slope[2][m] > 0.0)
            slope_limited[m](0) = std::min(slope[0][m], 2.0 * std::min(slope[1][m], slope[2][m]));
          else if (slope[0][m] < 0.0 && slope[1][m] < 0.0 && slope[2][m] < 0.0)
            slope_limited[m](0) = std::max(slope[0][m], 2.0 * std::max(slope[1][m], slope[2][m]));
        }
      }
      break;

    // superbee limiter
    case FlowModel::Superbee:

      if (is_internal_cell)
      {
        for (unsigned int m = 0; m < RELAP73Eqn::N_EQ; m++)
        {
          Real slope1 = 0.0;
          Real slope2 = 0.0;

          // calculate slope1 with minmod
          if (slope[2][m] > 0.0 && slope[1][m] > 0.0)
            slope1 = std::min(slope[2][m], 2.0 * slope[1][m]);
          else if (slope[2][m] < 0.0 && slope[1][m] < 0.0)
            slope1 = std::max(slope[2][m], 2.0 * slope[1][m]);

          // calculate slope2 with minmod
          if (slope[2][m] > 0.0 && slope[1][m] > 0.0)
            slope2 = std::min(2.0 * slope[2][m], slope[1][m]);
          else if (slope[2][m] < 0.0 && slope[1][m] < 0.0)
            slope2 = std::max(2.0 * slope[2][m], slope[1][m]);

          // calculate slope with maxmod
          if (slope1 > 0.0 && slope2 > 0.0)
            slope_limited[m](0) = std::max(slope1, slope2);
          else if (slope1 < 0.0 && slope2 < 0.0)
            slope_limited[m](0) = std::min(slope1, slope2);
        }
      }
      break;

    default:
      mooseError("Unknown slope reconstruction scheme");
      break;
  }

  return slope_limited;
}

void
RDG3EqnMaterial::computeElementPrimitiveVariables(const Elem * elem,
                                                  const unsigned int & i,
                                                  std::vector<std::vector<Real>> & U) const
{
  // get the cell-average conserved variables
  Real A, rhoA, rhouA, rhoEA;
  if (_is_implicit)
  {
    A = _A_var->getElementalValue(elem);
    rhoA = _rhoA_var->getElementalValue(elem);
    rhouA = _rhouA_var->getElementalValue(elem);
    rhoEA = _rhoEA_var->getElementalValue(elem);
  }
  else
  {
    A = _A_var->getElementalValueOld(elem);
    rhoA = _rhoA_var->getElementalValueOld(elem);
    rhouA = _rhouA_var->getElementalValueOld(elem);
    rhoEA = _rhoEA_var->getElementalValueOld(elem);
  }

  // compute primitive variables

  const Real rho = rhoA / A;
  const Real vel = rhouA / rhoA;
  const Real v = 1.0 / rho;
  const Real e = rhoEA / rhoA - 0.5 * vel * vel;

  U[i][RELAP73Eqn::SLOPE_PRESSURE] = _fp.p_from_v_e(v, e);
  U[i][RELAP73Eqn::SLOPE_VELOCITY] = vel;
  U[i][RELAP73Eqn::SLOPE_TEMPERATURE] = _fp.T_from_v_e(v, e);
}
