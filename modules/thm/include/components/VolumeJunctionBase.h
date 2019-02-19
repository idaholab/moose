#ifndef VOLUMEJUNCTIONBASE_H
#define VOLUMEJUNCTIONBASE_H

#include "FlowJunction.h"

class VolumeJunctionBase;

template <>
InputParameters validParams<FlowJunction>();

/**
 * Base class for volumetric junction components
 */
class VolumeJunctionBase : public FlowJunction
{
public:
  VolumeJunctionBase(const InputParameters & params);

protected:
  /**
   * Computes junction volume using the areas of the connected flow channels
   *
   * @param[in] areas   Areas of connected flow channels
   */
  Real computeVolumeFromAreas(const std::vector<Real> & areas) const;

  /// Volume of the junction
  const Real _volume;

  /// Spatial position of center of the junction
  const Point & _position;
};

#endif /* VOLUMEJUNCTIONBASE_H */
