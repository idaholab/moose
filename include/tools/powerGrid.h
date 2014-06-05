#ifndef POWERGRID_H
#define POWERGRID_H

#include "CrowTools.h"

class powerGrid;

template<>
InputParameters validParams<powerGrid>();

class powerGrid : public CrowTools
{
public:
  powerGrid(const std::string & name, InputParameters parameters);
  ~powerGrid();

};


#endif /* POWERGRID_H */
