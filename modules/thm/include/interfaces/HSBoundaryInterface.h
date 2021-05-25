#pragma once

#include "InputParameters.h"
#include "HeatStructureBase.h"
#include "LoggingInterface.h"

class Component;

/**
 * Interface class for coupling to a heat structure boundary
 */
class HSBoundaryInterface
{
public:
  HSBoundaryInterface(Component * component);

  void check(const Component * const component) const;

protected:
  /**
   * Gets the boundary name corresponding to the heat structure and side
   *
   * @param[in] component   Component pointer
   */
  const BoundaryName & getHSBoundaryName(const Component * const component) const;

  /// Heat structure name
  const std::string & _hs_name;
  /// Heat structure side enum
  const MooseEnum & _hs_side_enum;
  /// Heat structure side
  const HeatStructureSideType _hs_side;
  /// True of valid heat structure side was provided
  bool _hs_side_valid;

public:
  static InputParameters validParams();
};
