# RepeatableRayStudyBase

The `RepeatableRayStudyBase` is a specialized [RayTracingStudy.md] that simplifies the [Ray.md] generation process. To describe how exactly it simplifies this process, we will describe the difficulties of [Ray.md] generation and how this study overcomes many of those issues.

!alert tip
The use of the `RepeatableRayStudyBase` is considered an +advanced+ use of the [ray_tracing/index.md]. Please consider the [RepeatableRayStudy.md] before using this object.

### Finding a Ray's Starting Element

The [ray_tracing/index.md] requires that a [Ray.md] be on the processor that contains the element that it starts in when it is moved into the buffer to be traced. If one wants to only specify a set of starting points for rays, it becomes necessary to determine which elements said points are contained in and then to communicate ahead of time the [Ray.md] to the processor that owns each element.

The `RepeatableRayStudyBase` has an option that only requires the defined rays to have their starting points set. Internally, the rays will be "claimed" and communicated to the processor that owns their starting element based on the user-set starting points. In addition, the incoming side on said starting elements will be set (if any).

The "claiming" of rays after they are defined is controlled by the `_claim_after_define_rays` private parameter. An example of a study that uses this claiming is the [RepeatableRayStudy.md].

### Tracing After Mesh Changes

In the case of mesh changes (for example, mesh adaptivity steps), the element that a [Ray.md] starts in may change, which may also result in a change of the processor that a [Ray.md] starts on.

The `RepeatableRayStudyBase` keeps a copy of the user-generated rays such that at any time the mesh changes, the information is available to trace rays with the same information (start point, direction, data, etc). Internally, when the mesh changes the `RepeatableRayStudyBase` will re-claim the users' rays to ensure that they are again on the correct processor with the correct starting elements.

## Process

The process by which the `RepeatableRayStudyBase` generates rays follows:

1. [Define Rays](#define-rays) - Only done on the first execution of the study.
2. [Claim Rays](#claim-rays) - Done on the first execution of the study and after all mesh changes.
3. [Copy Rays](#copy-rays) - Done on every execution of the study.

### Define Rays

The user-derived object will overload the `defineRays()` method. Upon first execution of the study, this method will be called. Within `defineRays()`, you are to create rays (see [Ray.md#defining-a-ray-trajectory]) and move them into the `_rays` member variable. This action by default is only performed once, when `generateRays()` is first called.

If you are defining rays that need to be "claimed", that is they are being defined with only their start points (and not starting elements or starting incoming sides), ensure that the `_claim_after_define_rays` parameter is set to `true`. When this parameter is `true`, it is assumed that the starting elements and starting incoming sides have not been set and that the rays need to be "claimed". After claiming, internally they will be placed on the correct processors with a starting element that contains their starting points.

If you are defining rays that:

- have their starting point set,
- have their starting element set (which contains the starting point),
- have their starting incoming sides set (if any - the rays can also start within an element),
- are filled into `_rays` on the processor that contains their respective starting elements,

then it is not necessary to utilize claiming. You would set the `_claim_after_define_rays` parameter to false.

Any [Ray.md] data or auxiliary data that is set at this point will also be used in any further executions of this study.

The other important parameter that can be changed is the `_define_rays_replicated` private parameter. If this parameter is true, the rays that are filled into `_rays` during `defineRays()` are replicated. That is, the same rays were filled into `_rays` across all processors. If `_claim_after_define_rays == false`, the `_define_rays_replicated` parameter is set to false regardless of the user's setting because it is not possible for rays that are on their correct processors with their correct starting elements to be replicated.

#### Example

For an example of the define process, see [RepeatableRayStudy.md]:

!listing modules/ray_tracing/src/userobjects/RepeatableRayStudy.C re=void\sRepeatableRayStudy::defineRays.*?^}

In this case, the rays defined during `defineRays()` are replicated across all processors. Their start points are set but the starting elements and starting incoming sides are not set, therefore claiming is required.

### Claim Rays

Note that the actions that follow in this section are performed on the first execution of the study and thereafter only after each mesh change, because claiming afterwards is only needed when the mesh changes.

If the private parameter `_claim_after_define_rays == true`, the rays within `_rays` do not have their starting elements set and are not necessarily on the correct starting processor. The `_rays` are passed to the `ClaimRays` object and the result is `_local_rays` being filled with the rays that can be started in the local processor. This claiming is only performed in the first call of `generateRays()` and thereafter is only called after each mesh change to re-determine the starting elements and starting processors.

If the private parameter `_claim_after_define_rays == false`, the rays within `_rays` are already on their starting processor with the starting elements set. The `_rays` are then simply copied into `_local_rays`. Because all of the [Ray.md] objects are actually shared pointers (`std::shared_ptr<Ray>`), this copying process does not actually "copy" the rays, it just points to the same objects that are in `_rays`. This "copying" is seen as:

!listing modules/ray_tracing/src/userobjects/RepeatableRayStudyBase.C re=\s\s//\sThe\sRays\sin\s_rays.*?^\s\s}

### Copy Rays

In every execution of the study, all of the rays in `_local_rays` are copied and inserted into the buffer to be traced. An actual copy takes place here - we want the rays within `_local_rays` to always be valid so that on later executions of the study, we can produce repeatable behavior in terms of the rays that are being traced.

!listing modules/ray_tracing/src/userobjects/RepeatableRayStudyBase.C re=\s\s//\sReserve\sahead\sof\stime.*?^\s\s}
