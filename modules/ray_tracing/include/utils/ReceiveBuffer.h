//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose Includes
#include "LIFOBuffer.h"
#include "SharedPool.h"

// Local includes
#include "ParallelStudyMethod.h"

// libMesh Includes
#include "libmesh/parallel.h"
#include "libmesh/parallel_object.h"

// System Includes
#include <list>
#include <chrono>

template <typename Object, typename Context>
class ReceiveBuffer : public libMesh::ParallelObject
{
public:
  ReceiveBuffer(const libMesh::Parallel::Communicator & comm,
                Context * const context,
                const ParallelStudyMethod method,
                const unsigned int clicks_per_receive,
                const Parallel::MessageTag object_buffer_tag);

  /**
   * Destructor: ensures that all send requests have completed
   */
  ~ReceiveBuffer() override;

  /**
   * Whether or not there are messages that are being currently received
   */
  bool currentlyReceiving() const { return _requests.size(); }

  /**
   * The work received since the last reset
   */
  unsigned long int objectsReceived() const { return _objects_received; }

  /**
   * The number of buffers received since the last reset
   */
  unsigned long int buffersReceived() const { return _buffers_received; }

  /**
   * The total number of times we've polled for messages
   */
  unsigned long int numProbes() const { return _num_probes; }

  /**
   * Number of buffers created in the object buffer pool
   */
  unsigned long int objectPoolCreated() const { return _object_buffer_pool.num_created(); }

  /**
   * Number of buffers created in the buffer pool
   */
  unsigned long int bufferPoolCreated() const { return _buffer_pool.num_created(); }

  /**
   * Start receives for all currently available messages
   *
   * Adds the to the working buffer
   */
  void receive(const bool start_receives_only = false);

  /**
   * Clear all existing data
   */
  void clear();

  /**
   * Amount of time (in seconds) spent in the loop checking for messages and creating Receives
   */
  Real receiveLoopTime() const { return std::chrono::duration<Real>(_receive_loop_time).count(); }

  /**
   * Amount of time (in seconds) spent finishing receives and collecting objects
   */
  Real cleanupRequestsTime() const
  {
    return std::chrono::duration<Real>(_cleanup_requests_time).count();
  }

  /**
   * Checks to see if any Requests can be finished
   */
  void cleanupRequests();

  /**
   * Gets the buffer that the received objects are filled into after the requests are finished.
   */
  MooseUtils::LIFOBuffer<std::shared_ptr<Object>> & buffer() { return _buffer; }

private:
  /// The context
  Context * const _context;
  /// The buffer that finished requests are filled into
  MooseUtils::LIFOBuffer<std::shared_ptr<Object>> _buffer;

  /// The method
  const ParallelStudyMethod _method;
  /// Iterations to wait before checking for new objects
  const unsigned int _clicks_per_receive;

  /// List of Requests and buffers for each request
  std::list<std::pair<std::shared_ptr<Parallel::Request>,
                      std::shared_ptr<std::vector<std::shared_ptr<Object>>>>>
      _requests;

  /// MessageTag for sending objects
  const Parallel::MessageTag _object_buffer_tag;

  /// Receive loop time
  std::chrono::steady_clock::duration _receive_loop_time;
  /// Time cleaning up requests
  std::chrono::steady_clock::duration _cleanup_requests_time;

  /// Total objects received
  unsigned long int _objects_received;
  /// Total object buffers received
  unsigned long int _buffers_received;
  /// Total number of times we've polled for messages
  unsigned long int _num_probes;

  /// Shared pool of object buffers for incoming messages
  MooseUtils::SharedPool<std::vector<std::shared_ptr<Object>>> _object_buffer_pool;
  /// Shared pool of buffers
  MooseUtils::SharedPool<
      std::vector<typename Parallel::Packing<std::shared_ptr<Object>>::buffer_type>>
      _buffer_pool;

  template <typename C, typename OutputIter, typename T>
  inline void blocking_receive_packed_range(const Parallel::Communicator & comm,
                                            const processor_id_type src_processor_id,
                                            C * context,
                                            OutputIter out,
                                            const T * /* output_type */,
                                            Parallel::Request & req,
                                            Parallel::Status & stat,
                                            const Parallel::MessageTag & tag) const
  {
    libmesh_experimental();

    typedef typename Parallel::Packing<T>::buffer_type buffer_t;

    // Receive serialized variable size objects as a sequence of
    // buffer_t.
    // Allocate a buffer on the heap so we don't have to free it until
    // after the Request::wait()
    std::vector<buffer_t> * buffer = new std::vector<buffer_t>(stat.size());
    comm.receive(src_processor_id, *buffer, tag);

    // Make the Request::wait() handle unpacking the buffer
    req.add_post_wait_work(
        new libMesh::Parallel::PostWaitUnpackBuffer<std::vector<buffer_t>, C, OutputIter, T>(
            *buffer, context, out));

    // Make the Request::wait() handle deleting the buffer
    req.add_post_wait_work(
        new libMesh::Parallel::PostWaitDeleteBuffer<std::vector<buffer_t>>(buffer));
  }
};

template <typename Object, typename Context>
ReceiveBuffer<Object, Context>::ReceiveBuffer(const libMesh::Parallel::Communicator & comm,
                                              Context * const context,
                                              const ParallelStudyMethod method,
                                              const unsigned int clicks_per_receive,
                                              const Parallel::MessageTag object_buffer_tag)
  : ParallelObject(comm),
    _context(context),
    _method(method),
    _clicks_per_receive(clicks_per_receive),
    _object_buffer_tag(object_buffer_tag)
{
}

template <typename Object, typename Context>
ReceiveBuffer<Object, Context>::~ReceiveBuffer()
{
  if (!_requests.empty())
    mooseError("Some requests not serviced!");
}

template <typename Object, typename Context>
void
ReceiveBuffer<Object, Context>::receive(const bool start_receives_only /* = false */)
{
  bool flag = false;
  Parallel::Status stat;

  static unsigned int current_clicks = 0;

  if (current_clicks % _clicks_per_receive == 0)
  {
    current_clicks = 0;

    // Receive and process a bunch of objects
    do
    {
      stat = _communicator.template packed_range_probe<std::shared_ptr<Object>>(
          Parallel::any_source, _object_buffer_tag, flag);

      _num_probes++;

      if (flag)
      {
        auto req = std::make_shared<Parallel::Request>();
        std::shared_ptr<std::vector<std::shared_ptr<Object>>> objects =
            _object_buffer_pool.acquire();

        // Make sure the buffer is clear - this shouldn't resize the storage though.
        objects->clear();

        if (_method == ParallelStudyMethod::HARM || _method == ParallelStudyMethod::BS)
          blocking_receive_packed_range(comm(),
                                        stat.source(),
                                        _context,
                                        std::back_inserter(*objects),
                                        (std::shared_ptr<Object> *)(libmesh_nullptr),
                                        *req,
                                        stat,
                                        _object_buffer_tag);
        else
        {
          std::shared_ptr<
              std::vector<typename Parallel::Packing<std::shared_ptr<Object>>::buffer_type>>
              buffer = _buffer_pool.acquire();

          _communicator.nonblocking_receive_packed_range(
              stat.source(),
              _context,
              std::back_inserter(*objects),
              (std::shared_ptr<Object> *)(libmesh_nullptr),
              *req,
              stat,
              buffer,
              _object_buffer_tag);
        }

        _requests.emplace_back(req, objects);
      }
    } while (flag);
  }

  current_clicks++;

  if (!start_receives_only)
    cleanupRequests();
}

template <typename Object, typename Context>
void
ReceiveBuffer<Object, Context>::clear()
{
  _requests.clear();

  _receive_loop_time = std::chrono::steady_clock::duration::zero();
  _cleanup_requests_time = std::chrono::steady_clock::duration::zero();

  _objects_received = 0;
  _buffers_received = 0;
  _num_probes = 0;
}

template <typename Object, typename Context>
void
ReceiveBuffer<Object, Context>::cleanupRequests()
{
  _requests.remove_if(
      [&](std::pair<std::shared_ptr<Parallel::Request>,
                    std::shared_ptr<std::vector<std::shared_ptr<Object>>>> & request_pair)
      {
        auto req = request_pair.first;
        auto objects = request_pair.second;

        if (req->test()) // See if the receive has completed
        {
          req->wait(); // MUST call wait() to do post_wait_work which actually fills the object
                       // buffer

          _buffers_received++;
          _objects_received += objects->size();

          if (_buffer.capacity() < _buffer.size() + objects->size())
            _buffer.setCapacity(_buffer.size() + objects->size());

          for (auto & object : *objects)
            _buffer.move(object);

          objects->clear();

          return true;
        }
        else
          return false;
      });
}
