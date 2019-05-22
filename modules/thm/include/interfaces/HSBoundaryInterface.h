#pragma once

#include "InputParameters.h"
#include "HeatStructureBase.h"

class HSBoundaryInterface;
class Component;

template <>
InputParameters validParams<HSBoundaryInterface>();

/**
 * Interface class for coupling to a heat structure boundary
 */
class HSBoundaryInterface
{
public:
  HSBoundaryInterface(const Component * const component);

  void check(const Component * const component) const;

protected:
  const BoundaryName & getHSBoundaryName(const Component * const component) const;

  /// Heat structure name
  const std::string & _hs_name;
  /// Heat structure side enum
  const MooseEnum & _hs_side_enum;
  /// Heat structure side
  const HeatStructureBase::SideType _hs_side;
  /// True of valid heat structure side was provided
  bool _hs_side_valid;
};
