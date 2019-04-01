#ifndef CONNECTORBASE_H
#define CONNECTORBASE_H

#include "Component.h"

class ConnectorBase;

template <>
InputParameters validParams<ConnectorBase>();

/**
 * Base class for creating component that connect other components together (e.g. a flow channel and
 * a heat structure)
 */
class ConnectorBase : public Component
{
public:
  ConnectorBase(const InputParameters & params);
};

#endif /* CONNECTORBASE_H */
