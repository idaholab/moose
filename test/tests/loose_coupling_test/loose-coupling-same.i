[SubProblems]
	active = 'problem1 problem2'
	
	[./problem1]
		file = problem1.i
	[../]
	
	[./problem2]
		file = problem2.i
	[../]
[]

[Executioner]
	type = LooseCoupling
	solve_order = 'problem1 problem2'
	start_time = 0
	end_time = 1
	dt = 0.1
[]

[Output]
	file_base = out_same
	output_initial = true
	interval = 1
	exodus = true
	print_linear_residuals = true
[]

[Debug]
	show_actions = true
[]
