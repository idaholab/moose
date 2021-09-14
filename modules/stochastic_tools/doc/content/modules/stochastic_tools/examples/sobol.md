# SOBOL Sensitivity Analysis

The following example converts [parameter_study.md] to perform a global variance-based
sensitivity analysis following the method of [!cite](saltelli2002making).

## Input File

To perform a the sensitivity analysis only two minor changes to the input file from
[parameter_study.md] need to occur: the sampler needs be changed and another statistics
calculation object needs to be added, the remainder of the file remains the same. The exception
being that the sampler name in a few of the objects was updated to use the "sobol" Sampler
name, as defined in the following sub-section.

### Sobol Sampler

The Sobol sampling scheme consists of using a sample and re-sample matrix to create a series of
matrices that can be used to compute first-order, second-order, and total-effect sensitivity indices.
The first-order indices are the portion of the variance in the quantity of interest (e.g., $T_{avg}$)
due to the variance of the uncertain parameters (e.g., $\gamma$). The second-order indices are the
portion of the variance in the quantity of interest due to the variance two uncertain parameters
interacting (e.g., $\gamma$ as $s$).  Finally, the total-effect indices are the portion of the
variance of the quantity of interest due to the variance of an uncertain parameter and all
interactions with the other parameters. For complete details of the method please refer to
[!cite](saltelli2002making).

The [SobolSampler.md] object requires two input samplers to form the sample and re-sample matrix.
Thus, the `Samplers` block contains three sample objects. The first two are used by the third,
the "sobol" sampler, which is the Sampler used by the other objects in the simulation.

!listing examples/sobol/master.i block=Samplers

The sobol method implemented here requires $n(2k+2)$ model evaluations, where $k$ is the number
of uncertain parameters (4) and $n$ is the number of replicates (10,000). Therefore, for this example
100,000 model evaluations were performed to compute the indices.

### Sobol Statistics

The values of the first-order, second-order, and total-effect are computed by the
[SobolReporter.md] object. This object is a Reporter, as such it is added to the
`Reporters` block. The output of the object includes all the indices for each of
the vectors supplied.

!listing examples/sobol/master.i block=Reporters

## Results

If [JSONOutput.md] output is enabled, the [SobolReporter.md] object will write a file that contains
columns of data. Each column comprises of the computed indices for the quantities of
interest. For example, [sobol_out] is the complete output from the [SobolReporter.md] object
for this example problem.

!listing caption=Computed Sobol indices for the example heat conduction problem. id=sobol_out language=json
{
    "reporters": {
        "sobol": {
            "confidence_intervals": {
                "levels": [
                    0.05,
                    0.95
                ],
                "method": "percentile",
                "replicates": 10000,
                "seed": 1
            },
            "indices": [
                "FIRST_ORDER",
                "TOTAL",
                "SECOND_ORDER"
            ],
            "num_params": 4,
            "type": "SobolReporter",
            "values": {
                "results_results:T_avg:value": {
                    "type": "SobolIndices<double>"
                },
                "results_results:q_left:value": {
                    "type": "SobolIndices<double>"
                }
            }
        },
    },
    "time_steps": [
        {
            "sobol": {
                "results_results:T_avg:value": {
                    "FIRST_ORDER": [
                        [
                            0.7828157644195685,
                            0.2024457166671371,
                            0.002897136551452905,
                            0.0005763795289419524
                        ],
                        [
                            [
                                0.733777090732234,
                                0.18794335174620977,
                                0.0024178681206137383,
                                0.00013524415404111495
                            ],
                            [
                                0.8389031197370065,
                                0.21893171791138458,
                                0.0034053890508129038,
                                0.001013113381183262
                            ]
                        ]
                    ],
                    "SECOND_ORDER": [
                        [
                            [
                                0.7828157644195685,
                                0.0013657906960888866,
                                -0.026286609856115972,
                                -0.02709392736809313
                            ],
                            [
                                0.0013657906960888866,
                                0.2024457166671371,
                                -0.0038658094815994526,
                                -0.0034417727808645215
                            ],
                            [
                                -0.026286609856115972,
                                -0.0038658094815994526,
                                0.002897136551452905,
                                -2.4392856782177574e-05
                            ],
                            [
                                -0.02709392736809313,
                                -0.0034417727808645215,
                                -2.4392856782177574e-05,
                                0.0005763795289419524
                            ]
                        ],
                        [
                            [
                                [
                                    0.733777090732234,
                                    -0.11803538104538913,
                                    -0.12172193388301167,
                                    -0.12221828695888393
                                ],
                                [
                                    -0.11803538104538913,
                                    0.18794335174620977,
                                    -0.015612493016766121,
                                    -0.014994033091004527
                                ],
                                [
                                    -0.12172193388301167,
                                    -0.015612493016766121,
                                    0.0024178681206137383,
                                    -0.0008958725175674106
                                ],
                                [
                                    -0.12221828695888393,
                                    -0.014994033091004527,
                                    -0.0008958725175674106,
                                    0.00013524415404111495
                                ]
                            ],
                            [
                                [
                                    0.8389031197370065,
                                    0.11689677438768464,
                                    0.06554033901199585,
                                    0.06463203560148556
                                ],
                                [
                                    0.11689677438768464,
                                    0.21893171791138458,
                                    0.007783594794879001,
                                    0.008032250449225842
                                ],
                                [
                                    0.06554033901199585,
                                    0.007783594794879001,
                                    0.0034053890508129038,
                                    0.0008566035169649605
                                ],
                                [
                                    0.06463203560148556,
                                    0.008032250449225842,
                                    0.0008566035169649605,
                                    0.001013113381183262
                                ]
                            ]
                        ]
                    ],
                    "TOTAL": [
                        [
                            0.7962125505072905,
                            0.22205277595127304,
                            0.007827888001001648,
                            0.00576020990502113
                        ],
                        [
                            [
                                0.7801854878321646,
                                0.16830255153682194,
                                -0.059398033017196816,
                                -0.061782302915692755
                            ],
                            [
                                0.8109338531968722,
                                0.2706477496581634,
                                0.06792776357927821,
                                0.06569038187030007
                            ]
                        ]
                    ]
                },
                "results_results:q_left:value": {
                    "FIRST_ORDER": [
                        [
                            0.7775784663837111,
                            0.20835461645183223,
                            0.0012495184877842089,
                            -7.780391891513346e-06
                        ],
                        [
                            [
                                0.7279707224186921,
                                0.1932828873014937,
                                0.000942273644098236,
                                -4.1832655634983365e-05
                            ],
                            [
                                0.8342351099370122,
                                0.22557562178414162,
                                0.0015737877385167286,
                                2.5774660863561836e-05
                            ]
                        ]
                    ],
                    "SECOND_ORDER": [
                        [
                            [
                                0.7775784663837111,
                                0.0023676473484735006,
                                -0.028546434327486048,
                                -0.028893421278486153
                            ],
                            [
                                0.0023676473484735006,
                                0.20835461645183223,
                                -0.003997335975515964,
                                -0.003658648629372402
                            ],
                            [
                                -0.028546434327486048,
                                -0.003997335975515964,
                                0.0012495184877842089,
                                2.253877758561949e-05
                            ],
                            [
                                -0.028893421278486153,
                                -0.003658648629372402,
                                2.253877758561949e-05,
                                -7.780391891513346e-06
                            ]
                        ],
                        [
                            [
                                [
                                    0.7279707224186921,
                                    -0.11896935086448301,
                                    -0.1239975221783034,
                                    -0.12415495212606797
                                ],
                                [
                                    -0.11896935086448301,
                                    0.1932828873014937,
                                    -0.01622349465469486,
                                    -0.01574749349528906
                                ],
                                [
                                    -0.1239975221783034,
                                    -0.01622349465469486,
                                    0.000942273644098236,
                                    -4.552075747997703e-05
                                ],
                                [
                                    -0.12415495212606797,
                                    -0.01574749349528906,
                                    -4.552075747997703e-05,
                                    -4.1832655634983365e-05
                                ]
                            ],
                            [
                                [
                                    0.8342351099370122,
                                    0.11925244826523318,
                                    0.06382658793704044,
                                    0.06308750873405622
                                ],
                                [
                                    0.11925244826523318,
                                    0.22557562178414162,
                                    0.0082894937055141,
                                    0.008539489767919212
                                ],
                                [
                                    0.06382658793704044,
                                    0.0082894937055141,
                                    0.0015737877385167286,
                                    9.105737514753008e-05
                                ],
                                [
                                    0.06308750873405622,
                                    0.008539489767919212,
                                    9.105737514753008e-05,
                                    2.5774660863561836e-05
                                ]
                            ]
                        ]
                    ],
                    "TOTAL": [
                        [
                            0.7931022386847613,
                            0.23142827171251845,
                            0.007437646633820183,
                            0.006442278685175884
                        ],
                        [
                            [
                                0.7766372930040141,
                                0.17738324857147236,
                                -0.06068889531855026,
                                -0.061664391988613065
                            ],
                            [
                                0.8082098264143118,
                                0.2798375843800114,
                                0.0677642788602979,
                                0.06688096608679728
                            ]
                        ]
                    ]
                }
            },
            "time": 2.0,
            "time_step": 2
        }
    ]
}

For each set of indices (`FIRST_ORDER`, `SECOND_ORDER`, and `TOTAL`) contains a pair entries:
the first is the values computed and the second corresponds to the 5% and 95% confidence intervals.
This problem examines four uncertain parameters, so each element of the set of indices corresponds
to the parameter indicated in the input file ($\gamma$, $q_0$, $T_0$, and $s$). See [SobolReporter.md]
for further information regarding the output.

For the problem at hand, the first-order and second-order indices for the two quantities of interest
are presented in [S_T_avg] and [S_q_left]. The diagonal entries are the first-order incides and
the off-diagonal terms are the second-order indices. For example for $T_{avg}$ the first order-index
for $\gamma$ is $S_1 = 0.763$ and the second-order index $S_{1,2} = 0.014$ for $\gamma$ interacting
with $q_0$. The negative values are essentially zero, if more replicates were executed these
numbers would become closer to zero.

!table id=S_T_avg caption=First-order and second-order Sobol indices for $T_{avg}$.
| $S_{i,j}$ | $\gamma$ (95% CI)        | $q_0$ (95% CI)              | $T_0$ (95% CI)               | $s$ (95% CI)               |
| -         | -                        | -                           | -                            | -                          |
| $\gamma$  | 0.783 (0.734, 0.839)     | -                           | -                            | -                          |
| $q_0$     | 0.0137 (-0.118, 0.117)   | 0.202 (0.188, 0.219)        | -                            | -                          |
| $T_0$     | -0.0263 (-0.122, 0.0655) | -3.87e-3 (-0.0156, 7.78e-3) | 2.90e-3 (2.42e-3, 3.41e-3)   | -                          |
| $s$       | -0.0271 (-0.122, 0.0646) | -3.44e-3 (-0.0150, 8.03e-3) | -2.44e-5 (-8.96e-4, 8.57e-3) | 5.76e-4 (1.35e-4, 1.01e-3) |

!table id=S_q_left caption=First-order and second-order Sobol indices for $q_{left}$.
| $S_{i,j}$ | $\gamma$ (95% CI)        | $q_0$ (95% CI)              | $T_0$ (95% CI)               | $s$ (95% CI)                 |
| -         | -                        | -                           | -                            | -                            |
| $\gamma$  | 0.778 (0.728, 0.834)     | -                           | -                            | -                            |
| $q_0$     | 2.37e-3 (-0.119, 0.119)  | 0.208 (0.193, 0.226)        | -                            | -                            |
| $T_0$     | -0.0285 (-0.124, 0.0638) | -4.00e-3 (-0.0162, 8.29e-3) | 1.25e-3 (9.42e-4, 1.57e-3)   | -                            |
| $s$       | -0.0289 (-0.124, 0.0631) | -3.66e-3 (-0.0157, 8.54e-3) | -2.25e-5 (-4.55e-5, 9.12e-5) | -7.78e-6 (-4.18e-5, 2.58e-5) |

The data in these two tables clearly indicates that a majority of the variance of both quantities of
interest are due to the variance of $\gamma$ ($S_1$) and $q_0$ ($S_2$). Additionally, a small
contribution of the variance is from a second-order interaction, $S_{1,2}$, between $\gamma$ and
$q_0$. The importance of $\gamma$ and $q_0$ if further evident by the total-effect indices, as shown in
[total-effect].

!table id=total-effect caption=Total-effect Sobol indices for $T_{avg}$ and $q_{left}$.
| $S_T$      | $\gamma$ (95% CI)    | $q_0$ (95% CI)       | $T_0$ (95% CI)            | $s$ (95% CI)              |
| -          | -                    | -                    | -                         | -                         |
| $T_{avg}$  | 0.796 (0.780, 0.811) | 0.222 (0.168, 0.271) | 7.83e-3 (-0.0594, 0.0679) | 5.76e-3 (-0.0618, 0.0657) |
| $q_{left}$ | 0.793 (0.777, 0.808) | 0.231 (0.177, 0.280) | 7.44e-3 (-0.0607, 0.0678) | 6.44e-3 (-0.617, 0.0669)  |
