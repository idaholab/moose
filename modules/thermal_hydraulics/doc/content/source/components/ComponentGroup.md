# ComponentGroup

This is a helper object that allows nesting of components into groups.  Users never instantiate it
directly in an input file, but indirectly via input file syntax like so:

```
[Components]
  [./compA]
  [../]

  [./group]
    [./comp1]
    [../]
    [./comp2]
    [../]
  [../]
[]
```

This input file will build 3 components: `compA`, `group/comp1`, `group/comp2`. No matter the location
in the input file, components are always referred to with their full name, i.e. in our example above,
one cannot refer to `comp1` with name `comp1` inside the group `group` - it must be referred to
as `group/comp1`.

!syntax parameters /Components/ComponentGroup

!syntax inputs /Components/ComponentGroup

!syntax children /Components/ComponentGroup
