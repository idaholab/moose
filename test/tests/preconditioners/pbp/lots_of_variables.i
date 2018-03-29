[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Preconditioning]
  [./pbp]
    type = PBP
    solve_order = 'vars0 vars1 vars2 vars3 vars4 vars5 vars6 vars7 vars8 vars9 vars10 vars11 vars12 vars13 vars14 vars15 vars16 vars17 vars18 vars19 vars20 vars21 vars22 vars23 vars24 vars25 vars26 vars27 vars28 vars29 vars30 vars31 vars32 vars33 vars34 vars35 vars36 vars37 vars38 vars39 vars40 vars41 vars42 vars43 vars44 vars45 vars46 vars47 vars48 vars49 vars50 vars51 vars52 vars53 vars54 vars55 vars56 vars57 vars58 vars59 vars60 vars61 vars62 vars63 vars64 vars65 vars66 vars67 vars68 vars69 vars70 vars71 vars72 vars73 vars74 vars75 vars76 vars77 vars78 vars79 vars80 vars81 vars82 vars83 vars84 vars85 vars86 vars87 vars88 vars89 vars90 vars91 vars92 vars93 vars94 vars95 vars96 vars97 vars98 vars99 vars100 vars101 vars102 vars103 vars104 vars105 vars106 vars107 vars108 vars109 vars110 vars111 vars112 vars113 vars114 vars115 vars116 vars117 vars118 vars119 vars120 vars121 vars122 vars123 vars124 vars125 vars126 vars127 vars128 vars129 vars130 vars131 vars132 vars133 vars134 vars135 vars136 vars137 vars138 vars139 vars140 vars141 vars142 vars143 vars144 vars145 vars146 vars147 vars148 vars149 vars150 vars151 vars152 vars153 vars154 vars155 vars156 vars157 vars158 vars159 vars160 vars161 vars162 vars163 vars164 vars165 vars166 vars167 vars168 vars169 vars170 vars171 vars172 vars173 vars174 vars175 vars176 vars177 vars178 vars179 vars180 vars181 vars182 vars183 vars184 vars185 vars186 vars187 vars188 vars189 vars190 vars191 vars192 vars193 vars194 vars195 vars196 vars197 vars198 vars199 vars200 vars201 vars202 vars203 vars204 vars205 vars206 vars207 vars208 vars209 vars210 vars211 vars212 vars213 vars214 vars215 vars216 vars217 vars218 vars219 vars220 vars221 vars222 vars223 vars224 vars225 vars226 vars227 vars228 vars229 vars230 vars231 vars232 vars233 vars234 vars235 vars236 vars237 vars238 vars239 vars240 vars241 vars242 vars243 vars244 vars245 vars246 vars247 vars248 vars249 vars250 vars251 vars252 vars253 vars254 vars255 vars256 vars257 vars258 vars259 vars260 vars261 vars262 vars263 vars264 vars265 vars266 vars267 vars268 vars269 vars270 vars271 vars272 vars273 vars274 vars275 vars276 vars277 vars278 vars279 vars280 vars281 vars282 vars283 vars284 vars285 vars286 vars287 vars288 vars289 vars290 vars291 vars292 vars293 vars294 vars295 vars296 vars297 vars298 vars299 vars300 vars301 vars302 vars303 vars304 vars305 vars306 vars307 vars308 vars309 vars310 vars311 vars312 vars313 vars314 vars315 vars316 vars317 vars318 vars319 vars320 vars321 vars322 vars323 vars324 vars325 vars326 vars327 vars328 vars329 vars330 vars331 vars332 vars333 vars334 vars335 vars336 vars337 vars338 vars339 vars340 vars341 vars342 vars343 vars344 vars345 vars346 vars347 vars348 vars349 vars350 vars351 vars352 vars353 vars354 vars355 vars356 vars357 vars358 vars359 vars360 vars361 vars362 vars363 vars364 vars365 vars366 vars367 vars368 vars369 vars370 vars371 vars372 vars373 vars374 vars375 vars376 vars377 vars378 vars379 vars380 vars381 vars382 vars383 vars384 vars385 vars386 vars387 vars388 vars389 vars390 vars391 vars392 vars393 vars394 vars395 vars396 vars397 vars398 vars399 vars400 vars401 vars402 vars403 vars404 vars405 vars406 vars407 vars408 vars409 vars410 vars411 vars412 vars413 vars414 vars415 vars416 vars417 vars418 vars419 vars420 vars421 vars422 vars423 vars424 vars425 vars426 vars427 vars428 vars429 vars430 vars431 vars432 vars433 vars434 vars435 vars436 vars437 vars438 vars439 vars440 vars441 vars442 vars443 vars444 vars445 vars446 vars447 vars448 vars449 vars450 vars451 vars452 vars453 vars454 vars455 vars456 vars457 vars458 vars459 vars460 vars461 vars462 vars463 vars464 vars465 vars466 vars467 vars468 vars469 vars470 vars471 vars472 vars473 vars474 vars475 vars476 vars477 vars478 vars479 vars480 vars481 vars482 vars483 vars484 vars485 vars486 vars487 vars488 vars489 vars490 vars491 vars492 vars493 vars494 vars495 vars496 vars497 vars498 vars499'
    preconditioner = 'AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG AMG'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = JFNK
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]

[Testing]
  [./LotsOfDiffusion]
    [./vars]
      number = 500
    [../]
  [../]
[]
