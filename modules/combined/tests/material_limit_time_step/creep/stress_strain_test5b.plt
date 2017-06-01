set terminal postscript enhanced
set output "stress_strain_test5b.ps"
set autoscale
set yrange [250:]
set key top right
set title "Stress Comparison (Plane Strain NAFEMS TEST 5B Test)"
set xlabel "Time (s)"
set ylabel "{/Symbol s}_{xx} (MPa)"
plot "test5b_out.dat" using 1:2 title "ABAQUS, CETOL=1e-5" with linespoints lt 3 lc 0 lw 4 pt 5,\
"nafems_test5b_outfile.txt" using 1:19 title "MOOSE, max creep strain incr=1e-5" with linespoints lt 1 lc 0 lw 2 pt 6

#"test5b_out.dat" using 1:3 title "{/Symbol s}_{yy} (ABAQUS)" with linespoints lt 3 lc 1 lw 4 pt 4,\
#"test5b_out.dat" using 1:4 title "{/Symbol s}_{zz} (ABAQUS)" with linespoints lt 3 lc 2 lw 4 pt 4,\
#"test5b_out.dat" using 1:6 title "{/Symbol s}_{mises} (ABAQUS)" with linespoints lt 3 lc 3 lw 4 pt 4,\
#"nafems_test5b_outfile.txt" using 1:20 title "{/Symbol s}_{yy} (MOOSE)" with linespoints lt 1 lc 1 lw 1 pt 6,\
#"nafems_test5b_outfile.txt" using 1:21 title "{/Symbol s}_{zz} (MOOSE)" with linespoints lt 1 lc 2 lw 1 pt 6,\
#"nafems_test5b_outfile.txt" using 1:22 title "{/Symbol s}_{mises} (MOOSE)" with linespoints lt 1 lc 3 lw 1 pt 6
