> In the original input we're fixing the y displacement to 0 in the entire
> bottom boundary. Try to relax this constraint a bit try to add a second single
> node boundary using the `ExtraNodesetGenerator` (chained in after the `pin`
> generator). Use one of the two bottom corner nodes. Now think about where you
> have to apply the `disp_x = 0` boundary condition.

Under `[Mesh]` add

```
  [pin2]
    type = ExtraNodesetGenerator
    input = pin
    new_boundary = pin2
    coord = '-0.25 0 0'
  []
```

Replace the `[bottom_y]` block under `[BCs]` with

```
  [pin_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'pin pin2'
    value = 0
  []
```

Note that we need to pin the first node (`pin`) in both direction to completely
eliminate translational modes. The second node is only constrained in the y
direction. The spatial arrangement of the pinned nodes matters. To optimally pin
the rotational mode the vector from the first to the second node should be
perpendicular to the direction that you are constraining on the second node
(this should intuitively make sense).

When you run the modified input you should see that the bottom surface of the
bimetallic strip does not remain exactly straight. We've changed the simulated
scenario to represent a strip that is suspended in air.
