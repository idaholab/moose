#ifndef INTERNALVOLUMERZ_H
#define INTERNALVOLUMERZ_H

#include "InternalVolume.h"

class InternalVolumeRZ;

template<>
InputParameters validParams<InternalVolumeRZ>();

/**
 * This class computes the volume of an interior space in RZ coordinates.
 *
 * This class relies on being handed sidesets that define a closed space.
 *
 * If the sideset defines an interior surface, the volume reported will be
 * positive.  If the sideset defines an exterior surface, the volume
 * reported will be negative.  It is therefore possible to compute the net
 * interior volume by including an interior and an exterior surface
 * in the same sideset.
 */
class InternalVolumeRZ : public InternalVolume
{
public:

  InternalVolumeRZ( const std::string & name,
                    InputParameters parameters );

  virtual ~InternalVolumeRZ() {}

protected:

  virtual Real computeQpIntegral();

};

#endif
