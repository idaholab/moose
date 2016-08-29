/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "PackedRangeOverflow.h"

template<>
InputParameters validParams<PackedRangeOverflow>()
{
  InputParameters params = validParams<GeneralUserObject >();
  params.addRequiredParam<unsigned int>("buffer_size", "String buffer size (should be around 1000000)");
  return params;
}

PackedRangeOverflow::PackedRangeOverflow(const InputParameters & parameters) :
    GeneralUserObject(parameters),
    _buffer_size(getParam<unsigned int>("buffer_size"))
{
}

void
PackedRangeOverflow::initialize()
{
}

void
PackedRangeOverflow::execute()
{
}

void
PackedRangeOverflow::finalize()
{
  std::vector<std::string> send_buffers(1);
  std::vector<std::string> recv_buffers;

  send_buffers[0].assign(_buffer_size, '*');

  _communicator.allgather_packed_range((void *)(nullptr), send_buffers.begin(), send_buffers.end(),
                                       std::back_inserter(recv_buffers));

  if (recv_buffers.size() != _communicator.size())
    mooseError("recv_buffers size is wrong");
}

void
PackedRangeOverflow::threadJoin(const UserObject & /*uo*/)
{
}
