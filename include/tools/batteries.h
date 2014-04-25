/*
 * batteries.h
 *
 *  Created on: Aug 9, 2012
 *      Author: mandd
 */

#ifndef BATTERIES_H_
#define BATTERIES_H_

#include "RavenTools.h"

class batteries;

template<>
InputParameters validParams<batteries>();


class batteries : public RavenTools{

public:
  //batteries(bool BATTstatus, double BATTintialLife);
  batteries(const std::string & name, InputParameters parameters);
  virtual ~batteries();
  double compute(double time);

protected:

};

#endif /* BATTERIES_H_ */
