/*
 * DieselGeneratorBase.h
 *
 *  Created on: Jun 6, 2013
 *      Author: alfoa
 */

#ifndef DIESELGENERATORBASE_H_
#define DIESELGENERATORBASE_H_

#include "RavenTools.h"

class DieselGeneratorBase;

template<>
InputParameters validParams<DieselGeneratorBase>();


class DieselGeneratorBase : public RavenTools{

public:
  DieselGeneratorBase(const std::string & name, InputParameters parameters);
  virtual ~DieselGeneratorBase();
  double compute(double time);

protected:

};


#endif /* DIESELGENERATORBASE_H_ */
