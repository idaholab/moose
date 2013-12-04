//  Base class for fluid density as a function of pressure
//
#ifndef RICHARDSDENSITY_H
#define RICHARDSDENSITY_H

#include "GeneralUserObject.h"

class RichardsDensity;


template<>
InputParameters validParams<RichardsDensity>();

class RichardsDensity : public GeneralUserObject
{
 public:
  RichardsDensity(const std::string & name, InputParameters parameters);

  void initialize();
  void execute();
  void finalize();

  // These functions must be over-ridden in the derived class
  // to provide the actual values of density and its derivatives
  virtual Real density(Real p) const;
  virtual Real ddensity(Real p) const;
  virtual Real d2density(Real p) const;

};

#endif // RICHARDSDENSITY_H
