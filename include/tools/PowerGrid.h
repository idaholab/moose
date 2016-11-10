#ifndef POWERGRID_H
#define POWERGRID_H

#include "CrowTools.h"

class PowerGrid;

template<>
InputParameters validParams<PowerGrid>();

class PowerGrid : public CrowTools
{
public:
  PowerGrid(const InputParameters & parameters);
  ~PowerGrid();

};


#endif /* POWERGRID_H */
