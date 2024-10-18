//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ParallelStudyMethod.h"

// MOOSE includes
#include "InputParameters.h"
#include "SharedPool.h"
#include "MooseEnum.h"
#include "CircularBuffer.h"
#include "LIFOBuffer.h"

// Local includes
#include "SendBuffer.h"
#include "ReceiveBuffer.h"

// libMesh Includes
#include "libmesh/parallel_object.h"

template <typename WorkType, typename ParallelDataType>
class ParallelStudy : public libMesh::ParallelObject
{
public:
  typedef typename MooseUtils::Buffer<WorkType>::iterator work_iterator;
  typedef typename MooseUtils::Buffer<std::shared_ptr<ParallelDataType>>::iterator
      parallel_data_iterator;

  static InputParameters validParams();

  ParallelStudy(const libMesh::Parallel::Communicator & comm,
                const InputParameters & params,
                const std::string & name);

  /**
   * Pre-execute method that MUST be called before execute() and before adding work.
   */
  void preExecute();
  /**
   * Execute method.
   */
  void execute();

  /**
   * Adds work to the buffer to be executed. This will move the work into the buffer
   * (with std::move), therefore the passed in work will be invalid after this call.
   * For the purposes of the completion algorithm, this added work is considered
   * NEW work.
   *
   * During pre-execution (between preExecute() and execute()), this method can ONLY
   * be called on thread 0.
   *
   * During execute(), this method is thread safe and can be used to add work during execution.
   */
  ///@{
  void moveWorkToBuffer(WorkType & work, const THREAD_ID tid);
  void moveWorkToBuffer(const work_iterator begin, const work_iterator end, const THREAD_ID tid);
  void moveWorkToBuffer(std::vector<WorkType> & work, const THREAD_ID tid);
  ///@}

  /**
   * Acquire a parallel data object from the pool.
   */
  template <typename... Args>
  typename MooseUtils::SharedPool<ParallelDataType>::PtrType
  acquireParallelData(const THREAD_ID tid, Args &&... args)
  {
    return _parallel_data_pools[tid].acquire(std::forward<Args>(args)...);
  }

  /**
   * Moves parallel data objects to the send buffer to be communicated to processor \p dest_pid.
   */
  void moveParallelDataToBuffer(std::shared_ptr<ParallelDataType> & data,
                                const processor_id_type dest_pid);

  /**
   * Gets the receive buffer.
   */
  const ReceiveBuffer<ParallelDataType, ParallelStudy<WorkType, ParallelDataType>> &
  receiveBuffer() const
  {
    return *_receive_buffer;
  }

  /**
   * Gets the work buffer.
   */
  const MooseUtils::Buffer<WorkType> & workBuffer() const { return *_work_buffer; }

  /**
   * Gets the total number of send buffer pools created.
   */
  unsigned long long int sendBufferPoolCreated() const;
  /**
   * Gets the total number of parallel data objects sent from this processor.
   */
  unsigned long long int parallelDataSent() const;
  /**
   * Gets the total number of buffers sent from this processor.
   */
  unsigned long long int buffersSent() const;
  /**
   * Gets the total number of parallel data created in all of the threaded pools
   */
  unsigned long long int poolParallelDataCreated() const;

  /**
   * Gets the total amount of work started from this processor.
   */
  unsigned long long int localWorkStarted() const { return _local_work_started; }
  /**
   * Gets the total amount of work executed on this processor.
   */
  unsigned long long int localWorkExecuted() const { return _local_work_executed; }
  /**
   * Gets the total amount of work completeed across all processors.
   */
  unsigned long long int totalWorkCompleted() const { return _total_work_completed; }
  /**
   * Gets the total number of chunks of work executed on this processor.
   */
  unsigned long long int localChunksExecuted() const { return _local_chunks_executed; }

  /**
   * Whether or not this object is currently in execute().
   */
  bool currentlyExecuting() const { return _currently_executing; }
  /**
   * Whether or not this object is between preExecute() and execute().
   */
  bool currentlyPreExecuting() const { return _currently_pre_executing; }

  /**
   * Gets the max buffer size
   */
  unsigned int maxBufferSize() const { return _max_buffer_size; }
  /**
   * Gets the chunk size
   */
  unsigned int chunkSize() const { return _chunk_size; }

  /**
   * Gets the number of iterations to wait before communicating
   */
  unsigned int clicksPerCommunication() const { return _clicks_per_communication; }
  /**
   * Gets the number of iterations to wait before communicating with root
   */
  unsigned int clicksPerRootCommunication() const { return _clicks_per_root_communication; }
  /**
   * Gets the number of iterations to wait before checking for new parallel data
   */
  unsigned int clicksPerReceive() const { return _clicks_per_receive; }

  /**
   * Gets the method
   */
  ParallelStudyMethod method() const { return _method; }

  /**
   * Reserve \p size entries in the work buffer.
   *
   * This can only be used during the pre-execution phase (between preExecute() and execute()).
   *
   * This is particularly useful when one wants to move many work objects into the buffer using
   * moveWorkToBuffer() and wants to allocate the space ahead of time.
   */
  void reserveBuffer(const std::size_t size);

protected:
  /**
   * Enum for providing useful errors during work addition in moveWorkError().
   */
  enum MoveWorkError
  {
    DURING_EXECUTION_DISABLED,
    PRE_EXECUTION_AND_EXECUTION_ONLY,
    PRE_EXECUTION_ONLY,
    PRE_EXECUTION_THREAD_0_ONLY,
    CONTINUING_DURING_EXECUTING_WORK
  };

  /**
   * Creates the work buffer
   *
   * This is virtual so that derived classes can use their own specialized buffers
   */
  virtual std::unique_ptr<MooseUtils::Buffer<WorkType>> createWorkBuffer();

  /**
   * Pure virtual to be overridden that executes a single object of work on a given thread
   */
  virtual void executeWork(const WorkType & work, const THREAD_ID tid) = 0;

  /**
   * Virtual that allows for the customization of error text for moving work into the buffer.
   */
  virtual void moveWorkError(const MoveWorkError error, const WorkType * work = nullptr) const;

  /**
   * Insertion point for derived classes to provide an alternate ending criteria for
   * SMART execution. Only called when _has_alternate_ending_criteria == true.
   */
  virtual bool alternateSmartEndingCriteriaMet();

  /**
   * Insertion point for acting on work that was just executed.
   *
   * This is not called in threads.
   */
  virtual void postExecuteChunk(const work_iterator /* begin */, const work_iterator /* end */) {}

  /**
   * Insertion point called just after trying to receive work and just before beginning
   * work on the work buffer
   */
  virtual void preReceiveAndExecute() {}

  /**
   * Pure virtual for acting on parallel data that has JUST been received and
   * filled into the buffer.
   *
   * The parallel data in the range passed here will have its use count reduced
   * by one if it still exists after this call.
   */
  virtual void postReceiveParallelData(const parallel_data_iterator begin,
                                       const parallel_data_iterator end) = 0;

  /**
   * Can be overridden to denote if a piece of work is not complete yet.
   *
   * The complete terminology is used within the execution algorithms to
   * determine if the study is complete.
   */
  virtual bool workIsComplete(const WorkType & /* work */) { return true; }

  /**
   * Moves work that is considered continuing for the purposes of the execution
   * algorithm into the buffer.
   */
  ///@{
  void moveContinuingWorkToBuffer(WorkType & Work);
  void moveContinuingWorkToBuffer(const work_iterator begin, const work_iterator end);
  ///@}

  /**
   * Whether or not ALL of the buffers are empty:
   * Working buffer, threaded buffers, receive buffer, and send buffers.
   */
  bool buffersAreEmpty() const;

  /// This rank
  const processor_id_type _pid;
  /// Name for this object for use in error handling
  const std::string _name;
  /// The InputParameters
  const InputParameters & _params;
  /// The study method
  const ParallelStudyMethod _method;
  /// Whether or not this object has alternate ending criteria
  bool _has_alternate_ending_criteria;

private:
  /**
   * Flushes all parallel data out of the send buffers
   */
  void flushSendBuffers();

  /**
   * Execute work using SMART
   */
  void smartExecute();
  /**
   * Execute work using HARM
   */
  void harmExecute();
  /**
   * Execute work using BS
   */
  void bsExecute();

  /**
   * Receive packets of parallel data from other processors and executes work
   */
  bool receiveAndExecute();

  /**
   * Execute a chunk of work and buffer
   */
  void executeAndBuffer(const std::size_t chunk_size);

  /**
   * Internal check for if it is allowed to currently add work in moveWorkToBuffer().
   */
  void canMoveWorkCheck(const THREAD_ID tid);

  /**
   * Internal method for acting on the parallel data that has just been received into
   * the parallel buffer
   */
  void postReceiveParallelDataInternal();

  /// Minimum size of a SendBuffer
  const unsigned int _min_buffer_size;
  /// Number of objects to buffer before communication
  const unsigned int _max_buffer_size;
  /// Multiplier for the buffer size for growing the buffer
  const Real _buffer_growth_multiplier;
  /// Multiplier for the buffer size for shrinking the buffer
  const Real _buffer_shrink_multiplier;
  /// Number of objects to execute at once during communication
  const unsigned int _chunk_size;
  /// Whether or not to allow the addition of new work to the buffer during execution
  const bool _allow_new_work_during_execution;

  /// Iterations to wait before communicating
  const unsigned int _clicks_per_communication;
  /// Iterations to wait before communicating with root
  const unsigned int _clicks_per_root_communication;
  /// Iterations to wait before checking for new objects
  const unsigned int _clicks_per_receive;

  /// MessageTag for sending parallel data
  Parallel::MessageTag _parallel_data_buffer_tag;
  /// Pools for re-using destructed parallel data objects (one for each thread)
  std::vector<MooseUtils::SharedPool<ParallelDataType>> _parallel_data_pools;
  /// Threaded temprorary storage for work added while we're using the _work_buffer (one for each thread)
  std::vector<std::vector<WorkType>> _temp_threaded_work;
  /// Buffer for executing work
  const std::unique_ptr<MooseUtils::Buffer<WorkType>> _work_buffer;
  /// The receive buffer
  const std::unique_ptr<ReceiveBuffer<ParallelDataType, ParallelStudy<WorkType, ParallelDataType>>>
      _receive_buffer;
  /// Send buffers for each processor
  std::unordered_map<
      processor_id_type,
      std::unique_ptr<SendBuffer<ParallelDataType, ParallelStudy<WorkType, ParallelDataType>>>>
      _send_buffers;

  /// Number of chunks of work executed on this processor
  unsigned long long int _local_chunks_executed;
  /// Amount of work completed on this processor
  unsigned long long int _local_work_completed;
  /// Amount of work started on this processor
  unsigned long long int _local_work_started;
  /// Amount of work executed on this processor
  unsigned long long int _local_work_executed;
  /// Amount of work started on all processors
  unsigned long long int _total_work_started;
  /// Amount of work completed on all processors
  unsigned long long int _total_work_completed;

  /// Whether we are within execute()
  bool _currently_executing;
  /// Whether we are between preExecute() and execute()
  bool _currently_pre_executing;
  /// Whether or not we are currently within executeAndBuffer()
  bool _currently_executing_work;
};

template <typename WorkType, typename ParallelDataType>
ParallelStudy<WorkType, ParallelDataType>::ParallelStudy(
    const libMesh::Parallel::Communicator & comm,
    const InputParameters & params,
    const std::string & name)
  : ParallelObject(comm),
    _pid(comm.rank()),
    _name(name),
    _params(params),

    _method((ParallelStudyMethod)(int)(params.get<MooseEnum>("method"))),
    _has_alternate_ending_criteria(false),
    _min_buffer_size(params.isParamSetByUser("min_buffer_size")
                         ? params.get<unsigned int>("min_buffer_size")
                         : params.get<unsigned int>("send_buffer_size")),
    _max_buffer_size(params.get<unsigned int>("send_buffer_size")),
    _buffer_growth_multiplier(params.get<Real>("buffer_growth_multiplier")),
    _buffer_shrink_multiplier(params.get<Real>("buffer_shrink_multiplier")),
    _chunk_size(params.get<unsigned int>("chunk_size")),
    _allow_new_work_during_execution(params.get<bool>("allow_new_work_during_execution")),

    _clicks_per_communication(params.get<unsigned int>("clicks_per_communication")),
    _clicks_per_root_communication(params.get<unsigned int>("clicks_per_root_communication")),
    _clicks_per_receive(params.get<unsigned int>("clicks_per_receive")),

    _parallel_data_buffer_tag(comm.get_unique_tag()),
    _parallel_data_pools(libMesh::n_threads()),
    _temp_threaded_work(libMesh::n_threads()),
    _work_buffer(createWorkBuffer()),
    _receive_buffer(std::make_unique<
                    ReceiveBuffer<ParallelDataType, ParallelStudy<WorkType, ParallelDataType>>>(
        comm, this, _method, _clicks_per_receive, _parallel_data_buffer_tag)),

    _currently_executing(false),
    _currently_pre_executing(false),
    _currently_executing_work(false)
{
#ifndef LIBMESH_HAVE_OPENMP
  if (libMesh::n_threads() != 1)
    mooseWarning(_name, ": Threading will not be used without OpenMP");
#endif

  if (_method != ParallelStudyMethod::SMART && _allow_new_work_during_execution)
    mooseError(_name,
               ": When allowing new work addition during execution\n",
               "('allow_new_work_during_execution = true'), the method must be SMART");
}

template <typename WorkType, typename ParallelDataType>
std::unique_ptr<MooseUtils::Buffer<WorkType>>
ParallelStudy<WorkType, ParallelDataType>::createWorkBuffer()
{
  std::unique_ptr<MooseUtils::Buffer<WorkType>> buffer;

  const auto buffer_type = _params.get<MooseEnum>("work_buffer_type");
  if (buffer_type == "lifo")
    buffer = std::make_unique<MooseUtils::LIFOBuffer<WorkType>>();
  else if (buffer_type == "circular")
    buffer = std::make_unique<MooseUtils::CircularBuffer<WorkType>>();
  else
    mooseError("Unknown work buffer type");

  return buffer;
}

template <typename WorkType, typename ParallelDataType>
InputParameters
ParallelStudy<WorkType, ParallelDataType>::validParams()
{
  auto params = emptyInputParameters();

  params.addRangeCheckedParam<unsigned int>(
      "send_buffer_size", 100, "send_buffer_size > 0", "The size of the send buffer");
  params.addRangeCheckedParam<unsigned int>(
      "chunk_size",
      100,
      "chunk_size > 0",
      "The number of objects to process at one time during execution");
  params.addRangeCheckedParam<unsigned int>("clicks_per_communication",
                                            10,
                                            "clicks_per_communication >= 0",
                                            "Iterations to wait before communicating");
  params.addRangeCheckedParam<unsigned int>("clicks_per_root_communication",
                                            10,
                                            "clicks_per_root_communication > 0",
                                            "Iterations to wait before communicating with root");
  params.addRangeCheckedParam<unsigned int>("clicks_per_receive",
                                            1,
                                            "clicks_per_receive > 0",
                                            "Iterations to wait before checking for new objects");

  params.addParam<unsigned int>("min_buffer_size",
                                "The initial size of the SendBuffer and the floor for shrinking "
                                "it.  This defaults to send_buffer_size if not set (i.e. the "
                                "buffer won't change size)");
  params.addParam<Real>("buffer_growth_multiplier",
                        2.,
                        "How much to grow a SendBuffer by if the buffer completely fills and "
                        "dumps.  Will max at send_buffer_size");
  params.addRangeCheckedParam<Real>("buffer_shrink_multiplier",
                                    0.5,
                                    "0 < buffer_shrink_multiplier <= 1.0",
                                    "Multiplier (between 0 and 1) to apply to the current buffer "
                                    "size if it is force dumped.  Will stop at "
                                    "min_buffer_size.");

  params.addParam<bool>(
      "allow_new_work_during_execution",
      true,
      "Whether or not to allow the addition of new work to the work buffer during execution");

  MooseEnum methods("smart harm bs", "smart");
  params.addParam<MooseEnum>("method", methods, "The algorithm to use");

  MooseEnum work_buffers("lifo circular", "circular");
  params.addParam<MooseEnum>("work_buffer_type", work_buffers, "The work buffer type to use");

  params.addParamNamesToGroup(
      "send_buffer_size chunk_size clicks_per_communication clicks_per_root_communication "
      "clicks_per_receive min_buffer_size buffer_growth_multiplier buffer_shrink_multiplier method "
      "work_buffer_type allow_new_work_during_execution",
      "Advanced");

  return params;
}

template <typename WorkType, typename ParallelDataType>
void
ParallelStudy<WorkType, ParallelDataType>::executeAndBuffer(const std::size_t chunk_size)
{
  _currently_executing_work = true;

  // If chunk_size > the number of objects left, this will properly grab all of them
  const auto begin = _work_buffer->beginChunk(chunk_size);
  const auto end = _work_buffer->endChunk(chunk_size);

  _local_chunks_executed++;

#ifdef LIBMESH_HAVE_OPENMP
#pragma omp parallel
#endif
  {
    const THREAD_ID tid =
#ifdef LIBMESH_HAVE_OPENMP
        omp_get_thread_num();
#else
        0;
#endif

#ifdef LIBMESH_HAVE_OPENMP
#pragma omp for schedule(dynamic, 20) nowait
#endif
    for (auto it = begin; it < end; ++it)
      executeWork(*it, tid);
  }

  // Increment the executed and completed counters
  _local_work_executed += std::distance(begin, end);
  for (auto it = begin; it != end; ++it)
    if (workIsComplete(*it))
      ++_local_work_completed;

  // Insertion point for derived classes to do something to the completed work
  // Example: Create ParallelData to spawn additional work on another processor
  postExecuteChunk(begin, end);

  // Remove the objects we just worked on from the buffer
  _work_buffer->eraseChunk(chunk_size);

  // If new work is allowed to be geneated during execution, it goes into _temp_threaded_work
  // during the threaded execution phase and then must be moved into the working buffer
  if (_allow_new_work_during_execution)
  {
    // Amount of work that needs to be moved into the main working buffer from
    // the temporary working buffer
    std::size_t threaded_work_size = 0;
    for (const auto & work_objects : _temp_threaded_work)
      threaded_work_size += work_objects.size();

    if (threaded_work_size)
    {
      // We don't ever want to decrease the capacity, so only set it if we need more entries
      if (_work_buffer->capacity() < _work_buffer->size() + threaded_work_size)
        _work_buffer->setCapacity(_work_buffer->size() + threaded_work_size);

      // Move the work into the buffer
      for (auto & threaded_work_vector : _temp_threaded_work)
      {
        for (auto & work : threaded_work_vector)
          _work_buffer->move(work);
        threaded_work_vector.clear();
      }

      // Variable that must be set when adding work so that the algorithm can keep count
      // of how much work still needs to be executed
      _local_work_started += threaded_work_size;
    }
  }

  if (_method == ParallelStudyMethod::HARM)
    flushSendBuffers();

  _currently_executing_work = false;
}

template <typename WorkType, typename ParallelDataType>
void
ParallelStudy<WorkType, ParallelDataType>::moveParallelDataToBuffer(
    std::shared_ptr<ParallelDataType> & data, const processor_id_type dest_pid)
{
  mooseAssert(comm().size() > dest_pid, "Invalid processor ID");
  mooseAssert(_pid != dest_pid, "Processor ID is self");

  if (!_currently_executing && !_currently_pre_executing)
    mooseError(_name, ": Cannot sendParallelData() when not executing");

  // Get the send buffer for the proc this object is going to
  auto find_pair = _send_buffers.find(dest_pid);
  // Need to create a send buffer for said processor
  if (find_pair == _send_buffers.end())
    _send_buffers
        .emplace(dest_pid,
                 std::make_unique<
                     SendBuffer<ParallelDataType, ParallelStudy<WorkType, ParallelDataType>>>(
                     comm(),
                     this,
                     dest_pid,
                     _method,
                     _min_buffer_size,
                     _max_buffer_size,
                     _buffer_growth_multiplier,
                     _buffer_shrink_multiplier,
                     _parallel_data_buffer_tag))
        .first->second->moveObject(data);
  // Send buffer exists for this processor
  else
    find_pair->second->moveObject(data);
}

template <typename WorkType, typename ParallelDataType>
void
ParallelStudy<WorkType, ParallelDataType>::flushSendBuffers()
{
  for (auto & send_buffer_iter : _send_buffers)
    send_buffer_iter.second->forceSend();
}

template <typename WorkType, typename ParallelDataType>
void
ParallelStudy<WorkType, ParallelDataType>::reserveBuffer(const std::size_t size)
{
  if (!_currently_pre_executing)
    mooseError(_name, ": Can only reserve in object buffer during pre-execution");

  // We don't ever want to decrease the capacity, so only set if we need more entries
  if (_work_buffer->capacity() < size)
    _work_buffer->setCapacity(size);
}

template <typename WorkType, typename ParallelDataType>
void
ParallelStudy<WorkType, ParallelDataType>::postReceiveParallelDataInternal()
{
  if (_receive_buffer->buffer().empty())
    return;

  // Let derived classes work on the data and then clear it after
  postReceiveParallelData(_receive_buffer->buffer().begin(), _receive_buffer->buffer().end());
  for (auto & data : _receive_buffer->buffer())
    if (data)
      data.reset();

  _receive_buffer->buffer().clear();
}

template <typename WorkType, typename ParallelDataType>
bool
ParallelStudy<WorkType, ParallelDataType>::receiveAndExecute()
{
  bool executed_some = false;

  if (_receive_buffer->currentlyReceiving() && _method == ParallelStudyMethod::SMART)
    _receive_buffer->cleanupRequests();
  else
    _receive_buffer->receive();

  postReceiveParallelDataInternal();

  preReceiveAndExecute();

  while (!_work_buffer->empty())
  {
    executed_some = true;

    // Switch between tracing a chunk and buffering with SMART
    if (_method == ParallelStudyMethod::SMART)
    {
      // Look for extra work first so that these transfers can be finishing while we're executing
      // Start receives only if our work buffer is decently sized
      const bool start_receives_only = _work_buffer->size() > (2 * _chunk_size);
      _receive_buffer->receive(_work_buffer->size() > (2 * _chunk_size));
      if (!start_receives_only)
        postReceiveParallelDataInternal();

      // Execute some objects
      executeAndBuffer(_chunk_size);
    }
    // Execute all of them and then buffer with the other methods
    else
      executeAndBuffer(_work_buffer->size());
  }

  return executed_some;
}

template <typename WorkType, typename ParallelDataType>
void
ParallelStudy<WorkType, ParallelDataType>::smartExecute()
{
  mooseAssert(_method == ParallelStudyMethod::SMART, "Should be called with SMART only");

  // Request for the sum of the started work
  Parallel::Request started_request;
  // Request for the sum of the completed work
  Parallel::Request completed_request;

  // Temp for use in sending the current value in a nonblocking sum instead of an updated value
  unsigned long long int temp;

  // Whether or not to make the started request first, or after every finished request.
  // When allowing adding new work during the execution phase, the starting object counts could
  // change after right now, so we must update them after each finished request is complete.
  // When not allowing generation during propagation, we know the counts up front.
  const bool started_request_first = !_allow_new_work_during_execution;

  // Get the amount of work that was started in the whole domain, if applicable
  if (started_request_first)
    comm().sum(_local_work_started, _total_work_started, started_request);

  // Whether or not the started request has been made
  bool made_started_request = started_request_first;
  // Whether or not the completed request has been made
  bool made_completed_request = false;

  // Good time to get rid of whatever's currently in our SendBuffers
  flushSendBuffers();

  // Use these to try to delay some forced communication
  unsigned int non_executing_clicks = 0;
  unsigned int non_executing_root_clicks = 0;
  bool executed_some = true;

  // Keep executing work until it has all completed
  while (true)
  {
    executed_some = receiveAndExecute();

    if (executed_some)
    {
      non_executing_clicks = 0;
      non_executing_root_clicks = 0;
    }
    else
    {
      non_executing_clicks++;
      non_executing_root_clicks++;
    }

    if (non_executing_clicks >= _clicks_per_communication)
    {
      non_executing_clicks = 0;

      flushSendBuffers();
    }

    if (_has_alternate_ending_criteria)
    {
      if (buffersAreEmpty() && alternateSmartEndingCriteriaMet())
      {
        comm().barrier();
        return;
      }
    }
    else if (non_executing_root_clicks >= _clicks_per_root_communication)
    {
      non_executing_root_clicks = 0;

      // We need the starting work sum first but said request isn't complete yet
      if (started_request_first && !started_request.test())
        continue;

      // At this point, we need to make a request for the completed work sum
      if (!made_completed_request)
      {
        made_completed_request = true;
        temp = _local_work_completed;
        comm().sum(temp, _total_work_completed, completed_request);
        continue;
      }

      // We have the completed work sum
      if (completed_request.test())
      {
        // The starting work sum must be requested /after/ we have finishing counts and we
        // need to make the request for said sum
        if (!made_started_request)
        {
          made_started_request = true;
          temp = _local_work_started;
          comm().sum(temp, _total_work_started, started_request);
          continue;
        }

        // The starting work sum must be requested /after/ we have finishing sum and we
        // don't have the starting sum yet
        if (!started_request_first && !started_request.test())
          continue;

        // Started count is the same as the finished count - we're done!
        if (_total_work_started == _total_work_completed)
          return;

        // Next time around we should make a completed sum request
        made_completed_request = false;
        // If we need the starting work sum after the completed work sum, we need those now as well
        if (!started_request_first)
          made_started_request = false;
      }
    }
  }
}

template <typename WorkType, typename ParallelDataType>
void
ParallelStudy<WorkType, ParallelDataType>::harmExecute()
{
  if (_has_alternate_ending_criteria)
    mooseError("ParallelStudy: Alternate ending criteria not yet supported for HARM");
  if (_allow_new_work_during_execution)
    mooseError(_name, ": The addition of new work during execution is not supported by HARM");
  mooseAssert(_method == ParallelStudyMethod::HARM, "Should be called with HARM only");

  // Request for the total amount of work started
  Parallel::Request work_started_request;
  // Requests for sending the amount of finished worked to every other processor
  std::vector<Parallel::Request> work_completed_requests(comm().size());
  // Whether or not the finished requests have been sent to each processor
  std::vector<bool> work_completed_requests_sent(comm().size(), false);
  // Values of work completed on this processor that are being sent to other processors
  std::vector<unsigned long long int> work_completed_requests_temps(comm().size(), 0);
  // Work completed by each processor
  std::vector<unsigned long long int> work_completed_per_proc(comm().size(), 0);
  // Tag for sending work finished
  const auto work_completed_requests_tag = comm().get_unique_tag();

  // Get the amount of work that was started in the whole domain
  comm().sum(_local_work_started, _total_work_started, work_started_request);

  // All work has been executed, so time to communicate
  flushSendBuffers();

  // HARM only does some communication based on times through the loop.
  // This counter will be used for that
  unsigned int communication_clicks = 0;

  Parallel::Status work_completed_probe_status;
  int work_completed_probe_flag;

  // Keep working until done
  while (true)
  {
    receiveAndExecute();

    flushSendBuffers();

    if (communication_clicks > comm().size())
    {
      // Receive messages about work being finished
      do
      {
        MPI_Iprobe(MPI_ANY_SOURCE,
                   work_completed_requests_tag.value(),
                   comm().get(),
                   &work_completed_probe_flag,
                   work_completed_probe_status.get());

        if (work_completed_probe_flag)
        {
          auto proc = work_completed_probe_status.source();
          comm().receive(proc, work_completed_per_proc[proc], work_completed_requests_tag);
        }
      } while (work_completed_probe_flag);

      _total_work_completed = std::accumulate(
          work_completed_per_proc.begin(), work_completed_per_proc.end(), _local_work_completed);

      // Reset
      communication_clicks = 0;
    }

    // Send messages about objects being finished
    for (processor_id_type pid = 0; pid < comm().size(); ++pid)
      if (pid != _pid &&
          (!work_completed_requests_sent[pid] || work_completed_requests[pid].test()) &&
          _local_work_completed > work_completed_requests_temps[pid])
      {
        work_completed_requests_temps[pid] = _local_work_completed;
        comm().send(pid,
                    work_completed_requests_temps[pid],
                    work_completed_requests[pid],
                    work_completed_requests_tag);
        work_completed_requests_sent[pid] = true;
      }

    // All procs agree on the amount of work started and we've finished all the work started
    if (work_started_request.test() && _total_work_started == _total_work_completed)
    {
      // Need to call the post wait work for all of the requests
      for (processor_id_type pid = 0; pid < comm().size(); ++pid)
        if (pid != _pid)
          work_completed_requests[pid].wait();

      return;
    }

    communication_clicks++;
  }
}

template <typename WorkType, typename ParallelDataType>
void
ParallelStudy<WorkType, ParallelDataType>::bsExecute()
{
  if (_has_alternate_ending_criteria)
    mooseError("ParallelStudy: Alternate ending criteria not yet supported for BS");
  if (_allow_new_work_during_execution)
    mooseError(_name, ": The addition of new work during execution is not supported by BS");
  mooseAssert(_method == ParallelStudyMethod::BS, "Should be called with BS only");

  Parallel::Request work_completed_probe_status;
  Parallel::Request work_completed_request;

  // Temp for use in sending the current value in a nonblocking sum instead of an updated value
  unsigned long long int temp;

  // Get the amount of work that were started in the whole domain
  comm().sum(_local_work_started, _total_work_started, work_completed_probe_status);

  // Keep working until done
  while (true)
  {
    bool receiving = false;
    bool sending = false;

    Parallel::Request some_left_request;
    unsigned int some_left = 0;
    unsigned int all_some_left = 1;

    do
    {
      _receive_buffer->receive();
      postReceiveParallelDataInternal();
      flushSendBuffers();

      receiving = _receive_buffer->currentlyReceiving();

      sending = false;
      for (auto & send_buffer : _send_buffers)
        sending = sending || send_buffer.second->currentlySending() ||
                  send_buffer.second->currentlyBuffered();

      if (!receiving && !sending && some_left_request.test() && all_some_left)
      {
        some_left = receiving || sending;
        comm().sum(some_left, all_some_left, some_left_request);
      }
    } while (receiving || sending || !some_left_request.test() || all_some_left);

    executeAndBuffer(_work_buffer->size());

    comm().barrier();

    if (work_completed_probe_status.test() && work_completed_request.test())
    {
      if (_total_work_started == _total_work_completed)
        return;

      temp = _local_work_completed;
      comm().sum(temp, _total_work_completed, work_completed_request);
    }
  }
}

template <typename WorkType, typename ParallelDataType>
void
ParallelStudy<WorkType, ParallelDataType>::preExecute()
{
  if (!buffersAreEmpty())
    mooseError(_name, ": Buffers are not empty in preExecute()");

  // Clear communication buffers
  for (auto & send_buffer_pair : _send_buffers)
    send_buffer_pair.second->clear();
  _send_buffers.clear();
  _receive_buffer->clear();

  // Clear counters
  _local_chunks_executed = 0;
  _local_work_completed = 0;
  _local_work_started = 0;
  _local_work_executed = 0;
  _total_work_started = 0;
  _total_work_completed = 0;

  _currently_pre_executing = true;
}

template <typename WorkType, typename ParallelDataType>
void
ParallelStudy<WorkType, ParallelDataType>::execute()
{
  if (!_currently_pre_executing)
    mooseError(_name, ": preExecute() was not called before execute()");

  _currently_pre_executing = false;
  _currently_executing = true;

  switch (_method)
  {
    case ParallelStudyMethod::SMART:
      smartExecute();
      break;
    case ParallelStudyMethod::HARM:
      harmExecute();
      break;
    case ParallelStudyMethod::BS:
      bsExecute();
      break;
    default:
      mooseError("Unknown ParallelStudyMethod");
  }

  _currently_executing = false;

  // Sanity checks on if we're really done
  comm().barrier();

  if (!buffersAreEmpty())
    mooseError(_name, ": Buffers are not empty after execution");
}

template <typename WorkType, typename ParallelDataType>
void
ParallelStudy<WorkType, ParallelDataType>::moveWorkError(
    const MoveWorkError error, const WorkType * /* work = nullptr */) const
{
  if (error == MoveWorkError::DURING_EXECUTION_DISABLED)
    mooseError(_name,
               ": The moving of new work into the buffer during work execution requires\n",
               "that the parameter 'allow_new_work_during_execution = true'");
  if (error == MoveWorkError::PRE_EXECUTION_AND_EXECUTION_ONLY)
    mooseError(
        _name,
        ": Can only move work into the buffer in the pre-execution and execution phase\n(between "
        "preExecute() and the end of execute()");
  if (error == MoveWorkError::PRE_EXECUTION_ONLY)
    mooseError(_name,
               ": Can only move work into the buffer in the pre-execution phase\n(between "
               "preExecute() and execute()");
  if (error == MoveWorkError::PRE_EXECUTION_THREAD_0_ONLY)
    mooseError(_name,
               ": Can only move work into the buffer in the pre-execution phase\n(between "
               "preExecute() and execute()) on thread 0");
  if (error == CONTINUING_DURING_EXECUTING_WORK)
    mooseError(_name, ": Cannot move continuing work into the buffer during executeAndBuffer()");

  mooseError("Unknown MoveWorkError");
}

template <typename WorkType, typename ParallelDataType>
void
ParallelStudy<WorkType, ParallelDataType>::canMoveWorkCheck(const THREAD_ID tid)
{
  if (_currently_executing)
  {
    if (!_allow_new_work_during_execution)
      moveWorkError(MoveWorkError::DURING_EXECUTION_DISABLED);
  }
  else if (!_currently_pre_executing)
  {
    if (_allow_new_work_during_execution)
      moveWorkError(MoveWorkError::PRE_EXECUTION_AND_EXECUTION_ONLY);
    else
      moveWorkError(MoveWorkError::PRE_EXECUTION_ONLY);
  }
  else if (tid != 0)
    moveWorkError(MoveWorkError::PRE_EXECUTION_THREAD_0_ONLY);
}

template <typename WorkType, typename ParallelDataType>
void
ParallelStudy<WorkType, ParallelDataType>::moveWorkToBuffer(WorkType & work, const THREAD_ID tid)
{
  // Error checks for moving work into the buffer at unallowed times
  canMoveWorkCheck(tid);

  // Can move directly into the work buffer on thread 0 when we're not executing work
  if (!_currently_executing_work && tid == 0)
  {
    ++_local_work_started; // must ALWAYS increment when adding new work to the working buffer
    _work_buffer->move(work);
  }
  // Objects added during execution go into a temporary threaded vector (is thread safe) to be
  // moved into the working buffer when possible
  else
    _temp_threaded_work[tid].emplace_back(std::move(work));
}

template <typename WorkType, typename ParallelDataType>
void
ParallelStudy<WorkType, ParallelDataType>::moveWorkToBuffer(const work_iterator begin,
                                                            const work_iterator end,
                                                            const THREAD_ID tid)
{
  // Error checks for moving work into the buffer at unallowed times
  canMoveWorkCheck(tid);

  // Get work size beforehand so we can resize
  const auto size = std::distance(begin, end);

  // Can move directly into the work buffer on thread 0 when we're not executing work
  if (!_currently_executing_work && tid == 0)
  {
    if (_work_buffer->capacity() < _work_buffer->size() + size)
      _work_buffer->setCapacity(_work_buffer->size() + size);
    _local_work_started += size;
  }
  else
    _temp_threaded_work[tid].reserve(_temp_threaded_work[tid].size() + size);

  // Move the objects
  if (!_currently_executing_work && tid == 0)
    for (auto it = begin; it != end; ++it)
      _work_buffer->move(*it);
  else
    for (auto it = begin; it != end; ++it)
      _temp_threaded_work[tid].emplace_back(std::move(*it));
}

template <typename WorkType, typename ParallelDataType>
void
ParallelStudy<WorkType, ParallelDataType>::moveWorkToBuffer(std::vector<WorkType> & work_vector,
                                                            const THREAD_ID tid)
{
  moveWorkToBuffer(work_vector.begin(), work_vector.end(), tid);
}

template <typename WorkType, typename ParallelDataType>
void
ParallelStudy<WorkType, ParallelDataType>::moveContinuingWorkToBuffer(WorkType & work)
{
  if (_currently_executing_work)
    moveWorkError(MoveWorkError::CONTINUING_DURING_EXECUTING_WORK);

  _work_buffer->move(work);
}

template <typename WorkType, typename ParallelDataType>
void
ParallelStudy<WorkType, ParallelDataType>::moveContinuingWorkToBuffer(const work_iterator begin,
                                                                      const work_iterator end)
{
  if (_currently_executing_work)
    moveWorkError(MoveWorkError::CONTINUING_DURING_EXECUTING_WORK);

  const auto size = std::distance(begin, end);
  if (_work_buffer->capacity() < _work_buffer->size() + size)
    _work_buffer->setCapacity(_work_buffer->size() + size);

  for (auto it = begin; it != end; ++it)
    _work_buffer->move(*it);
}

template <typename WorkType, typename ParallelDataType>
unsigned long long int
ParallelStudy<WorkType, ParallelDataType>::sendBufferPoolCreated() const
{
  unsigned long long int total = 0;

  for (const auto & buffer : _send_buffers)
    total += buffer.second->bufferPoolCreated();

  return total;
}

template <typename WorkType, typename ParallelDataType>
unsigned long long int
ParallelStudy<WorkType, ParallelDataType>::parallelDataSent() const
{
  unsigned long long int total_sent = 0;

  for (const auto & buffer : _send_buffers)
    total_sent += buffer.second->objectsSent();

  return total_sent;
}

template <typename WorkType, typename ParallelDataType>
unsigned long long int
ParallelStudy<WorkType, ParallelDataType>::buffersSent() const
{
  unsigned long long int total_sent = 0;

  for (const auto & buffer : _send_buffers)
    total_sent += buffer.second->buffersSent();

  return total_sent;
}

template <typename WorkType, typename ParallelDataType>
unsigned long long int
ParallelStudy<WorkType, ParallelDataType>::poolParallelDataCreated() const
{
  unsigned long long int num_created = 0;

  for (const auto & pool : _parallel_data_pools)
    num_created += pool.num_created();

  return num_created;
}

template <typename WorkType, typename ParallelDataType>
bool
ParallelStudy<WorkType, ParallelDataType>::alternateSmartEndingCriteriaMet()
{
  mooseError(_name, ": Unimplemented alternateSmartEndingCriteriaMet()");
}

template <typename WorkType, typename ParallelDataType>
bool
ParallelStudy<WorkType, ParallelDataType>::buffersAreEmpty() const
{
  if (!_work_buffer->empty())
    return false;
  for (const auto & threaded_buffer : _temp_threaded_work)
    if (!threaded_buffer.empty())
      return false;
  if (_receive_buffer->currentlyReceiving())
    return false;
  for (const auto & map_pair : _send_buffers)
    if (map_pair.second->currentlySending() || map_pair.second->currentlyBuffered())
      return false;

  return true;
}
