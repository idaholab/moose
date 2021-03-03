#pragma once

#include "FlowJunction.h"

/**
 * Junction connecting one flow channel to one other flow channel
 *
 * The assumptions made by this component are as follows:
 * @li The connected channels are parallel.
 * @li The connected channels have the same flow area at the junction.
 *
 * Deprecated
 */
class JunctionOneToOne : public FlowJunction
{
public:
  JunctionOneToOne(const InputParameters & params);

public:
  static InputParameters validParams();
};
