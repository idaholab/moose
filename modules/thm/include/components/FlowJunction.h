#ifndef FLOWJUNCTION_H
#define FLOWJUNCTION_H

#include "FlowConnection.h"

class FlowJunction;

template <>
InputParameters validParams<FlowJunction>();

/**
 * Base class for flow junctions
 */
class FlowJunction : public FlowConnection
{
public:
  FlowJunction(const InputParameters & params);

protected:
  virtual void setupMesh() override;

private:
  virtual std::string createBoundaryName(const std::string & comp_name) const override;
};

#endif /* FLOWJUNCTION_H */
