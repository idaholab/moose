# Physics system

A Physics can be tied to a discretization.
## Implementing your own Physics

### Interaction with components

### Advice on implementation

- Use as much parameter checking as you. [PhysicsBase.md] defined utilities such as the ones below
  that let you check that the user inputs to your physics are correct.

```
  void checkParamsBothSetOrNotSet(std::string param1, std::string param2) const;
  template <typename T, typename S>
  void checkVectorParamsSameLength(std::string param1, std::string param2) const;
  template <typename T>
  void checkVectorParamsNoOverlap(std::vector<std::string> param_vec) const;
```

The Physics class you will create will hold the parameters that are shared between all the
discretized versions of it.

Physics and spatial discretizations are as separated as we could, but they are still very much intertwined. So
when you are adding a parameter you need to think about:

- is this more tied to the Physics? If so then it likely belongs in a `PhysicsBase` base class
- is this more tied to the discretization of the equation? If so then it likely belong in the derived, user-instantiated,
  `XYZPhysics` class.
- do you need to keep track of it to retrieve it from another Physics? Or is it very local to this one physics you are
  writing? If it needs to be accessible to unrelated Physics, consider storing it as an attribute and writing a public getter.
