/*
 * batteries.h
 *
 *  Created on: Aug 9, 2012
 *      Author: mandd
 */

#ifndef BATTERIES_H_
#define BATTERIES_H_

#include "CrowTools.h"

class batteries;

template<>
InputParameters validParams<batteries>();


class batteries : public CrowTools{

public:
  //batteries(bool BATTstatus, double BATTintialLife);
  batteries(const std::string & name, InputParameters parameters);
  virtual ~batteries();
  double compute(double time);

protected:

};

#endif /* BATTERIES_H_ */
