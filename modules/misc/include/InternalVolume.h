#ifndef INTERNALVOLUME_H
#define INTERNALVOLUME_H

#include "SideIntegral.h"

class InternalVolume;

template<>
InputParameters validParams<InternalVolume>();

/**
 * This class computes the volume of an interior space.
 * 
 * This class relies on being handed sidesets that define a closed space.
 * 
 * If the sideset defines an interior surface, the volume reported will be
 * positive.  If the sideset defines an exterior surface, the volume
 * reported will be negative.  It is therefore possible to compute the net
 * interior volume by processing an interior and an exterior surface
 * simultaneously.
 */
class InternalVolume : public SideIntegral
{
public:

  InternalVolume( const std::string & name,
                  MooseSystem & moose_system,
                  InputParameters parameters );

  virtual ~InternalVolume() {}

protected:

  virtual Real computeQpIntegral();

};

#endif
