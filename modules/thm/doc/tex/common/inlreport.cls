\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{inlreport}[2020/07/29 LaTeX class for INL reports]

\LoadClass[letterpaper,11pt]{article}

\RequirePackage[margin=1in]{geometry}
\RequirePackage{authblk}
\RequirePackage{mathtools}
\RequirePackage{setspace}
\RequirePackage[labelfont=bf]{caption}
\RequirePackage[explicit]{titlesec}  %section title formatting
\RequirePackage{abstract}  %abstract formatting
\RequirePackage{pdfpages}  %insert pdf pages for front mater
\RequirePackage{epstopdf}
\RequirePackage{graphicx}
\RequirePackage{fixltx2e}
\RequirePackage{hyperref}
\RequirePackage{bm}
\RequirePackage{float}
% subfigure is deprecated, subfig is its replacement, subcaption is newer than subfig
% http://tex.stackexchange.com/questions/122314/figures-what-is-the-difference-between-using-subfig-or-subfigure
\RequirePackage{subcaption}
% Change lists of references from [1,2,3] to [1-3]
\RequirePackage[numbers,sort&compress]{natbib}
\RequirePackage{listings}
\RequirePackage{acro} % acronyms
\RequirePackage{algorithm,algpseudocode}
\RequirePackage{amsfonts}
\RequirePackage{amssymb}
\RequirePackage{amstext}
\RequirePackage{amsbsy}
\RequirePackage{arydshln}
\RequirePackage{xifthen}
\RequirePackage{graphicx}
\RequirePackage{amsmath}
\RequirePackage{amsfonts}
\RequirePackage{amssymb}
\RequirePackage{amstext}
\RequirePackage{amsbsy}
\RequirePackage{epsfig}
\RequirePackage{epsf}
\RequirePackage{multirow}
\RequirePackage{float}
\RequirePackage{stmaryrd}
\RequirePackage{xspace}
\RequirePackage{url}
\RequirePackage{verbatim}
\RequirePackage{ifthen}
\RequirePackage{rotating}
\RequirePackage{mathpazo}
\RequirePackage{bbding}
\RequirePackage{booktabs}
\RequirePackage[bbgreekl]{mathbbol}
\RequirePackage{amsfonts}
\DeclareSymbolFontAlphabet{\mathbbl}{bbold}
\RequirePackage[noprefix]{nomencl}  % for nomenclature
\RequirePackage{bbm, dsfont}
\RequirePackage{newtxtext}
\RequirePackage{indentfirst}


% Hyperlinks
\hypersetup{
    colorlinks=true,
    linkcolor=blue,
    filecolor=magenta,
    urlcolor=blue,
}
% ---------------------------------------------------------------------------- %
% Treatment of floats
\renewcommand{\topfraction}{0.85}       % max fraction of floats at top
\renewcommand{\textfraction}{0.1}       % allow minimal text w. figs
\renewcommand{\floatpagefraction}{0.7}  % require fuller float pages
\floatplacement{figure}{H}

% ---------------------------------------------------------------------------- %
% Capitalize some sections names
%\renewcommand{\summaryname}{SUMMARY}
\renewcommand{\abstractname}{ABSTRACT}
\renewcommand{\abstractnamefont}{\sffamily\Large\bfseries}
\renewcommand{\contentsname}{CONTENTS}
\renewcommand{\listfigurename}{FIGURES}
\renewcommand{\listtablename}{TABLES}

\titleformat{\section}{\fontsize{14}{20}\sffamily\bfseries\filcenter}{\thesection}{1em}{\MakeUppercase{#1}}
\titleformat{\subsection}{\fontsize{14}{20}\sffamily\bfseries\filcenter}{\thesubsection}{1em}{#1}
\titleformat{\subsubsection}{\fontsize{12}{17}\sffamily\bfseries\filcenter}{\thesubsubsection}{1em}{#1}

\makeatletter

% fixing authblk font for affiliation index
\renewcommand\AB@authnote[1]{\textsuperscript{#1}}
\renewcommand\AB@affilnote[1]{\textsuperscript{#1}}
\renewcommand\Authsep{\par}
\renewcommand\Authand{\par}
\renewcommand\Authands{\par}

% ---------------------------------------------------------------------------- %
% Subtitle of the document
\newcommand\subtitle[1]{\renewcommand\@subtitle{#1}}
\newcommand\@subtitle{}

% ---------------------------------------------------------------------------- %
% Department that produced the document
\newcommand\department[1]{\renewcommand\@department{#1}}
\newcommand\@department{}

% ---------------------------------------------------------------------------- %
% Report no. (get one from LRS)
\newcommand\reportno[1]{\renewcommand\@reportno{#1}}
\newcommand\@reportno{}

% ---------------------------------------------------------------------------- %
% Revision no.
\newcommand\revisionno[1]{\renewcommand\@revisionno{#1}}
\newcommand\@revisionno{}

% ---------------------------------------------------------------------------- %
% Background image
\newcommand\BackgroundPic[1]{%
\put(0,0){%
\parbox[b][\paperheight]{\paperwidth}{%
\vfill
\centering
\includegraphics[width=\paperwidth,height=\paperheight,keepaspectratio]{#1}%
\vfill
}}}

% ---------------------------------------------------------------------------- %
% Cover page
\definecolor{inlblue}{RGB}{31,82,153}
\definecolor{inllightblue}{RGB}{83,168,220}

\newcommand\@coverpage{%
\bgroup
\thispagestyle{empty}
\AddToShipoutPicture*{\BackgroundPic{pics/cover.pdf}}

\begingroup%
\newgeometry{margin=3.05in, top=1in, right=0.5in, bottom=1in}
\setlength{\parindent}{0pt}
\setlength{\parskip}{0pt}

\vbox to 0.5in{%
% report no.
\rightline{\fontfamily{phv}\fontsize{10}{12}\selectfont{\color{inlblue}\@reportno}}
% revision no.
\ifnum\@revisionno>0
\rightline{\fontfamily{phv}\fontsize{10}{12}\selectfont{\color{inlblue}Revision \@revisionno}}
\fi
\vfil
}%
\vbox to 2.68in{%
  % title
  \vfil
  \begin{raggedright}
  \color{inlblue}
  {\fontfamily{ptm}\fontsize{28}{34}\selectfont\bfseries{\selectfont\@title\par}}
  \end{raggedright}
  % subtitle
  \begin{raggedright}
  \color{inlblue}
  {\fontfamily{ptm}\fontsize{28}{34}\selectfont\bfseries{\@subtitle\par}}
  \end{raggedright}
}
\vspace{0.33in}
\vbox to 0.3in{%
  \vspace{6pt}
  % date
  {\color{white}\fontfamily{phv}\fontsize{10}{12}\selectfont\@date\par}
  \vfil
}
\vspace{0.83in}
\vbox{%
  % authors
  \bgroup%
  \color{inlblue}
  \renewcommand*\Authfont{\fontfamily{ptm}\fontsize{11}{14}\selectfont}
  \renewcommand*\Affilfont{\fontfamily{ptm}\fontsize{11}{14}\selectfont\color{inllightblue}}
  \@author\par
  \egroup

  \vfill
}

\clearpage
\restoregeometry
\endgroup

\egroup
\clearpage
}

% ---------------------------------------------------------------------------- %
% Disclaimer page
\newcommand\@disclaimerpage{%
\thispagestyle{empty}

\hbox{\space}

\vfill

\bgroup
\begin{center}
\setlength{\fboxrule}{0.8pt}
\setlength{\fboxsep}{0.125in}
\framebox{%
\begin{minipage}{4in}
\fontfamily{ptm}\fontsize{9}{10}\selectfont
\centerline{\textbf{DISCLAIMER}}
\parindent0.25in
\vspace{0.125in}

This information was prepared as an account of work sponsored by an agency of the U.S. Government.
Neither the U.S. Government nor any agency thereof, nor any of their employees, makes any warranty,
expressed or implied, or assumes any legal liability or responsibility for the accuracy,
completeness, or usefulness, of any information, apparatus, product, or process disclosed, or
represents that its use would not infringe privately owned rights. References herein to any
specific commercial product, process, or service by trade name, trade mark, manufacturer, or
otherwise, does not necessarily constitute or imply its endorsement, recommendation, or favoring
by the U.S. Government or any agency thereof. The views and opinions of authors expressed herein
do not necessarily state or reflect those of the U.S. Government or any agency thereof.
\end{minipage}}
\end{center}
\egroup
\clearpage
}


% ---------------------------------------------------------------------------- %
% Title page
\newcommand\@titlepage{%
\thispagestyle{empty}%
\begin{center}%
\vbox to 1.44in{%
  \rightline{\fontfamily{phv}\fontsize{11}{14}\selectfont\bfseries{\@reportno}}
  \ifnum\@revisionno>0
  \rightline{\fontfamily{phv}\fontsize{11}{14}\selectfont\bfseries{Revision \@revisionno}}
  \fi
}%
\vbox to 2.31in{%

  {\fontfamily{phv}\fontsize{18}{22}\selectfont\bfseries{\strut\@title\par}}

  {\fontfamily{phv}\fontsize{18}{22}\selectfont\bfseries{\@subtitle\par}}

  \vfil
}%
\vbox to 1.45in{%
  \bgroup%
  \renewcommand*\Authfont{\fontfamily{phv}\fontsize{12}{15}\bfseries\selectfont}
  \renewcommand*\Affilfont{\fontfamily{phv}\fontsize{12}{15}\bfseries\selectfont}
  \fontfamily{phv}\selectfont
  \@author\par
  \egroup
  \vfil
}%
\vbox to 0.86in{%
  {\fontfamily{phv}\fontsize{12}{15}\selectfont\bfseries{\@date\par}}
}%
\vbox to 1.44in{%
  {\fontfamily{phv}\fontsize{14}{17}\selectfont\bfseries{Idaho National Laboratory\par}}

  {\fontfamily{phv}\fontsize{14}{17}\bfseries{\@department\par}}

  {\fontfamily{phv}\fontsize{14}{17}\selectfont\bfseries{Idaho Falls, Idaho 83415\par}}

  \vfil

  {\fontfamily{phv}\fontsize{14}{17}\selectfont\bfseries{\href{http://www.inl.gov}{http://www.inl.gov}\par}}
}%
\vbox to 1.3in{%
  \vfil
  {\fontfamily{phv}\fontsize{11}{14}\selectfont\bfseries{%
  Prepared for the\break
  U.S. Department of Energy\break
  Office of Nuclear Energy\break
  Under DOE Idaho Operations Office\break
  Contract DE-AC07-05ID14517\par}}
}
\end{center}
\clearpage
}

\newcommand\frontmatter{\bgroup%
\parindent=0pt
\@coverpage%
\@disclaimerpage%
\@titlepage%
\egroup
}

\makeatother


% ---------------------------------------------------------------------------- %
% Summary
\newenvironment{summary}%
{\section*{SUMMARY}\addcontentsline{toc}{section}{Summary}}%
{\clearpage}

% ---------------------------------------------------------------------------- %
% Empty page
\newenvironment{emptypage}%
{\quad\par\vspace{1in}\centerline{\itshape{Page intentionally left blank}}\vfil}%
{\clearpage}

% ---------------------------------------------------------------------------- %
% Paragraph spacing
\setlength{\parindent}{0.25in}
\setlength{\parskip}{1ex}
