#pragma once

#include "IntegratedBC.h"

class HeatFluxBaseBC;
class HeatFluxFromHeatStructureBaseUserObject;

template <>
InputParameters validParams<HeatFluxBaseBC>();

/**
 * Base class for handling heat flux between flow channels and heat structures
 *
 * Since variables on flow channels and heat structures are subdomain restricted and
 * they do not share mesh elements, we cannot use the usual MOOSE computeOffDiagJacobian
 * method.  To enable this flow channel/heat structure coupling, the heat flux and its
 * Jacobians are first computed in HeatFluxFromHeatStructureBaseUserObject.  This, class
 * pulls the data from the user object and puts the residuals and Jacobians into the right
 * spots.  For this to properly work, the child class has to implement getOffDiagVariableNumbers
 * and computeQpOffDiagJacobianNeighbor methods.
 */
class HeatFluxBaseBC : public IntegratedBC
{
public:
  HeatFluxBaseBC(const InputParameters & parameters);

  virtual void initialSetup() override;

  virtual void computeJacobian() override;
  virtual void computeJacobianBlock(unsigned jvar) override;
  virtual void computeJacobianBlock(MooseVariableFEBase & jvar) override;

protected:
  /**
   * Get the list of variable numbers that are used in off-diagonal Jacobian blocks
   */
  virtual std::vector<unsigned int> getOffDiagVariableNumbers() = 0;

  /**
   * Compute the off-diagonal Jacobian w.r.t. the variable jvar on the neighboring element
   */
  virtual Real computeQpOffDiagJacobianNeighbor(unsigned int jvar) = 0;

  /// shape function values (in QPs)
  const VariablePhiValue & _phi_neighbor;

  /// User object that computes the heat flux
  const HeatFluxFromHeatStructureBaseUserObject & _q_uo;
  /// Perimeter of a single unit of heat structure
  const Real _P_hs_unit;
  /// Number of units of heat structure
  const unsigned int _n_unit;
  /// Is the heat structure coordinate system cylindrical?
  const bool _hs_coord_system_is_cylindrical;
  /// Coordinate transformation
  const Real _hs_coord;
  /// Factor by which to scale term on the flow channel side for the heat structure side
  const Real _hs_scale;
  /// Variable numbers for the off-diagonal jacobian computation
  std::vector<unsigned int> _off_diag_var_nums;
};
