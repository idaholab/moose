/*
 * decayHeat.h
 *
 *  Created on: Aug 8, 2012
 *      Author: mandd
 */

#ifndef DECAYHEAT_H_
#define DECAYHEAT_H_

#include "RavenTools.h"

class decayHeat;

template<>
InputParameters validParams<decayHeat>();


class decayHeat : public RavenTools{
public:
  decayHeat(const std::string & name, InputParameters parameters);
  virtual ~decayHeat();
  double compute(double time);

protected:
  int _equationType;

};


#endif /* DECAYHEAT_H_ */
