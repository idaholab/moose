#pragma once

#include "BoundaryBase.h"
#include "HeatStructureBase.h"

class HSBoundary;

template <>
InputParameters validParams<HSBoundary>();

/**
 * Base class for heat structure boundary components
 */
class HSBoundary : public BoundaryBase
{
public:
  HSBoundary(const InputParameters & params);

  virtual void check() const override;

protected:
  /**
   * Extracts the heat structure sides for each boundary
   *
   * @param[in] boundary_names   Boundary names
   */
  std::vector<HeatStructureBase::SideType>
  extractHeatStructureSides(const std::vector<BoundaryName> & boundary_names) const;

  /// Boundary names for which the boundary component applies
  const std::vector<BoundaryName> & _boundary;

  /// Heat structure name
  const std::string & _hs_name;

  /// Heat structure sides for each boundary
  const std::vector<HeatStructureBase::SideType> _hs_sides;
};
