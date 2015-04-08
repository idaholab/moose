clear all
clf
hold on

% cells_per_pixel,  n. grains,  volume intersecting boundary,  intersecting vol/total vol,  /usr/bin/time -p
data = [
    0.10             317        7.4311731817216682e-03        8.6136177194421609e-03      2.15
    0.20             325        7.2247517044515732e-03        8.3743505605687104e-03      10.97
    0.25             326        7.1098420155934977e-03        8.2411564998357330e-03      18.73
    0.30             325        7.1264557628943296e-03        8.2604138182479549e-03      29.80
    0.40             325        7.0088073910520602e-03        8.1240452966722942e-03      58.84
    0.50             325        6.9576877147021548e-03        8.0647914831421825e-03      101.66
    0.60             325        6.9334132083482400e-03        8.0366544295508210e-03      157.66
    0.75             325        6.8595002576961666e-03        7.9509804873798967e-03      264.66
    1.00             325        6.8785441180426862e-03        7.9730545972029384e-03      510.03 % 'true' solution (highest resolution solution)
];

N=length(data);

% Compute relative error in scaled volume intersecting the boundary
rel_err = abs(data(1:N-1,4) - data(N,4)) / data(N,4);

% Compute normalized (vs. true solution) solution times
norm_times = data(1:N-1,5) / data(N,5);

% Plot vs. cells_per_pixel
x = data(1:N-1,1);
[haxis, h1, h2] = plotyy(x, 100*rel_err, x, norm_times);

% Label the axes
ylabel (haxis(1), '% relative error in normalized boundary grain volume');
ylabel (haxis(2), 'Normalized solution times');
xlabel ('Cells/pixel');

% Make thick lines - this looks better in PDF
set([h1, h2], 'linewidth', 6);

% Turn on markers
set(h1, 'marker', 'o');
set(h2, 'marker', 's');

% Set marker size
set([h1, h2], 'markersize', 6);

% Make dashed line in the PDF
set(h1, 'linestyle', '--');

% Print to PDF
set (gcf, "papersize", [11, 8.5]);
set (gcf, "paperorientation", 'landscape');

% I was using these paper settings in older versions of Octave, but
% they changed in 3.8.0
is_380 = strcmp(version(), '3.8.0');

if (!is_380)
  set (gcf, "paperposition", [0.25, 0.25, 10.75, 8.25]);
else
  % In Octave 3.8.0, the default paperposition is [0.25000, 2.50000, 8.00000, 6.00000],
  % the third number makes the plot taller instead of wider!
  set (gcf, "paperposition", [0.25, 0.25, 8.0, 10.5]);
end

% Make a PDF of this plot.
print('-dpdf', 'results.pdf', '-FHelvetica:20');
