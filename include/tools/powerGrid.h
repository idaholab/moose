/*
 * powerGrid.h
 *
 *  Created on: Aug 9, 2012
 *      Author: mandd
 */

#ifndef POWERGRID_H_
#define POWERGRID_H_

#include "RavenTools.h"

class powerGrid;

template<>
InputParameters validParams<powerGrid>();

class powerGrid : public RavenTools{

public:
  powerGrid(const std::string & name, InputParameters parameters);
  ~powerGrid();

protected:

};


#endif /* POWERGRID_H_ */
