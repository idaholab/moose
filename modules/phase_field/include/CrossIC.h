#ifndef CROSSIC_H
#define CROSSIC_H

//Portions of this code Copyright 2007-2009 Roy Stogner
//
//Permission is hereby granted, free of charge, to any person obtaining
//a copy of this software and associated documentation files (the
//"Software"), to deal in the Software without restriction, including
//without limitation the rights to use, copy, modify, merge, publish,
//distribute, sublicense, and/or sell copies of the Software, and to
//permit persons to whom the Software is furnished to do so, subject to
//the following conditions:
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "Kernel.h"
#include "InitialCondition.h"
#include "C1ICBase.h"

// LibMesh includes
#include <parameters.h>
#include <point.h>
#include <vector_value.h>

// System includes
#include <string>

// Forward Declarations
class CrossIC;

template<>
InputParameters validParams<CrossIC>();

/**
 * CrossIC creates a C1 continuous initial condition that looks like a cross in
 * the middle of the domain.
 *
 * Note that the domain should be on the unit square 0,1 x 0,1.
 */
class CrossIC : public C1ICBase
{
public:

  /**
   * Constructor
   *
   * @param name The name given to the initial condition in the input file.
   * @param parameters The parameters object holding data for the class to use.
   * @param var_name The variable this InitialCondtion is supposed to provide values for.
   */
  CrossIC(const std::string & name,
          MooseSystem & moose_system,
          InputParameters parameters);

  /**
   * The value of the variable at a point.
   */
  virtual Real value(const Point & p);

  /**
   * The graident of the variable at a point.
   */
  virtual RealGradient gradient(const Point & p);
};

#endif //CROSSIC_H
