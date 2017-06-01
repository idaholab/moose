set terminal postscript enhanced
set output "stress_strain_nl1_lim.ps"
set autoscale
set key top right
set title "Stress Comparison (Plane Strain NAFEMS NL1 Test, H = 0, matl limit)"
set xlabel "Time (s)"
set ylabel "Stress (Pa)"
plot "nl1_abaq.dat" using 1:2 title "{/Symbol s}_{xx} (ABAQUS)" with line lt 3 lc 0 lw 4,\
"nl1_abaq.dat" using 1:3 title "{/Symbol s}_{yy} (ABAQUS)" with line lt 3 lc 1 lw 4,\
"nl1_abaq.dat" using 1:4 title "{/Symbol s}_{zz} (ABAQUS)" with line lt 3 lc 2 lw 4,\
"nl1_abaq.dat" using 1:10 title "{/Symbol s}_{mises} (ABAQUS)" with line lt 3 lc 3 lw 4,\
"nafems_nl1_lim_out.txt" using 1:9 title "{/Symbol s}_{xx} (MOOSE)" with line lt 1 lc 0 lw 1,\
"nafems_nl1_lim_out.txt" using 1:10 title "{/Symbol s}_{yy} (MOOSE)" with line lt 1 lc 1 lw 1,\
"nafems_nl1_lim_out.txt" using 1:11 title "{/Symbol s}_{zz} (MOOSE)" with line lt 1 lc 2 lw 1,\
"nafems_nl1_lim_out.txt" using 1:13 title "{/Symbol s}_{mises} (MOOSE)" with linespoints lt 1 lc 3 lw 1 pt 6
