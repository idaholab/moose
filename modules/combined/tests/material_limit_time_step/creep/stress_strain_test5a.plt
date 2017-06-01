set terminal postscript enhanced
set output "stress_strain_test5a.ps"
set autoscale
set key top center
set title "Stress Comparison (Plane Strain NAFEMS TEST 5A Test)"
set xlabel "Time (s)"
set ylabel "Creep Strain"
plot "test5a_abaq_crp.dat" using 1:2 title "ABAQUS CE11" with linespoints lt 3 lc 0 lw 2 pt 6 ps 2,\
"test5a_abaq_crp.dat" using 1:3 title "ABAQUS CE22" with linespoints lt 3 lc 1 lw 2 pt 6 ps 2,\
"test5a_abaq_crp.dat" using 1:8 title "ABAQUS CEMAG" with linespoints lt 3 lc 3 lw 2 pt 6 ps 2,\
"nafems_test5a_outfile.txt" using 1:11 title "MOOSE {/Symbol e}_{xx}" with linespoints lt 1 lc 0 lw 1 pt 5,\
"nafems_test5a_outfile.txt" using 1:12 title "MOOSE {/Symbol e}_{yy}" with linespoints lt 1 lc 1 lw 1 pt 5,\
"nafems_test5a_outfile.txt" using 1:10 title "MOOSE {/Symbol e}_{mag}" with linespoints lt 1 lc 3 lw 1 pt 5

