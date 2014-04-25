/*
 * TableFunction.h
 *
 *  Created on: July 20, 2012
 *      Author: alfoa
 */

#ifndef TABLEFUNCTION_H_
#define TABLEFUNCTION_H_

#include "CrowTools.h"
#include "Interpolation_Functions.h"

class TableFunction;

template<>
InputParameters validParams<TableFunction>();


class TableFunction : public RavenTools{
public:
  TableFunction(const std::string & name, InputParameters parameters);
  virtual ~TableFunction();
  double compute(double time);

protected:
  Interpolation_Functions _interpolation;         ///< Interpolation class

};


#endif /* TABLEFUNCTION_H_ */
