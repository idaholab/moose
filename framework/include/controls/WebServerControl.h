//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Control.h"

#include "WebServerControlTypeRegistry.h"

#include "libmesh/parallel_eigen.h"

#include "tinyhttp/http.h"

#include <atomic>
#include <memory>
#include <thread>

class StartWebServerControlAction;

/**
 * Starts a webserver that an external process can connect to
 * in order to send JSON messages to control the solve
 */
class WebServerControl : public Control
{
public:
  static InputParameters validParams();

  WebServerControl(const InputParameters & parameters);
  ~WebServerControl();

  /**
   * Start the server
   *
   * Called by the StartWebServerControlAction after controls are setup
   */
  void startServer(const Moose::PassKey<StartWebServerControlAction>);

  virtual void execute() override final;

  using ControlledValueBase = Moose::WebServerControlTypeRegistry::ControlledValueBase;

  /**
   * Class containing a value to be controlled.
   *
   * Is responsible for building the value from JSON input,
   * broadcasting the value over all ranks, and setting the
   * value on all ranks.
   *
   * These objects are registered in
   * Moose::WebServerControlTypeRegistration.
   */
  template <class T>
  class ControlledValue : public ControlledValueBase
  {
  public:
    ControlledValue(const std::string & name, const std::string & type)
      : ControlledValueBase(name, type)
    {
    }
    ControlledValue(const std::string & name, const std::string & type, const T & value)
      : ControlledValueBase(name, type), _value(value)
    {
    }

    /// The underlying type of the value
    using value_type = T;

    virtual void setControllableValue(WebServerControl & control) override final
    {
      control.comm().broadcast(_value);
      control.setControllableValueByName<T>(name(), _value);
    }

  private:
    /// The underlying value
    T _value;
  };

protected:
  /**
   * Define the valid methods for a client request.
   */
  enum class RequestMethod
  {
    GET,
    POST
  };

  /**
   * Represents a request from the client.
   */
  struct Request
  {
    Request() = default;

    /**
     * @return Whether or not the request has JSON data
     */
    bool hasJSON() const { return _json.has_value(); }

    /**
     * @return Get the JSON data from the request
     */
    const nlohmann::json & getJSON() const;

    /**
     * Set the JSON data in the request.
     */
    void setJSON(const nlohmann::json & json, const Moose::PassKey<WebServerControl>)
    {
      _json = json;
    }

  private:
    /// The underlying JSON data, if any
    std::optional<nlohmann::json> _json;
  };

  /**
   * Represents a response to the client from the server.
   */
  struct Response
  {
    Response() = default;

    /**
     * Construct a response given a status code.
     */
    Response(const unsigned int status_code);
    /**
     * Construct a response given a status code and JSON data.
     */
    Response(const unsigned int status_code, const nlohmann::json & json);

    /**
     * @return The status code for the response
     */
    unsigned int getStatusCode() const { return _status_code; }

    /**
     * @return Whether or not the response has JSON data
     */
    bool hasJSON() const { return _json.has_value(); }
    /**
     * @return Get the JSON data in the response if it exists
     */
    const nlohmann::json & getJSON() const;

    /**
     * @return Whether or not the response is an error
     */
    bool hasError() const { return _error.has_value(); }
    /**
     * @return Get the error message if it exists
     */
    const std::string & getError() const;

  protected:
    /**
     * See the error message.
     */
    void setError(const std::string & error) { _error = error; }

  private:
    /// The status code
    unsigned int _status_code = 0;
    /// The JSON data, if any
    std::optional<nlohmann::json> _json;
    /// The error message, if any
    std::optional<std::string> _error;
  };

  /**
   * Represents an error response to the client from the server.
   */
  struct ErrorResponse : public Response
  {
    ErrorResponse(const std::string & error, const unsigned int status_code = 400);
  };

  /**
   * Options to be passed to addServerAction
   */
  struct ServerActionOptions
  {
    ServerActionOptions() = default;

    /**
     * @return Whether or not to require waiting
     */
    bool getRequireWaiting() const { return _require_waiting; }
    /**
     * Set the require waiting flag; only accessible by the WebServerControl
     */
    void setRequireWaiting(const bool value, const Moose::PassKey<WebServerControl>)
    {
      _require_waiting = value;
    }

    /**
     * @return Whether or not to require initialization
     */
    bool getRequireInitialized() const { return _require_initialized; }
    /**
     * Set the require initialized flag; only accessible by the WebServerControl
     */
    void setRequireInitialized(const bool value, const Moose::PassKey<WebServerControl>)
    {
      _require_initialized = value;
    }

    /**
     * @return The JSON keys that are required in the request data
     */
    const std::set<std::string> getRequiredJSONKeys() const { return _required_json_keys; }
    /**
     * Append a key to be required in JSON in the request data
     */
    void requireJSONKey(const std::string & key) { _required_json_keys.insert(key); }
    /**
     * Append keys to be required in JSON in the request data
     */
    void requireJSONKeys(std::initializer_list<std::string> && keys)
    {
      _required_json_keys.insert(keys);
    }

  private:
    /// Whether or not to require waiting
    bool _require_waiting = true;
    /// Whether or not to require initialization
    bool _require_initialized = true;
    /// JSON keys that are required in the data
    std::set<std::string> _required_json_keys;
  };

  /**
   * Adds an action for the server to perform at the given path.
   *
   * @param path The path for the webserver to act on
   * @param action The action to perform
   * @param options Options to apply to the endpoint
   */
  template <RequestMethod method>
  void addServerAction(const std::string & path,
                       std::function<Response(const Request &, WebServerControl &)> && action,
                       const ServerActionOptions & options = {});

  /**
   * Entrypoint for controls derived from this one to add additional actions
   */
  virtual void addServerActions() {};

  /**
   * Helper for converting a value to JSON for the given key
   *
   * Will raise exceptions with the context of the key if encountered
   */
  template <class value_T, class key_T>
  static value_T convertJSON(const nlohmann::json & json_value, const key_T & key);

  /**
   * Get whether or not the control is currently waiting
   */
  bool isCurrentlyWaiting() const { return _currently_waiting.load(); }

  /**
   * Stores the information sent by the client on initialize
   */
  struct ClientInfo
  {
    /// Client name
    std::string name;
    /// Client host
    std::string host;
    /// Client user
    std::string user;
    /// Raw data
    nlohmann::json data;
  };

  /**
   * Get the information sent by the client on initialize
   */
  ClientInfo getClientInfo() const;

  /**
   * Output a message with the prefix of this control type and name
   */
  void outputMessage(const std::string & message) const;

private:
  /**
   * Adds the internal actions to the server
   *
   * Enables derived classes to override addServerActions()
   * without mucking with the standard actions.
   */
  void addServerActionsInternal();

  /**
   * Set the ClientInfo object received from the client during /initialize
   */
  void setClientInfo(const ClientInfo & info);

  /**
   * Store a client's poke, which is a timing used to determine the client timeout
   */
  void clientPoke();

  /**
   * Whether or not the client has called /initialize
   */
  bool isClientInitialized() const { return _client_initialized.load(); }
  /**
   * Set that the client has called /initialized; used by the server
   */
  void setClientInitialized() { _client_initialized.store(true); }

  /**
   * Set that the control is currently waiting; used by the server
   */
  void setCurrentlyWaiting(const bool value = true) { _currently_waiting.store(value); }

  /**
   * Whether or not the client has called /terminate
   */
  bool isTerminateRequested() const { return _terminate_requested.load(); }
  /**
   * Set for the control to terminate the solve; used by /terminate in the server
   */
  void setTerminateRequested(const bool value = true) { _terminate_requested.store(value); }

  /**
   * Get whether or not the client sent the kill command.
   */
  bool isKillRequested() const { return _kill_requested.load(); }
  /**
   * Set for the control to kill the solve; used by /kill in the server
   */
  void setKillRequested() { _kill_requested.store(true); }

  /**
   * Output a timing message with the prefix of this control
   */
  void outputClientTiming(const std::string & message,
                          const std::chrono::time_point<std::chrono::steady_clock> & start) const;

  /**
   * Helper for producing an error message about a client timeout
   *
   * Used for both the initial and during-run timeout
   */
  std::string clientTimeoutErrorMessage(const Real timeout,
                                        const std::string & timeout_param_name,
                                        const std::optional<std::string> & suffix = {}) const;

  /**
   * Stop the server if it exists and is running
   */
  void stopServer();

  /// Port to listen on, if any
  const unsigned int * const _port;
  /// File socket to listen on, if any
  const FileName * const _file_socket;
  /// Time in seconds to allow the client to initially communicate before timing out
  const Real _initial_client_timeout;
  /// Time in seconds to allow the client to communicate after init before timing out
  const Real _client_timeout;

  /// Whether or not the client has called /initialize
  std::atomic<bool> _client_initialized = false;
  /// Whether or not the Control is currently waiting
  std::atomic<bool> _currently_waiting = false;
  /// Whether or not the solve should be terminated in the next execute() call
  std::atomic<bool> _terminate_requested = false;
  /// Whether or not the client has called /kill
  std::atomic<bool> _kill_requested = false;
  /// The most recent time we've heard from the client
  std::atomic<int64_t> _last_client_poke = 0;

  /// Client information received on /initialize by the server
  std::optional<ClientInfo> _client_info;
  /// Lock for _client_info as it is written by the server thread
  mutable std::mutex _client_info_lock;

  /// Weak pointer to the server; the server itself is owned by the server thread
  std::weak_ptr<HttpServer> _server_weak_ptr;
  /// The server thread
  std::unique_ptr<std::thread> _server_thread_ptr;

  /// The values received to control; filled on rank 0 from the server and then broadcast
  std::vector<std::unique_ptr<ControlledValueBase>> _controlled_values;
  /// Mutex to prevent threaded writes to _controlled_values
  std::mutex _controlled_values_mutex;
};

template <class value_T, class key_T>
value_T
WebServerControl::convertJSON(const nlohmann::json & json_value, const key_T & key)
{
  try
  {
    return json_value[key].template get<value_T>();
  }
  catch (const std::exception & e)
  {
    std::ostringstream message;
    message << "While parsing '" << key << "' " << e.what();
    throw std::runtime_error(message.str());
  }
}
