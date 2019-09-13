#ifndef PSBTIC_H
#define PSBTIC_H

#include "InitialCondition.h"

class PsbtIC;

template <>
InputParameters validParams<PsbtIC>();

//! An abstract class for ICs relating to the PSBT fluid temperature benchmarks.
//!
//! This class and its subclasses assume the PSBT 5x5 bundle geometry
//! (assembly A1) with the lower-left corner of the lower-left channel at
//! x=0 y=0.

class PsbtIC : public InitialCondition
{
public:
  PsbtIC(const InputParameters & params);

protected:
  //! Find the (row, column) indices of the subchannel containing a given point.
  std::pair<int, int>
  index_point(const Point & p) const;
};

#endif // PSBTIC_H
