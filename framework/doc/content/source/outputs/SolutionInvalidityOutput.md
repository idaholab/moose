# SolutionInvalidityOutput

The `SolutionInvalidityOutput` handles the output of the time history of solution invalidity object.

!syntax description /Outputs/SolutionInvalidityOutput

## Description

The easiest way to print out the `SolutionInvalidityOutput` is simply to add `solution_invalidity_history = true` to your `[Outputs]` block like so:

```
[Outputs]
  solution_invalidity_history = true
[]
```

This will print out a solution warning table with default time interval as 1 as below:

```
Solution Invalid Warnings History:
-----------------------------------------------------------------------------------------
|                   Object                    | Time | Timeinterval Count | Total Count |
-----------------------------------------------------------------------------------------
| NonsafeMaterial : Solution invalid warning! | 0-1  |                  0 |         288 |
| NonsafeMaterial : Solution invalid warning! | 1-2  |                 48 |         288 |
| NonsafeMaterial : Solution invalid warning! | 2-3  |                 48 |         288 |
| NonsafeMaterial : Solution invalid warning! | 3-4  |                 48 |         288 |
| NonsafeMaterial : Solution invalid warning! | 4-5  |                 48 |         288 |
| NonsafeMaterial : Solution invalid warning! | 5-6  |                 48 |         288 |
-----------------------------------------------------------------------------------------
```

If you want to custimized the time interval, you can create a sub-block in `[Outputs]` and change the [!param](/Outputs/SolutionInvalidityOutput/time_interval) like so:

```
[Outputs]
  [solution_invalid]
    type = SolutionInvalidityOutput
    time_interval = 2
  []
[]
```

!syntax parameters /Outputs/SolutionInvalidityOutput

!syntax inputs /Outputs/SolutionInvalidityOutput

!syntax children /Outputs/SolutionInvalidityOutput

