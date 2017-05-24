#ifndef CONTROLDATA_H
#define CONTROLDATA_H

// C++ includes
#include <vector>

// Forward declarations
class ControlDataValue;

/**
 * Abstract definition of a ControlData value.
 */
class ControlDataValue
{
public:
  /**
   * Constructor
   * @param name The full (unique) name for this piece of data.
   */
  ControlDataValue(std::string name) : _name(name), _declared(false) {}

  /**
   * Destructor.
   */
  virtual ~ControlDataValue() = default;

  /**
   * String identifying the type of parameter stored.
   * Must be reimplemented in derived classes.
   */
  virtual std::string type() = 0;

  /**
   * The full (unique) name of this particular piece of data.
   */
  const std::string & name() const { return _name; }

  /**
   * Mark the data as declared
   */
  void setDeclared() { _declared = true; }

  /**
   * Get the declared state
   */
  bool getDeclared() { return _declared; }

protected:
  /// The full (unique) name of this particular piece of data.
  const std::string _name;
  /// true if the data was declared by calling declareControlData. All data must be declared up front.
  bool _declared;
};

/**
 * Concrete definition of a parameter value
 * for a specified type.
 */
template <typename T>
class ControlData : public ControlDataValue
{
public:
  /**
   * Constructor
   * @param name The full (unique) name for this piece of data.
   */
  ControlData(std::string name) : ControlDataValue(name) { _value_ptr = new T; }

  virtual ~ControlData() { delete _value_ptr; }

  /**
   * @returns a read-only reference to the parameter value.
   */
  const T & get() { return *_value_ptr; }

  /**
   * @returns a writable reference to the parameter value.
   */
  T & set() { return *_value_ptr; }

  /**
   * String identifying the type of parameter stored.
   */
  virtual std::string type() override;

private:
  /// Stored value.
  T * _value_ptr;
};

// ------------------------------------------------------------
// ControlData<> class inline methods
template <typename T>
inline std::string
ControlData<T>::type()
{
  return typeid(T).name();
}

#endif
