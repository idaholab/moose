# SolutionInvalidityOutput

The `SolutionInvalidityOutput` handles the output of the time history of solution invalidity object.

## Description

The easiest way to print out the `SolutionInvalidityOutput` is simply to add `solution_invalidity_history = true` to your `[Outputs]` block like so:

```
[Outputs]
  solution_invalidity_history = true
[]
```

This will print out a solution warning table with default timestep interval as 1 as below:

```
Solution Invalid Warnings History:
-----------------------------------------------------------------------------------------
|                   Object                    | Time | Stepinterval Count | Total Count |
-----------------------------------------------------------------------------------------
| NonsafeMaterial : Solution invalid warning! | 0-1  |                  0 |           0 |
| NonsafeMaterial : Solution invalid warning! | 1-2  |                 48 |          48 |
| NonsafeMaterial : Solution invalid warning! | 2-3  |                 48 |          96 |
| NonsafeMaterial : Solution invalid warning! | 3-4  |                 48 |         144 |
| NonsafeMaterial : Solution invalid warning! | 4-5  |                 48 |         192 |
| NonsafeMaterial : Solution invalid warning! | 5-6  |                 48 |         240 |
-----------------------------------------------------------------------------------------
```

If you want to custimized the time interval, you can create a sub-block in `[Outputs]` and change the [!param](/Outputs/SolutionInvalidityOutput/solution_invalidity_timestep_interval) like so:

```
[Outputs]
  [solution_invalid]
    type = SolutionInvalidityOutput
    solution_invalidity_timestep_interval = 2
  []
[]
```
The new solution warning table is as below:

```
Solution Invalid Warnings History:
-----------------------------------------------------------------------------------------
|                   Object                    | Time | Stepinterval Count | Total Count |
-----------------------------------------------------------------------------------------
| NonsafeMaterial : Solution invalid warning! | 0-2  |                 48 |          48 |
| NonsafeMaterial : Solution invalid warning! | 1-3  |                 96 |         144 |
| NonsafeMaterial : Solution invalid warning! | 2-4  |                 96 |         240 |
-----------------------------------------------------------------------------------------
```

!alert note
No table will be output when there are no invalid solution warnings.



!syntax parameters /Outputs/SolutionInvalidityOutput

!syntax inputs /Outputs/SolutionInvalidityOutput

!syntax children /Outputs/SolutionInvalidityOutput

