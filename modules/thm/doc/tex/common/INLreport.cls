% Package definitions
%
% 02/05/2021 v1.29a Logan Harbour (logan.harbour@inl.gov)
%    - The package xwatermark has a dependency, catoptions, that is no longer supported
%      with the packages available on CTAN as of this date. For now... don't allow the DRAFT
%      option until someone fixes it. This is hopefully just temprorary, but it's worth
%      adding a note about.

    % The version and date of this file
    \newcommand\INLreportVersionOnly{ v1.28 }		% leave the spaces before and after version
    \newcommand\INLreportVersionDate{2007/12/13}	% Date has to bis this format!

    \typeout{Using INL Technical Report class: \INLreportVersionDate, \INLreportVersionOnly}
    \newcommand\INLreportVersion{\INLreportVersionDate \INLreportVersionOnly}

    % We need LaTeX 2e or better
    \NeedsTeXFormat{LaTeX2e}

    % We need ifthenelse. \newboolean are false by default
    \RequirePackage{ifthen}
    \RequirePackage{color}
    \RequirePackage{graphicx}
    % Not compatible with current latex, see notes for v1.29a
    % \RequirePackage[printwatermark]{xwatermark}

    % Variables
    \newboolean{relaxedINL}
    \newboolean{strictINL}
    \newboolean{reportINL}
    \newboolean{INLmainProvided}
    \newboolean{INLnumProvided}
    \newboolean{INLprintDateProvided}
    \newboolean{INLrePrintDateProvided}
    \newboolean{INLauthorProvided}
    \newboolean{INLsupersedProvided}
    \newboolean{INLdistributionProvided}
    \newboolean{INLreleaseTypeProvided}
    \newboolean{inAppendix}
    \newboolean{INLmarkingsRequired}
    \newboolean{MarkCover}
    \newboolean{MarkBack}
    \newboolean{MarkTitle}
    \newboolean{MarkBody}
    \newboolean{MarkBottom}
    \newboolean{distCA}
    \newboolean{distNM}
    \newboolean{INLdraft}


    % Added 1/11/07 M. Cooper to remove additional text unnecessary for OUO documents on pg 2.
    \newboolean{INLnoticeOUO}

    % Counters
    \newcounter{INLprintingCnt}
    \setcounter{INLprintingCnt}{1}
    \newcounter{INLreDistCnt}
    \setcounter{INLreDistCnt}{1}


    % We need calc to divide by 3 in setlength and other calculations
    \RequirePackage{calc}

    % We need the fancy headers for the report markings
    \RequirePackage{fancyhdr}


    % Specify what this file provides
    \ProvidesPackage{INLreport}[\INLreportVersion]
    \ProvidesFile{INLreport.cls}[\INLreportVersion]


    % We understand three options: "relax", "strict", and "report"
    % Using "relax" make the document look more LaTeXy
    % If no option are given, then we follow the format of INL2002-2068P
    % Using "strict" tries to follow the guideline even more closely
    % Specifying "report" uses the report class instead of the article class
    %
    \DeclareOption{relax}{
	\typeout{Option "relax": Relaxing some of the INL98-0730 requirements}
	\setboolean{relaxedINL}{true}
    }
    \DeclareOption{strict}{
	\typeout{Option "strict": Follow the INL98-0730 requirements more strictly}
	\setboolean{strictINL}{true}
    }
    \ifthenelse{\boolean{relaxedINL} \and \boolean{strictINL}}   {
	\ClassWarning{INLreport.cls}{Both, "strict" and "relax" options given}
    }{
    }
    \DeclareOption{report}{
	\typeout{Option "report": Using the report class as a base.}
	\setboolean{reportINL}{true}
    }
    \DeclareOption{CA}{
	\ClassError{INLreport.cls}{"CA" option is obsolete}{Use
	\@backslashchar begin{INLdistribution}[CA] instead.}
    }
    \DeclareOption{NM}{
	\ClassError{INLreport.cls}{"NM" option is obsolete}{Use
	\@backslashchar begin{INLdistribution}[NM] instead.}
    }


    % Added 1/11/07 M. Cooper to remove additional text unnecessary for OUO documents on pg 2.
    \DeclareOption{OUO}{
	\typeout{Option "OUO": Formatting page 2 for OUO reports}
	\setboolean{INLnoticeOUO}{true}
    }

    % We're based on the standard article or report class and assume certain
    % options. Set those, and pass all others to the article class
    \ProcessOptions % We need to do it here so we get "reportINL", and then again
    \ifthenelse{\boolean{reportINL}}   {
	\DeclareOption*{\PassOptionsToClass{\CurrentOption}{report}}
	\ProcessOptions % Repeat it here, otherwise LaTeX gets confused
	\LoadClass[letterpaper,oneside,final,fleqn]{report}
    }{
	\DeclareOption*{\PassOptionsToClass{\CurrentOption}{article}}
	\ProcessOptions % Repeat it here, otherwise LaTeX gets confused
	\LoadClass[letterpaper,oneside,final,fleqn]{article}
    }


    %
    % Setup for letter paper (This IS a INL report!)
    %
    \RequirePackage{geometry}
    % \geometry{pdftex, showframe, inner=1.5in, textwidth=6in, textheight=9in}
    % Changed textwidth to 6.5in and inner to 1in to more closely match official INL cover templates - MJK, 1/12/07
    %\geometry{pdftex, inner=1in, textwidth=6.5in, textheight=9in}



% ******************************************************************************
% Check some of the user directives and setup a few things
%

    \AtBeginDocument{

	% Do some usage checking
	\ifthenelse{\boolean{INLnumProvided}}   {
	}{
	    \ClassError{INLreport.cls}{\@backslashchar INLnum
		not provided}{Insert \@backslashchar INLnum in the preamble
		of your document}
	}
	\ifthenelse{\boolean{INLauthorProvided}}   {
	}{
	    \ClassError{INLreport.cls}{\@backslashchar INLauthor
		not provided}{Insert \@backslashchar INLauthor in the preamble
		of your document}
	}
	\ifthenelse{\boolean{INLprintDateProvided}}   {
	}{
	    \ClassError{INLreport.cls}{\@backslashchar INLprintDate
		not provided}{Insert \@backslashchar INLprintDate in the
		preamble of your document}
	}
	\ifthenelse{\boolean{INLreleaseTypeProvided}}   {
	}{
	    \ClassError{INLreport.cls}
        {\@backslashchar INLlimitedRelease not provided}
        {Insert \@backslashchar INLlimitedRelease in the preamble of your document}
	}

  \ifthenelse{\boolean{INLdraft}}{%
    \PackageError{xwatermark}{Draft option not currently supported due to new latex incompatibility}{See notes for v1.29a in INLReport.cls for more info}
    \g@addto@macro{\@title}{\space (DRAFT)}%
    \newwatermark[allpages,color=gray!33,angle=53,scale=4,xpos=-20,ypos=0]{DRAFT}%
  }{}

	% Save the title, it will be cleared by \maketitle, but we need it later
	\newsavebox{\INLtitletempbox}
	\sbox{\INLtitletempbox}{\parbox{\textwidth - 1.5in}{
	    \centering \LARGE \bf \@title \par}
	}

	%
	% Some measurements to format the title page properly
	%
	\newlength{\textwidththird}
	\newlength{\topheight}
	\newlength{\topleftwidth}
	\newlength{\topmiddlewidth}
	\newlength{\toprightwidth}

	\setlength{\textwidththird}{(\textwidth / 3) - (\fboxsep * 2)}
	\setlength{\topleftwidth}{\the\textwidththird}
	\setlength{\topmiddlewidth}{\the\textwidththird}
	\setlength{\toprightwidth}{\the\textwidththird}

	% Changed following to -6, +10, and -6 from -3, +4, -3, to get "Official Use Only - Export Controlled Information
	% to fit on one line on title page - MJK, 1/12/07
	\addtolength{\topleftwidth}{-6em}
	\addtolength{\topmiddlewidth}{+10em}
	\addtolength{\toprightwidth}{-6em}


	\ifthenelse{\boolean{INLsupersedProvided}}   {
	    \setlength{\topheight}{1.5in}
	}{
	    \setlength{\topheight}{1.0in}
	}

	%
	% We want a blank line between paragraphs
	%
	\setlength\parskip{2ex}


	% We don't want a head rule
	\renewcommand{\headrulewidth}{0pt}
    } % End of AtBeginDocument{}



% ******************************************************************************
% Cover pages
%

    % ------------------------------------------------------------------------ %
    % Font selections for the cover page \Coverfont{series}{size}{skip}
    %
    \newcommand{\CoverFont}[3]{
	\fontencoding{OT1}
	\fontfamily{phv}
	\fontseries{#1}
	\fontshape{n}
	\fontsize{#2}{#3}
	\selectfont
    }

    \newcommand{\doCover}{
	%
	% Print the cover pages
	%
	\INLmarks{cover}
	\newlength{\coverindent}
	\setlength{\coverindent}{1em}
	\newlength{\parindentsave}
	\setlength{\parindentsave}{\parindent}
	\setlength{\parindent}{0em}


	%
	% Front Cover
	%
	%\hspace*{-0.375in}
	\begin{minipage}[t]{6.0in}
	    \CoverFont{b}{18}{20pt}
	    INL REPORT
	    \vspace{0.125in}\\
	    \CoverFont{m}{12}{14pt}
	    \INLnumVar\\
	    \INLreleaseTypeVar\\
	    \ifthenelse{\boolean{INLrePrintDateProvided}}   {
		Printed \INLrePrintDateVar
	    }{
		Printed \INLprintDateVar
	    }
	    \ifthenelse{\boolean{INLsupersedProvided}}   {
		\vspace{0.125in}\\
		\INLsupersedVar
	    }{
		\\
	    }

	    \vspace{0.25in}
	    \CoverFont{b}{24}{28pt}
	    \parbox[t]{\textwidth}{
		\raggedright
		\@title
	    }


            % Changed this space from 0.75in to 0.375in to match official INL covers, and provide more space to prevent
	    % overfilling the page - MJK, 1/12/07
	    \vspace{0.375in}
	    \CoverFont{m}{12}{14pt}
	    \bgroup
      \providecommand{\Authfont}{}
      \providecommand{\Affilfont}{}
	    \renewcommand\Authfont{\fontfamily{phv}\selectfont}
	    \renewcommand\Affilfont{\fontfamily{phv}\selectfont}
	    \@author
	    \egroup

	    \vspace{0.25in}
	    \CoverFont{m}{8}{10pt}
	    \selectfont
	    Prepared by\\
	    Idaho National Laboratory\\
	    Idaho Falls, Idaho 83415\\
	    \\
	    The Idaho National Laboratory is a multiprogram laboratory operated by\\
	    Battelle Energy Alliance for the United States Department of Energy\\
	    under DOE Idaho Operations Office. Contract DE-AC07-05ID14517.\\
	    \\
	    \MarkCoverVar
	\end{minipage}

	\vspace*{\fill} % Move the logo to the bottom of the cover page

	%\hspace*{-0.5in}
	\begin{minipage}[b]{6.0in}
	    \includegraphics{inl-left_black}
	\end{minipage}



	%
	% Back cover
	%
	\newpage
	% Added centering and reduced minipage width to match official INL covers better - MJK, 1/12/07
	\begin{center}
	\begin{minipage}[t]{5.5in}
	    \fontencoding{OT1}
	    \fontfamily{pnc}
	    \fontseries{m}
	    \fontshape{n}
	    \fontsize{9}{11pt}
	    \selectfont
            Issued by the Idaho National Laboratory, operated for the
	    United States Department of Energy by Battelle Energy Alliance.
	    \vspace{\baselineskip}

            % Removed \fontsize{11}{13} to make NOTICE simply bold, to match official INL covers better, MJK - 1/12/07
	    {\fontseries{b}\selectfont NOTICE:}
	    This report was prepared
	    as an account of work sponsored by an agency of
	    the United States Government. Neither the United
	    States Government, nor any agency thereof, nor any
	    of their employees, nor any of their contractors,
	    subcontractors, or their employees, make any warranty,
	    express or implied, or assume any legal liability or
	    responsibility for the accuracy, completeness, or
	    usefulness of any information, apparatus, product,
	    or process disclosed, or represent that its use
	    would not infringe privately owned rights. Reference
	    herein to any specific commercial product, process,
	    or service by trade name, trademark, manufacturer, or
	    otherwise, does not necessarily constitute or imply
	    its endorsement, recommendation, or favoring by the
	    United States Government, any agency thereof, or any
	    of their contractors or subcontractors. The views and
	    opinions expressed herein do not necessarily state
	    or reflect those of the United States Government,
	    any agency thereof, or any of their contractors.
	    \vspace{\baselineskip}

	    % Added 1/11/07 M. Cooper to remove additional text unnecessary for OUO documents on pg 2.
	    \ifthenelse{\boolean{INLnoticeOUO}}{}{
	    Printed in the United States of America. This report has
	    been reproduced directly from the best available copy.
	    \vspace{\baselineskip}

	    }% end of text for non-OUO reports
	    \vspace{\baselineskip}
	    \vspace{0.125in}

	    \includegraphics[width=0.75in]{DOEbwlogo}
	\end{minipage}
	\end{center}

	% Restore the original paragraph indent
	\setlength{\parindent}{\parindentsave}

	\INLmarks{back}
    }


% ******************************************************************************
% The user commands
%

    % ------------------------------------------------------------------------ %
    % Here we define the mandatory declarations
    %
    \newcommand{\INLnum}[1]{
	\newcommand{\INLnumVar}{#1}
	\setboolean{INLnumProvided}{true}
    }

    \newcommand{\INLauthor}[1]{
	\newcommand{\INLauthorVar}{#1}
	\setboolean{INLauthorProvided}{true}
    }

    \newcommand{\INLprintDate}[1]{
	\newcommand{\INLprintDateVar}{#1}
	\setboolean{INLprintDateProvided}{true}
    }


    % This one is optional
    \newsavebox{\INLrePrintDistBox}
    \newsavebox{\INLrePrintDistBoxSecond}
    \newsavebox{\INLrePrintDistBoxThird}
    \newsavebox{\INLrePrintDistBoxFourth}
    \newsavebox{\INLrePrintDistBoxFifth}
    \newcommand{\INLrePrintDateVar}{}

    \newcommand{\INLrePrintDate}[1]{
	\renewcommand{\INLrePrintDateVar}{#1}
	\setboolean{INLrePrintDateProvided}{true}
	\stepcounter{INLprintingCnt}
	\ifthenelse{\value{INLprintingCnt} = 2}   {
	    \sbox{\INLrePrintDistBoxSecond}{Second Printing, (#1):}
	}{
	    \ifthenelse{\value{INLprintingCnt} = 3}   {
		\sbox{\INLrePrintDistBoxThird}{Third Printing, (#1):}
	    }{
		\ifthenelse{\value{INLprintingCnt} = 4}   {
		    \sbox{\INLrePrintDistBoxFourth}{Fourth Printing, (#1):}
		}{
		    \ifthenelse{\value{INLprintingCnt} = 5}   {
			\sbox{\INLrePrintDistBoxFifth}{Fifth Printing, (#1):}
		    }{
			\ClassError{INLreport.cls}{\@backslashchar INLreDistribution
			    used more than four times!}{Do you really have more than
			    five printings? If so, you need to amend INLreport.cls}
		    }
		}
	    }
	}
    }


    % Used to need the title at the top of the first section/chapter, but that is
    % not required anymore.  Make sure we start on an odd page.
    \newcommand{\INLmain}{
	\cleardoublepage
	\setboolean{INLmainProvided}{true}
    }


    \ifthenelse{\boolean{reportINL}}   {
	%
	% new 1
	% Once we have left the preamble and are in the body of the text,
	% we want each chapter to begin on an odd numbered page
	%
	% new 2
	% After we're done with the title page, we switch to markings for body pages.
	%
	\renewcommand\chapter{%						% \chapter from report.cls
	    %\if@openright\cleardoublepage\else\clearpage\fi		% deleted
	    \ifINLmainProvided\cleardoublepage\else\clearpage\fi	% new 1
	    \INLmarks{body}						% new 2
	    % \thispagestyle{plain}%					% deleted
	    \global\@topnum\z@						% \chapter from report.cls
	    \@afterindentfalse						% \chapter from report.cls
	    \secdef\@chapter\@schapter					% \chapter from report.cls
	}								% \chapter from report.cls

	%
	% We redefine \@makechapterhead and \@schapter so we can move
	% the chapter title a little bit higher up.
	%

	%
	% This is for chapters in the body
	\def\@makechapterhead#1{%					% \@makechapterhead from report.cls
	    \vspace*{-10\p@}%						% Changed
	    {\parindent \z@ \raggedright \normalfont			% \@makechapterhead from report.cls
	    \ifnum \c@secnumdepth >\m@ne				% \@makechapterhead from report.cls
		\huge\bfseries \@chapapp\space \thechapter		% \@makechapterhead from report.cls
		\par\nobreak						% \@makechapterhead from report.cls
		\vskip 20\p@						% \@makechapterhead from report.cls
	    \fi								% \@makechapterhead from report.cls
	    \interlinepenalty\@M					% \@makechapterhead from report.cls
	    \Huge \bfseries #1\par\nobreak				% \@makechapterhead from report.cls
	    \vskip 40\p@						% \@makechapterhead from report.cls
	}}								% \@makechapterhead from report.cls

	%
	% This is for chapters in the preamble
	\def\@schapter#1{\if@twocolumn					% \@schapter from report.cls
		\@topnewpage[\@makeschapterhead{#1}]%			% \@schapter from report.cls
	    \else							% \@schapter from report.cls
		\@makeschapterhead{#1}%					% \@schapter from report.cls
		\@afterheading						% \@schapter from report.cls
	    \fi}							% \@schapter from report.cls
	    \def\@makeschapterhead#1{%					% \@schapter from report.cls
	    \vspace*{-10\p@}%						% Changed
	    {\parindent \z@ \raggedright				% \@schapter from report.cls
	    \normalfont							% \@schapter from report.cls
	    \interlinepenalty\@M					% \@schapter from report.cls
	    \Huge \bfseries  #1\par\nobreak				% \@schapter from report.cls
	    \vskip 40\p@						% \@schapter from report.cls
	}}

    }{
	% new 3
	% We want each section starting on a new page The report class already does this
	%
	% new 4
	% Center the section headings
	%
	\renewcommand\section{\clearpage%				% new 3
	    \@startsection {section}{1}{\z@}%				% \section from article.cls
	    {-3.5ex \@plus -1ex \@minus -.2ex}%				% \section from article.cls
	    {2.3ex \@plus.2ex}%						% \section from article.cls
	    %{\normalfont\Large\bfseries}%				% deleted
	    {\normalfont\Large\bfseries}%			% new 4
	}								% \section from article.cls
    }



    % ------------------------------------------------------------------------ %
    % Distribution page at the end
    % We have to create our own description environment, so we don't
    % disturb the original one.
    \newlength{\INLdistwidth}			% Max width of distribution lines
    \setlength{\INLdistwidth}{0.8\textwidth}
    \newlength{\INLMSdistwidth}		% Width of field for MS number
    \setlength{\INLMSdistwidth}{9ex}

    \newcommand{\INLdescriptionlabel}[1]{
	    \makebox[3em][r]{#1}
    }
    \newenvironment{INLdescription}{
	\list{}
	{\labelwidth\z@ \itemindent-\leftmargin \let\makelabel\INLdescriptionlabel}
    }{
	\endlist
    }

    \newenvironment{INLdistribution}[1][nothing]{
	\setboolean{INLdistributionProvided}{true}

	\newboolean{distCRADA}
	\newboolean{distPatent}
	\newboolean{distLDRD}
	\newboolean{distClassified}
	\newcommand{\INLdistCRADA}{
	    \setboolean{distCRADA}{true}
	}

	\newcommand{\INLdistPatent}{
	    \setboolean{distPatent}{true}
	}

	\newcommand{\INLdistLDRD}{
	    \setboolean{distLDRD}{true}
	}

	\newcommand{\INLdistClassified}{
	    \setboolean{distClassified}{true}
	    \INLdistInternalM{2}{M2497}{Central Technical Files}{8944}
	}



	% Option NM given
	\ifthenelse{\equal{#1}{NM}}   {
	    \typeout{Option "NM": Using the SNL NM default distribution list.}
	    \setboolean{distNM}{true}
	}{
	}

	% Option CA given
	\ifthenelse{\equal{#1}{CA}}   {
	    \typeout{Option "CA": Using the SNL CA default distribution list.}
	    \setboolean{distCA}{true}
	}{
	}

	\clearpage
	{\Large DISTRIBUTION:}
	\normalsize
	\raggedbottom
	\begin{INLdescription}
    }{
	\ifthenelse{\boolean{distCA}}   {
	    % Insert the standard CA distribution
	    \ifthenelse{\boolean{distClassified}}   {
		\INLdistInternal{2}{9018}{Central Technical Files}{8944 (1 electronic and 1 hardcopy)}
	    }{}
	    \INLdistInternal{2}{0899}{Technical Library}{8944 (1 electronic and 1 hardcopy)}

	    % If this report has a Patent Caution or Patent Interest
	    \ifthenelse{\boolean{distPatent}}   {
		\INLdistInternal{1}{0161}{Legal Intellectual Property}{11500}
	    }{}

	    % If this report is for CRADA work
	    \ifthenelse{\boolean{distCRADA}}   {
		\INLdistInternal{1}{0115}{OFA/NFE Agreements}{10112}
	    }{}

	    % If this report is for LDRD work
	    \ifthenelse{\boolean{distLDRD}}   {
		\INLdistInternal{1}{0123}{D. Chavez, LDRD Office}{1011}
	    }{}

	}{}

	\ifthenelse{\boolean{distNM}}   {
	    % Insert the standard NM distribution
	    \ifthenelse{\boolean{distClassified}}   {
		\INLdistInternalM{2}{2497}{Central Technical Files}{8944 (1 electronic and 1 hardcopy)}
	    }{}

	   % \INLdistInternal{2}{0899}{Technical Library}{9536 (1 electronic and 1 hardcopy)}

	    % If this report has a Patent Caution or Patent Interest
	    \ifthenelse{\boolean{distPatent}}   {
		\INLdistInternal{1}{0161}{Legal Intellectual Property}{11500}
	    }{}

	    % If this report is for CRADA work
	    \ifthenelse{\boolean{distCRADA}}   {
		\INLdistInternal{1}{0115}{OFA/NFE Agreements}{10112}
	    }{}

	    % If this report is for LDRD work
	    \ifthenelse{\boolean{distLDRD}}   {
		\INLdistInternal{1}{0123}{D. Chavez, LDRD Office}{1011}
	    }{}

	}{}

	\end{INLdescription}
    }



    %
    % The distribution list for a reprint
    % No housekeeping copies are needed.
    \newenvironment{INLreDistribution}{
	\ifthenelse{\boolean{INLrePrintDateProvided}}   {
	    \setboolean{INLdistributionProvided}{true}

	    \stepcounter{INLreDistCnt}
	    \ifthenelse{\value{INLreDistCnt} = 2}   {
		\sbox{\INLrePrintDistBox}{\usebox{\INLrePrintDistBoxSecond}}
	    }{
		\ifthenelse{\value{INLreDistCnt} = 3}   {
		    \sbox{\INLrePrintDistBox}{\usebox{\INLrePrintDistBoxThird}}
		}{
		    \ifthenelse{\value{INLreDistCnt} = 4}   {
			\sbox{\INLrePrintDistBox}{\usebox{\INLrePrintDistBoxFourth}}
		    }{
			\ifthenelse{\value{INLreDistCnt} = 5}   {
			    \sbox{\INLrePrintDistBox}{\usebox{\INLrePrintDistBoxFifth}}
			}{
			    \ClassError{INLreport.cls}{\@backslashchar INLreDistribution
				used more than four times!}{Do you really have more than
				five printings? If so, you need to amend INLreport.cls}
			}
		    }
		}
	    }

	    \usebox{\INLrePrintDistBox}
	    \normalsize
	    \raggedbottom
	    \begin{INLdescription}
	}{
	}
    }{
	\ifthenelse{\boolean{INLrePrintDateProvided}}   {
	    \end{INLdescription}
	}{
	}
    }


    \newcommand{\INLdistInternal}[4]{
	\ifthenelse{\boolean{INLdistributionProvided}}   {
	}{
	    \ClassError{INLreport.cls}{\@backslashchar INLdistribution
		not (yet) provided}{\@backslashchar INLdistribution must
		be used before any \@backslashchar INLdistInternal}
	}
	\item[#1] \parbox[t]{\INLdistwidth}{\makebox[\INLMSdistwidth][l]{MS \hfill #2}%
		\hspace{2em} \raggedright #3, #4}
	\vspace{-1.5ex}
    }


    % Use a mail channel
    \newcommand{\INLdistInternalM}[4]{
	\ifthenelse{\boolean{INLdistributionProvided}}   {
	}{
	    \ClassError{INLreport.cls}{\@backslashchar INLdistribution
		not (yet) provided}{\@backslashchar INLdistribution must
		be used before any \@backslashchar INLdistInternal}
	}
	\item[#1] \parbox[t]{\INLdistwidth}{\makebox[\INLMSdistwidth][l]{#2}\hspace{2em} \raggedright #3, #4}
	\vspace{-1.5ex}
    }


    \newcommand{\INLdistExternal}[2]{
	\ifthenelse{\boolean{INLdistributionProvided}}   {
	}{
	    \ClassError{INLreport.cls}{\@backslashchar INLdistribution
		not (yet) provided}{\@backslashchar INLdistribution must
		be used before any \@backslashchar INLdistExternal}
	}
	\item[#1] \parbox[t]{\INLdistwidth}{#2}
	\vspace{-1.5ex}
    }



    % ------------------------------------------------------------------------ %
    % And now some optional declarations
    %
    \newcommand{\INLsupersed}[2]{
	\newsavebox{\INLsupersedtempbox}
	\newcommand{\INLsupersedVar}{
	    Supersedes #1 \\
	    Dated #2
	}
	\sbox{\INLsupersedtempbox}{
	    \begin{tabular}{c}
		Supersedes #1 \\
		dated #2
	    \end{tabular}
	}
	\setboolean{INLsupersedProvided}{true}
    }


    % Distribution category is no longer needed
    \newcommand{\INLdistcategory}[1]{
	\ClassWarning{INLreport.cls}{Distribution category is no longer needed!}
    }


% ******************************************************************************
% The commands to specify the report markings.
%

    % Printed on the title page
    \newsavebox{\MarkTitletempbox}
    \sbox{\MarkTitletempbox}{} % default

    % ----------------------------

    % Printed on top of cover page
    \newcommand{\MarkTopCoverVar}{} % default

    % Printed on bottom of cover page
    \newcommand{\MarkBotCoverVar}{} % default

    % Printed on top of back cover page
    \newcommand{\MarkTopBackVar}{} % default

    % Printed on bottom of back cover page
    \newcommand{\MarkBotBackVar}{} % default

    % Printed on top of title page
    \newcommand{\MarkTopTitleVar}{} % default

    % Printed on bottom of title page
    \newcommand{\MarkBotTitleVar}{} % default

    % Printed on bottom of all body pages
    \newcommand{\MarkBotBodyVar}{} % default

    % Printed on top of all body pages
    \newcommand{\MarkTopBodyVar}{} % default


    %
    % The user marking commands
    %
    \newcommand{\INLlimitedRelease}[1]{
      \ifthenelse{\boolean{#1}}
        {
          \newcommand{\INLreleaseTypeVar}{Limited Release}
          \newcommand{\MarkCoverVar}{Approved for limited release; further dissemination limited to distribution list.}
        }
        {
          \newcommand{\INLreleaseTypeVar}{Unlimited Release}
          \newcommand{\MarkCoverVar}{Approved for public release; further dissemination unlimited.}
        }
      \setboolean{INLreleaseTypeProvided}{true}
    }

    \newcommand{\INLmarkBottomCover}[1]{
	\renewcommand{\MarkBotCoverVar}{#1}
	\setboolean{MarkCover}{true}
	\setboolean{INLmarkingsRequired}{true}
    }

    \newcommand{\INLmarkTopBottom}[1]{
	\renewcommand{\MarkBotBodyVar}{#1}
	\renewcommand{\MarkTopBodyVar}{#1}
	\renewcommand{\MarkBotBackVar}{#1}
	\renewcommand{\MarkTopBackVar}{#1}
	\renewcommand{\MarkBotTitleVar}{#1}
	\renewcommand{\MarkTopTitleVar}{#1}
	\renewcommand{\MarkBotCoverVar}{#1}
	\renewcommand{\MarkTopCoverVar}{#1}
	\setboolean{MarkTitle}{true}
	\setboolean{MarkBack}{true}
	\setboolean{MarkBody}{true}
	\setboolean{MarkCover}{true}
	\setboolean{INLmarkingsRequired}{true}
	\setboolean{MarkBottom}{true}
    }

    \newcommand{\INLmarkTop}[1]{
	\renewcommand{\MarkTopBodyVar}{#1}
	\renewcommand{\MarkTopBackVar}{#1}
	\renewcommand{\MarkTopTitleVar}{#1}
	\renewcommand{\MarkTopCoverVar}{#1}
	\setboolean{MarkTitle}{true}
	\setboolean{MarkBack}{true}
	\setboolean{MarkBody}{true}
	\setboolean{MarkCover}{true}
	\setboolean{INLmarkingsRequired}{true}
    }

    \newcommand{\INLmarkBottom}[1]{
	\renewcommand{\MarkBotBodyVar}{#1}
	\renewcommand{\MarkBotBackVar}{#1}
	\renewcommand{\MarkBotTitleVar}{#1}
	\renewcommand{\MarkBotCoverVar}{#1}
	\setboolean{MarkTitle}{true}
	\setboolean{MarkBack}{true}
	\setboolean{MarkBody}{true}
	\setboolean{MarkCover}{true}
	\setboolean{INLmarkingsRequired}{true}
	\setboolean{MarkBottom}{true}
    }

    \newcommand{\INLmarkTopBottomCoverBackTitle}[1]{
	\renewcommand{\MarkTopCoverVar}{#1}
	\renewcommand{\MarkBotCoverVar}{#1}
	\renewcommand{\MarkTopBackVar}{#1}
	\renewcommand{\MarkBotBackVar}{#1}
	\renewcommand{\MarkTopTitleVar}{#1}
	\renewcommand{\MarkBotTitleVar}{#1}
	\setboolean{MarkTitle}{true}
	\setboolean{MarkBack}{true}
	\setboolean{MarkCover}{true}
	\setboolean{INLmarkingsRequired}{true}
	\setboolean{MarkBottom}{true}
    }

    \newcommand{\INLmarkTopBottomCoverTitle}[1]{
	\renewcommand{\MarkBotTitleVar}{#1}
	\renewcommand{\MarkTopTitleVar}{#1}
	\renewcommand{\MarkBotCoverVar}{#1}
	\renewcommand{\MarkTopCoverVar}{#1}
	\setboolean{MarkTitle}{true}
	\setboolean{MarkCover}{true}
	\setboolean{INLmarkingsRequired}{true}
	\setboolean{MarkBottom}{true}
    }


    %
    % Internal command used to activate markings depending on what part of
    % the document we are currently processing; e.g. \INLmarks{body}
    %
    \newcommand{\INLmarks}[1]{
	\ifthenelse{\boolean{INLmarkingsRequired}}   {
	    \fancyfoot{} % Clear all foot markings
	    \fancyhead{} % Clear all head markings

	    % Fancyhdr needs a little room
	    \setlength{\headheight}{\baselineskip}

	    \pagestyle{fancy}
	    \thispagestyle{fancy}

	    \ifthenelse{\equal{#1}{cover}}   {
		% We're on the cover page
		\ifthenelse{\boolean{MarkCover}}   {
		    \chead{\MarkTopCoverVar}
		    \cfoot{\MarkBotCoverVar}
		}{
		    \chead{}
		    \cfoot{}
		}
	    }{
	    }

	    \ifthenelse{\equal{#1}{back}}   {
		% We're on the back cover page
		\ifthenelse{\boolean{MarkBack}}   {
		    \chead{\MarkTopBackVar}
		    \cfoot{\MarkBotBackVar}
		}{
		    \chead{}
		    \cfoot{}
		}

		% We do want a page number on the back cover
		\ifthenelse{\boolean{MarkBottom}}   {
		    \fancyfoot[LE,RO]{\thepage}
		}{% else
		    \fancyfoot[CE,CO]{\thepage}
		}
	    }{
	    }

	    \ifthenelse{\equal{#1}{title}}   {
		% We're on the title page
		\ifthenelse{\boolean{MarkTitle}}   {
		    \chead{\MarkTopTitleVar}
		    \cfoot{\MarkBotTitleVar}
		}{% else
		    \chead{}
		    \cfoot{}
		}

		\ifthenelse{\boolean{MarkBottom}}   {
		    \fancyfoot[LE,RO]{\thepage}
		}{% else
		    \fancyfoot[CE,CO]{\thepage}
		}
	    }{
	    }

	    \ifthenelse{\equal{#1}{body}}   {
		\ifthenelse{\boolean{MarkBody}}   {
		    \chead{\MarkTopBodyVar}
		    \cfoot{\MarkBotBodyVar}
		}{% else
		    \chead{}
		    \cfoot{}
		}

		% We're on a body page. If we're not going to mark the bottom,
		% or body pages at all, then we might as well put the page number
		% at the center
		\ifthenelse{\boolean{MarkBottom} \and \boolean{MarkBody}}   {
		    \fancyfoot[LE,RO]{\thepage}
		}{% else
		    \fancyfoot[CE,CO]{\thepage}
		}
	    }{
	    }
	}{
	    %
	    % If not INLmarkingsRequired
	    %
	    \pagestyle{plain}% The default when we have no markings

	    \ifthenelse{\equal{#1}{cover}}   {
		% We're on the cover page

		% No page number on cover
		\thispagestyle{empty}
	    }{
	    }

	    \ifthenelse{\equal{#1}{back}}   {
		% We're on the back cover page

		% We do want a page number on the back cover
		\thispagestyle{plain}
	    }{
	    }

	}
    }  % end of \INLmarks{}


    \newcommand{\INLmarkCoverTitle}[2]{
	\ClassError{INLreport.cls}{\@backslashchar INLmarkCoverTitle doesn't work yet; sorry.}{Don't
	know where to put the text on the title page.}
	\sbox{\MarkTitletempbox}{{\bf #1\\}{#2}}
	\renewcommand{\MarkCoverVar}{{{\CoverFont{b}{12}{12pt}#1}\\ {\CoverFont{m}{8}{10pt}}{#2}}}
    }

    \newcommand{\INLmarkCover}[1]{
	\renewcommand{\MarkCoverVar}{{\CoverFont{m}{8}{10pt}}{#1}}
    }


% ******************************************************************************
% Table of Contents
% The names we use to label the table of contents, and lists of figures,
% and tables.
%

    \ifthenelse{\boolean{reportINL}}   {
	\renewcommand\bibname{References}
	\setcounter{tocdepth}{3}
    }{
	\renewcommand{\contentsname}{\raggedright \normalfont\Large\bfseries Contents}
	\renewcommand{\listfigurename}{\raggedright \bf Figures}
	\renewcommand\listtablename{\raggedright \bf Tables}
	\renewcommand\refname{\raggedright \bf \centering References}
	\setcounter{tocdepth}{2}
    }

    %
    % Don't enter figures, tables, and subsections of an appendix in
    % the main table of contents, if "strict" is used.
    % Code from James Gruetzner and Todd Pitts. Avoids the ifthen package in
    % \addcontentsline which interferes with the subfigure package.
    \renewcommand{\addcontentsline}[3]{
	\ifinAppendix%
	    % We are in the Appendix
	    \ifthenelse{\equal{#1}{toc}}   {
		% And we are attempting to add something to the TOC
		\ifreportINL
		    \ifthenelse{\equal{#2}{chapter}}   {
			% It is a chapter, so we're OK
			\addtocontents{#1}{\protect\contentsline{#2}{#3}{\thepage}}
		    }{% else
			% It's a section; we'll accept that too
			\ifthenelse{\equal{#2}{section}}   {
			    % It is a section, so we're OK
			    \addtocontents{#1}{\protect\contentsline{#2}{#3}{\thepage}}
			}{% else
			    % It is not a section; skip it
			    \relax
			}
		    }
		\else
		    \ifthenelse{\equal{#2}{section}}   {
			% It is a section, so we're OK
			\addtocontents{#1}{\protect\contentsline{#2}{#3}{\thepage}}
		    }{% else
			% It is not a section; skip it
			\relax
		    }
		\fi
	    }{
		% We're not in TOC. Probably a figure or table; skip it
		\relax
	    }

	\else%
	    \addtocontents{#1}{\protect\contentsline{#2}{#3}{\thepage}}
	\fi
    }



    % ------------------------------------------------------------------------ %
    % Modify the look of the table of contents if we are using the article style
    % ------------------------------------------------------------------------ %

    % We want the dots closer together, and the same as in \dotfill
    %
    \renewcommand{\@dotsep}{2.27}

    \ifthenelse{\boolean{reportINL}}   {
    }{
	% For article.sty
	% We don't want blank lines between entries and no boldface.
	% This is the l@section definition from article.sty. We get rid of
	% \addvspace{1.0em... and \bfseries (if relax is not set in the main doc)
	\renewcommand*\l@section[2]{
	    \setlength\parskip{0ex}
	    \ifnum \c@tocdepth >\z@
	    \addpenalty\@secpenalty
	    \ifthenelse{\boolean{relaxedINL}}   {
		\addvspace{1.0em \@plus\p@}
	    }{
	    }
	    \setlength\@tempdima{1.5em}
	    \begingroup
		\parindent \z@ \rightskip \@pnumwidth
		\parfillskip -\@pnumwidth
		\leavevmode
		\ifthenelse{\boolean{relaxedINL}}   {
		    \bfseries
		}{
		}
		\advance\leftskip\@tempdima
		\hskip -\leftskip
		#1\nobreak\dotfill \nobreak\hb@xt@\@pnumwidth{\hss #2}\par
	    \endgroup
	\fi}

	\renewcommand*\l@figure{\setlength\parskip{0ex}
	    \@dottedtocline{1}{1.5em}{2.3em}
	}

	\let\l@table\l@figure
    }

    % ------------------------------------------------------------------------ %
    % If there is an appendix, set it apart in the table of contents
    %
    \renewcommand\appendix{
	\setboolean{inAppendix}{true}
	\par
	\setcounter{section}{0}
	\setcounter{subsection}{0}
	\setcounter{subsubsection}{0}
	\setcounter{figure}{0}
	\setcounter{table}{0}

	\ifthenelse{\boolean{reportINL}}   {
	    \setcounter{chapter}{0}
	    \gdef\@chapapp{\appendixname}
	    \gdef\thechapter{\@Alph\c@chapter}
	    % Insert the Appendix name in the TOC
	    \addtocontents{toc}{\protect\subsection*{\appendixname}}
	}{
	    \renewcommand{\thefigure}{\thesection.\arabic{figure}}
	    \renewcommand{\thetable}{\thesection.\arabic{table}}
	    \gdef\thesection{\@Alph\c@section}
	    % Insert the Appendix name in the TOC
	    \addtocontents{toc}{\protect\subsection*{\appendixname}}
	}

	\ifthenelse{\boolean{strictINL}}   {
	    \setcounter{secnumdepth}{2}
	}{
	    \setcounter{secnumdepth}{3}
	}
    }



% ******************************************************************************
% Sections & Chapters
% We want unnumbered section/chapter headings
%

    \ifthenelse{\boolean{reportINL}}   {
	\ifthenelse{\boolean{strictINL}}   {
	    % The samples in INL98-0730 don't number sections.
	    \setcounter{secnumdepth}{0}
	}{
	    % The report INL98-0730 itself does, though ;-)
	    \setcounter{secnumdepth}{3}
	}
    }{% else
	\ifthenelse{\boolean{strictINL}}   {
	    % The samples in INL98-0730 don't number sections.
	    \setcounter{secnumdepth}{0}
	}{
	    % The report INL98-0730 itself does, though ;-)
	    \setcounter{secnumdepth}{3}
	}
    }


% ******************************************************************************
% Captions
% We want the label bold face, and the whole captionwidth about 4",
% and the text small
%

    \newlength{\INLcaptionwidth}
    \setlength{\INLcaptionwidth}{4in}
    \newsavebox{\INLtempbox}
    \renewcommand{\@makecaption}[2]{
	\par\vspace{\abovecaptionskip}
	\sbox{\INLtempbox}{{\bf #1. }\ #2}
	\ifthenelse{\lengthtest{\wd\INLtempbox < \INLcaptionwidth}}   {
	    \begin{center}
		{\bf #1. }\small #2
	    \end{center}
	}{
	    \begin{center}
		\parbox[t]{\INLcaptionwidth}{{\bf #1. }\small #2}
	    \end{center}
	}
    }



% ******************************************************************************
% We have to import a lot of code from report.cls and article.cls so we can
% change a few things. We do report.cls first.
%

    \ifthenelse{\boolean{reportINL}}   {
	% We need to redo \maketitle for the report class.
	% Omit the date and add the INL number and so on.
	% We also need to redo the titlepage environment to not
	% issue a newpage command, so the abstract can go there
	% if it is short enough. (INL reports are supposed to have
	% an abstract!)

	%
	% Renew the report.cls \maketitle macro
	%
	\if@titlepage							% \maketitle from report.cls
	    \renewcommand\maketitle{%					% \maketitle from report.cls
		\doCover{}
		\begin{titlepage}%					% \maketitle from report.cls
		    \INLmarks{title}
		    {
			\begin{center}
			% use framebox instead of makebox for debugging
			\makebox[\topleftwidth][t]{
			    \begin{minipage}[c][\topheight][t]{\topleftwidth}
				% There is nothing at top left
			    \end{minipage}
			}
			\makebox[\topmiddlewidth][c]{
			    \begin{minipage}[c][\topheight][t]{\topmiddlewidth}
			    \begin{center}
				\INLnumVar\\
				\INLreleaseTypeVar\\
				Printed \INLprintDateVar\\
				\ifthenelse{\boolean{INLrePrintDateProvided}}   {
				    Reprinted \INLrePrintDateVar\\
				}{
				}
				\ifthenelse{\boolean{INLsupersedProvided}}   {
				    \bigskip
				    \usebox{\INLsupersedtempbox}\\
				}{
				}
			    \end{center}
			    \end{minipage}
			}
			% Added topright box to balance topleft, and center topmiddle - MJK, 1/12/07
			\makebox[\toprightwidth][c]{
			    \begin{minipage}[c][\topheight][t]{\toprightwidth}
			    % There is nothing at top right
			    \end{minipage}
			}
			\end{center}
		    }
		    \let\footnotesize\small				% \maketitle from report.cls
		    \let\footnoterule\relax				% \maketitle from report.cls
		    \let \footnote \thanks				% \maketitle from report.cls
		    % \null\vfil					% deleted
		    % \vskip 60\p@					% deleted
		    \begin{center}%					% \maketitle from report.cls
			{\LARGE \@title \par}%				% \maketitle from report.cls
			\vskip 3em%					% \maketitle from report.cls
			{\large						% \maketitle from report.cls
			\lineskip .75em%				% \maketitle from report.cls
			\begin{tabular}[t]{c}%				% \maketitle from report.cls
			    \renewcommand\Authfont{\fontfamily{phv}\selectfont\bfseries}
			    \renewcommand\Affilfont{\fontfamily{phv}\selectfont\bfseries}
			    \@author					% \maketitle from report.cls
			\end{tabular}\par}%				% \maketitle from report.cls
			\vskip 1.5em%					% \maketitle from report.cls
			% {\large \@date \par}%				% deleted
		    \end{center}\par					% \maketitle from report.cls
		    \@thanks						% \maketitle from report.cls
		    \vfil\null						% \maketitle from report.cls
% added by rcm
            \begin{center}
	        \CoverFont{b}{12}{12pt}
		Prepared for the \\
                U.S. Department of Energy\\
                Office of Nuclear Energy\\
                Under DOE Idaho Operations Office\\
                Contract DE-AC07-05ID14517
		\end{center}

		\end{titlepage}%					% \maketitle from report.cls
		\setcounter{footnote}{0}%				% \maketitle from report.cls
		\global\let\thanks\relax				% \maketitle from report.cls
		\global\let\maketitle\relax				% \maketitle from report.cls
		\global\let\@thanks\@empty				% \maketitle from report.cls
		\global\let\@author\@empty				% \maketitle from report.cls
		\global\let\@date\@empty				% \maketitle from report.cls
		\global\let\@title\@empty				% \maketitle from report.cls
		\global\let\title\relax					% \maketitle from report.cls
		\global\let\author\relax				% \maketitle from report.cls
		\global\let\date\relax					% \maketitle from report.cls
		\global\let\and\relax					% \maketitle from report.cls
	    }
	\else
	    % {---------------------- I doubt we ever get here ------------------------}
	    \ClassError{INLreport.cls}{\@backslashchar maketitle definition outside
		a title page.}{The INLreport class cannot handle this case.}
	\fi

	%
	% \titlepage in the report class resets the page
	% number, which is not correct for INLIA Reports.
	% It also sets the page style to empty.
	\if@compatibility						% titlepage from report.cls
	\renewenvironment{titlepage}					% titlepage from report.cls
	    {%								% titlepage from report.cls
	      \if@twocolumn						% titlepage from report.cls
		\@restonecoltrue\onecolumn				% titlepage from report.cls
	      \else							% titlepage from report.cls
		\@restonecolfalse\newpage				% titlepage from report.cls
	      \fi							% titlepage from report.cls
	      %\thispagestyle{empty}%					% deleted
	      %\setcounter{page}\z@					% deleted
	    }%								% titlepage from report.cls
	    %{\if@restonecol\twocolumn \else \newpage \fi		% deleted, we want abstract on this page
	    {\if@restonecol\twocolumn \fi				% new version
	    }								% titlepage from report.cls
	\else								% titlepage from report.cls
	\renewenvironment{titlepage}					% titlepage from report.cls
	    {%								% titlepage from report.cls
	      \if@twocolumn						% titlepage from report.cls
		\@restonecoltrue\onecolumn				% titlepage from report.cls
	      \else							% titlepage from report.cls
		\@restonecolfalse\newpage				% titlepage from report.cls
	      \fi							% titlepage from report.cls
	      %\thispagestyle{empty}%					% deleted
	      %\setcounter{page}\@ne					% deleted
	    }%								% titlepage from report.cls
	    %{\if@restonecol\twocolumn \else \newpage \fi		% deleted, we want abstract on this page
	    {\if@restonecol\twocolumn \fi				% new version
	     \if@twoside\else						% titlepage from report.cls
		%\setcounter{page}\@ne					% deleted
	     \fi							% titlepage from report.cls
	    }								% titlepage from report.cls
	\fi								% titlepage from report.cls

	\if@titlepage							% abstract from report.cls
	    \renewenvironment{abstract}{%				% abstract from report.cls
		% \titlepage						% deleted
		\null\vfil						% abstract from report.cls
		\@beginparpenalty\@lowpenalty				% abstract from report.cls
		\begin{center}%						% abstract from report.cls
		    \bfseries \abstractname				% abstract from report.cls
		    \@endparpenalty\@M					% abstract from report.cls
		\end{center}}%						% abstract from report.cls
		% {\par\vfil\null\endtitlepage}				% deleted
		{\par\vfil\null}					% new version
	\else								% abstract from report.cls
	    % {---------------------- We should never get here ------------------------}
	    \ClassError{INLreport.cls}{Abstract definition outside
		a title page.}{The INLreport class cannot handle this case.}
	\fi								% abstract from report.cls

    }{
	%
	% Now for the changes to article.cls functions
	%


	%
	% In \@maketitle, we omit the \@date and insert the INL number and so on
	%
	\renewcommand{\@maketitle}{
	    \doCover{}
	    \newpage							% \@maketitle from article.cls
	    {								% new
		\begin{center}						% new
		% use framebox instead of makebox for debugging		% new
		\makebox[\topleftwidth][t]{				% new
		    \begin{minipage}[c][\topheight][t]{\topleftwidth}	% new
			% There is nothing at top left			% new
		    \end{minipage}					% new
		}							% new
		\makebox[\topmiddlewidth][c]{				% new
		    \begin{minipage}[c][\topheight][t]{\topmiddlewidth}	% new
		    \begin{center}					% new
			\INLnumVar\\					% new
			\INLreleaseTypeVar\\				% new
			Printed \INLprintDateVar\\			% new
			\ifthenelse{\boolean{INLrePrintDateProvided}}   {	% new
			    Reprinted \INLrePrintDateVar\\		% new
			}{						% new
			}						% new
			\ifthenelse{\boolean{INLsupersedProvided}}   {	% new
			    \bigskip					% new
			    \usebox{\INLsupersedtempbox}\\		% new
			}{						% new
			}						% new
		    \end{center}					% new
		    \end{minipage}					% new
		}							% new
		% Add topright box to balance topleft to center topmiddle - MJK, 1/12/07
		\makebox[\toprightwidth][c]{				% new
		    \begin{minipage}[c][\topheight][t]{\toprightwidth}	% new
			% There is nothing at top right			% new
		    \end{minipage}					% new
		}							% new
		\end{center}						% new
	    }								% new

	    \begin{center}						% \@maketitle from article.cls
		\let \footnote \thanks					% \@maketitle from article.cls
		{\LARGE \bf \@title\par}				% \@maketitle from article.cls
		\vskip 1.5em						% \@maketitle from article.cls
		{\large							% \@maketitle from article.cls
		    \lineskip .5em%					% \@maketitle from article.cls
		    \begin{tabular}[t]{c}				% \@maketitle from article.cls
		    \@author						% \@maketitle from article.cls
		    \end{tabular}\par					% \@maketitle from article.cls
		}							% \@maketitle from article.cls
	    \end{center}						% \@maketitle from article.cls
	    \par							% \@maketitle from article.cls
	    \vskip 1.5em						% \@maketitle from article.cls
	}


	%
	% For fancy headers, we have to redefine \maketitle. All we are doing,
	% is to activate the headers again, and then redefine \newpage so it
	% happens for every body page.
	%
	\renewcommand{\maketitle}{\par
	    \begingroup							% \maketitle from article.cls
	    \renewcommand\thefootnote{\@fnsymbol\c@footnote}%		% \maketitle from article.cls
	    \def\@makefnmark{						% \maketitle from article.cls
		\rlap{\@textsuperscript{\normalfont\@thefnmark}}}%	% \maketitle from article.cls
	    \long\def\@makefntext##1{\parindent 1em\noindent		% \maketitle from article.cls
		\hb@xt@1.8em{%						% \maketitle from article.cls
		\hss\@textsuperscript{\normalfont\@thefnmark}}##1}%	% \maketitle from article.cls
	    \if@twocolumn						% \maketitle from article.cls
		\ifnum \col@number=\@ne					% \maketitle from article.cls
		    \@maketitle						% \maketitle from article.cls
		\else							% \maketitle from article.cls
		    \twocolumn[\@maketitle]%				% \maketitle from article.cls
		\fi							% \maketitle from article.cls
	    \else							% \maketitle from article.cls
		\newpage						% \maketitle from article.cls
		% Prevents figures from going at top of page.		% \maketitle from article.cls
		\global\@topnum\z@					% \maketitle from article.cls
		\@maketitle						% \maketitle from article.cls
	    \fi								% \maketitle from article.cls
	    \INLmarks{title}						% new
	    \@thanks							% \maketitle from article.cls
	    \endgroup							% \maketitle from article.cls
	    \setcounter{footnote}{0}%					% \maketitle from article.cls
	    \global\let\thanks\relax					% \maketitle from article.cls
	    \global\let\maketitle\relax					% \maketitle from article.cls
	    \global\let\@maketitle\relax				% \maketitle from article.cls
	    \global\let\@thanks\@empty					% \maketitle from article.cls
	    \global\let\@author\@empty					% \maketitle from article.cls
	    \global\let\@date\@empty					% \maketitle from article.cls
	    \global\let\@title\@empty					% \maketitle from article.cls
	    \global\let\title\relax					% \maketitle from article.cls
	    \global\let\author\relax					% \maketitle from article.cls
	    \global\let\date\relax					% \maketitle from article.cls
	    \global\let\and\relax					% \maketitle from article.cls


	    %
	    % Now we renew \newpage so that from here on the markings
	    % of the body are used.
	    %
	    \renewcommand{\newpage}{%				% \newpage from latex.ltx
		\if@noskipsec					% \newpage from latex.ltx
		    \ifx \@nodocument\relax			% \newpage from latex.ltx
			\leavevmode				% \newpage from latex.ltx
			\global \@noskipsecfalse		% \newpage from latex.ltx
		    \fi						% \newpage from latex.ltx
		\fi						% \newpage from latex.ltx
		\if@inlabel					% \newpage from latex.ltx
		    \leavevmode					% \newpage from latex.ltx
		    \global \@inlabelfalse			% \newpage from latex.ltx
		\fi						% \newpage from latex.ltx
		\if@nobreak \@nobreakfalse \everypar{}\fi	% \newpage from latex.ltx
		\par						% \newpage from latex.ltx
		\vfil						% \newpage from latex.ltx
		\penalty -\@M					% \newpage from latex.ltx
		\INLmarks{body}				% new
	    }							% \newpage from latex.ltx
	}								% \maketitle from article.sty
    }



% ******************************************************************************
% Do some checking at the end of the document
%

    \AtEndDocument{
	% Do some more usage checking
	\ifthenelse{\boolean{INLmainProvided}}   {
	}{
	    \ClassError{INLreport.cls}{\@backslashchar INLmain
		not provided}{Add \@backslashchar INLmain before your
		introduction (first) section of your document}
	}


	%
	% Put the INL Logo on the back page
	%

	\cleardoublepage	% We're on the inside end page now
	\mbox{}			% Some invisible text
	\vfill
	\hfill {\CoverFont{m}{6}{6pt} \INLreportVersionOnly}
	\setboolean{INLmarkingsRequired}{false}
	\thispagestyle{empty}
	\pagestyle{empty}
	\newpage		% Now we're on the outside of the end page

	\vspace*{\fill}		% Move the logo to the bottom of the page
	\thispagestyle{empty}
	\begin{center}
	    \includegraphics{inl-left_black}
	\end{center}

    } % End of AtEndDocument{}
