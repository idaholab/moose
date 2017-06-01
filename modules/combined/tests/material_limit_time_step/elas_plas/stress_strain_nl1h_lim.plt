set terminal postscript enhanced
set output "stress_strain_nl1h_lim.ps"
set autoscale
set key bottom right
set title "Stress Comparison (Plane Strain NAFEMS NL1 Test, H = 62.5 GPa)"
set xlabel "Time (s)"
set ylabel "Von Mises Stress (Pa)"
plot "nl1_abaq.dat" using 1:11 title "ABAQUS" with line lt 3 lc 0 lw 4,\
"nafems_nl1h_out.txt" using 1:12 title "MOOSE, {/Symbol D}t = 0.1" with linespoints lt 1 lc 1 lw 1 pt 8,\
"nafems_nl1h_lim_out.txt" using 1:13 title "MOOSE, max plas strain incr = 1e-6 " with linespoints lt 1 lc 3 lw 1 pt 6

#plot "nl1_abaq.dat" using 1:11 title "{/Symbol s}_{mises} (ABAQUS)" with line lt 3 lc 0 lw 4,\
#"nafems_nl1h_lim_out.txt" using 1:13 title "{/Symbol s}_{mises} (MOOSE, limit)" with linespoints lt 1 lc 3 lw 1 pt 6,\
#"nafems_nl1h_out.txt" using 1:12 title "{/Symbol s}_{mises} (MOOSE, no lim)" with linespoints lt 1 lc 1 lw 1 pt 8

#plot "nl1_abaq.dat" using 1:6 title "{/Symbol s}_{xx} (ABAQUS)" with line lt 3 lc 0 lw 4,\
#"nl1_abaq.dat" using 1:7 title "{/Symbol s}_{yy} (ABAQUS)" with line lt 3 lc 1 lw 4,\
#"nl1_abaq.dat" using 1:8 title "{/Symbol s}_{zz} (ABAQUS)" with line lt 3 lc 2 lw 4,\
#"nl1_abaq.dat" using 1:11 title "{/Symbol s}_{mises} (ABAQUS)" with line lt 3 lc 3 lw 4,\
#"nafems_nl1h_lim_out.txt" using 1:9 title "{/Symbol s}_{xx} (MOOSE)" with line lt 1 lc 0 lw 1,\
#"nafems_nl1h_lim_out.txt" using 1:10 title "{/Symbol s}_{yy} (MOOSE)" with line lt 1 lc 1 lw 1,\
#"nafems_nl1h_lim_out.txt" using 1:11 title "{/Symbol s}_{zz} (MOOSE)" with line lt 1 lc 2 lw 1,\
