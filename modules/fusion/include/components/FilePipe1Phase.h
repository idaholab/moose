#pragma once

#include "FlowChannel1Phase.h"

/**
 * This class constructs a flow pipe by reading in mesh from a CSV file.
 * The flow pipe can be curved or straight.
 */
class FilePipe1Phase : public FlowChannel1Phase
{
public:
  FilePipe1Phase(const InputParameters & params);

protected:
  virtual void buildMeshNodes() override;

private:
  /// X coordinate
  std::vector<double> _x;
  /// Y coordinate
  std::vector<double> _y;
  /// Z coordinate
  std::vector<double> _z;

public:
  static InputParameters validParams();
};
