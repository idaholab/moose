%!PS-Adobe-3.0 EPSF-3.0
%%Creator: Mayura Draw, Version 4.3
%%Title: inf_cosserat.md
%%CreationDate: Thu Aug 02 13:51:03 2007
%%BoundingBox: 25 350 605 640
%%DocumentFonts: ArialMT
%%+ SymbolMT
%%Orientation: Portrait
%%EndComments
%%BeginProlog
%%BeginResource: procset MayuraDraw_ops
%%Version: 4.3
%%Copyright: (c) 1993-2003 Mayura Software
/PDXDict 100 dict def
PDXDict begin
% width height matrix proc key cache
% definepattern -\> font
/definepattern { %def
  7 dict begin
    /FontDict 9 dict def
    FontDict begin
      /cache exch def
      /key exch def
      /proc exch cvx def
      /mtx exch matrix invertmatrix def
      /height exch def
      /width exch def
      /ctm matrix currentmatrix def
      /ptm matrix identmatrix def
      /str
      (xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx)
      def
    end
    /FontBBox [ %def
      0 0 FontDict /width get
      FontDict /height get
    ] def
    /FontMatrix FontDict /mtx get def
    /Encoding StandardEncoding def
    /FontType 3 def
    /BuildChar { %def
      pop begin
      FontDict begin
        width 0 cache { %ifelse
          0 0 width height setcachedevice
        }{ %else
          setcharwidth
        } ifelse
        0 0 moveto width 0 lineto
        width height lineto 0 height lineto
        closepath clip newpath
        gsave proc grestore
      end end
    } def
    FontDict /key get currentdict definefont
  end
} bind def

% dict patternpath -
% dict matrix patternpath -
/patternpath { %def
  dup type /dicttype eq { %ifelse
    begin FontDict /ctm get setmatrix
  }{ %else
    exch begin FontDict /ctm get setmatrix
    concat
  } ifelse
  currentdict setfont
  FontDict begin
    FontMatrix concat
    width 0 dtransform
    round width div exch round width div exch
    0 height dtransform
    round height div exch
    round height div exch
    0 0 transform round exch round exch
    ptm astore setmatrix

    pathbbox
    height div ceiling height mul 4 1 roll
    width div ceiling width mul 4 1 roll
    height div floor height mul 4 1 roll
    width div floor width mul 4 1 roll

    2 index sub height div ceiling cvi exch
    3 index sub width div ceiling cvi exch
    4 2 roll moveto

    FontMatrix ptm invertmatrix pop
    { %repeat
      gsave
        ptm concat
        dup str length idiv { %repeat
          str show
        } repeat
        dup str length mod str exch
        0 exch getinterval show
      grestore
      0 height rmoveto
    } repeat
    pop
  end end
} bind def

% dict patternfill -
% dict matrix patternfill -
/patternfill { %def
  gsave
    eoclip patternpath
  grestore
  newpath
} bind def

/img { %def
  gsave
  /imgh exch def
  /imgw exch def
  concat
  imgw imgh 8
  [imgw 0 0 imgh neg 0 imgh]
  /colorstr 768 string def
  /colorimage where {
    pop
    { currentfile colorstr readhexstring pop }
    false 3 colorimage
  }{
    /graystr 256 string def
    {
      currentfile colorstr readhexstring pop
      length 3 idiv
      dup 1 sub 0 1 3 -1 roll
      {
        graystr exch
        colorstr 1 index 3 mul get 30 mul
        colorstr 2 index 3 mul 1 add get 59 mul
        colorstr 3 index 3 mul 2 add get 11 mul
        add add 100 idiv
        put
      } for
      graystr 0 3 -1 roll getinterval
    } image
  } ifelse
  grestore
} bind def

/arrowhead {
  gsave
    [] 0 setdash
    strokeC strokeM strokeY strokeK setcmykcolor
    2 copy moveto
    4 2 roll exch 4 -1 roll exch
    sub 3 1 roll sub
    exch atan rotate dup scale
    arrowtype
    dup 0 eq {
      -1 2 rlineto 7 -2 rlineto -7 -2 rlineto
      closepath fill
    } if
    dup 1 eq {
      0 3 rlineto 9 -3 rlineto -9 -3 rlineto
      closepath fill
    } if
    dup 2 eq {
      -6 -6 rmoveto 6 6 rlineto -6 6 rlineto
      -1.4142 -1.4142 rlineto 4.5858 -4.5858 rlineto
      -4.5858 -4.5858 rlineto closepath fill
    } if
    dup 3 eq {
      -6 0 rmoveto -1 2 rlineto 7 -2 rlineto -7 -2 rlineto
      closepath fill
    } if
    dup 4 eq {
      -9 0 rmoveto 0 3 rlineto 9 -3 rlineto -9 -3 rlineto
      closepath fill
    } if
    dup 5 eq {
      currentpoint newpath 3 0 360 arc
      closepath fill
    } if
    dup 6 eq {
      2.5 2.5 rmoveto 0 -5 rlineto -5 0 rlineto 0 5 rlineto
      closepath fill
    } if
    pop
  grestore
} bind def

/setcmykcolor where { %ifelse
  pop
}{ %else
  /setcmykcolor {
     /black exch def /yellow exch def
     /magenta exch def /cyan exch def
     cyan black add dup 1 gt { pop 1 } if 1 exch sub
     magenta black add dup 1 gt { pop 1 } if 1 exch sub
     yellow black add dup 1 gt { pop 1 } if 1 exch sub
     setrgbcolor
  } bind def
} ifelse

/RE { %def
  findfont begin
  currentdict dup length dict begin
    { %forall
      1 index /FID ne { def } { pop pop } ifelse
    } forall
    /FontName exch def dup length 0 ne { %if
      /Encoding Encoding 256 array copy def
      0 exch { %forall
        dup type /nametype eq { %ifelse
          Encoding 2 index 2 index put
          pop 1 add
        }{ %else
          exch pop
        } ifelse
      } forall
    } if pop
  currentdict dup end end
  /FontName get exch definefont pop
} bind def

/spacecount { %def
  0 exch
  ( ) { %loop
    search { %ifelse
      pop 3 -1 roll 1 add 3 1 roll
    }{ pop exit } ifelse
  } loop
} bind def

/WinAnsiEncoding [
  39/quotesingle 96/grave 130/quotesinglbase/florin/quotedblbase
  /ellipsis/dagger/daggerdbl/circumflex/perthousand
  /Scaron/guilsinglleft/OE 145/quoteleft/quoteright
  /quotedblleft/quotedblright/bullet/endash/emdash
  /tilde/trademark/scaron/guilsinglright/oe/dotlessi
  159/Ydieresis 164/currency 166/brokenbar 168/dieresis/copyright
  /ordfeminine 172/logicalnot 174/registered/macron/ring
  177/plusminus/twosuperior/threesuperior/acute/mu
  183/periodcentered/cedilla/onesuperior/ordmasculine
  188/onequarter/onehalf/threequarters 192/Agrave/Aacute
  /Acircumflex/Atilde/Adieresis/Aring/AE/Ccedilla
  /Egrave/Eacute/Ecircumflex/Edieresis/Igrave/Iacute
  /Icircumflex/Idieresis/Eth/Ntilde/Ograve/Oacute
  /Ocircumflex/Otilde/Odieresis/multiply/Oslash
  /Ugrave/Uacute/Ucircumflex/Udieresis/Yacute/Thorn
  /germandbls/agrave/aacute/acircumflex/atilde/adieresis
  /aring/ae/ccedilla/egrave/eacute/ecircumflex
  /edieresis/igrave/iacute/icircumflex/idieresis
  /eth/ntilde/ograve/oacute/ocircumflex/otilde
  /odieresis/divide/oslash/ugrave/uacute/ucircumflex
  /udieresis/yacute/thorn/ydieresis
] def

/SymbolEncoding [
  32/space/exclam/universal/numbersign/existential/percent
  /ampersand/suchthat/parenleft/parenright/asteriskmath/plus
  /comma/minus/period/slash/zero/one/two/three/four/five/six
  /seven/eight/nine/colon/semicolon/less/equal/greater/question
  /congruent/Alpha/Beta/Chi/Delta/Epsilon/Phi/Gamma/Eta/Iota
  /theta1/Kappa/Lambda/Mu/Nu/Omicron/Pi/Theta/Rho/Sigma/Tau
  /Upsilon/sigma1/Omega/Xi/Psi/Zeta/bracketleft/therefore
  /bracketright/perpendicular/underscore/radicalex/alpha
  /beta/chi/delta/epsilon/phi/gamma/eta/iota/phi1/kappa/lambda
  /mu/nu/omicron/pi/theta/rho/sigma/tau/upsilon/omega1/omega
  /xi/psi/zeta/braceleft/bar/braceright/similar
  161/Upsilon1/minute/lessequal/fraction/infinity/florin/club
  /diamond/heart/spade/arrowboth/arrowleft/arrowup/arrowright
  /arrowdown/degree/plusminus/second/greaterequal/multiply
  /proportional/partialdiff/bullet/divide/notequal/equivalence
  /approxequal/ellipsis/arrowvertex/arrowhorizex/carriagereturn
  /aleph/Ifraktur/Rfraktur/weierstrass/circlemultiply
  /circleplus/emptyset/intersection/union/propersuperset
  /reflexsuperset/notsubset/propersubset/reflexsubset/element
  /notelement/angle/gradient/registerserif/copyrightserif
  /trademarkserif/product/radical/dotmath/logicalnot/logicaland
  /logicalor/arrowdblboth/arrowdblleft/arrowdblup/arrowdblright
  /arrowdbldown/lozenge/angleleft/registersans/copyrightsans
  /trademarksans/summation/parenlefttp/parenleftex/parenleftbt
  /bracketlefttp/bracketleftex/bracketleftbt/bracelefttp
  /braceleftmid/braceleftbt/braceex
  241/angleright/integral/integraltp/integralex/integralbt
  /parenrighttp/parenrightex/parenrightbt/bracketrighttp
  /bracketrightex/bracketrightbt/bracerighttp/bracerightmid
  /bracerightbt
] def

/patarray [
/leftdiagonal /rightdiagonal /crossdiagonal /horizontal
/vertical /crosshatch /fishscale /wave /brick
] def
/arrowtype 0 def
/fillC 0 def /fillM 0 def /fillY 0 def /fillK 0 def
/strokeC 0 def /strokeM 0 def /strokeY 0 def /strokeK 1 def
/pattern -1 def
/mat matrix def
/mat2 matrix def
/nesting 0 def
/deferred /N def
/c /curveto load def
/c2 { pop pop c } bind def
/C /curveto load def
/C2 { pop pop C } bind def
/e { gsave concat 0 0 moveto } bind def
/F {
  nesting 0 eq { %ifelse
    pattern -1 eq { %ifelse
      fillC fillM fillY fillK setcmykcolor eofill
    }{ %else
      gsave fillC fillM fillY fillK setcmykcolor eofill grestore
      0 0 0 1 setcmykcolor
      patarray pattern get findfont patternfill
    } ifelse
  }{ %else
    /deferred /F def
  } ifelse
} bind def
/f { closepath F } bind def
/K { /strokeK exch def /strokeY exch def
     /strokeM exch def /strokeC exch def } bind def
/k { /fillK exch def /fillY exch def
     /fillM exch def /fillC exch def } bind def
/opc { pop } bind def
/Opc { pop } bind def
/L /lineto load def
/L2 { pop pop L } bind def
/m /moveto load def
/m2 { pop pop m } bind def
/n /newpath load def
/N {
  nesting 0 eq { %ifelse
    newpath
  }{ %else
    /deferred /N def
  } ifelse
} def
/S {
  nesting 0 eq { %ifelse
    strokeC strokeM strokeY strokeK setcmykcolor stroke
  }{ %else
    /deferred /S def
  } ifelse
} bind def
/s { closepath S } bind def
/Tx { fillC fillM fillY fillK setcmykcolor show
      0 leading neg translate 0 0 moveto } bind def
/T { grestore } bind def
/TX { pop } bind def
/Ts { pop } bind def
/tal { pop } bind def
/tld { pop } bind def
/tbx { pop exch pop sub /jwidth exch def } def
/tpt { %def
  fillC fillM fillY fillK setcmykcolor
  moveto show
} bind def
/tpj { %def
  fillC fillM fillY fillK setcmykcolor
  moveto
  dup stringwidth pop
  3 -1 roll
  exch sub
  1 index spacecount
  dup 0 eq { %ifelse
    pop pop show
  }{ %else
    div 0 8#040 4 -1 roll widthshow
  } ifelse
} bind def
/u {} def
/U {} def
/*u { /nesting nesting 1 add def } def
/*U {
  /nesting nesting 1 sub def
  nesting 0 eq {
    deferred cvx exec
  } if
} def
/w /setlinewidth load def
/d /setdash load def
/B {
  nesting 0 eq { %ifelse
    gsave F grestore S
  }{ %else
    /deferred /B def
  } ifelse
} bind def
/b { closepath B } bind def
/z { /align exch def pop /leading exch def exch findfont
     exch scalefont setfont } bind def
/tfn { exch findfont
     exch scalefont setfont } bind def
/Pat { /pattern exch def } bind def
/cm { 6 array astore concat } bind def
/q { mat2 currentmatrix pop } bind def
/Q { mat2 setmatrix } bind def
/Ah {
  pop /arrowtype exch def
  currentlinewidth 5 1 roll arrowhead
} bind def
/Arc {
  mat currentmatrix pop
    translate scale 0 0 1 5 -2 roll arc
  mat setmatrix
} bind def
/Arc2 { pop pop Arc } bind def
/Bx {
  mat currentmatrix pop
    concat /y1 exch def /x1 exch def /y2 exch def /x2 exch def
    x1 y1 moveto x1 y2 lineto x2 y2 lineto x2 y1 lineto
  mat setmatrix
} bind def
/Rr {
  mat currentmatrix pop
    concat /yrad exch def /xrad exch def
    2 copy gt { exch } if /x2 exch def /x1 exch def
    2 copy gt { exch } if /y2 exch def /y1 exch def
    x1 xrad add y2 moveto
    matrix currentmatrix x1 xrad add y2 yrad sub translate xrad yrad scale
    0 0 1 90 -180 arc setmatrix
    matrix currentmatrix x1 xrad add y1 yrad add translate xrad yrad scale
    0 0 1 180 270 arc setmatrix
    matrix currentmatrix x2 xrad sub y1 yrad add translate xrad yrad scale
    0 0 1 270 0 arc setmatrix
    matrix currentmatrix x2 xrad sub y2 yrad sub translate xrad yrad scale
    0 0 1 0 90 arc setmatrix
    closepath
  mat setmatrix
} bind def
/Ov {
  mat currentmatrix pop
    concat translate scale 1 0 moveto 0 0 1 0 360 arc closepath
  mat setmatrix
} bind def
end
%%EndResource
%%EndProlog
%%BeginSetup
%PDX g 5 5 1 1
%%IncludeFont: ArialMT
%%IncludeFont: SymbolMT
PDXDict begin
%%EndSetup
%%Page: 1 1
%%BeginPageSetup
/_PDX_savepage save def

15 15 [300 72 div 0 0 300 72 div 0 0]
{ %definepattern
  2 setlinecap
  7.5 0 moveto 15 7.5 lineto
  0 7.5 moveto 7.5 15 lineto
  2 setlinewidth stroke
} bind
/rightdiagonal true definepattern pop

15 15 [300 72 div 0 0 300 72 div 0 0]
{ %definepattern
  2 setlinecap
  7.5 0 moveto 0 7.5 lineto
  15 7.5 moveto 7.5 15 lineto
  2 setlinewidth stroke
} bind
/leftdiagonal true definepattern pop

15 15 [300 72 div 0 0 300 72 div 0 0]
{ %definepattern
  2 setlinecap
  0 7.5 moveto 15 7.5 lineto
  2 setlinewidth stroke
} bind
/horizontal true definepattern pop

15 15 [300 72 div 0 0 300 72 div 0 0]
{ %definepattern
  2 setlinecap
  7.5 0 moveto 7.5 15 lineto
  2 setlinewidth stroke
} bind
/vertical true definepattern pop

15 15 [300 72 div 0 0 300 72 div 0 0]
{ %definepattern
  2 setlinecap
  0 7.5 moveto 15 7.5 lineto
  7.5 0 moveto 7.5 15 lineto
  2 setlinewidth stroke
} bind
/crosshatch true definepattern pop

30 30 [300 72 div 0 0 300 72 div 0 0]
{ %definepattern
  2 setlinecap
  0 7.5 moveto 30 7.5 lineto
  0 22.5 moveto 30 22.5 lineto
  7.5 0 moveto 7.5 7.5 lineto
  7.5 22.5 moveto 7.5 30 lineto
  22.5 7.5 moveto 22.5 22.5 lineto
  1 setlinewidth stroke
} bind
/brick true definepattern pop

30 30 [300 72 div 0 0 300 72 div 0 0]
{ %definepattern
  2 2 scale
  2 setlinecap
  7.5 0 moveto 15 7.5 lineto
  0 7.5 moveto 7.5 15 lineto
  7.5 0 moveto 0 7.5 lineto
  15 7.5 moveto 7.5 15 lineto
  0.5 setlinewidth stroke
} bind
/crossdiagonal true definepattern pop

30 30 [300 72 div 0 0 300 72 div 0 0]
{ %definepattern
  2 2 scale
  1 setlinecap
  0 7.5 moveto 0 15 7.5 270 360 arc
  7.5 15 moveto 15 15 7.5 180 270 arc
  0 7.5 moveto 7.5 7.5 7.5 180 360 arc
  0.5 setlinewidth stroke
} bind
/fishscale true definepattern pop

30 30 [300 72 div 0 0 300 72 div 0 0]
{ %definepattern
  1 setlinecap 0.5 setlinewidth
  7.5 0 10.6 135 45 arcn
  22.5 15 10.6 225 315 arc
  stroke
  7.5 15 10.6 135 45 arcn
  22.5 30 10.6 225 315 arc
  stroke
} bind
/wave true definepattern pop

WinAnsiEncoding /_ArialMT /ArialMT RE
SymbolEncoding /_SymbolMT /SymbolMT RE

newpath 2 setlinecap 0 setlinejoin 2 setmiterlimit
[] 0 setdash
25 350 moveto 25 640 lineto 605 640 lineto 605 350 lineto closepath clip
newpath
%%EndPageSetup
3 w
200 450 100 550 [1 0 0 1 0 0] Bx
s
2 w
q
1 0 0 1 0 0 cm
210 480 m
210 514 210 530 L2
Q
S
q
1 0 0 1 0 0 cm
210 480 210 530 4 2 Ah
Q
q
1 0 0 1 0 0 cm
210 480 m
234 480 250 480 L2
Q
S
q
1 0 0 1 0 0 cm
210 480 250 480 4 2 Ah
Q
q
1 0 0 1 0 0 cm
120 560 m
154 560 170 560 L2
Q
S
q
1 0 0 1 0 0 cm
120 560 170 560 4 2 Ah
Q
q
1 0 0 1 0 0 cm
120 560 m
120 594 120 610 L2
Q
S
q
1 0 0 1 0 0 cm
120 560 120 610 4 2 Ah
Q
q
1 0 0 1 0 0 cm
90 520 m
90 486 90 470 L2
Q
S
q
1 0 0 1 0 0 cm
90 520 90 470 4 2 Ah
Q
q
1 0 0 1 0 0 cm
90 520 m
56 520 40 520 L2
Q
S
q
1 0 0 1 0 0 cm
90 520 40 520 4 2 Ah
Q
q
1 0 0 1 0 0 cm
170 440 m
136 440 120 440 L2
Q
S
q
1 0 0 1 0 0 cm
170 440 120 440 4 2 Ah
Q
q
1 0 0 1 0 0 cm
170 440 m
170 406 170 390 L2
Q
S
q
1 0 0 1 0 0 cm
170 440 170 390 4 2 Ah
Q
3 w
500 450 400 550 [1 0 0 1 0 0] Bx
s
2 w
q
1 0 0 1 0 0 cm
-95.0063 79.4453 31.6228 31.6228 520 500 -95.0063 108.435 Arc2
Q
S
q
1 0 0 1 0 0 cm
522.76 531.502 510 530 4 2 Ah
Q
q
1 0 0 1 0 0 cm
-1.93086 149.442 30 30 450 570 -1.93086 180 Arc2
Q
S
q
1 0 0 1 0 0 cm
420.017 571.011 420 570 4 2 Ah
Q
q
-1 0 0 1 760 0 cm
-89.3274 59.4422 30 30 380 500 -89.3274 90 Arc2
Q
S
q
-1 0 0 1 760 0 cm
391.592 527.678 380 530 4 2 Ah
Q
q
1 0 0 -1 0 860 cm
0.437012 149.442 30 30 450 430 0.437012 180 Arc2
Q
S
q
1 0 0 -1 0 860 cm
422.334 441.622 420 430 4 2 Ah
Q
14.1421 14.1421 380 500 [1 0 0 1 0 0] Ov
s
14.1421 14.1421 520 500 [1 0 0 1 0 0] Ov
s
14.1421 14.1421 450 430 [1 0 0 1 0 0] Ov
s
14.1421 14.1421 450 570 [1 0 0 1 0 0] Ov
s
1 1 1 0 k
0.25 w
7.07107 7.07107 520 500 [1 0 0 1 0 0] Ov
b
7.07107 7.07107 520 500 [1 0 0 1 -70 70] Ov
b
2 w
q
1 0 0 1 0 0 cm
390 510 m
370 490 L
Q
B
q
1 0 0 1 0 0 cm
370 510 m
390 490 L
Q
B
q
1 0 0 1 0 0 cm
440 440 m
460 420 L
Q
B
q
1 0 0 1 0 0 cm
460 440 m
440 420 L
Q
B
-1.42109e-016 -1.42109e-016 -1.42109e-016 0 k
q
1 0 0 1 0 0 cm
0.444583 -149.116 36.0555 36.0555 450 500 0.444583 -123.69 Arc2
Q
S
q
1 0 0 1 0 0 cm
423.308 475.76 430 470 4 2 Ah
Q
[1 0 0 1 0 0] e
440 470 440 470 tbx
0 tal
13 tld
1 1 1 0 k
/_ArialMT 12 tfn
() 440 459.14 tpt
T
[1 0 0 1 0 0] e
440 470 440 470 tbx
0 tal
19 tld
/_ArialMT 18 tfn
() 440 453.71 tpt
T
[1 0 0 1 0 10] e
440 480 440 480 tbx
0 tal
26 tld
/_ArialMT 24 tfn
(M) 440 458.28 tpt
T
[1 0 0 1 -120 -90] e
560 490 560 490 tbx
0 tal
26 tld
/_ArialMT 24 tfn
(m) 560 468.28 tpt
T
[1 0 0 1 -250 30] e
560 490 560 490 tbx
0 tal
26 tld
/_ArialMT 24 tfn
(m) 560 468.28 tpt
T
[1 0 0 1 -120 140] e
560 490 560 490 tbx
0 tal
26 tld
/_ArialMT 24 tfn
(m) 560 468.28 tpt
T
[1 0 0 1 0 10] e
460 610 460 610 tbx
0 tal
19 tld
/_ArialMT 18 tfn
(32) 460 593.71 tpt
T
[1 0 0 1 0 -220] e
460 610 460 610 tbx
0 tal
19 tld
/_ArialMT 18 tfn
(32) 460 593.71 tpt
T
[1 0 0 1 -5 30] e
560 490 560 490 tbx
0 tal
26 tld
/_ArialMT 24 tfn
(m) 560 468.28 tpt
T
[1 0 0 1 260 -45] e
315 555 315 555 tbx
0 tal
19 tld
/_ArialMT 18 tfn
(31) 315 538.71 tpt
T
[1 0 0 1 15 -45] e
315 555 315 555 tbx
0 tal
19 tld
/_ArialMT 18 tfn
(31) 315 538.71 tpt
T
q
1 0 0 1 0 0 cm
150 500 m
166.52 526.432 175 540 L2
Q
S
q
1 0 0 1 0 0 cm
150 500 175 540 4 2 Ah
Q
[1 0 0 1 0 20] e
175 505 175 505 tbx
0 tal
26 tld
/_ArialMT 24 tfn
(F) 175 483.28 tpt
T
[1 0 0 1 0 10] e
255 485 255 485 tbx
0 tal
26 tld
/_SymbolMT 24 tfn
(s) 255 460.88 tpt
T
[1 0 0 1 -35 65] e
255 485 255 485 tbx
0 tal
26 tld
/_SymbolMT 24 tfn
(s) 255 460.88 tpt
T
[1 0 0 1 -95 -90] e
255 485 255 485 tbx
0 tal
26 tld
/_SymbolMT 24 tfn
(s) 255 460.88 tpt
T
[1 0 0 1 -140 -40] e
255 485 255 485 tbx
0 tal
26 tld
/_SymbolMT 24 tfn
(s) 255 460.88 tpt
T
[1 0 0 1 -90 105] e
255 485 255 485 tbx
0 tal
26 tld
/_SymbolMT 24 tfn
(s) 255 460.88 tpt
T
[1 0 0 1 -130 145] e
255 485 255 485 tbx
0 tal
26 tld
/_SymbolMT 24 tfn
(s) 255 460.88 tpt
T
[1 0 0 1 -220 40] e
255 485 255 485 tbx
0 tal
26 tld
/_SymbolMT 24 tfn
(s) 255 460.88 tpt
T
[1 0 0 1 -190 0] e
255 485 255 485 tbx
0 tal
26 tld
/_SymbolMT 24 tfn
(s) 255 460.88 tpt
T
[1 0 0 1 -5 10] e
275 470 275 470 tbx
0 tal
19 tld
/_ArialMT 18 tfn
(11) 275 453.71 tpt
T
[1 0 0 1 -225 40] e
275 470 275 470 tbx
0 tal
19 tld
/_ArialMT 18 tfn
(11) 275 453.71 tpt
T
[1 0 0 1 -20 0] e
255 535 255 535 tbx
0 tal
19 tld
/_ArialMT 18 tfn
(21) 255 518.71 tpt
T
[1 0 0 1 -175 -65] e
255 535 255 535 tbx
0 tal
19 tld
/_ArialMT 18 tfn
(21) 255 518.71 tpt
T
[1 0 0 1 -115 -135] e
245 565 245 565 tbx
0 tal
19 tld
/_ArialMT 18 tfn
(12) 245 548.71 tpt
T
[1 0 0 1 -65 10] e
245 565 245 565 tbx
0 tal
19 tld
/_ArialMT 18 tfn
(12) 245 548.71 tpt
T
[1 0 0 1 -80 -205] e
255 585 255 585 tbx
0 tal
19 tld
/_ArialMT 18 tfn
(22) 255 568.71 tpt
T
[1 0 0 1 -115 30] e
255 585 255 585 tbx
0 tal
19 tld
/_ArialMT 18 tfn
(22) 255 568.71 tpt
T
%%PageTrailer
_PDX_savepage restore
%%Trailer
end
showpage
%%EOF
