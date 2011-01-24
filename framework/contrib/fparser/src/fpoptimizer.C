/***************************************************************************\
|* Function Parser for C++ v4.2                                            *|
|*-------------------------------------------------------------------------*|
|* Function optimizer                                                      *|
|*-------------------------------------------------------------------------*|
|* Copyright: Joel Yliluoma                                                *|
|*                                                                         *|
|* This library is distributed under the terms of the                      *|
|* GNU Lesser General Public License version 3.                            *|
|* (See lgpl.txt and gpl.txt for the license text.)                        *|
\***************************************************************************/

/* NOTE:
 This file contains generated code (from the optimizer sources) and is
 not intended to be modified by hand. If you want to modify the optimizer,
 download the development version of the library.
*/

#include "fpconfig.h"
#ifdef FP_SUPPORT_OPTIMIZER
#include "fparser.h"
#include "fptypes.h"
#define tZ3 (p0.min
#define tY3 size_t>
#define tX3 ),has_min(
#define tW3 ;size_t
#define tV3 );Value
#define tU3 ?cNotNot:cAbsNotNot);
#define tT3 nD cIf?
#define tS3 ).xP 2))
#define tR3 ).xP 1))
#define tQ3 >yM1 lF
#define tP3 xE2 eV1
#define tO3 yD,y1))
#define tN3 info.xR
#define tM3 ),info,
#define tL3 .n32 p.
#define tK3 sim.xA
#define tJ3 addgroup
#define tI3 lR1 2)
#define tH3 {pow.lH1
#define tG3 "Found "
#define tF3 if(eM1==
#define tE3 stackpos
#define tD3 "dup(%u) "
#define tC3 "%d, cost "
#define tB3 FuncParsers
#define tA3 "immed "<<
#define t93 "PUSH ";nX3(
#define t83 .GetOpcode
#define t73 eH2 assert
#define t63 stderr
#define t53 sep2=" "
#define t43 FPHASH_CONST
#define t33 cache_needed[
#define t23 fprintf
#define t13 "Applying "
#define t03 ||tree t83
#define eZ3 HANDLE_UNARY_CONST_FUNC
#define eY3 within,
#define eX3 ==false
#define eW3 ),Depth(
#define eV3 ),Hash(
#define eU3 1)yK1){
#define eT3 c_count
#define eS3 s_count
#define eR3 else n5
#define eQ3 c01 max
#define eP3 c01 min
#define eO3 =fp_pow
#define eN3 else{m.
#define eM3 .l22&&(
#define eL3 ,cMul);
#define eK3 codes[b
#define eJ3 whydump
#define eI3 lD1.c93
#define eH3 nparams
#define eG3 first!=
#define eF3 true,1,
#define eE3 cTan,l2
#define eD3 cLog,l2
#define eC3 cAtan2,
#define eB3 x4 0,
#define eA3 cAbs x4
#define e93 std::cK
#define e83 :tree
#define e73 base,iQ
#define e63 lH1 cC.
#define e53 .data.
#define e43 iK;++b)
#define e33 IfBalanceGood
#define e23 =false;
#define e13 :{n21 r=
#define e03 iQ(1)))
#define cZ3 .second
#define cY3 ]cZ3
#define cX3 ].first
#define cW3 Ne_Mask
#define cV3 Gt_Mask
#define cU3 Lt_Mask
#define cT3 FindPos
#define cS3 (lR1 a),
#define cR3 public:
#define cQ3 result
#define cP3 cQ3[n]
#define cO3 cQ3 yQ
#define cN3 }lO static
#define cM3 cQ3))iF1
#define cL3 cQ3(
#define cK3 yI tY&b
#define cJ3 ++a)if(
#define cI3 switch(
#define cH3 cM tJ3)
#define cG3 cM yB1
#define cF3 middle2
#define cE3 )const
#define cD3 (cE3{
#define cC3 (value
#define cB3 pclone
#define cA3 fpdata
#define c93 Immeds
#define c83 CalculateGroupFunction
#define c73 ,cD2 cC2 start_at,y93
#define c63 ,tree
#define c53 b.Value)
#define c43 b.Opcode)
#define c33 .SetParamsMove(
#define c23 x5(),x91
#define c13 0x1},{
#define c03 newpow
#define yZ3 yI tW&
#define yY3 change
#define yX3 (count
#define yW3 133,2,
#define yV3 Params
#define yU3 Needs
#define yT3 byteCode
#define yS3 cQ3;}
#define yR3 AddFrom(
#define yQ3 cLog2by
#define yP3 factor_t
#define yO3 value1
#define yN3 ))nT iS1
#define yM3 >(iQ(1),
#define yL3 )l23::
#define yK3 model);
#define yJ3 has_min)
#define yI3 {iQ tmp=
#define yH3 x5());tC
#define yG3 iO ifp2
#define yF3 ,nU 1,
#define yE3 nR[++IP]
#define yD3 .eK1 n]
#define yC3 .what nO1
#define yB3 )yK1){iQ
#define yA3 cLog);n3
#define y93 t6 info
#define y83 Ge0Lt1
#define y73 1),iQ(1));
#define y63 yS3 case
#define y53 cQ3.max
#define y43 cQ3.min
#define y33 =y3 a));if(
#define y23 iG cQ3
#define y13 if(&*tX){
#define y03 y7 cMul);
#define xZ3 (*this c63
#define xY3 ,nZ1);lB
#define xX3 ){if(
#define xW3 default_function_handling
#define xV3 n6 2,cAdd);
#define xU3 n6 1,
#define xT3 .empty()
#define xS3 stack e72)
#define xR3 stack[xS3-
#define xQ3 stack yX
#define xP3 )){n5 iQ(
#define xO3 ),child);
#define xN3 ;range.n52
#define xM3 lN 0));
#define xL3 nR[IP]==
#define xK3 opcodes
#define xJ3 did_muli
#define xI3 y7 data.
#define xH3 =GetParam(
#define xG3 ;if(half
#define xF3 break;}}
#define xE3 *xB)[a].
#define xD3 yC<bool>
#define xC3 e1(),yC<
#define xB3 sizeof(
#define xA3 cAbsIf,
#define x93 225496,
#define x83 x73 tL
#define x73 456911
#define x63 444,{3,
#define x53 Gt0Le1
#define x43 402,{3,
#define x33 398,{3,
#define x23 0x12 y4
#define x13 |lR 0,
#define x03 cI3 n02.first tV2
#define nZ3 param.
#define nY3 lC1;case
#define nX3 DumpTree
#define nW3 &&p.max
#define nV3 cAdd i01
#define nU3 (op1==
#define nT3 lR3 n91
#define nS3 cAbsIf)
#define nR3 ,leaf1 lN
#define nQ3 Become(
#define nP3 iterator
#define nO3 parent
#define nN3 insert(i
#define nM3 lO bool
#define nL3 newrel
#define nK3 b_needed
#define nJ3 cachepos
#define nI3 half=
#define nH3 ,tS1&i6
#define nG3 src_pos
#define nF3 xS2|eS1)
#define nE3 reserve(
#define nD3 lO void
#define nC3 treeptr
#define nB3 }case lA2
#define nA3 .begin();
#define n93 .resize(
#define n83 };enum
#define n73 eF1 void
#define n63 ImmedTag
#define n53 lO class
#define n43 a,const
#define n33 RefCount
#define n23 :l9 m.eB1
#define n13 (half&63)-1;
#define n03 Birth();
#define lZ3 typename
#define lY3 mulgroup
#define lX3 return
#define lW3 unsigned
#define lV3 cost_t
#define lU3 middle
#define lT3 ,l0 2,
#define lS3 info l83
#define lR3 ;tree.
#define lQ3 sqrt_cost
#define lP3 const int
#define lO3 lE1 tmp2)
#define lN3 cQ3=
#define lM3 mul_count
#define lL3 maxValue1
#define lK3 minValue1
#define lJ3 maxValue0
#define lI3 minValue0
#define lH3 ValueType
#define lG3 abs_mul
#define lF3 lN a));
#define lE3 pos_set
#define lD3 nD1);}if(
#define lC3 cI3 tB
#define lB3 subtree
#define lA3 invtree
#define l93 specs;if(r.found){
#define l83 =(*xB)[
#define l73 rulenumit
#define l63 cHypot,l0
#define l53 yV3.
#define l43 MakeEqual
#define l33 lR3 eI1
#define l23 ;nZ l5
#define l13 nA1,l5::
#define l03 e83.lT
#define iZ2 (cQ3.
#define iY2 nA1,{l5::
#define iX2 newbase
#define iW2 branch1op
#define iV2 branch2op
#define iU2 (cJ lN iS
#define iT2 overlap
#define iS2 truth_b
#define iR2 truth_a
#define iQ2 found_dup
#define iP2 );lB yU
#define iO2 1,l91+1);
#define iN2 goto cX
#define iM2 constvalue
#define iL2 .l22 nW3
#define iK2 1,2,1,4,1,2,
#define iJ2 Plan_Has(
#define iI2 StackMax)
#define iH2 ;}void
#define iG2 +1)iH2
#define iF2 namespace
#define iE2 template
#define iD2 iE2<
#define iC2 inverted
#define iB2 IsNever:
#define iA2 m.has_min
#define i92 >=iQ(0))
#define i82 model.
#define i72 iftree
#define i62 }cI3
#define i52 depcodes
#define i42 tree iK)
#define i32 explicit
#define i22 cPow,tA1
#define i12 :start_at()
#define i02 const xI
#define tZ2 VarBegin
#define tY2 ].data);
#define tX2 y1)));nZ
#define tW2 lL1 tW
#define tV2 ){case
#define tU2 lR1 1)
#define tT2 begin(),
#define tS2 cond_add
#define tR2 cond_mul
#define tQ2 cond_and
#define tP2 if(op==
#define tO2 const std::eL
#define tN2 IsLogicalValue(lR1
#define tM2 const tW
#define tL2 bool cL1
#define tK2 exponent
#define tJ2 costree
#define tI2 sintree
#define tH2 leaf_count
#define tG2 sub_params
#define tF2 nD cLog2&&
#define tE2 printf(
#define tD2 cbrt_count
#define tC2 sqrt_count
#define tB2 tK2);
#define tA2 PlusInf
#define t92 Finite
#define t82 p1.eO1 p1)
#define t72 p1 iO ifp1
#define t62 pcall_tree
#define t52 ;sim.Push(
#define t42 after_powi
#define t32 GetHash().
#define t22 info=info;
#define t12 ;tC lB
#define t02 (nZ3
#define eZ2 grammar
#define eY2 cCos,l2
#define eX2 xA1 1,
#define eW2 ,cPow,l1
#define eV2 e21 0x1 y4
#define eU2 0x12},{{3,
#define eT2 cNeg,xO 1,
#define eS2 std::move(
#define eR2 MakeNEqual
#define eQ2 Dump(std::
#define eP2 isInteger(
#define eO2 yI iQ&value
#define eN2 lD1.SubTrees
#define eM2 lD1.Others
#define eL2 }break;}case
#define eK2 *)n02 cZ3;
#define eJ2 param=*yI
#define eI2 cZ3)
#define eH2 tW&tree){
#define eG2 .n_int_sqrt
#define eF2 ){tW r;r y7
#define eE2 tmp lE1 tree);
#define eD2 lR3 SetParam(
#define eC2 {tW tmp;tmp y7
#define eB2 (p0.l22&&p0.max tQ
#define eA2 (p0.n32 p0.min>=0.0)
#define e92 for cB1 a=y6 a-->0;)
#define e82 );tmp2.nK
#define e72 .size(
#define e62 if(fp_equal(
#define e52 );for cB1 a=
#define e42 Suboptimal
#define e32 break;i62
#define e22 e72);++
#define e12 Comparison
#define e02 needs_flip
#define cZ2 value]
#define cY2 {l5::yK,{l5
#define cX2 );nR[xN2.ofs
#define cW2 ~size_t(0)
#define cV2 ,yI void*)&
#define cU2 constraints=
#define cT2 ||(cond.cB yG2
#define cS2 .constraints
#define cR2 ,o);o<<"\n";
#define cQ2 Rule&rule,
#define cP2 tree))c9
#define cO2 (leaf1 lN
#define cN2 if(iP Find(
#define cM2 }data;data.
#define cL2 (lY3
#define cK2 info.lS[b].
#define cJ2 ,l7 x23
#define cI2 for(lZ3
#define cH2 );break;}
#define cG2 <nO yH;++a)
#define cF2 else{xB=new
#define cE2 tree.GetHash()
#define cD2 tM2&tree
#define cC2 ,const cV&
#define cB2 >::res,b8<
#define cA2 n92 true;
#define c92 mul_item
#define c82 innersub
#define c72 ;t1=!t1;}
#define c62 cbrt_cost
#define c52 best_cost
#define c42 lC1 lI
#define c32 fp_mod(m.
#define c22 nD lQ2){tree x1
#define c12 nD cNot||
#define c02 IsAlways
#define yZ2 condition
#define yY2 TopLevel)
#define yX2 per_item
#define yW2 item_type
#define yV2 first2
#define yU2 tL 386426
#define yT2 Decision
#define yS2 not_tree
#define yR2 group_by
#define yQ2 ->second
#define yP2 iP DoDup(
#define yO2 targetpos
#define yN2 eat_count
#define yM2 ParamSpec
#define yL2 ,lW3
#define yK2 rhs.hash2;}
#define yJ2 rhs.hash1
#define yI2 struct
#define yH2 Forget()
#define yG2 &&cond eA))
#define yF2 source_tree
#define yE2 nD cPow&&tE
#define yD2 <eD,lV3>
#define yC2 i1 tK2
#define yB2 .swap(tmp);
#define yA2 p1_evenness
#define y92 isNegative(
#define y82 ,std::cout)
#define y72 neg_set
#define y62 StackTopIs(
#define y52 cNop,cNop}}
#define y42 cTanh,cNop,
#define y32 ;i6.Remember(
#define y22 :{xW1.hash1
#define y12 matches
#define y02 cSin,l2
#define xZ2 cTan x4
#define xY2 ,cLog x4
#define xX2 cCos x4
#define xW2 tU2 yK1&&
#define xV2 tree.nS if(
#define xU2 negated
#define xT2 ,eN,synth);
#define xS2 nR yX yE
#define xR2 ,bool abs){
#define xQ2 ;lW3
#define xP2 :8 xQ2
#define xO2 factor
#define xN2 ifdata
#define xM2 best_score
#define xL2 mulvalue
#define xK2 pow_item
#define xJ2 PowiResult
#define xI2 maxValue
#define xH2 minValue
#define xG2 fp_min(x9,
#define xF2 set_min_max(
#define xE2 has_min=
#define xD2 div_tree
#define xC2 ;else cQ3.
#define xB2 pow_tree
#define xA2 preserve
#define x92 tree nD
#define x82 dup_or_fetch
#define x72 nominator]
#define x62 test_order
#define x52 shift(index)
#define x42 rulenumber
#define x32 AnyParams,l4
#define x22 ,{1,209,
#define x12 cLessOrEq,
#define x02 cTanh x4
#define nZ2 cMul,xO 2,
#define nY2 cInv,xO 1,
#define nX2 GetDepth()
#define nW2 lE GetParam(
#define nV2 factor_immed
#define nU2 changes
#define nT2 iO leaf2 lN
#define nS2 iO leaf1 lN
#define nR2 iO cond lN
#define nQ2 exp_diff
#define nP2 ExponentInfo
#define nO2 lower_bound(
#define nN2 is_logical
#define nM2 newrel_and
#define nL2 c02;if(
#define nK2 res_stackpos
#define nJ2 half_pos
#define nI2 .match_tree
#define nH2 );tK3 n6 2,
#define nG2 (tK2
#define nF2 nF OPCODE
#define nE2 yC<tY>&
#define nD2 yC x5&immed,
#define nC2 fphash_t
#define nB2 >>1)):(
#define nA2 CodeTreeData
#define n92 )lX3
#define n82 <lW3>&lN2
#define n72 ;xW1.hash2+=
#define n62 cB1 a=0;a<
#define n52 multiply(
#define n42 ;tK2
#define n32 has_min&&
#define n22 var_trees
#define n12 cOr,lQ 2,
#define n02 parampair
#define lZ2 lO inline
#define lY2 (xO2
#define lX2 second cZ3;
#define lW2 second.first;
#define lV2 {cV start_at;
#define lU2 )cM lY3)
#define lT2 changed=true;
#define lS2 log2_exponent
#define lR2 yI iQ&v,iQ(lW
#define lQ2 cAbsNot
#define lP2 .PullResult()
#define lO2 dup_fetch_pos
#define lN2 nR,size_t&l51
#define lM2 :iP PushImmed(
#define lL2 Rehash(false)
#define lK2 GetOpcode()!=
#define lJ2 *)&*start_at;
#define lI2 cPow,l2
#define lH2 ){iQ tK2=
#define lG2 ;}static y61
#define lF2 ):Value(Value::
#define lE2 ,lL 0 cR 2,
#define lD2 0x4 y4 6144,
#define lC2 cSin x4
#define lB2 Value_EvenInt
#define lA2 SubFunction:{
#define l92 ParamHolder:{
#define l82 MakeFalse,{l5
#define l72 if(list.first
#define l62 AddCollection
#define l52 ConditionType
#define l42 ),lU 3*
#define l32 yI iQ&v,iQ(lH
#define l22 has_max
#define l12 AddOperation(
#define l02 SpecialOpcode
#define iZ1 ByteCodeSynth
#define iY1 synth_it
#define iX1 fp_max(x9);
#define iW1 ;n6 2,cPow);
#define iV1 lJ1 y6++a){
#define iU1 assimilated
#define iT1 fraction
#define iS1 lO nC tY
#define iR1 DUP_BOTH();
#define iQ1 -1-offset].
#define iP1 IsDescendantOf
#define iO1 parent_opcode)
#define iN1 TreeCounts
#define iM1 bool t1 e23
#define iL1 SetOpcode(
#define iK1 found_log2
#define iJ1 div_params
#define iI1 case c02:
#define iH1 immed_sum
#define iG1 OPCODE(opcode)
#define iF1 break;cQ3*=
#define iE1 FactorStack x5
#define iD1 AnyParams,0 cQ
#define iC1 238799 tL
#define iB1 cAnd,lQ 2,
#define iA1 cNot x4
#define i91 l0 2,6144,
#define i81 DumpHashesFrom
#define i71 replacing_slot
#define i61 RefParams
#define i51 if_always[
#define i41 WhatDoWhenCase
#define i31 exponent_immed
#define i21 new_base_immed
#define i11 base_immed
#define i01 ||op1==
#define tZ1 data[a cY3
#define tY1 if(newrel_or==
#define tX1 MinMaxTree
#define tW1 ;changed_if
#define tV1 DUP_ONE(apos);
#define tU1 flipped
#define tT1 .UseGetNeeded(
#define tS1 PowiCache
#define tR1 e2 2,131,
#define tQ1 lC1 true;}
#define tP1 OptimizedUsing
#define tO1 Var_or_Funcno
#define tN1 tO1;
#define tM1 GetParams(
#define tL1 crc32_t
#define tK1 signed_chain
#define tJ1 MinusInf
#define tI1 n_immeds
#define tH1 FindClone(xK
#define tG1 denominator]
#define tF1 needs_rehash
#define tE1 AnyWhere_Rec
#define tD1 minimum_need
#define tC1 ~lW3(0)
#define tB1 45,46,47,48,
#define tA1 l1 162816 tL
#define t91 constraints&
#define t81 p1_logical_b
#define t71 p0_logical_b
#define t61 p1_logical_a
#define t51 p0_logical_a
#define t41 else if(
#define t31 cache_needed
#define t21 e2 2,1,e2 2,
#define t11 treelist
#define t01 const yC<tW>
#define eZ1 >::Optimize(){}
#define eY1 has_bad_balance
#define eX1 yP3 xO2
#define eW1 static const iG
#define eV1 true;min=iQ(0);
#define eU1 AddParamMove n0
#define eT1 std::cout<<"POP "
#define eS1 (lW3
#define eR1 for eS1 n=
#define eQ1 Value_IsInteger
#define eP1 tP1(
#define eO1 Rehash()c7
#define eN1 eO1 r);}
#define eM1 reltype
#define eL1 SequenceOpcodes
#define eK1 sep_list[
#define eJ1 },0,0x4},{{1,
#define eI1 DelParam(
#define eH1 ContainsOtherCandidates
#define eG1 x5(),0},{
#define eF1 i61);
#define eE1 },0,0x0},{{
#define eD1 TreeCountItem
#define eC1 IsDefined()){
#define eB1 set_min(fp_floor
#define eA1 pihalf_limits
#define e91 tX1<nJ
#define e81 ParsePowiMuli
#define e71 MaxChildDepth
#define e61 repl_param_list,
#define e51 e31;if(&*start_at){xB=(
#define e41 ;for(e31=0;a cG2
#define e31 lW3 a
#define e21 ,cPow,l7
#define e11 std::pair<It,It>
#define e01 cNotNot x4
#define cZ1 Value_Logical
#define cY1 new_factor_immed
#define cX1 y7 leaf1.e7 tree.
#define cW1 y7 tree.e7
#define cV1 occurance_pos
#define cU1 exponent_hash
#define cT1 exponent_list
#define cS1 CollectionSet x5
#define cR1 CollectMulGroup(
#define cQ1 source_set
#define cP1 tK2,TreeSet
#define cO1 pair<iQ,TreeSet>
#define cN1 TriTruthValue
#define cM1 produce_count
#define cL1 operator
#define cK1 lL1 tG2 yX
#define cJ1 if(iP FindAndDup(
#define cI1 retry_anyparams_3
#define cH1 retry_anyparams_2
#define cG1 needlist_cached_t
#define cF1 grammar_rules[*r]
#define cE1 nA 2 eE1 1,
#define cD1 yG,std::ostream&o
#define cC1 <<std::dec<<")";}
#define cB1 (size_t
#define cA1 Oneness_NotOne
#define c91 for cB1 b=0;b<
#define c81 by_float_exponent
#define c71 new_exp
#define c61 CodeTreeImmed(iQ(
#define c51 end()&&i->first==
#define c41 lX3 BecomeZero;
#define c31 lX3 BecomeOne;
#define c21 if(lS e72)<=lY)
#define c11 MakesInteger
#define c01 )cQ3.
#define yZ1 tO1)
#define yY1 branch1_backup
#define yX1 branch2_backup
#define yW1 exponent_map
#define yV1 plain_set
#define yU1 .back().thenbranch
#define yT1 );iP l12
#define yS1 ());n6 2 eL3 lB
#define yR1 );xK c33
#define yQ1 LightWeight(
#define yP1 ,i6 xT2
#define yO1 if cC3
#define yN1 set_max(fp_ceil cD
#define yM1 {iD2
#define yL1 0.5)iW1 lB
#define yK1 .IsImmed()
#define yJ1 should_regenerate=true;
#define yI1 should_regenerate,
#define yH1 Collection
#define yG1 RelationshipResult
#define yF1 Subdivide_Combine(
#define yE1 iP nP 1
#define yD1 long value
#define yC1 PositionalParams,0
#define yB1 subgroup
#define yA1 best_sep_factor
#define y91 SynthesizeParam
#define y81 needlist_cached
#define y71 lW3 index){lX3
#define y61 inline lW3
#define y51 cLess,l1
#define y41 opcode,bool pad
#define y31 n_occurrences
#define y21 found_log2by
#define y11 eI1 a);}
#define y01 best_sep_cost
#define xZ1 MultiplicationRange
#define xY1 if(keep_powi)
#define xX1 n_stacked
#define xW1 NewHash
#define xV1 AnyParams_Rec
#define xU1 continue;
#define xT1 nQ3 value lN 0))
#define xS1 =comp.AddItem(atree
#define xR1 lX3 false;
#define xQ1 needs_sincos
#define xP1 Recheck_RefCount_Div
#define xO1 Recheck_RefCount_Mul
#define xN1 divgroup
#define xM1 lY3.
#define xL1 lY3;lY3 y7
#define xK1 t41!cQ3.
#define xJ1 covers_plus1
#define xI1 iG1);
#define xH1 AssembleSequence
#define xG1 grammar_func
#define xF1 :public e1,public yC<
#define xE1 =yE|lW3(nR e72)
#define xD1 xR1}
#define xC1 tree t83()
#define xB1 212414 tL 24791,
#define xA1 ,l7 0x4},{{
#define x91 Modulo_Radians},
#define x81 ,lZ3 nC
#define x71 lE GetOpcode()==
#define x61 PositionType
#define x51 CollectionResult
#define x41 const_offset
#define x31 stacktop_desired
#define x21 StackTop
#define x11 cEqual,l1
#define x01 cIf,l0 3,
#define nZ1 cond_type
#define nY1 fphash_value_t
#define nX1 lZ3 i0::nP3
#define nW1 Recheck_RefCount_RDiv
#define nV1 ParamSpec_Extract x5(
#define nU1 DataP slot_holder(xF[
#define nT1 fPExponentIsTooLarge(
#define nS1 CollectMulGroup_Item(
#define nR1 covers_full_cycle
#define nQ1 212201 tL 225495,
#define nP1 },{l5::MakeNotP1,l5::
#define nO1 !=xG)if(TestCase(
#define nN1 &&IsLogicalValue(
#define nM1 std::pair<T1,T2>&
#define nL1 iD2 lZ3
#define nK1 has_good_balance_found
#define nJ1 found_log2_on_exponent
#define nI1 covers_minus1
#define nH1 needs_resynth
#define nG1 immed_product
#define nF1 ,2,1)nV if(found[data.
#define nE1 GetPositivityInfo(tree
#define nD1 tree.eI1 a
#define nC1 cNEqual,l1
#define nB1 Sign_Positive
#define nA1 ::MakeTrue
#define n91 SetParamMove(
#define n81 yC<lW3>&yT3,
#define n71 cE3{lX3
#define n61 rhs n71 hash1
#define n51 opposite=
#define n41 Constness_Const
#define n31 lC1 e42;
#define n21 MatchResultType
#define n11 resulting_exponent
#define n01 Unknown:default:;}
#define lZ1 inverse_nominator]
#define lY1 nY1 key
#define lX1 ;if(yY2 info.SaveMatchedParamIndex(
#define lW1 &1)?(poly^(
#define lV1 y7 cLog);tree y03
#define lU1 ,nR,IP,limit,xZ,stack);
#define lT1 SetParams(tM1));
#define lS1 o<<"("<<std::hex<<data.
#define lR1 tree lN
#define lQ1 n_as_tan_param
#define lP1 changed_exponent
#define lO1 MultiplyAndMakeLong
#define lN1 retry_positionalparams_2
#define lM1 119,120,121,122,
#define lL1 .Rehash();
#define lK1 for cB1 a=GetParamCount();a
#define lJ1 ;for n62
#define lI1 {l5::MakeNotP0,l5::
#define lH1 CopyOnWrite();
#define lG1 PositionalParams,l4
#define lF1 ,cIf,l2
#define lE1 .AddParamMove(
#define lD1 NeedList
#define lC1 ;lX3
#define lB1 IsNever lC1 Unknown;}
#define lA1 iZ1 x5&synth){
#define l91 recursioncount
#define l81 PlanNtimesCache(
#define l71 FPoptimizer_Grammar
#define l61 ;const eD1&occ=
#define l51 IP,size_t limit,size_t xZ
#define l41 yP2 found[data.
#define l31 l12 cInv,1,1)nV}
#define l21 !=IsNever)xR1 lB
#define l11 AddParamMove(lR1 0));
#define l01 CalculateResultBoundaries
#define iZ ParamSpec_SubFunctionData
#define iY inverse_denominator]
#define iX PositionalParams_Rec
#define iW =e33(root lN
#define iV DumpTreeWithIndent(*this);
#define iU cI3 type tV2 cond_or:
#define iT ;n91 0,tB2 eI1 1);
#define iS a).IsIdenticalTo(
#define iR ;tmp y03 tmp.nK 0));tmp
#define iQ Value_t
#define iP synth.
#define iO .AddParam(
#define iN tree yK1)xR1
#define iM )!=c02)xR1 lB
#define iL iJ std::endl;DumpHashes(
#define iK .GetParamCount()
#define iJ ;std::cout<<
#define iI {l5::MakeNotNotP1,l5::
#define iH {l5::MakeNotNotP0,l5::
#define iG tX1 x5
#define iF edited_powgroup
#define iE has_unknown_max
#define iD has_unknown_min
#define iC l22 e23
#define iB ,cAdd,SelectedParams,0},0,
#define iA 1.5)*fp_const_pi x5()
#define i9 synthed_tree
#define i8 6144 tL 260348,
#define i7 collections
#define i6 cache
#define i5 i32 nA2
#define i4 }inline
#define i3 !=xG n92 i51
#define i2 c81.data
#define i1 ,(long double)
#define i0 TreeCountType x5
#define tZ matched_params
#define tY CodeTree
#define tX (xE3 start_at
#define tW tY x5
#define tV needs_cow){lH1 goto
#define tU +2]=yE|lW3(Immed e72));
#define tT ;bool needs_cow=GetRefCount()>1;
#define tS MakeFalse,l5::
#define tR ].relationship
#define tQ <=fp_const_negativezero x5())
#define tP GetParam(a)
#define tO by_exponent
#define tN .GetImmed()
#define tM xR1 e32 bitmask&
#define tL ,{2,
#define tK cOr,x32
#define tJ 457728 tL 27911,
#define tI 347441 tL 24833,
#define tH 455790 tL 24697,
#define tG [lY cX3=true;lS[lY cY3
#define tF l71::Grammar*
#define tE powgroup lN
#define tD }lO iD2 lW3 Compare>nQ
#define tC lX3;}
#define tB GetLogicalValue(lR1
#define tA cW2&&found[data.
#define t9 c61(
#define t8 has_mulgroups_remaining
#define t7 iZ&params
#define t6 MatchInfo x5&
#define t5 RootPowerTable x5::RootPowers[
#define t4 MatchPositionSpec_AnyParams x5
#define t3 iD2 lW3 Compare>void
#define t2 iF2 FPoptimizer_ByteCode
#define t1 is_signed
#define t0 result_positivity
#define eZ biggest_minimum
#define eY nY1(
#define eX cAnd,AnyParams,
#define eW 314674 tL 266497,
#define eV 113774 tL 120949,
#define eU (tM1));lY3 lL1
#define eT GetParamCount();a-->0;)if(
#define eS 127086 tL 126073,
#define eR lL 2 eE1 1,
#define eQ cond_tree
#define eP else_tree
#define eO then_tree
#define eN sequencing
#define eM StackState
#define eL string FP_GetOpcodeName(
#define eK i32 tY
#define eJ if_stack
#define eI {lX3 tW(
#define eH void OutFloatHex(std::ostream&o,
#define eG n_as_sin_param
#define eF n_as_cos_param
#define eE PowiResolver::
#define eD int_exponent_t
#define eC ];};extern"C"{
#define eB cGreater,l1
#define eA .BalanceGood
#define e9 valueType
#define e8 back().endif_location
#define e7 GetOpcode());
#define e6 AddParamMove(mul);
#define e5 goto ReplaceTreeWithOne;case
#define e4 :goto ReplaceTreeWithZero;case
#define e3 ;p2.eO1 p2);tree y7 i72.e7 c9}
#define e2 130,1,
#define e1 MatchPositionSpecBase
#define e0 smallest_maximum
#define cZ iD2>void FunctionParserBase<
#define cY ++IP;xU1}if(xL3 xK3.
#define cX ReplaceTreeWithParam0;
#define cW factor_needs_rehashing
#define cV MatchPositionSpecBaseP
#define cU n5 iQ(-y73
#define cT {AdoptChildrenWithSameOpcode(tree);
#define cS nV1 nO.param_list,
#define cR }},{ReplaceParams,false,
#define cQ }},{ProduceNewTree,false,1,
#define cP data.subfunc_opcode
#define cO }l72 tN==iQ(
#define cN otherhalf
#define cM ;AddParamMove(
#define cL x12 l1
#define cK map<nC2,std::set<std::string> >
#define cJ branch1
#define cI const SequenceOpCode x5
#define cH =fp_cosh(m.min);m.max=fp_cosh(m.max);
#define cG MatchPositionSpec_PositionalParams x5
#define cF T1,lZ3 T2>inline tL2()
#define cE best_factor
#define cD )lC1 m;}case
#define cC branch2
#define cB FoundChild
#define cA CalculatePowiFactorCost(
#define c9 goto redo;
#define c8 ;nX3(tree)iJ"\n";
#define c7 ;tree lE1
#define c6 AddParamMove(comp.yV1[a].value);
#define c5 has_nonlogical_values
#define c4 from_logical_context)
#define c3 l3 2,
#define c2 POWI_CACHE_SIZE
#define c1 l12 GetOpcode(),
#define c0 static inline tW
#define yZ BalanceResultType
#define yY n33(0),Opcode(
#define yX .push_back(
#define yW const{lX3 data->
#define yV iO CodeTreeImmed(
#define yU ComparisonSetBase::
#define yT +=fp_const_twopi x5();
#define yS (keep_powi){xU3 cInv cH2 n3-1)iW1 lB
#define yR for n62 GetParamCount();++a xX3
#define yQ +=cQ3 lC1 yS3 lZ2 iQ
#define yP MatchPositionSpec_AnyWhere
#define yO if t02 data.match_type==
#define yN }PACKED_GRAMMAR_ATTRIBUTE;
#define yM b;}};iD2>yI2 Comp<
#define yL ParamSpec_NumConstant x5
#define yK xG,l5::Never},{l5::xG,l5::Never}}
#define yJ relationships
#define yI (const
#define yH .param_count
#define yG yZ3 tree
#define yF AssembleSequence_Subdivide(
#define yE 0x80000000u
#define yD (lE GetImmed()
#define yC std::vector
#define yB for cB1 a=cJ iK;a-->0;)
#define yA fp_const_twopi x5());if(
#define y9 iE2 set_min_max_if<cGreater>(iQ(0),
#define y8 for n62 nO3 iK;cJ3
#define y7 .iL1
#define y6 tree iK;
#define y5 !=cW2){l41
#define y4 },{{2,
#define y3 l01(lR1
#define y2 y3 0));iG
#define y1 tU2 tN
#define y0 x8&&tU2 yK1
#define xZ factor_stack_base
#define xY for(l73 r=range.first;r!=range cZ3;++r){
#define xX {iN1.erase(i);xU1}
#define xW ,iQ(-1))xX3 tV
#define xV cGreaterOrEq,l1
#define xU ));tmp y7 cInv);tmp lO3 lC1
#define xT n62 y6 cJ3 remaining[a])
#define xS nA 1 eE1
#define xR paramholder_matches
#define xQ cD2,std::ostream&o=std::cout
#define xP IsIdenticalTo(leaf2 lN
#define xO GroupFunction,0},lR{
#define xN SetStackTop(x21
#define xM FPOPT_autoptr
#define xL int_exponent
#define xK newnode
#define xJ ParamSpec_SubFunction
#define xI ParamSpec_ParamHolder
#define xH has_highlevel_opcodes
#define xG Unchanged
#define xF data->yV3
#define xE best_selected_sep
#define xD 27,28,29,30,31,32,33,34,35,36,37,38,39,40,
#define xC fp_nequal(tmp,iQ(0)))lM iQ(1)/tmp);tC}lB
#define xB position
#define xA SwapLastTwoInStack();
#define x9 fp_sin(min),fp_sin(max))
#define x8 if(lE IsImmed()
#define x7 )){tree.FixIncompleteHashes();}
#define x6 <y6 cJ3 ApplyGrammar(eZ2,lR1 a),
#define x5 <iQ>
#define x4 ,l0 1,
#define x3 FPoptimizer_CodeTree::tW&tree
#define x2 TestImmedConstraints(param cS2 c63))xR1
#define x1 .SetParam(0,i72 xM3 tW p1;p1 y7
#define x0 paramholder_index
#define nZ lX3 true;case
#define nY occurance_counts
#define nX for n62 y6++a){iG
#define nW -->0;){tM2&powgroup=tP;if(powgroup
#define nV ;iP y62*this);tC
#define nU lL 1 eE1
#define nT {data->Recalculate_Hash_NoRecursion();}
#define nS AddParamMove(changed_if)tQ1}
#define nR ByteCode
#define nQ void iG::
#define nP GetStackTop()-
#define nO model_tree
#define nN yC<tW>&i61
#define nM lW3 c xQ2 char l[
#define nL tW tmp,tmp2;tmp2 y7
#define nK AddParam(lR1
#define nJ iQ>p y33 p.
#define nI :data(new nA2 x5(
#define nH nL1 Ref>inline void xM<Ref>::
#define nG 157,158,159,169,191,203,230,231,232,233,234,237,238,239,240,243,244,245,247,248}};}yI2
#define nF FUNCTIONPARSERTYPES::
#define nE using iF2 FUNCTIONPARSERTYPES;
#define nD t83()==
#define nC tW::
#define nB lC1 ConstantFolding_LogicCommon(tree,yU
#define nA cAdd,AnyParams,
#define n9 SynthesizeByteCode(synth);
#define n8 ,tO1(),yV3(eV3 eW3 1),eP1 0){}
#define n7 nA2 x5::nA2(
#define n6 sim.Eat(
#define n5 lX3 iG(
#define n4 GetIntegerInfo(lR1 0))==c02)iN2
#define n3 sim.AddConst(
#define n2 cPow,l7 0x4 y4
#define n1 while(ApplyGrammar(yI Grammar&)
#define n0 (pow lN 1));pow.eI1 1);pow lL1 tree.n91 0,pow);goto NowWeAreMulGroup;}
#define lZ DumpParams x5 t02 data.param_list,nZ3 data yH,o);
#define lY restholder_index
#define lX tW tK2 n42 y7 cMul)n42 iO
#define lW *const func)(iQ),iG model){
#define lV eC2 cPow);tmp.nK 0));tmp yV iQ(
#define lU iQ(1)/iQ(
#define lT SetParam(0,nW2 0))eD2 1,CodeTreeImmed(
#define lS restholder_matches
#define lR n41,0x0},{
#define lQ SelectedParams,0 eE1
#define lP :if(ParamComparer x5()(yV3[1],yV3[0])){std::swap(yV3[0],yV3[1]);Opcode=
#define lO nL1 iQ>
#define lN .GetParam(
#define lM {tree.ReplaceWithImmed(
#define lL cMul,AnyParams,
#define lK ,nA l4
#define lJ cMul,SelectedParams,0},0,
#define lI l01(tmp);}case
#define lH *const func)(iQ),iG model=iG());
#define lG cPow,l0 2
#define lF lZ3 iQ>tL2()yI iQ&n43 iQ&b){lX3 a
#define lE lR1 0).
#define lD ,lL l4
#define lC :yY3=comp.AddRelationship(atree lN 0),atree lN 1),yU
#define lB break;case
#define lA nD3 nC
#define l9 {iG m=y3 0));
#define l8 )?0:1));tW changed_if tW1 cW1 changed_if c33 tree.tM1))tW1 lL1 tree y7
#define l7 yC1},0,
#define l6 cAdd,lQ 2,
#define l5 RangeComparisonData
#define l4 0 cR 1,
#define l3 lJ 0x0},{{
#define l2 yC1 cQ
#define l1 yC1 cR 2,
#define l0 l7 0x0},{{
#ifdef _MSC_VER
typedef
lW3
int
tL1;
#else
#include <stdint.h>
typedef
uint_least32_t
tL1;
#endif
iF2
crc32{enum{startvalue=0xFFFFFFFFUL,poly=0xEDB88320UL}
;iD2
tL1
crc>yI2
b8{enum{b1=(crc
lW1
crc
nB2
crc>>1),b2=(b1
lW1
b1
nB2
b1>>1),b3=(b2
lW1
b2
nB2
b2>>1),b4=(b3
lW1
b3
nB2
b3>>1),b5=(b4
lW1
b4
nB2
b4>>1),b6=(b5
lW1
b5
nB2
b5>>1),b7=(b6
lW1
b6
nB2
b6>>1),res=(b7
lW1
b7
nB2
b7>>1)}
;}
;inline
tL1
update(tL1
crc
yL2
b){
#define B4(n) b8<n cB2 n+1 cB2 n+2 cB2 n+3>::res
#define R(n) B4(n),B4(n+4),B4(n+8),B4(n+12)
static
const
tL1
table[256]={R(0x00),R(0x10),R(0x20),R(0x30),R(0x40),R(0x50),R(0x60),R(0x70),R(0x80),R(0x90),R(0xA0),R(0xB0),R(0xC0),R(0xD0),R(0xE0),R(0xF0)}
;
#undef R
#undef B4
lX3((crc>>8))^table[(crc^b)&0xFF];i4
tL1
calc_upd(tL1
c,const
lW3
char*buf,size_t
size){tL1
value=c;for
cB1
p=0;p<size;++p)value=update
cC3,buf[p])lC1
value;i4
tL1
calc
yI
lW3
char*buf,size_t
size){lX3
calc_upd(startvalue,buf,size);}
}
#ifndef FPOptimizerAutoPtrHH
#define FPOptimizerAutoPtrHH
nL1
Ref>class
xM{cR3
xM():p(0){}
xM(Ref*b):p(b){n03}
xM
yI
xM&b):p(b.p){n03
i4
Ref&cL1*(n71*p;i4
Ref*cL1->(n71
p;}
xM&cL1=(Ref*b){Set(b)lC1*this;}
xM&cL1=yI
xM&b){Set(b.p)lC1*this;}
#ifdef __GXX_EXPERIMENTAL_CXX0X__
xM(xM&&b):p(b.p){b.p=0;}
xM&cL1=(xM&&b
xX3
p!=b.p){yH2;p=b.p;b.p=0;}
lX3*this;}
#endif
~xM(){yH2
iH2
UnsafeSetP(Ref*newp){p=newp
iH2
swap(xM<Ref>&b){Ref*tmp=p;p=b.p;b.p=tmp;}
private:inline
static
void
Have(Ref*p2);inline
void
yH2;inline
void
n03
inline
void
Set(Ref*p2);private:Ref*p;}
;nH
yH2{if(!p
n92;p->n33-=1;if(!p->n33)delete
p;}
nH
Have(Ref*p2
xX3
p2)++(p2->n33);}
nH
Birth(){Have(p);}
nH
Set(Ref*p2){Have(p2);yH2;p=p2;}
#endif
#include <utility>
yI2
Compare2ndRev{nL1
T>inline
tL2()yI
T&n43
T&b
n71
a
cZ3>b
cZ3;}
}
;yI2
Compare1st{nL1
cF
yI
nM1
n43
nM1
b
n71
a.first<b.first;}
nL1
cF
yI
nM1
a,T1
b
n71
a.first<b;}
nL1
cF(T1
n43
nM1
b
n71
a<b.first;}
}
;
#ifndef FPoptimizerHashHH
#define FPoptimizerHashHH
#ifdef _MSC_VER
typedef
lW3
long
long
nY1;
#define FPHASH_CONST(x) x##ULL
#else
#include <stdint.h>
typedef
uint_fast64_t
nY1;
#define FPHASH_CONST(x) x##ULL
#endif
iF2
FUNCTIONPARSERTYPES{yI2
nC2{nY1
hash1,hash2;nC2():hash1(0),hash2(0){}
nC2
yI
nY1&n43
nY1&b):hash1(a),hash2(b){}
tL2==yI
nC2&n61==yJ2&&hash2==yK2
tL2!=yI
nC2&n61!=yJ2||hash2!=yK2
tL2<yI
nC2&n61!=yJ2?hash1<yJ2:hash2<yK2}
;}
#endif
#ifndef FPOptimizer_CodeTreeHH
#define FPOptimizer_CodeTreeHH
#ifdef FP_SUPPORT_OPTIMIZER
#include <vector>
#include <utility>
iF2
l71{yI2
Grammar;}
t2{n53
iZ1;}
iF2
FPoptimizer_CodeTree{n53
tY;lO
yI2
nA2;n53
tY{typedef
xM<nA2
x5>DataP;DataP
data;cR3
tY();~tY();yI2
OpcodeTag{}
;eK(nF2
o,OpcodeTag);yI2
FuncOpcodeTag{}
;eK(nF2
o
yL2
f,FuncOpcodeTag);yI2
n63{}
;eK
yI
iQ&v,n63);
#ifdef __GXX_EXPERIMENTAL_CXX0X__
eK(iQ&&v,n63);
#endif
yI2
VarTag{}
;eK
eS1
varno,VarTag);yI2
CloneTag{}
;eK
cK3,CloneTag);void
GenerateFrom
yI
n81
const
nD2
const
lZ3
FunctionParserBase
x5::Data&data,bool
keep_powi=false);void
GenerateFrom
yI
n81
const
nD2
const
lZ3
FunctionParserBase
x5::Data&data,const
nE2
n22,bool
keep_powi=false);void
SynthesizeByteCode(n81
nD2
size_t&stacktop_max);void
SynthesizeByteCode(FPoptimizer_ByteCode::iZ1
x5&synth,bool
MustPopTemps=true
cE3
tW3
SynthCommonSubExpressions(FPoptimizer_ByteCode::iZ1
x5&synth
cE3;void
SetParams
yI
nE2
n73
SetParamsMove(nE2
eF1
tY
GetUniqueRef();
#ifdef __GXX_EXPERIMENTAL_CXX0X__
void
SetParams(yC<tY>&&eF1
#endif
void
SetParam
cB1
which,const
tY&b);void
n91
size_t
which,tY&b);void
AddParam
yI
tY&param);void
AddParamMove(tY&param);void
AddParams
yI
nE2
n73
AddParamsMove(nE2
n73
AddParamsMove(nE2
i61,size_t
i71);void
DelParam
cB1
index);void
DelParams();void
Become
cK3);inline
size_t
GetParamCount(n71
tM1)e72);i4
tY&GetParam
cB1
n){lX3
tM1)[n];i4
const
tY&GetParam
cB1
n
n71
tM1)[n];i4
void
iL1
nF2
o){data->Opcode=o;i4
nF2
GetOpcode()yW
Opcode;i4
nF
nC2
GetHash()yW
Hash;i4
const
nE2
tM1
n71
xF;i4
nE2
tM1){lX3
xF;i4
size_t
nX2
yW
Depth;i4
const
iQ&GetImmed()yW
Value;i4
lW3
GetVar()yW
tN1
i4
lW3
GetFuncNo()yW
tN1
i4
bool
IsDefined(n71
lK2
nF
cNop;i4
bool
IsImmed(n71
GetOpcode()==nF
cImmed;i4
bool
IsVar(n71
GetOpcode()==nF
tZ2;i4
lW3
GetRefCount()yW
n33
iH2
ReplaceWithImmed
yI
iQ&i);void
Rehash(bool
constantfolding=true);void
Sort();inline
void
Mark_Incompletely_Hashed(){data->Depth=0;i4
bool
Is_Incompletely_Hashed()yW
Depth==0;i4
const
tF
GetOptimizedUsing()yW
tP1;i4
void
SetOptimizedUsing
yI
tF
g){data->tP1=g;}
bool
RecreateInversionsAndNegations(bool
prefer_base2=false);void
FixIncompleteHashes();void
swap(tY&b){data.swap(b.data);}
bool
IsIdenticalTo
cK3
cE3;void
lH1}
;lO
yI2
nA2{int
n33;nF2
Opcode;iQ
Value
xQ2
tN1
yC<tW>yV3;nF
nC2
Hash
tW3
Depth;const
tF
tP1;nA2();nA2
yI
nA2&b);i5(nF2
o);i5(nF2
o
yL2
f);i5
yI
iQ&i);
#ifdef __GXX_EXPERIMENTAL_CXX0X__
i5(iQ&&i);nA2(nA2&&b);
#endif
bool
IsIdenticalTo
yI
nA2&b
cE3;void
Sort();void
Recalculate_Hash_NoRecursion();private:void
cL1=yI
nA2&b);}
;lO
c0
CodeTreeImmed
yI
iQ&i)eI
i
x81
n63());}
#ifdef __GXX_EXPERIMENTAL_CXX0X__
lO
c0
CodeTreeImmed(iQ&&i)eI
eS2
i)x81
n63());}
#endif
lO
c0
CodeTreeOp(nF2
opcode)eI
opcode
x81
OpcodeTag());}
lO
c0
CodeTreeFuncOp(nF2
opcode
yL2
f)eI
opcode,f
x81
FuncOpcodeTag());}
lO
c0
CodeTreeVar
eS1
varno)eI
varno
x81
VarTag());}
#ifdef FUNCTIONPARSER_SUPPORT_DEBUG_OUTPUT
nD3
DumpHashes(xQ);nD3
nX3(xQ);nD3
DumpTreeWithIndent(xQ,const
std::string&indent="\\"
);
#endif
}
#endif
#endif
#ifndef FPOPT_NAN_CONST
#include <iostream>
#define FPOPT_NAN_CONST (-1712345.25)
iF2
FPoptimizer_CodeTree{n53
tY;}
iF2
l71{enum
ImmedConstraint_Value{ValueMask=0x07,Value_AnyNum=0x0,lB2=0x1,Value_OddInt=0x2,eQ1=0x3,Value_NonInteger=0x4,cZ1=0x5
n83
ImmedConstraint_Sign{SignMask=0x18,Sign_AnySign=0x00,nB1=0x08,Sign_Negative=0x10,Sign_NoIdea=0x18
n83
ImmedConstraint_Oneness{OnenessMask=0x60,Oneness_Any=0x00,Oneness_One=0x20,cA1=0x40
n83
ImmedConstraint_Constness{ConstnessMask=0x80,Constness_Any=0x00,n41=0x80
n83
Modulo_Mode{Modulo_None=0,Modulo_Radians=1
n83
l02{NumConstant,ParamHolder,SubFunction
n83
ParamMatchingType{PositionalParams,SelectedParams,AnyParams,GroupFunction
n83
RuleType{ProduceNewTree,ReplaceParams}
;
#ifdef __GNUC__
# define PACKED_GRAMMAR_ATTRIBUTE __attribute__((packed))
#else
# define PACKED_GRAMMAR_ATTRIBUTE
#endif
typedef
std::pair<l02,const
void*>yM2;lO
yM2
ParamSpec_Extract
eS1
paramlist
yL2
index);nM3
ParamSpec_Compare
yI
void*n43
void*b,l02
type)xQ2
ParamSpec_GetDepCode
yI
yM2&b);yI2
xI{lW3
index
xP2
constraints
xP2
depcode:16;yN
lO
yI2
ParamSpec_NumConstant{iQ
iM2
xQ2
modulo;yN
yI2
iZ{lW3
param_count:2
xQ2
param_list:30;nF2
subfunc_opcode:8;ParamMatchingType
match_type:3
xQ2
lY:5;yN
yI2
xJ{iZ
data
xQ2
constraints
xP2
depcode:8;yN
yI2
Rule{RuleType
ruletype:2;bool
logical_context:1
xQ2
repl_param_count:2+13
xQ2
repl_param_list:30;iZ
match_tree;yN
yI2
Grammar{lW3
rule_count
xQ2
char
rule_list[999
eC
extern
const
Rule
grammar_rules[];}
nD3
DumpParam
yI
yM2&p,std::ostream&o=std::cout);nD3
DumpParams
eS1
paramlist
yL2
count,std::ostream&o=std::cout);}
#endif
#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif
#define CONSTANT_POS_INF HUGE_VAL
#define CONSTANT_NEG_INF (-HUGE_VAL)
iF2
FUNCTIONPARSERTYPES{lZ2
iQ
fp_const_pihalf(){lX3
fp_const_pi
x5()*iQ(0.5);}
lZ2
iQ
fp_const_twopi(){iQ
cL3
fp_const_pi
x5());cO3
fp_const_twoe(){iQ
cL3
fp_const_e
x5());cO3
fp_const_twoeinv(){iQ
cL3
fp_const_einv
x5());cO3
fp_const_negativezero(){
#ifdef FP_EPSILON
lX3-fp_epsilon
x5();
#else
lX3
iQ(-1e-14);
#endif
}
}
#ifdef FP_SUPPORT_OPTIMIZER
#include <vector>
#include <utility>
#include <iostream>
iF2
FPoptimizer_Optimize{using
iF2
l71;using
iF2
FPoptimizer_CodeTree;nE
n53
MatchInfo{cR3
yC<std::pair<bool,yC<tW> > >lS;yC<tW>xR;yC<lW3>tZ;cR3
MatchInfo():lS(),xR(),tZ(){}
cR3
bool
SaveOrTestRestHolder
eS1
lY,t01&t11){c21{lS
n93
lY+1);lS
tG=t11
tQ1
if(lS[lY
cX3
eX3){lS
tG=t11
tQ1
t01&found=lS[lY
cY3;if(t11
e72)!=found
e72)n92
false
lJ1
t11
e22
a)if(!t11[a].IsIdenticalTo(found[a])n92
false
tQ1
void
SaveRestHolder
eS1
lY,yC<tW>&t11){c21
lS
n93
lY+1);lS
tG.swap(t11);}
bool
SaveOrTestParamHolder
eS1
x0,tM2&nC3
xX3
xR
e72)<=x0){xR.nE3
x0+1);xR
n93
x0);xR
yX
nC3)tQ1
if(!xR[x0].eC1
xR[x0]=nC3
tQ1
lX3
nC3.IsIdenticalTo(xR[x0])iH2
SaveMatchedParamIndex
eS1
index){tZ
yX
index);}
tM2&GetParamHolderValueIfFound
eS1
x0
cE3{static
tM2
dummytree;if(xR
e72)<=x0
n92
dummytree
lC1
xR[x0];}
tM2&GetParamHolderValue
eS1
x0
n71
xR[x0];}
bool
HasRestHolder
eS1
lY
n71
lS
e72)>lY&&lS[lY
cX3==true;}
t01&GetRestHolderValues
eS1
lY
cE3{static
t01
empty_result;c21
lX3
empty_result
lC1
lS[lY
cY3;}
const
yC<lW3>&GetMatchedParamIndexes(n71
tZ
iH2
swap(t6
b){lS.swap(b.lS);xR.swap(b.xR);tZ.swap(b.tZ);}
t6
cL1=yI
t6
b){lS=b.lS;xR=b.xR;tZ=b.tZ
lC1*this;}
}
;class
e1;typedef
xM<e1>cV;class
e1{cR3
int
n33;cR3
e1():n33(0){}
virtual~e1(){}
}
;yI2
n21{bool
found;cV
specs;n21(bool
f):found(f),specs(){}
n21(bool
f
cC2
s):found(f),specs(s){}
}
;nD3
SynthesizeRule
yI
cQ2
tW&tree,y93);lO
n21
TestParam
yI
yM2&n02
c73);lO
n21
TestParams
yI
iZ&nO
c73,bool
yY2;nM3
ApplyGrammar
yI
Grammar&eZ2,x3,bool
from_logical_context=false);nD3
ApplyGrammars(x3);nM3
IsLogisticallyPlausibleParamsMatch
yI
t7,cD2);}
iF2
l71{nD3
DumpMatch
yI
cQ2
const
x3,const
FPoptimizer_Optimize::y93,bool
DidMatch,std::ostream&o=std::cout);nD3
DumpMatch
yI
cQ2
const
x3,const
FPoptimizer_Optimize::y93,const
char*eJ3,std::ostream&o=std::cout);}
#endif
#include <string>
tO2
l71::l02
y41=false);tO2
nF2
y41=false);
#include <string>
#include <sstream>
#include <assert.h>
#include <iostream>
using
iF2
l71;nE
tO2
l71::l02
y41){
#if 1
const
char*p=0;cI3
opcode
tV2
NumConstant:p="NumConstant"
;lB
ParamHolder:p="ParamHolder"
;lB
SubFunction:p="SubFunction"
;break;}
std::ostringstream
tmp;assert(p);tmp<<p;if(pad)while(tmp.str()e72)<12)tmp<<' 'lC1
tmp.str();
#else
std::ostringstream
tmp;tmp<<opcode;if(pad)while(tmp.str()e72)<5)tmp<<' 'lC1
tmp.str();
#endif
}
tO2
nF2
y41){
#if 1
const
char*p=0;cI3
opcode
tV2
cAbs:p="cAbs"
;lB
cAcos:p="cAcos"
;lB
cAcosh:p="cAcosh"
;lB
cAsin:p="cAsin"
;lB
cAsinh:p="cAsinh"
;lB
cAtan:p="cAtan"
;lB
cAtan2:p="cAtan2"
;lB
cAtanh:p="cAtanh"
;lB
cCbrt:p="cCbrt"
;lB
cCeil:p="cCeil"
;lB
cCos:p="cCos"
;lB
cCosh:p="cCosh"
;lB
cCot:p="cCot"
;lB
cCsc:p="cCsc"
;lB
cEval:p="cEval"
;lB
cExp:p="cExp"
;lB
cExp2:p="cExp2"
;lB
cFloor:p="cFloor"
;lB
cHypot:p="cHypot"
;lB
cIf:p="cIf"
;lB
cInt:p="cInt"
;lB
cLog:p="cLog"
;lB
cLog2:p="cLog2"
;lB
cLog10:p="cLog10"
;lB
cMax:p="cMax"
;lB
cMin:p="cMin"
;lB
cPow:p="cPow"
;lB
cSec:p="cSec"
;lB
cSin:p="cSin"
;lB
cSinh:p="cSinh"
;lB
cSqrt:p="cSqrt"
;lB
cTan:p="cTan"
;lB
cTanh:p="cTanh"
;lB
cTrunc:p="cTrunc"
;lB
cImmed:p="cImmed"
;lB
cJump:p="cJump"
;lB
cNeg:p="cNeg"
;lB
cAdd:p="cAdd"
;lB
cSub:p="cSub"
;lB
cMul:p="cMul"
;lB
cDiv:p="cDiv"
;lB
cMod:p="cMod"
;lB
cEqual:p="cEqual"
;lB
cNEqual:p="cNEqual"
;lB
cLess:p="cLess"
;lB
cLessOrEq:p="cLessOrEq"
;lB
cGreater:p="cGreater"
;lB
cGreaterOrEq:p="cGreaterOrEq"
;lB
cNot:p="cNot"
;lB
cAnd:p="cAnd"
;lB
cOr:p="cOr"
;lB
cDeg:p="cDeg"
;lB
cRad:p="cRad"
;lB
cFCall:p="cFCall"
;lB
cPCall:p="cPCall"
;break;
#ifdef FP_SUPPORT_OPTIMIZER
case
cFetch:p="cFetch"
;lB
cPopNMov:p="cPopNMov"
;lB
yQ3:p="cLog2by"
;lB
cSinCos:p="cSinCos"
;break;
#endif
case
lQ2:p="cAbsNot"
;lB
cAbsNotNot:p="cAbsNotNot"
;lB
cAbsAnd:p="cAbsAnd"
;lB
cAbsOr:p="cAbsOr"
;lB
cAbsIf:p="cAbsIf"
;lB
cDup:p="cDup"
;lB
cInv:p="cInv"
;lB
cSqr:p="cSqr"
;lB
cRDiv:p="cRDiv"
;lB
cRSub:p="cRSub"
;lB
cNotNot:p="cNotNot"
;lB
cRSqrt:p="cRSqrt"
;lB
cNop:p="cNop"
;lB
tZ2:p="VarBegin"
;break;}
std::ostringstream
tmp;assert(p);tmp<<p;if(pad)while(tmp.str()e72)<12)tmp<<' 'lC1
tmp.str();
#else
std::ostringstream
tmp;tmp<<opcode;if(pad)while(tmp.str()e72)<5)tmp<<' 'lC1
tmp.str();
#endif
}
#ifdef FP_SUPPORT_OPTIMIZER
#include <vector>
#include <utility>
#ifndef FP_GENERATING_POWI_TABLE
enum{MAX_POWI_BYTECODE_LENGTH=20}
;
#else
enum{MAX_POWI_BYTECODE_LENGTH=999}
;
#endif
enum{MAX_MULI_BYTECODE_LENGTH=3}
;t2{n53
iZ1{cR3
iZ1():nR(),Immed(),eM(),x21(0),StackMax(0){nR.nE3
64);Immed.nE3
8);eM.nE3
16)iH2
Pull(yC<lW3>&bc,yC
x5&imm,size_t&StackTop_max){for(e31=0;a<nR
e22
a){nR[a]&=~yE;}
nR.swap(bc);Immed.swap(imm);StackTop_max=StackMax;}
size_t
GetByteCodeSize(n71
nR
e72);}
size_t
GetStackTop(n71
x21
iH2
PushVar
eS1
varno){nR
yX
varno);xN
iG2
PushImmed(iQ
immed){nE
nR
yX
cImmed);Immed
yX
immed);xN
iG2
StackTopIs
yI
x3,int
offset=0
xX3(int)x21>offset){eM[x21
iQ1
first=true;eM[x21
iQ1
second=tree;}
}
bool
IsStackTop
yI
x3,int
offset=0
n71(int)x21>offset&&eM[x21
iQ1
first&&eM[x21
iQ1
second.IsIdenticalTo(tree);i4
void
EatNParams
eS1
yN2){x21-=yN2
iH2
ProducedNParams
eS1
cM1){xN+cM1)iH2
AddOperation
eS1
opcode
yL2
yN2
yL2
cM1=1){EatNParams(yN2);nE
AddFunctionOpcode(opcode);ProducedNParams(cM1)iH2
DoPopNMov
cB1
yO2,size_t
srcpos){nE
nR
yX
cPopNMov);nF3
yO2);nF3
srcpos);SetStackTop(srcpos+1);eM[yO2]=eM[srcpos];SetStackTop(yO2
iG2
DoDup
cB1
nG3){nE
if(nG3==x21-1){nR
yX
cDup);}
else{nR
yX
cFetch);nF3
nG3);}
xN+1);eM[x21-1]=eM[nG3];}
size_t
cT3
yI
x3
cE3{for
cB1
a=x21;a-->0;)if(eM[a
cX3&&eM[a
cY3.IsIdenticalTo(tree)n92
a
lC1
cW2;}
bool
Find
yI
x3
n71
cT3(tree)!=cW2;}
bool
FindAndDup
yI
x3){size_t
pos=cT3(tree);if(pos!=cW2){
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<tG3"duplicate at ["
<<pos<<"]: "
;nX3(tree)iJ" -- issuing cDup or cFetch\n"
;
#endif
DoDup(pos)tQ1
xD1
yI2
IfData{size_t
ofs;}
;void
SynthIfStep1(IfData&xN2,nF2
op){nE
xN-1);xN2.ofs=nR
e72);nR
yX
op);xS2);xS2)iH2
SynthIfStep2(IfData&xN2){nE
xN-1
cX2+1]xE1+2
cX2
tU
xN2.ofs=nR
e72);nR
yX
cJump);xS2);xS2)iH2
SynthIfStep3(IfData&xN2){nE
xN-1);nR.back()|=yE;nR[xN2.ofs+1]xE1-1
cX2
tU
xN+1)lJ1
xN2.ofs;++a
xX3
nR[a]==cJump&&nR[a+1]==(yE|(xN2.ofs-1))){nR[a+1]xE1-1);nR[a
tU
i62
nR[a]tV2
cAbsIf:case
cIf:case
cJump:case
cPopNMov:a+=2;lB
cFCall:case
cPCall:case
cFetch:a+=1;break;default:xF3}
protected:void
SetStackTop
cB1
value){x21=value;if(x21>iI2{StackMax=x21;eM
n93
iI2;}
}
void
AddFunctionOpcode
eS1
opcode);private:yC<lW3>nR;yC
x5
Immed;yC<std::pair<bool,FPoptimizer_CodeTree::tW> >eM
tW3
x21
tW3
StackMax;}
;lO
yI2
SequenceOpCode;lO
yI2
eL1{static
cI
AddSequence;static
cI
MulSequence;}
;nD3
xH1(long
count,cI&eN,iZ1
x5&synth);}
#endif
#ifdef FP_SUPPORT_OPTIMIZER
nE
t2{lO
yI2
SequenceOpCode{iQ
basevalue
xQ2
op_flip
xQ2
op_normal,op_normal_flip
xQ2
op_inverse,op_inverse_flip;}
;lO
cI
eL1
x5::AddSequence={0.0,cNeg,cAdd,cAdd,cSub,cRSub}
;lO
cI
eL1
x5::MulSequence={1.0,cInv,cMul,cMul,cDiv,cRDiv}
;nD3
iZ1
x5::AddFunctionOpcode
eS1
opcode){int
StackPtr=0;
#define incStackPtr() do{if(x21+2>iI2 eM n93 StackMax=x21+2);}while(0)
#define findName(a,b,c) "var"
#define TryCompilePowi(o) false
#define data this
# define FP_FLOAT_VERSION 1
# include "fp_opcode_add.h"
# undef FP_FLOAT_VERSION
#undef data
#undef TryCompilePowi
#undef incStackPtr
}
}
using
t2;
#define POWI_TABLE_SIZE 256
#define POWI_WINDOW_SIZE 3
t2{
#ifndef FP_GENERATING_POWI_TABLE
extern
const
lW3
char
powi_table[POWI_TABLE_SIZE];const
#endif
lW3
char
powi_table[POWI_TABLE_SIZE]={0,1,1,1,2,iK2
1,4,1,2,131,8,iK2
1,8,yW3
131,4,1,15,1,16,iK2
131,8,1,2,1,4,yW3
1,16,1,25,131,4,1,27,5,8,3,2,1,30,1,31,3,32,iK2
1,8,1,2,131,4,1,39,1,16,137,2,1,4,yW3
131,8,1,45,135,4,31,2,5,32,1,2,131,50,1,51,1,8,3,2,1,54,1,55,3,16,1,57,133,4,137,2,135,60,1,61,3,62,133,63,1,t21
131,t21
139,tR1
e2
30,1,130,137,2,31,tR1
e2
e2
130,yW3
1,e2
e2
2,1,130,133,t21
61,130,133,62,139,130,137,e2
tR1
e2
e2
t21
131,e2
e2
130,131,2,133,tR1
130,141,e2
130,yW3
1,e2
5,135,e2
tR1
e2
tR1
130,133,130,141,130,131,e2
e2
2,131}
;}
static
lP3
c2=256;
#define FPO(x)
iF2{class
tS1{private:int
i6[c2];int
t31[c2];cR3
tS1():i6(),t31(){i6[1]=1;}
bool
Plan_Add(yD1,int
count){yO1>=c2)xR1
t31[cZ2+=count
lC1
i6[cZ2!=0
iH2
iJ2
yD1){yO1<c2)i6[cZ2=1
iH2
Start
cB1
value1_pos){for(int
n=2;n<c2;++n)i6[n]=-1;Remember(1,value1_pos);DumpContents();}
int
Find(yD1
cE3{yO1<c2
xX3
i6[cZ2>=0){FPO(t23(t63,"* I found %ld from cache (%u,%d)\n",value,(unsigned)cache[value],t33 value]))lC1
i6[cZ2;}
}
lX3-1
iH2
Remember(yD1,size_t
tE3){yO1>=c2
n92;FPO(t23(t63,"* Remembering that %ld can be found at %u (%d uses remain)\n",value,(unsigned)tE3,t33 value]));i6[cZ2=(int)tE3
iH2
DumpContents
cD3
FPO(for(int a=1;a<POWI_CACHE_SIZE;++a)if(cache[a]>=0||t33 a]>0){t23(t63,"== cache: sp=%d, val=%d, needs=%d\n",cache[a],a,t33 a]);})}
int
UseGetNeeded(yD1){yO1>=0&&value<c2
n92--t31[cZ2
lC1
0;}
}
;lO
size_t
yF
long
count
nH3,cI&eN,iZ1
x5&synth);nD3
yF1
size_t
apos,long
aval,size_t
bpos,long
bval
nH3
yL2
cumulation_opcode
yL2
cimulation_opcode_flip,iZ1
x5&synth);void
l81
yD1
nH3,int
need_count,int
l91=0){yO1<1
n92;
#ifdef FP_GENERATING_POWI_TABLE
if(l91>32)throw
false;
#endif
if(i6.Plan_Add
cC3,need_count)n92;long
nI3
1;yO1<POWI_TABLE_SIZE){nI3
powi_table[cZ2
xG3&128){half&=127
xG3&64)nI3-n13
FPO(t23(t63,"value=%ld, half=%ld, otherhalf=%ld\n",value,half,value/half));l81
half,i6,iO2
i6.iJ2
half);tC
t41
half&64){nI3-n13}
}
else
yO1&1)nI3
value&((1<<POWI_WINDOW_SIZE)-1);else
nI3
value/2;long
cN=value-half
xG3>cN||half<0)std::swap(half,cN);FPO(t23(t63,"value=%ld, half=%ld, otherhalf=%ld\n",value,half,otherhalf))xG3==cN){l81
half,i6,2,l91+1);}
else{l81
half,i6,iO2
l81
cN>0?cN:-cN,i6,iO2}
i6.iJ2
value);}
lO
size_t
yF
yD1
nH3,cI&eN,lA1
int
nJ3=i6.Find
cC3);if(nJ3>=0){lX3
nJ3;}
long
nI3
1;yO1<POWI_TABLE_SIZE){nI3
powi_table[cZ2
xG3&128){half&=127
xG3&64)nI3-n13
FPO(t23(t63,"* I want %ld, my plan is %ld * %ld\n",value,half,value/half))tW3
nJ2=yF
half
yP1
if(i6
tT1
half)>0||nJ2!=yE1){yP2
nJ2)y32
half,yE1);}
xH1
cC3/half
xT2
size_t
tE3=yE1
y32
value,tE3);i6.DumpContents()lC1
tE3;}
t41
half&64){nI3-n13}
}
else
yO1&1)nI3
value&((1<<POWI_WINDOW_SIZE)-1);else
nI3
value/2;long
cN=value-half
xG3>cN||half<0)std::swap(half,cN);FPO(t23(t63,"* I want %ld, my plan is %ld + %ld\n",value,half,value-half))xG3==cN){size_t
nJ2=yF
half
yP1
yF1
nJ2,half,nJ2,half,i6,eN.op_normal,eN.op_normal_flip,synth);}
else{long
part1=half;long
part2=cN>0?cN:-cN
tW3
part1_pos=yF
part1
yP1
size_t
part2_pos=yF
part2
yP1
FPO(t23(t63,"Subdivide(%ld: %ld, %ld)\n",value,half,otherhalf));yF1
part1_pos,part1,part2_pos,part2,i6,cN>0?eN.op_normal:eN.op_inverse,cN>0?eN.op_normal_flip:eN.op_inverse_flip,synth);}
size_t
tE3=yE1
y32
value,tE3);i6.DumpContents()lC1
tE3;}
nD3
yF1
size_t
apos,long
aval,size_t
bpos,long
bval
nH3
yL2
cumulation_opcode
yL2
cumulation_opcode_flip,lA1
int
a_needed=i6
tT1
aval);int
nK3=i6
tT1
bval);bool
tU1
e23
#define DUP_BOTH() do{if(apos<bpos){size_t tmp=apos;apos=bpos;bpos=tmp;tU1=!tU1;}FPO(t23(t63,"-> "tD3 tD3"op\n",(unsigned)apos,(unsigned)bpos));yP2 apos);yP2 apos==bpos?yE1:bpos);}while(0)
#define DUP_ONE(p) do{FPO(t23(t63,"-> "tD3"op\n",(unsigned)p));yP2 p);}while(0)
if(a_needed>0
xX3
nK3>0){iR1}
else{if(bpos!=yE1)iR1
else{tV1
tU1=!tU1;}
}
}
t41
nK3>0
xX3
apos!=yE1)iR1
else
DUP_ONE(bpos);}
else{if(apos==bpos&&apos==yE1)tV1
t41
apos==yE1&&bpos==iP
nP
2){FPO(t23(t63,"-> op\n"));tU1=!tU1;}
t41
apos==iP
nP
2&&bpos==yE1)FPO(t23(t63,"-> op\n"));t41
apos==yE1)DUP_ONE(bpos);t41
bpos==yE1){tV1
tU1=!tU1;}
else
iR1}
iP
l12
tU1?cumulation_opcode_flip:cumulation_opcode,2);}
nD3
yQ1
long
count,cI&eN,lA1
while
yX3<256){int
nI3
FPoptimizer_ByteCode::powi_table[count]xG3&128){half&=127;yQ1
half
xT2
count/=half;}
else
break;}
if
yX3==1
n92;if(!yX3&1)){iP
l12
cSqr,1);yQ1
count/2
xT2}
else{yP2
yE1);yQ1
count-1,eN,synth
yT1
cMul,2);}
}
}
t2{nD3
xH1(long
count,cI&eN,lA1
if
yX3==0)iP
PushImmed(eN.basevalue);else{bool
e02
e23
if
yX3<0){e02=true;count=-count;}
if(false)yQ1
count
xT2
t41
count>1){tS1
i6;l81
count,i6,1)tW3
x31=iP
GetStackTop();i6.Start(yE1);FPO(t23(t63,"Calculating result for %ld...\n",count))tW3
nK2=yF
count
yP1
size_t
n_excess=iP
nP
x31;if(n_excess>0||nK2!=x31-1){iP
DoPopNMov(x31-1,nK2);}
}
if(e02)iP
l12
eN.op_flip,1);}
}
}
#endif
#ifndef FPOptimizer_RangeEstimationHH
#define FPOptimizer_RangeEstimationHH
iF2
FPoptimizer_CodeTree{enum
cN1{c02,IsNever,Unknown}
;lO
yI2
tX1{iQ
min,max;bool
has_min,l22;tX1():min(),max(tX3
false),l22(false){}
tX1(iQ
mi,iQ
ma):min(mi),max(ma
tX3
true),l22(true){}
tX1(bool,iQ
ma):min(),max(ma
tX3
false),l22(true){}
tX1(iQ
mi,bool):min(mi),max(tX3
true),l22(false){}
void
set_abs();void
set_neg();t3
set_min_max_if
l32
t3
set_min_if
l32
t3
set_max_if
l32
void
set_min(iQ(lH
void
set_max(iQ(lH
void
xF2
iQ(lH}
;lZ2
bool
IsLogicalTrueValue
yI
iG&p
xR2
if(p
tL3
min>=0.5
cA2
if(!abs&&p
iL2<=-0.5
n92
true
lC1
false;}
lZ2
bool
IsLogicalFalseValue
yI
iG&p
xR2
if(abs
n92
p
iL2<0.5;else
lX3
p
tL3
l22&&p.min>-0.5
nW3<0.5;}
lO
iG
l01
yG);nM3
IsLogicalValue
yG);lO
cN1
GetIntegerInfo
yG);lZ2
cN1
GetEvennessInfo
yG
xX3!tree
yK1
n92
Unknown;const
iQ&value=tree
tN;if(isEvenInteger
cC3)n92
nL2
isOddInteger
cC3)n92
lB1
lZ2
cN1
GetPositivityInfo
yG){iG
p=l01(tree);if(p
tL3
min>=iQ(0)n92
nL2
p
iL2<iQ(0)n92
lB1
lZ2
cN1
GetLogicalValue
yG
xR2
iG
p=l01(tree);if(IsLogicalTrueValue(p,abs)n92
nL2
IsLogicalFalseValue(p,abs)n92
lB1}
#endif
#ifndef FPOptimizer_ConstantFoldingHH
#define FPOptimizer_ConstantFoldingHH
iF2
FPoptimizer_CodeTree{nD3
ConstantFolding(tW&tree);}
#endif
iF2{nE
using
iF2
FPoptimizer_CodeTree;yI2
ComparisonSetBase{enum{cU3=0x1,Eq_Mask=0x2,Le_Mask=0x3,cV3=0x4,cW3=0x5,Ge_Mask=0x6}
;static
int
Swap_Mask(int
m){lX3(m&Eq_Mask)|((m&cU3)?cV3:0)|((m&cV3)?cU3:0);}
enum
yG1{Ok,BecomeZero,BecomeOne,e42
n83
l52{cond_or,tQ2,tR2,tS2}
;}
;lO
yI2
ComparisonSet:public
ComparisonSetBase{yI2
e12{tW
a;tW
b;int
relationship;e12():a(),b(),relationship(){}
}
;yC<e12>yJ;yI2
Item{tW
value;bool
xU2;Item():value(),xU2(false){}
}
;yC<Item>yV1;int
x41;ComparisonSet():yJ(),yV1(),x41(0){}
yG1
AddItem
yZ3
a,bool
xU2,l52
type){for
cB1
c=0;c<yV1
e22
c)if(yV1[c].value.IsIdenticalTo(a)xX3
xU2!=yV1[c].xU2){iU
c31
case
tS2:yV1.erase(yV1.begin()+c);x41+=1
n31
case
tQ2:case
tR2:c41}
}
lX3
e42;}
Item
pole;pole.value=a;pole.xU2=xU2;yV1
yX
pole)lC1
Ok;}
yG1
AddRelationship(tW
a,tW
b,int
eM1,l52
type){iU
tF3
7)c31
lB
tS2:tF3
7){x41+=1
n31}
lB
tQ2:case
tR2:tF3
0)c41
break;}
if(!(a.GetHash()<b.GetHash())){a.swap(b);eM1=Swap_Mask(eM1);}
for
cB1
c=0;c<yJ
e22
c
xX3
yJ[c].a.IsIdenticalTo(a)&&yJ[c].b.IsIdenticalTo(b)){iU{int
nL3=yJ[c
tR|eM1;if(nL3==7)c31
yJ[c
tR=nL3;break;}
case
tQ2:case
tR2:{int
nL3=yJ[c
tR&eM1;if(nL3==0)c41
yJ[c
tR=nL3;break;}
case
tS2:{int
newrel_or=yJ[c
tR|eM1;int
nM2=yJ[c
tR&eM1;tY1
5&&nM2==0){yJ[c
tR=cW3
n31}
tY1
7&&nM2==0){x41+=1;yJ.erase(yJ.begin()+c)n31}
tY1
7&&nM2==Eq_Mask){yJ[c
tR=Eq_Mask;x41+=1
n31}
xU1}
}
lX3
e42;}
}
e12
comp;comp.a=a;comp.b=b;comp.relationship=eM1;yJ
yX
comp)lC1
Ok;}
}
;nL1
iQ,lZ3
CondType>bool
ConstantFolding_LogicCommon(tW&tree,CondType
nZ1,bool
nN2){bool
should_regenerate
e23
ComparisonSet
x5
comp
iV1
lZ3
yU
yG1
yY3=yU
Ok;tM2&atree=lR1
a);cI3
atree
t83()tV2
cEqual
lC
Eq_Mask
xY3
cNEqual
lC
cW3
xY3
cLess
lC
cU3
xY3
cLessOrEq
lC
Le_Mask
xY3
cGreater
lC
cV3
xY3
cGreaterOrEq
lC
Ge_Mask
xY3
cNot:yY3
xS1
lN
0),true
xY3
cNotNot:yY3
xS1
lN
0),false,nZ1);break;default:if(nN2||IsLogicalValue(atree))yY3
xS1,false,nZ1);i62
yY3){ReplaceTreeWithZero
e83.ReplaceWithImmed(0)lC1
true;ReplaceTreeWithOne
e83.ReplaceWithImmed(1);nZ
yU
Ok:lB
yU
BecomeZero
e4
yU
BecomeOne:e5
yU
e42:yJ1
xF3
if(should_regenerate){
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<"Before ConstantFolding_LogicCommon: "
c8
#endif
if(nN2){tree.DelParams();}
else{e92{tM2&atree=lR1
a);if(IsLogicalValue(atree))nD1);}
}
for
n62
comp.yV1
e22
a
xX3
comp.yV1[a].xU2
eF2
cNot);r.c6
r.eN1
t41!nN2
eF2
cNotNot);r.c6
r.eN1
else
tree.c6}
for
n62
comp.yJ
e22
a
eF2
cNop);cI3
comp.yJ[a
tR
tV2
yU
cU3:r
y7
cLess
iP2
Eq_Mask:r
y7
cEqual
iP2
cV3:r
y7
cGreater
iP2
Le_Mask:r
y7
cLessOrEq
iP2
cW3:r
y7
cNEqual
iP2
Ge_Mask:r
y7
cGreaterOrEq
cH2
r
lE1
comp.yJ[a].a);r
lE1
comp.yJ[a].b);r.eN1
if(comp.x41!=0)tree
yV
iQ(comp.x41)));
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<"After ConstantFolding_LogicCommon: "
c8
#endif
lX3
true;}
xD1
nM3
ConstantFolding_AndLogic(t73(tree
t83()==cAnd
t03()==cAbsAnd)nB
tQ2,true);}
nM3
ConstantFolding_OrLogic(t73(tree
t83()==cOr
t03()==cAbsOr)nB
cond_or,true);}
nM3
ConstantFolding_AddLogicItems(t73(tree
t83()==cAdd)nB
tS2,false);}
nM3
ConstantFolding_MulLogicItems(t73(tree
t83()==cMul)nB
tR2,false);}
}
#include <vector>
#include <map>
#include <algorithm>
iF2{nE
using
iF2
FPoptimizer_CodeTree;yI2
CollectionSetBase{enum
x51{Ok,e42}
;}
;lO
yI2
CollectionSet:public
CollectionSetBase{yI2
yH1{tW
value;tW
xO2;bool
cW;yH1():value(),xO2(),cW(false){}
yH1
yZ3
v,tM2&f):value(v),xO2(f),cW(false){}
}
;std::multimap<nC2,yH1>i7;typedef
lZ3
std::multimap<nC2,yH1>::nP3
x61;CollectionSet():i7(){}
x61
FindIdenticalValueTo
yZ3
value){nC2
hash=value.GetHash();for(x61
i=i7.nO2
hash);i!=i7.c51
hash;++i){yO1.IsIdenticalTo(i
yQ2.value)n92
i;}
lX3
i7.end();}
bool
Found
yI
x61&b){lX3
b!=i7.end();}
x51
AddCollectionTo
yZ3
xO2,const
x61&into_which){yH1&c=into_which
yQ2;if(c.cW)c.xO2
iO
xO2);else{tW
add;add
y7
cAdd);add
lE1
c.xO2);add
iO
xO2);c.xO2.swap(add);c.cW=true;}
lX3
e42;}
x51
l62
yZ3
value,tM2&xO2){const
nC2
hash=value.GetHash();x61
i=i7.nO2
hash);for(;i!=i7.c51
hash;++i
xX3
i
yQ2.value.IsIdenticalTo
cC3)n92
AddCollectionTo
lY2,i);}
i7.nN3,std::make_pair(hash,yH1
cC3,xO2)))lC1
Ok;}
x51
l62
yZ3
a){lX3
l62(a,c61
1)));}
}
;lO
yI2
ConstantExponentCollection{typedef
yC<tW>TreeSet;typedef
std::cO1
nP2;yC<nP2>data;ConstantExponentCollection():data(){}
void
MoveToSet_Unique
yI
iQ&cP1&cQ1){data
yX
std::cO1(cP1()));data.back()cZ3.swap(cQ1)iH2
MoveToSet_NonUnique
yI
iQ&cP1&cQ1){lZ3
yC<nP2>::nP3
i=std::nO2
data.tT2
data.end(),tK2,Compare1st());if(i!=data.c51
tK2){i
yQ2.nN3
yQ2.end(),cQ1.tT2
cQ1.end());}
else{data.nN3,std::cO1
nG2,cQ1));}
}
bool
Optimize(){bool
changed
e23
std::sort(data.tT2
data.end(),Compare1st());redo:for
n62
data
e22
a){iQ
exp_a=data[a
cX3;e62
exp_a,e03
xU1
for
cB1
b=a+1;b<data
e22
b){iQ
exp_b=data[b
cX3;iQ
nQ2=exp_b-exp_a;if(nQ2>=fp_abs(exp_a))break;iQ
exp_diff_still_probable_integer=nQ2*iQ(16);if(eP2
exp_diff_still_probable_integer)&&!(eP2
exp_b)&&!eP2
nQ2))){TreeSet&a_set=tZ1;TreeSet&b_set=data[b
cY3;
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<"Before ConstantExponentCollection iteration:\n"
;eQ2
cout);
#endif
if(isEvenInteger(exp_b)&&!isEvenInteger(nQ2+exp_a)){tW
tmp2;tmp2
y03
tmp2
c33
b_set);tmp2
tW2
tmp;tmp
y7
cAbs);tmp
lO3;tmp
lL1
b_set
n93
1);b_set[0]yB2}
a_set.insert(a_set.end(),b_set.tT2
b_set.end());TreeSet
b_copy=b_set;data.erase(data.begin()+b);MoveToSet_NonUnique(nQ2,b_copy);lT2
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<"After ConstantExponentCollection iteration:\n"
;eQ2
cout);
#endif
c9}
}
}
lX3
changed;}
#ifdef DEBUG_SUBSTITUTIONS
void
eQ2
ostream&out){for
n62
data
e22
a){out.precision(12);out<<data[a
cX3<<": "
;c91
tZ1
e22
b
xX3
b>0)out<<'*';nX3(tZ1[b],out);}
out<<std::endl;}
}
#endif
}
;lO
static
tW
nS1
tW&value,bool&xH){switch
cC3
t83()tV2
cPow:{tW
tK2=value
lN
1);value.xT1
lC1
tK2;}
case
cRSqrt:value.xT1;xH=true
lC1
c61-0.5));case
cInv:value.xT1;xH=true
lC1
c61-1));default:break;}
lX3
c61
1));cN3
void
cR1
cS1&mul,cD2,tM2&xO2,bool&yI1
bool&xH){for
n62
y6++a){tW
value(lR1
a));tW
tK2(nS1
value,xH));if(!xO2
yK1||xO2
tN!=1.0){tW
c71;c71
y03
c71
iO
tB2
c71
iO
xO2);c71
lL1
tK2.swap(c71);}
#if 0 /* FIXME: This does not work */
yO1
nD
cMul
xX3
1){bool
exponent_is_even=tK2
yK1&&isEvenInteger
nG2
tN);c91
value
e43{bool
tmp
e23
tW
val
cC3
lN
b));tW
exp(nS1
val,tmp));if(exponent_is_even||(exp
yK1&&isEvenInteger(exp
tN))){tW
c71;c71
y03
c71
iO
tB2
c71
lE1
exp);c71.ConstantFolding();if(!c71
yK1||!isEvenInteger(c71
tN)){goto
cannot_adopt_mul;}
}
}
}
cR1
mul,value,tK2,yI1
xH);}
else
cannot_adopt_mul:
#endif
{if(mul.l62
cC3,tK2)==CollectionSetBase::e42)yJ1}
}
}
nM3
ConstantFolding_MulGrouping(eH2
bool
xH
e23
bool
should_regenerate
e23
cS1
mul;cR1
mul
c63,c61
1)),yI1
xH);typedef
std::pair<tW,yC<tW> >cT1;typedef
std::multimap<nC2,cT1>yW1;yW1
tO;cI2
cS1::x61
j=mul.i7
nA3
j!=mul.i7.end();++j){tW&value=j
yQ2.value;tW&tK2=j
yQ2.xO2;if(j
yQ2.cW)tK2
lL1
const
nC2
cU1=tK2.GetHash();lZ3
yW1::nP3
i=tO.nO2
cU1);for(;i!=tO.c51
cU1;++i)if(i
yQ2.first.IsIdenticalTo
nG2)xX3!tK2
yK1||!fp_equal
nG2
tN,e03
yJ1
i
yQ2
cZ3
yX
value);goto
skip_b;}
tO.nN3,std::make_pair(cU1,std::make_pair
nG2,yC<tW>cB1(1),value))));skip_b:;}
#ifdef FP_MUL_COMBINE_EXPONENTS
ConstantExponentCollection
x5
c81;cI2
yW1::nP3
j,i=tO
nA3
i!=tO.end();i=j){j=i;++j;cT1&list=i
yQ2;l72
yK1
lH2
list.first
tN;if(!nG2==iQ(0)))c81.MoveToSet_Unique
nG2,list
eI2;tO.erase(i);}
}
if(c81.Optimize())yJ1
#endif
if(should_regenerate){tW
before=tree;before.lH1
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<"Before ConstantFolding_MulGrouping: "
;nX3(before)iJ"\n"
;
#endif
tree.DelParams();cI2
yW1::nP3
i=tO
nA3
i!=tO.end();++i){cT1&list=i
yQ2;
#ifndef FP_MUL_COMBINE_EXPONENTS
l72
yK1
lH2
list.first
tN;if
nG2==iQ(0))xU1
if(FloatEqual
nG2,e03{tree.AddParamsMove(list
eI2;xU1}
}
#endif
tW
mul;mul
y03
mul
c33
list
eI2;mul
lL1
if(xH&&list.first
yK1){l72
tN==lU
3)){tW
cbrt;cbrt
y7
cCbrt);cbrt.e6
cbrt.eO1
cbrt);xU1
cO
0.5)){tW
sqrt;sqrt
y7
cSqrt);sqrt.e6
sqrt.eO1
sqrt);xU1
cO-0.5)){tW
rsqrt;rsqrt
y7
cRSqrt);rsqrt.e6
rsqrt.eO1
rsqrt);xU1
cO-1)){tW
inv;inv
y7
cInv);inv.e6
inv.eO1
inv);xU1}
}
tW
pow;pow
y7
cPow);pow.e6
pow
lE1
list.first);pow.eO1
pow);}
#ifdef FP_MUL_COMBINE_EXPONENTS
tO.clear()lJ1
i2
e22
a
lH2
i2[a
cX3;if(fp_equal
nG2,e03{tree.AddParamsMove(i2[a]eI2;xU1}
tW
mul;mul
y03
mul
c33
i2[a]eI2;mul
tW2
pow;pow
y7
cPow);pow.e6
pow
yV
tK2));pow.eO1
pow);}
#endif
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<"After ConstantFolding_MulGrouping: "
c8
#endif
lX3!tree.IsIdenticalTo(before);}
xD1
nM3
ConstantFolding_AddGrouping(eH2
bool
should_regenerate
e23
cS1
add
iV1
if(lR1
a)nD
cMul)xU1
if(add.l62(lR1
a))==CollectionSetBase::e42)yJ1}
xD3
remaining(i42
tW3
t8=0
iV1
tM2&lY3=lR1
a);if
cL2
nD
cMul){c91
lY3
e43{if
cL2
lN
b)yK1)xU1
lZ3
cS1::x61
c=add.FindIdenticalValueTo
cL2
lN
b));if(add.Found(c)){tW
tmp
cL2
x81
CloneTag());tmp.eI1
b);tmp
lL1
add.AddCollectionTo(tmp,c);yJ1
goto
done_a;}
}
remaining[a]=true;t8+=1;done_a:;}
}
if(t8>0
xX3
t8>1){yC<std::pair<tW,tY3>nY;std::multimap<nC2,tY3
cV1;bool
iQ2
e23
for
xT{c91
lR1
a)e43{tM2&p=lR1
a)lN
b);const
nC2
p_hash=p.GetHash();for(std::multimap<nC2,tY3::const_iterator
i=cV1.nO2
p_hash);i!=cV1.c51
p_hash;++i
xX3
nY[i
yQ2
cX3.IsIdenticalTo(p)){nY[i
yQ2
cY3+=1;iQ2=true;goto
found_mulgroup_item_dup;}
}
nY
yX
std::make_pair(p,size_t(1)));cV1.insert(std::make_pair(p_hash,nY
e72)-1));found_mulgroup_item_dup:;}
}
if(iQ2){tW
yR2;{size_t
max=0;for
cB1
p=0;p<nY
e22
p)if(nY[p
cY3<=1)nY[p
cY3=0;else{nY[p
cY3*=nY[p
cX3.nX2;if(nY[p
cY3>max){yR2=nY[p
cX3;max=nY[p
cY3;}
}
}
tW
group_add;group_add
y7
cAdd);
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<"Duplicate across some trees: "
;nX3(yR2)iJ" in "
c8
#endif
for
xT
c91
lR1
a)e43
if(yR2.IsIdenticalTo(lR1
a)lN
b))){tW
tmp(lR1
a)x81
CloneTag());tmp.eI1
b);tmp
lL1
group_add
lE1
tmp);remaining[a]e23
break;}
group_add
tW2
group;group
y03
group
lE1
yR2);group
lE1
group_add);group
lL1
add.l62(group);yJ1}
}
for
xT{if(add.l62(lR1
a))==CollectionSetBase::e42)yJ1}
}
if(should_regenerate){
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<"Before ConstantFolding_AddGrouping: "
c8
#endif
tree.DelParams();cI2
cS1::x61
j=add.i7
nA3
j!=add.i7.end();++j){tW&value=j
yQ2.value;tW&coeff=j
yQ2.xO2;if(j
yQ2.cW)coeff
lL1
if(coeff
yK1){e62
coeff
tN,iQ(0)))xU1
e62
coeff
tN,e03{tree
lE1
value);xU1}
}
tW
mul;mul
y03
mul
lE1
value);mul
lE1
coeff);mul.eO1
mul);}
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<"After ConstantFolding_AddGrouping: "
c8
#endif
lX3
true;}
xD1}
iF2{nE
using
iF2
FPoptimizer_CodeTree;nM3
ConstantFolding_IfOperations(t73(tree
t83()==cIf
t03()==cAbsIf);for(;;xX3
x71
cNot){tree
y7
cIf);lE
nQ3
nW2
0));tU2.swap(tI3);}
t41
x71
lQ2){tree
y7
nS3;lE
nQ3
nW2
0));tU2.swap(tI3);}
else
e32
tB
0),x92
nS3){iI1
tree.nQ3
tU2);nZ
iB2
tree.nQ3
tI3);nZ
n01
if(x71
cIf||x71
nS3{tW
cond=lR1
0);tW
iR2;iR2
y7
cond
tT3
cNotNot:cAbsNotNot);iR2
nR2
1));ConstantFolding(iR2);tW
iS2;iS2
y7
cond
tT3
cNotNot:cAbsNotNot);iS2
nR2
2));ConstantFolding(iS2);if(iR2
yK1||iS2
yK1){tW
eO;eO
y7
cond.e7
eO
nR2
1));eO.nK
1));eO.nK
2));eO
tW2
eP;eP
y7
cond.e7
eP
nR2
2));eP.nK
1));eP.nK
2));eP
lL1
tree
y7
cond.e7
tree.SetParam(0,cond
lN
0))nT3
1,eO)nT3
2,eP)tQ1}
if(tU2
nD
tI3
t83()&&(tU2
nD
cIf||tU2
nD
nS3){tW&leaf1=tU2;tW&leaf2=tI3;if
cO2
0).xP
0))&&cO2
1
tR3||leaf1
lN
2
tS3)){tW
eO;eO
cW1
eO.nK
0));eO
nS2
1));eO
nT2
1));eO
tW2
eP;eP
cW1
eP.nK
0));eP
nS2
2));eP
nT2
2));eP
lL1
tree
cX1
SetParam(0
nR3
0))nT3
1,eO)nT3
2,eP)tQ1
if
cO2
1
tR3&&leaf1
lN
2
tS3){tW
eQ;eQ
cW1
eQ.l11
eQ
nS2
0));eQ
nT2
0));eQ
lL1
tree
cX1
n91
0,eQ)eD2
2
nR3
2))eD2
1
nR3
1))tQ1
if
cO2
1
tS3&&leaf1
lN
2
tR3){tW
yS2;yS2
y7
leaf2
tT3
cNot:lQ2);yS2
nT2
0));yS2
tW2
eQ;eQ
cW1
eQ.l11
eQ
nS2
0));eQ
lE1
yS2);eQ
lL1
tree
cX1
n91
0,eQ)eD2
2
nR3
2))eD2
1
nR3
1))tQ1}
tW&cJ=tU2;tW&cC=tI3;if(cJ.IsIdenticalTo(cC)){tree.nQ3
tU2)tQ1
const
OPCODE
op1=cJ
t83();const
OPCODE
op2=cC
t83();if
nU3
op2
xX3
cJ
iK==1){tW
changed_if
tW1
cW1
changed_if.l11
changed_if
iO
cJ
lN
0))tW1
iO
cC
lN
0))tW1
lL1
tree
y7
op1)lR3
DelParams()c7
changed_if)tQ1
if
nU3
nV3
cMul
i01
cAnd
i01
cOr
i01
cAbsAnd
i01
cAbsOr
i01
cMin
i01
cMax){yC<tW>iT2;yB{for
cB1
b=cC
iK;b-->0;){if
iU2
cC
lN
b))xX3
iT2
xT3){cJ.e63
lH1}
iT2
yX
cJ
lF3
cC.eI1
b);cJ.eI1
a
cH2}
}
if(!iT2
xT3){cJ
lL1
cC
tW2
changed_if
tW1
cW1
changed_if
c33
tree.tM1))tW1
lL1
tree
y7
op1)lR3
SetParamsMove(iT2)lR3
nS}
if
nU3
nV3
cMul||nU3
cAnd
nN1
cC))||nU3
cOr
nN1
cC))){yB
if
iU2
cC)){cJ.lH1
cJ.eI1
a);cJ
tW2
yX1=cC;cC=t9
op1==nV3
cOr
l8
op1)c7
yX1);xV2
nU3
cAnd
i01
cOr)&&op2==cNotNot){tW&iV2=cC
lN
0);yB
if
iU2
iV2)){cJ.lH1
cJ.eI1
a);cJ
tW2
yX1=iV2;cC=t9
op1==cOr
l8
op1)c7
yX1);xV2
op2==cAdd||op2==cMul||(op2==cAnd
nN1
cJ))||(op2==cOr
nN1
cJ))){for
cB1
a=cC.eT
cC
lN
iS
cJ)){cC.e63
eI1
a);cC
tW2
yY1=cJ;cJ=t9
op2==cAdd||op2==cOr
l8
op2)c7
yY1);xV2(op2==cAnd||op2==cOr)&&op1==cNotNot){tW&iW2=cJ
lN
0
e52
cC.eT
cC
lN
iS
iW2)){cC.e63
eI1
a);cC
tW2
yY1=iW2;cJ=t9
op2==cOr
l8
op2)c7
yY1)lR3
nS
xD1}
#include <limits>
iF2{nE
using
iF2
FPoptimizer_CodeTree;lO
int
maxFPExponent(){lX3
std::numeric_limits
x5::max_exponent;}
nM3
nT1
iQ
e73
tK2
xX3
base<iQ(0)cA2
e62
e73(0))||fp_equal(e73(1))n92
false
lC1
tK2>=iQ(maxFPExponent
x5())/fp_log2(base);}
nM3
ConstantFolding_PowOperations(t73(tree
t83()==cPow);y0){iQ
const_value
eO3
yD,y1)lR3
ReplaceWithImmed(const_value)lC1
false;}
if(xW2(float)y1==1.0){tree.nQ3
lR1
0))tQ1
x8&&(float)lE
GetImmed()==1.0)lM
1)lC1
false;}
x8&&tU2
nD
cMul){bool
nU2
e23
iQ
i11=lE
GetImmed();tW
lY3=lR1
1
e52
xM1
eT
lY3
lN
a
yB3
imm=lY3
lN
a)tN;{if(nT1
i11,imm))break;iQ
i21
eO3(i11,imm);e62
i21,iQ(0)))break;if(!nU2){nU2=true;xM1
lH1}
i11=i21;xM1
eI1
a
cH2}
if(nU2){lY3
lL1
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<"Before pow-mul change: "
c8
#endif
lE
nQ3
CodeTreeImmed(i11));tU2.Become
cL2);
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<"After pow-mul change: "
c8
#endif
}
}
if(xW2
x71
cMul){iQ
i31=y1;iQ
nV2=1.0;bool
nU2
e23
tW&lY3=lR1
0
e52
xM1
eT
lY3
lN
a
yB3
imm=lY3
lN
a)tN;{if(nT1
imm,i31))break;iQ
cY1
eO3(imm,i31);e62
cY1,iQ(0)))break;if(!nU2){nU2=true;xM1
lH1}
nV2*=cY1;xM1
eI1
a
cH2}
if(nU2){lY3
tW2
c03;c03
y7
cPow);c03
c33
tree.tM1));c03.lL2;tree
y7
cMul)c7
c03);tree
yV
nV2))tQ1}
if(x71
cPow&&xW2
nW2
1
yB3
a=nW2
1)tN;iQ
b=y1;iQ
c=a*b;if(isEvenInteger(a)&&!isEvenInteger(c)){tW
iX2;iX2
y7
cAbs);iX2
iO
nW2
0));iX2
lL1
tree.n91
0,iX2);}
else
tree.lT
c));}
xD1}
iF2{nE
using
iF2
FPoptimizer_CodeTree;yI2
l5{enum
yT2{MakeFalse=0,MakeTrue=1,eR2=2,l43=3,MakeNotNotP0=4,MakeNotNotP1=5,MakeNotP0=6,MakeNotP1=7,xG=8
n83
i41{Never=0,Eq0=1,Eq1=2,x53=3,y83=4}
;yT2
if_identical;yT2
i51
4];yI2{yT2
what:4;i41
when:4;}
t51,t61,t71,t81;lO
yT2
Analyze
yZ3
a,tM2&b
cE3{if(a.IsIdenticalTo(b)n92
if_identical;iG
p0=l01(a);iG
p1=l01(b);if(p0.l22&&p1.has_min
xX3
p0.max<p1.min&&i51
0]i3
0];if(p0.max<=p1.min&&i51
1]i3
1];}
if(p0.n32
p1.l22
xX3
p0.min>p1.max&&i51
2]i3
2];if
tZ3>=p1.max&&i51
3]i3
3];}
if(IsLogicalValue(a)xX3
t51
yC3
t51.when,p1)n92
t51.what;if(t71
yC3
t71.when,p1)n92
t71.what;}
if(IsLogicalValue(b)xX3
t61
yC3
t61.when,p0)n92
t61.what;if(t81
yC3
t81.when,p0)n92
t81.what;}
lX3
xG;cN3
bool
TestCase(i41
when,const
iG&p
xX3!p.has_min||!p.l22)xR1
cI3
when
tV2
Eq0:lX3
p.min==0.0
nW3==p.min;case
Eq1:lX3
p.min==1.0
nW3==p.max;case
x53:lX3
p.min>0
nW3<=1;case
y83:lX3
p.min>=0
nW3<1;default:;}
xD1}
;iF2
RangeComparisonsData{static
const
l5
Data[6]={{l5
iY2
tS
xG,l5::tS
xG}
,iH
Eq1}
,iI
Eq1}
,lI1
Eq0
nP1
Eq0}
}
,{l5::l82
l13
xG,l5
l13
xG}
,iH
Eq0}
,iI
Eq0}
,lI1
Eq1
nP1
Eq1}
}
,{l5::l82
l13
eR2,l5::tS
MakeFalse}
,lI1
x53}
,iI
y83}
,cY2
iY2
xG,l5
l13
tS
l43}
,lI1
y83}
,iI
x53}
,cY2::l82::tS
tS
MakeTrue,l5::eR2}
,iH
y83
nP1
x53}
,cY2
iY2
tS
l43,l5::xG,l5
nA1}
,iH
x53
nP1
y83}
,{l5::yK}
;}
nM3
ConstantFolding_Comparison(eH2
using
iF2
RangeComparisonsData;assert(tree t83()>=cEqual&&tree t83()<=cGreaterOrEq);cI3
Data[xC1-cEqual].Analyze(lR1
0),tU2)tV2
l5::MakeFalse
e83.ReplaceWithImmed(0)l23
nA1
e83.ReplaceWithImmed(1
yL3
l43
e83
y7
cEqual
yL3
eR2
e83
y7
cNEqual
yL3
MakeNotNotP0
e83
y7
cNotNot)l33
1
yL3
MakeNotNotP1
e83
y7
cNotNot)l33
0
yL3
MakeNotP0
e83
y7
cNot)l33
1
yL3
MakeNotP1
e83
y7
cNot)l33
0
yL3
xG:;}
if(tU2
yK1)cI3
lE
GetOpcode()tV2
cAsin
l03
fp_sin(tX2
cAcos
l03
fp_cos(y1)));tree
y7
x92
cLess?cGreater:x92
cLessOrEq?cGreaterOrEq:x92
cGreater?cLess:x92
cGreaterOrEq?cLessOrEq
e83.e7
nZ
cAtan
l03
fp_tan(tX2
cLog
l03
fp_exp(tX2
cSinh
l03
fp_asinh(tX2
cTanh:if(fp_less(fp_abs(y1),e03{tree.lT
fp_atanh(y1)))tQ1
break;default:break;}
xD1}
#include <list>
#include <algorithm>
#ifdef FP_SUPPORT_OPTIMIZER
nE
iF2{
#ifdef DEBUG_SUBSTITUTIONS
eH
double
d){union{double
d;uint_least64_t
h;cM2
d=d;lS1
h
cC1
eH
float
f){union{float
f;uint_least32_t
h;cM2
f=f;lS1
h
cC1
eH
long
double
ld){union{long
double
ld;yI2{uint_least64_t
a
xQ2
short
b;}
s;cM2
ld=ld;lS1
s.b<<data.s.a
cC1
#endif
}
iF2
FPoptimizer_CodeTree{iS1()nI)){}
iS1
yI
iQ&i
x81
n63)nI
i))nT
#ifdef __GXX_EXPERIMENTAL_CXX0X__
iS1(iQ&&i
x81
n63)nI
eS2
i)))nT
#endif
iS1
eS1
v
x81
VarTag)nI
tZ2,v
yN3(nF2
o
x81
OpcodeTag)nI
o
yN3(nF2
o
yL2
f
x81
FuncOpcodeTag)nI
o,f
yN3
yZ3
b
x81
CloneTag)nI*b.data)){}
lO
nC~tY(){}
lA
ReplaceWithImmed
yI
iQ&i){
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<"Replacing "
;nX3(*this);if(IsImmed())OutFloatHex(std::cout,GetImmed())iJ" with const value "
<<i;OutFloatHex(std::cout,i)iJ"\n"
;
#endif
data=new
nA2
x5(i);}
lO
yI2
ParamComparer{tL2()yZ3
a,tM2&b
cE3{if(a.nX2!=b.nX2
n92
a.nX2<b.nX2
lC1
a.GetHash()<b.GetHash();}
}
;nD3
nA2
x5::Sort(){cI3
Opcode
tV2
cAdd:case
cMul:case
cMin:case
cMax:case
cAnd:case
cOr:case
cHypot:case
cEqual:case
cNEqual:std::sort(l53
tT2
l53
end(),ParamComparer
x5());lB
cLess
lP
cGreater;}
lB
cLessOrEq
lP
cGreaterOrEq;}
lB
cGreater
lP
cLess;}
lB
cGreaterOrEq
lP
cLessOrEq;}
break;default:xF3
lA
AddParam
yZ3
param){xF
yX
param);}
lA
AddParamMove(tW&param){xF
yX
tW());xF.back().swap(param);}
lA
SetParam
cB1
which,tM2&b){nU1
which
tY2
xF[which]=b;}
lA
n91
size_t
which,tW&b){nU1
which
tY2
xF[which].swap(b);}
lA
AddParams
yI
nN){xF.insert(xF.end(),i61.tT2
i61.end());}
lA
AddParamsMove(nN){size_t
endpos=xF
e72),added=i61
e72);xF
n93
endpos+added,tW());for
cB1
p=0;p<added;++p)xF[endpos+p].swap(i61[p]);}
lA
AddParamsMove(nN,size_t
i71){nU1
i71
tY2
eI1
i71);AddParamsMove(eF1}
lA
SetParams
yI
nN){yC<tW>tmp(eF1
xF
yB2}
lA
SetParamsMove(nN){xF.swap(eF1
i61.clear();}
#ifdef __GXX_EXPERIMENTAL_CXX0X__
lA
SetParams(yC<tW>&&i61){SetParamsMove(eF1}
#endif
lA
DelParam
cB1
index){yC<tW>&yV3=xF;
#ifdef __GXX_EXPERIMENTAL_CXX0X__
l53
erase(l53
begin()+index);
#else
yV3[index].data=0;for
cB1
p=index;p+1<yV3
e22
p)yV3[p]e53
UnsafeSetP(&*yV3[p+1
tY2
yV3[yV3
e72)-1]e53
UnsafeSetP(0);l53
resize(yV3
e72)-1);
#endif
}
lA
DelParams(){xF.clear();}
nM3
nC
IsIdenticalTo
yZ3
b
cE3{if(&*data==&*b.data
n92
true
lC1
data->IsIdenticalTo(*b.data);}
nM3
nA2
x5::IsIdenticalTo
yI
nA2
x5&b
cE3{if(Hash!=b.Hash)xR1
if(Opcode!=c43
xR1
cI3
Opcode
tV2
cImmed:lX3
FloatEqual(Value,c53;case
tZ2:lX3
tO1==b.tN1
case
cFCall:case
cPCall:if(tO1!=b.yZ1
xR1
break;default:break;}
if(yV3
e72)!=b.yV3
e72)n92
false
lJ1
yV3
e22
a
xX3!yV3[a].IsIdenticalTo(b.yV3[a]))xD1
lX3
true;}
lA
Become
yZ3
b
xX3&b!=this&&&*data!=&*b.data){DataP
tmp=b.data;lH1
data
yB2}
}
lA
CopyOnWrite(xX3
GetRefCount()>1)data=new
nA2
x5(*data);}
lO
tW
nC
GetUniqueRef(xX3
GetRefCount()>1
n92
tW(*this,CloneTag())lC1*this;}
lO
n7):yY
cNop),Value()n8
lO
n7
const
nA2&b):yY
c43,Value(c53,tO1(b.yZ1,yV3(b.yV3
eV3
b.Hash
eW3
b.Depth),eP1
b.tP1){}
lO
n7
const
iQ&i):yY
cImmed),Value(i)n8
#ifdef __GXX_EXPERIMENTAL_CXX0X__
lO
n7
nA2
x5&&b):yY
c43,Value(eS2
c53),tO1(b.yZ1,yV3(eS2
b.yV3)eV3
b.Hash
eW3
b.Depth),eP1
b.tP1){}
lO
n7
iQ&&i):yY
cImmed),Value(eS2
i))n8
#endif
lO
n7
nF2
o):yY
o),Value()n8
lO
n7
nF2
o
yL2
f):yY
o),Value(),tO1(f),yV3(eV3
eW3
1),eP1
0){}
}
#endif
#ifdef FP_SUPPORT_OPTIMIZER
#include <sstream>
#include <string>
#include <map>
#include <set>
#include <iostream>
nE
#ifdef FUNCTIONPARSER_SUPPORT_DEBUG_OUTPUT
iF2{nD3
i81
yI
x3,e93&done,std::ostream&o){for
n62
y6++a)i81
cS3
done,o);std::ostringstream
buf;nX3(tree,buf);done[cE2].insert(buf.str());}
}
#endif
iF2
FPoptimizer_CodeTree{
#ifdef FUNCTIONPARSER_SUPPORT_DEBUG_OUTPUT
nD3
DumpHashes
cD1){e93
done;i81(tree,done,o);for(e93::const_iterator
i=done
nA3
i!=done.end();++i){const
std::set<std::string>&flist=i
yQ2;if(flist
e72)!=1)o<<"ERROR - HASH COLLISION?\n"
;for(std::set<std::string>::const_iterator
j=flist
nA3
j!=flist.end();++j){o<<'['<<std::hex<<i->first.hash1<<','<<i->first.hash2<<']'<<std::dec;o<<": "
<<*j<<"\n"
;}
}
}
nD3
nX3
cD1){const
char*t53;cI3
xC1
tV2
cImmed:o<<tree
tN
nY3
tZ2:o<<"Var"
<<(tree.GetVar()-tZ2)nY3
cAdd:t53"+"
;lB
cMul:t53"*"
;lB
cAnd:t53"&"
;lB
cOr:t53"|"
;lB
cPow:t53"^"
;break;default:t53;o<<FP_GetOpcodeName(tree.e7
if(x92
cFCall||x92
cPCall)o<<':'<<tree.GetFuncNo();}
o<<'(';if(tree
iK<=1&&sep2[1])o<<(sep2+1)<<' 'iV1
if(a>0)o<<' ';nX3
cS3
o);if(a+1<i42
o<<sep2;}
o<<')';}
nD3
DumpTreeWithIndent
cD1,const
std::string&indent){o<<'['<<std::hex<<(void*)(&tree.tM1))<<std::dec<<','<<tree.GetRefCount()<<']';o<<indent<<'_';cI3
xC1
tV2
cImmed:o<<"cImmed "
<<tree
tN;o<<'\n'nY3
tZ2:o<<"VarBegin "
<<(tree.GetVar()-tZ2);o<<'\n'lC1;default:o<<FP_GetOpcodeName(tree.e7
if(x92
cFCall||x92
cPCall)o<<':'<<tree.GetFuncNo();o<<'\n';}
for
n62
y6++a){std::string
ind=indent;for
cB1
p=0;p<ind
e72);p+=2)if(ind[p]=='\\')ind[p]=' ';ind+=(a+1<i42?" |"
:" \\"
;DumpTreeWithIndent
cS3
o,ind);}
o<<std::flush;}
#endif
}
#endif
using
iF2
l71;nE
#include <cctype>
iF2
l71{nM3
ParamSpec_Compare
yI
void*aa,const
void*bb,l02
type){cI3
type
tV2
l92
xI&a=*(xI*)aa;xI&b=*(xI*)bb
lC1
a
cS2==b
cS2&&a.index==b.index&&a.depcode==b.depcode;}
case
NumConstant:{yL&a=*(yL*)aa;yL&b=*(yL*)bb
lC1
FloatEqual(a.iM2,b.iM2)&&a.modulo==b.modulo;nB3
xJ&a=*(xJ*)aa;xJ&b=*(xJ*)bb
lC1
a
cS2==b
cS2&&a.cP==b.cP&&a
e53
match_type==b
e53
match_type&&a.data
yH==b.data
yH&&a
e53
param_list==b
e53
param_list&&a
e53
lY==b
e53
lY&&a.depcode==b.depcode;}
}
lX3
true;}
lW3
ParamSpec_GetDepCode
yI
yM2&b){cI3
b.first
tV2
l92
i02*s=yI
xI*)b
cZ3
lC1
s->depcode;nB3
const
xJ*s=yI
xJ*)b
cZ3
lC1
s->depcode;}
default:break;}
lX3
0;}
nD3
DumpParam
yI
yM2&n02,std::ostream&o){static
const
char
ParamHolderNames[][2]={"%"
,"&"
,"x"
,"y"
,"z"
,"a"
,"b"
,"c"
}
xQ2
cU2
0;x03
NumConstant:{const
yL&eJ2
yL
eK2
o.precision(12);o<<nZ3
iM2;break;}
case
l92
i02&eJ2
xI
eK2
o<<ParamHolderNames[nZ3
index];cU2
param
cS2;break;nB3
const
xJ&eJ2
xJ
eK2
cU2
param
cS2;yO
GroupFunction){if
t02
cP==cNeg){o<<"-"
;lZ}
t41
nZ3
cP==cInv){o<<"/"
;lZ}
else{std::string
opcode=FP_GetOpcodeName((nF2)nZ3
cP).substr(1)lJ1
opcode
e22
a)opcode[a]=(char)std::toupper(opcode[a]);o<<opcode<<"( "
;lZ
o<<" )"
;}
}
else{o<<'('<<FP_GetOpcodeName((nF2)nZ3
cP)<<' ';yO
PositionalParams)o<<'[';yO
SelectedParams)o<<'{';lZ
if
t02
data.lY!=0)o<<" <"
<<nZ3
data.lY<<'>';yO
PositionalParams)o<<"]"
;yO
SelectedParams)o<<"}"
;o<<')';}
break;}
i62
ImmedConstraint_Value(t91
ValueMask)tV2
ValueMask:lB
Value_AnyNum:lB
lB2:o<<"@E"
;lB
Value_OddInt:o<<"@O"
;lB
eQ1:o<<"@I"
;lB
Value_NonInteger:o<<"@F"
;lB
cZ1:o<<"@L"
;e32
ImmedConstraint_Sign(t91
SignMask)tV2
SignMask:lB
Sign_AnySign:lB
nB1:o<<"@P"
;lB
Sign_Negative:o<<"@N"
;e32
ImmedConstraint_Oneness(t91
OnenessMask)tV2
OnenessMask:lB
Oneness_Any:lB
Oneness_One:o<<"@1"
;lB
cA1:o<<"@M"
;e32
ImmedConstraint_Constness(t91
ConstnessMask)tV2
n41:if(n02.first==ParamHolder){i02&eJ2
xI
eK2
if
t02
index<2)break;}
o<<"@C"
;lB
Oneness_Any:xF3
nD3
DumpParams
eS1
paramlist
yL2
count,std::ostream&o){for(e31=0;a<count;++a
xX3
a>0)o<<' ';const
yM2&param=nV1
paramlist,a);DumpParam
x5(param,o)xQ2
depcode=ParamSpec_GetDepCode(param);if(depcode!=0)o<<"@D"
<<depcode;}
}
}
#include <algorithm>
using
iF2
l71;nE
iF2{i02
plist_p[35]={{2,0,0x0}
tL
0,0x4}
tL
nB1,0x0}
tL
Sign_NoIdea,0x0}
tL
cZ1,0x0}
,{3,Sign_NoIdea,0x0}
,{3,0,0x0}
,{3,cZ1,0x0}
,{3,0,0x8}
,{3,Value_OddInt,0x0}
,{3,Value_NonInteger,0x0}
,{3,lB2,0x0}
,{3,nB1,0x0}
,{0,Sign_Negative
x13
lR
0,nB1
x13
lB2
x13
n41,c13
0,eQ1|nB1
x13
cA1|n41,c13
0,cA1
x13
Oneness_One
x13
cZ1|lR
1,lB2|lR
1,lR
1,cA1|lR
1,eQ1|lR
1,nB1|lR
6,0,0x0}
,{4,0,0x0}
,{4,lR
4,0,0x16}
,{4,eQ1,0x0}
,{5,0,0x0}
,{5,n41,0x0}
,}
;lO
yI2
plist_n_container{static
const
yL
plist_n[19];}
;lO
const
yL
plist_n_container
x5::plist_n[19]={{-2,0}
,{-1,0}
,{-0.5,0}
,{0,0}
,{fp_const_deg_to_rad
eG1
fp_const_einv
eG1
fp_const_log10inv
eG1
0.5,0}
,{fp_const_log2
eG1
1,0}
,{fp_const_log2inv
x5(),0}
tL
0}
,{fp_const_log10
eG1
fp_const_e
eG1
fp_const_rad_to_deg
eG1-fp_const_pihalf
c23{0,x91{fp_const_pihalf
c23{fp_const_pi
c23}
;const
xJ
plist_s[448]={{{1,14,eT2
14,cNeg,GroupFunction,0}
,n41,c13{1,406,eT2
407,eT2
14,nY2
24,nY2
394,nY2
395,nY2
407,cInv,xO
2,291071,cAdd
lT3
268324
iB
0x5
y4
268324,l6
44
iB
0x4
y4
57388,l6
134188,l6
215084,l6
257060,l6
257068,l6
268332,l6
268332
iB
0x5
y4
134196,l6
214068,l6
271396,l6
186412
iB
0x1
y4
271404,l6
311340,l6
220212,l6
6144,l6
29702,l6
135168,l6
171014,l6
57358,l6
58382,l6
270357
iB
0x4
y4
58392,l6
28701,l6
33798,l6
17564,l6
56476,l6
176297,l6
45233,l6
277682,l6
276659,l6
164061,l6
165104,l6
231651,l6
234724,l6
37115,l6
45307,l6
277772,l6
14724,l6
24964,l6
176383,l6
1321,l6
401608,l6
403848,cAdd,lQ
0,0,xS
0,0,cE1
42,xS
1,50,xS
1,51,xS
1,52,xS
1,53,xS
1,0,nA
1
eJ1
0,nA
2
eJ1
0,xS
1,20,xS
1,14,xS
1,25,xS
1,24,nA
2}
,0,0x0
y4
55320,cE1
210,xS
1,218,cE1
231,nA
1}
,0,0x16}
,{{1,320,nA
1}
,0,0x16}
,{{0,0,nA
1}
,nB1,0x0
y4
45070,cAdd,xO
2,36,c3
6180,c3
112676,c3
273444,lJ
0x6
y4
337956,lJ
0x6
y4
322596,c3
341028,c3
363556,c3
368676,c3
369700,c3
373796,c3
378916,c3
422948,c3
426020,c3
431140,c3
434212,c3
443428,c3
451620,c3
293936,l3
3,60860416,c3
6144,c3
493568,c3
14,c3
70670,c3
128014,c3
271374,lJ
0x1
y4
284672,l3
3,44443648,c3
29702,c3
113688,c3
112664,c3
28701,c3
33798,c3
273422,lJ
0x7
y4
337934,lJ
0x7
y4
348160,l3
3,64360477,c3
404480,c3
261154,c3
289792,c3
349190,c3
290846,c3
54,c3
37106,c3
37106,lJ
0x4
y4
37108,lJ
0x4
y4
37115,c3
37132,c3
14618,c3
37229,lJ
0x4}
,{{3,14727534,c3
59780,c3
37285,lJ
0x4
y4
36263,c3
47472,l3
3,14727591,c3
337974,lJ
0x7
y4
47531,c3
101652,c3
102677,c3
253215,c3
293202,c3
295250,c3
342354,c3
47457,c3
299074,c3
366948,lJ
eU2
38115684,lJ
x23
420196,lJ
x23
6538,c3
367000,lJ
x23
420248,lJ
eU2
38168932,lJ
eU2
38168984,lJ
x23
6609,c3
6633,l3
0,0,nU
0,0,eR
36
yF3
36,eR
0,lL
1
eJ1
0,lL
2
eJ1
0
yF3
0,eR
13
yF3
15
yF3
15,lL
1}
,0,c13{1,20
yF3
14
yF3
24,lL
2}
,0,0x0
y4
24590
yF3
54
yF3
54,lL
2}
,0,c13{1,256,lL
1
eJ1
259,lL
2
eJ1
269,eR
272
yF3
273
yF3
274,eR
290
yF3
320,eR
342
yF3
446,lL
2}
,0,0x0
y4
397372,lL
1}
,0,0x1
y4
397632,lL
1}
,0,0x16}
,{{1,388
yF3
394,nU
2,59434,nZ2
43022,nZ2
24590,nZ2
43032,nZ2
40,lG,40,n2
48,lG,48,n2
356392,lG,213040,lG,356400,lG,357416,lG,357424,lG,47104,lG,14
eV2
6144,n2
6158
eV2
16384,lG,18432,lG,15360,lG,14336,lG,26624,lG,27654,lG,24582,lG,14,lG,15,lG,29696,lG,29698,lG,24
e21
0x6
y4
24,lG,6168,lG,68608,lG,83968,lG,86040,lG,87040,lG,88064,lG,90112,lG,244736,lG,415744,lG,416768,lG,36870,lG,9216,lG,10240,lG,6146,lG,6144,lG,13317,lG,23558,lG,55302,lG,112654,lG,112669,lG,403456,lG,246790,lG,6158,lG,36929
e21
0x5
y4
36932,n2
36936
e21
0x5
y4
37072,lG,37078,n2
37959,n2
43078,n2
43079,n2
37967,lG,43087,lG,43109,lG,43102,n2
43110,lG,47214,lG,59650,lG,63746,lG,63749,lG,37220,n2
37220,lG,37226
e21
0x5
y4
37227
eV2
37229,n2
37229,lG,47460,n2
47460,lG,14701,n2
133485,lG,37231
e21
0x5
y4
37252,lG,37254,lG,37260,lG,37261,lG,37272,n2
37285,n2
37292,n2
37292,lG,37301,lG,37301,n2
37303,lG,59,lG,59
e21
0x6
y4
47512,n2
47512,lG,24783,lG,244902,lG,246955,lG,6147,cPow,yC1}
,nB1,0x0
y4
24590,cPow,xO
2,59416,cPow,xO
2,59426,cPow,xO
2,60446,cPow,xO
1,0,eA3
6,eA3
151,eA3
0,cAcos
eB3
cAcosh
eB3
cAsin
eB3
cAsinh
x4
110,cAsinh
eB3
cAtan,i91
cAtan2
lT3
291840,cAtan2
eB3
cAtanh
eB3
cCeil
eX2
207,cCeil
eB3
xX2
0,cCos
eX2
6,xX2
81,xX2
83,xX2
110,xX2
173,xX2
222,xX2
0,cCosh
eX2
0,cCosh
x4
168,cCosh
x4
173,cCosh
x4
393,cCosh
eB3
cFloor
eX2
207,cFloor
lT3
296211,l63
2,344339,l63
2,344399,l63
3,30414848,x01
488658944,x01
489691136,x01
29393920,cIf
xA1
3,30414848,cIf
xA1
3,6769664,x01
23545856,x01
93415424,x01
170036224,x01
418791424,x01
422989824,x01
500660224,x01
508007424,cIf
x4
110,cInt
x4
0
xY2
6
xY2
29
xY2
151
xY2
207
xY2
267
xY2
14,cLog,xO
1,24,cLog,xO
1,0,cLog10
eB3
cLog2,i91
cMax
lT3
28701,cMax
lT3
33798,cMax
eB3
cMax,AnyParams,1}
,0,lD2
cMin
lT3
28701,cMin
lT3
33798,cMin
eB3
cMin,AnyParams,1}
,0,0x4
y4
45070,cMin,xO
2,24590,cMin,xO
1,0,lC2
0,cSin
eX2
6,lC2
81,lC2
83,lC2
110,lC2
133,lC2
153,cSin,l7
0x5}
,{{1,207,lC2
215,lC2
219,cSin,l7
c13{1,222,lC2
0,cSinh
eX2
0,cSinh
x4
153,cSinh,l7
0x5}
,{{1,168,cSinh
x4
207,cSinh
x4
215,cSinh
x4
222,cSinh
x4
393,cSinh
eB3
xZ2
0,cTan
eX2
74,cTan
eX2
75,xZ2
153,xZ2
207,xZ2
219,xZ2
215,xZ2
222,xZ2
0,x02
0,cTanh
eX2
158,x02
153,x02
207,x02
215,x02
222,x02
0,cTrunc
lT3
14360,cSub,xO
2,14360,cDiv,xO
2,403851,cDiv,xO
2,6144,cEqual
cJ2
6144,cEqual
lT3
29696,cEqual,l7
0x20
y4
29702,cEqual,l7
0x24
y4
29702,cEqual
lT3
38912,cLess,l7
0x4
y4
6,cLess,i91
cLess
cJ2
38912,x12
l7
lD2
x12
l7
x23
6144,x12
l0
2,243919,x12
l0
2,38912,cGreater,l7
lD2
cGreater
cJ2
38912,cGreaterOrEq,l7
lD2
cGreaterOrEq
cJ2
243919,cGreaterOrEq
eB3
iA1
6,iA1
14,iA1
29,iA1
150,iA1
476,iA1
479,iA1
480,iA1
483,iA1
486,iA1
487,cNot,i91
iB1
28701,iB1
33798,iB1
385053,iB1
388125,iB1
6609,cAnd,lQ
0,0,eX
1}
,0,0x0
y4
6144,n12
28701,n12
33798,n12
385053,n12
388125,n12
6609,cOr,lQ
1,0,e01
6,e01
81,e01
119,e01
150,e01
152,e01
207,cDeg
x4
207,cRad,i91
cAbsAnd,lQ
2,6144,cAbsOr,lQ
1,0,lQ2
eB3
cAbsNotNot,l0
3,30414848,xA3
l7
0x0}
,}
;}
iF2
l71{const
Rule
grammar_rules[250]={{ProduceNewTree,eF3
0,{1,0,cAbs,l2
352,{1,170,cAtan,l2
345
tL
1326,eC3
l2
347
tL
309249,eC3
l1
213199
tL
217299,eC3
l2
142
x22
cCeil,l2
419,{1,80,eY2
413,{1,113,eY2
414,{1,115,eY2
140,{1,116,eY2
361,{1,114,eY2
0,{1,345,cCos,lG1
0,{1,342,cCos,lG1
207
x22
eY2
303,{1,348,cCosh,lG1
0,{1,342,cCosh,lG1
207
x22
cCosh,l2
138
x22
cFloor,l2
387,{1,112,cFloor,l2
205,{3,6330368
lF1
501,{3,30414850
lF1
481,{3,7378944
lF1
483,{3,7385088
lF1
206,{3,39852032
lF1
476,{3,39853056
lF1
489,{3,39890944
lF1
488,{3,46144512
lF1
465,{3,46176256
lF1
x33
1057223
lF1
x33
1057225
lF1
x43
1057229
lF1
x43
1057231
lF1
x43
8390087
lF1
x43
8390089
lF1
x33
8390093
lF1
x33
8390095
lF1
x63
387287493
lF1
x63
387287496
lF1
x63
371574220
lF1
x63
371574222,cIf,yC1
cR
3,30442950,{3,34633162
lF1
109,{1,216,eD3
108,{1,230,eD3
346,{1,107,eD3
195,{1,196,cLog,lG1
383
yU2,cMax,x32
0
tL
414721,cMax,x32
384
yU2,cMin,x32
0
tL
410625,cMin,iD1
371
tL
43071,lI2
372
tL
43114,lI2
373
tL
43101,lI2
194
tL
24792,lI2
193
tL
107534,lI2
194
tL
23767,lI2
191
tL
130079,lI2
149
tL
131103,lI2
192
tL
124942,i22
29975,i22
29976,i22
29977
eW2
163158
tL
30033
eW2
403456
tL
397326
eW2
242688
tL
241678
eW2
211968
tL
240671
eW2
211968
tL
239633,i22
33050
eW2
6144
tL
11606
eW2
6357
tL
11496,lI2
144
x22
y02
363,{1,80,y02
140,{1,113,y02
361,{1,115,y02
143,{1,116,y02
413,{1,114,y02
0,{1,347,y02
146
x22
cSinh,l2
301,{1,346,cSinh,l2
147
x22
eE3
0,{1,350,eE3
157,{1,351,eE3
148
x22
cTanh,l2
250,{1,300,nA
1
cQ
249,{1,299,nA
1
cR
1,248
tL
1322
lK
246
tL
1320
lK
381
yU2
lK
391
tL
398724
lK
190
tL
230624
lK
189
tL
217312
lK
155
tL
228569
lK
154
tL
228371
lK
44
tL
321868
lK
315
tL
140332
lK
333
tL
139308
lK
411
tL
203977
lK
412
tL
208073
lK
359
tL
207045
lK
139
tL
207046
lK
360
tL
209093
lK
188
tL
138506
lK
185
tL
339210
lK
184
tL
339078
lK
186
tL
191652
lK
181
tL
169124
lK
244
tL
430444
lK
174
tL
430260
lK
145
tL
180588
lK
141
tL
180644
lK
242
tL
187756
lK
365
tL
251063
lK
421
tL
251060
lK
421
tL
179564
lK
365
tL
249252
lK
145
tL
249012,nA
0
cQ
104
tL
78865,cMul,SelectedParams,0
cQ
495,{1,49,lL
1
cQ
496,{1,39,lL
1
cR
1,294
tL
1319
lD
382
yU2
lD
473
tL
477649
lD
476
tL
502249
lD
481
tL
502225
lD
344
tL
351574
lD
97
tL
24702
lD
98
tL
24696
lD
96
tL
89341
lD
103
tL
80140
lD
95
tL
78092
lD
419
tL
427044
lD
426
tL
435236
lD
327
tL
373060
lD
421
tL
373174
lD
365
tL
430408
lD
356
tL
419141
lD
428
tL
315801
lD
309
tL
331181
lD
326
tL
365891
lD
408
tL
365997
lD
44
tL
439726
lD
434
tL
318882
lD
44
tL
443823
lD
436
tL
445476
lD
437
tL
319908
lD
313
tL
332214
lD
440
tL
326054
lD
432
tL
317855
lD
443
tL
452644
lD
439
tL
300096
lD
329
tL
298057
lD
396
tL
42372
lD
321
tL
48447
lD
397
tL
46468
lD
322
tL
44351
lE2
105486
tL
93452
lE2
97294
tL
94476
lE2
325045
tL
324004,lL
0}
}
,{ReplaceParams,eF3
0,{1,342,lL
0}
}
,{ReplaceParams,eF3
54,{1,13
lE2
38912
tL
350246,x11
tH
x11
x83
24795,x11
tI
x11
eS
x11
eV
x11
iC1
225499,x11
tJ
x11
eW
x11
i8
x11
38912
tL
350246,nC1
tH
nC1
x83
24795,nC1
tI
nC1
eS
nC1
eV
nC1
iC1
225499,nC1
tJ
nC1
eW
nC1
i8
cNEqual,l2
499
tL
43008,y51
x83
24792,y51
xB1
y51
tH
y51
tI
y51
eS
y51
eV
y51
iC1
x93
y51
nQ1
y51
tJ
y51
eW
y51
i8
cLess,l2
493
tL
350222,cL
x83
24792,cL
xB1
cL
tH
cL
tI
cL
eS
cL
eV
cL
iC1
x93
cL
nQ1
cL
tJ
cL
eW
cL
i8
x12
l2
469
tL
350222,eB
x83
24792,eB
xB1
eB
tH
eB
tI
eB
eS
eB
eV
eB
iC1
x93
eB
nQ1
eB
tJ
eB
eW
eB
i8
cGreater,l2
500
tL
43008,xV
x83
24792,xV
xB1
xV
tH
xV
tI
xV
eS
xV
eV
xV
iC1
x93
xV
nQ1
xV
tJ
xV
eW
xV
i8
cGreaterOrEq,l2
499,{1,2,cNot,l2
494,{1,4,eX
1
cR
1,497
tL
12290,eX
l4
385
yU2,eX
l4
473
tL
477649,eX
l4
474
tL
389588,eX
l4
475
tL
384468,eX
0
cR
2,463297,{3,472321472,eX
l4
498
tL
12290,tK
491
tL
7172,tK
386
yU2,tK
470
tL
477649,tK
471
tL
389588,tK
472
tL
384468,tK
492
tL
132100,cOr,iD1
500,{1,2,cNotNot,yC1}
}
,{ProduceNewTree,eF3
0,{1,0,cNotNot,l2
464,{1,216,cAbsNotNot,iD1
459,{1,215,cAbsNotNot,iD1
374,{3,30415337,xA3
l2
500,{3,39890944,xA3
l2
499,{3,46176256,xA3
yC1
cR
3,30442950,{3,34633162,xA3
yC1}
}
,}
;yI2
grammar_optimize_abslogical_type{nM
9
eC
grammar_optimize_abslogical_type
grammar_optimize_abslogical={9,{20,179,215,227,229,235,242,246,249}
}
;}
yI2
grammar_optimize_ignore_if_sideeffects_type{nM
51
eC
grammar_optimize_ignore_if_sideeffects_type
grammar_optimize_ignore_if_sideeffects={51,{0,19,21,22,23,24,25,26,xD
tB1
86,lM1
nG
grammar_optimize_nonshortcut_logical_evaluation_type{nM
48
eC
grammar_optimize_nonshortcut_logical_evaluation_type
grammar_optimize_nonshortcut_logical_evaluation={48,{0,25,xD
tB1
86,lM1
157,158,159,169,191,203,228,230,231,232,233,234,236,237,238,239,240,241,243,244,245,247,248}
}
;}
yI2
grammar_optimize_round1_type{nM
109
eC
grammar_optimize_round1_type
grammar_optimize_round1={109,{0,1,2,3,4,5,6,7,8,9,10,11,12,13,17,25,xD
41,42,tB1
52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,86,87,88,89,90,91,92,93,94,95,96,97,98,99,115,118,lM1
123,124,125,126,127,128,129,154,155,nG
grammar_optimize_round2_type{nM
92
eC
grammar_optimize_round2_type
grammar_optimize_round2={92,{0,14,15,16,25,xD
43,44,tB1
52,54,55,56,57,58,59,60,61,66,67,68,76,77,82,83,84,85,86,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,118,lM1
123,126,127,128,130,154,155,156,nG
grammar_optimize_round3_type{nM
85
eC
grammar_optimize_round3_type
grammar_optimize_round3={85,{78,79,80,81,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,160,161,162,163,164,165,166,167,168,170,171,172,173,174,175,176,177,178,180,181,182,183,184,185,186,187,188,189,190,192,193,194,195,196,197,198,199,200,201,202,204,205,206,207,208,209,210,211,212,213,214,216,217,218,219,220,221,222,223,224,225,226}
}
;}
yI2
grammar_optimize_round4_type{nM
10
eC
grammar_optimize_round4_type
grammar_optimize_round4={10,{18,49,50,51,116,117,150,151,152,153}
}
;}
yI2
grammar_optimize_shortcut_logical_evaluation_type{nM
45
eC
grammar_optimize_shortcut_logical_evaluation_type
grammar_optimize_shortcut_logical_evaluation={45,{0,25,xD
tB1
86,lM1
157,158,159,169,191,203,230,231,232,233,234,237,238,239,240,243,244,245,247,248}
}
;}
}
iF2
l71{lO
yM2
ParamSpec_Extract
eS1
paramlist
yL2
index){index=(paramlist>>(index*10))&1023;if(index>=54
n92
yM2(SubFunction
cV2
plist_s[index-54]);if(index>=35
n92
yM2(NumConstant
cV2
plist_n_container
x5::plist_n[index-35])lC1
yM2(ParamHolder
cV2
plist_p[index]);}
}
#ifdef FP_SUPPORT_OPTIMIZER
#include <stdio.h>
#include <algorithm>
#include <map>
#include <sstream>
nE
using
iF2
l71;using
iF2
FPoptimizer_CodeTree;using
iF2
FPoptimizer_Optimize;iF2{nL1
It,lZ3
T,lZ3
Comp>e11
MyEqualRange(It
first,It
last,const
T&val,Comp
comp){size_t
len=last-first;while(len>0){size_t
nI3
len/2;It
lU3(first);lU3+=half;if(comp(*lU3,val)){first=lU3;++first;len=len-half-1;}
t41
comp(val,*lU3)){len=half;}
else{It
left(first);{It&yV2=left;It
last2(lU3)tW3
len2=last2-yV2;while(len2>0){size_t
half2=len2/2;It
cF3(yV2);cF3+=half2;if(comp(*cF3,val)){yV2=cF3;++yV2;len2=len2-half2-1;}
else
len2=half2;}
}
first+=len;It
right(++lU3);{It&yV2=right;It&last2=first
tW3
len2=last2-yV2;while(len2>0){size_t
half2=len2/2;It
cF3(yV2);cF3+=half2;if(comp(val,*cF3))len2=half2;else{yV2=cF3;++yV2;len2=len2-half2-1;}
}
}
lX3
e11(left,right);}
}
lX3
e11(first,first);}
lO
yI2
OpcodeRuleCompare{tL2()yG
yL2
x42
cE3{const
Rule&rule=grammar_rules[x42]lC1
xC1<rule
nI2.subfunc_opcode;}
tL2()eS1
x42,cD2
cE3{const
Rule&rule=grammar_rules[x42]lC1
rule
nI2.subfunc_opcode<xC1;}
}
;nM3
TestRuleAndApplyIfMatch
yI
cQ2
tW&tree,bool
c4{MatchInfo
x5
info;n21
found(false,cV());if(rule.logical_context&&!c4{goto
fail;}
for(;;){
#ifdef DEBUG_SUBSTITUTIONS
#endif
found=TestParams(rule
nI2
c63,found.specs,info,true);if(found.found)break;if(!&*found.specs){fail:;
#ifdef DEBUG_SUBSTITUTIONS
DumpMatch(rule
c63,info,false);
#endif
xD1}
#ifdef DEBUG_SUBSTITUTIONS
DumpMatch(rule
c63,info,true);
#endif
SynthesizeRule(rule
c63,info)tQ1}
iF2
FPoptimizer_Optimize{nM3
ApplyGrammar
yI
Grammar&eZ2,tW&tree,bool
c4{if(tree.GetOptimizedUsing()==&eZ2){
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<"Already optimized:  "
;nX3(tree)iJ"\n"
<<std::flush;
#endif
xD1
if(true){bool
changed
e23
cI3
xC1
tV2
cNot:case
cNotNot:case
cAnd:case
cOr:for
cB1
a=0;a
x6
true))lT2
lB
cIf:case
cAbsIf:if(ApplyGrammar(eZ2,lR1
0),x92
cIf))lT2
for
cB1
a=1;a
x6
c4)lT2
break;default:for
cB1
a=0;a
x6
false))lT2}
if(changed){tree.Mark_Incompletely_Hashed()tQ1}
typedef
const
lW3
char*l73;std::pair<l73,l73>range=MyEqualRange(eZ2.rule_list,eZ2.rule_list+eZ2.rule_count
c63,OpcodeRuleCompare
x5());if(range.eG3
range
eI2{
#ifdef DEBUG_SUBSTITUTIONS
yC<lW3
char>rules;rules.nE3
range
cZ3-range.first);xY
if(IsLogisticallyPlausibleParamsMatch(cF1
nI2
c63))rules
yX*r);}
range.first=&rules[0];range
cZ3=&rules[rules
e72)-1]+1;if(range.eG3
range
eI2{std::cout<<"Input ("
<<FP_GetOpcodeName(xC1)<<")["
<<tree
iK<<"]"
;if(c4
std::cout<<"(Logical)"
xQ2
first=tC1,prev=tC1;const
char*sep=", rules "
;xY
if(first==tC1)first=prev=*r;t41*r==prev+1)prev=*r;else{std::cout<<sep<<first;sep=","
;if(prev!=first)std::cout<<'-'<<prev;first=prev=*r;}
}
if(eG3
tC1){std::cout<<sep<<first;if(prev!=first)std::cout<<'-'<<prev;}
std::cout<<": "
;nX3(tree)iJ"\n"
<<std::flush;}
#endif
bool
changed
e23
xY
#ifndef DEBUG_SUBSTITUTIONS
if(!IsLogisticallyPlausibleParamsMatch(cF1
nI2
c63))xU1
#endif
if(TestRuleAndApplyIfMatch(cF1
c63,c4){lT2
xF3
if(changed){
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<"Changed."
<<std::endl
iJ"Output: "
;nX3(tree)iJ"\n"
<<std::flush;
#endif
tree.Mark_Incompletely_Hashed()tQ1}
tree.SetOptimizedUsing(&eZ2)lC1
false;}
nD3
ApplyGrammars(x3){
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<t13"grammar_optimize_round1\n"
;
#endif
n1
grammar_optimize_round1
c63
x7
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<t13"grammar_optimize_round2\n"
;
#endif
n1
grammar_optimize_round2
c63
x7
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<t13"grammar_optimize_round3\n"
;
#endif
n1
grammar_optimize_round3
c63
x7
#ifndef FP_ENABLE_SHORTCUT_LOGICAL_EVALUATION
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<t13"grammar_optimize_nonshortcut_logical_evaluation\n"
;
#endif
n1
grammar_optimize_nonshortcut_logical_evaluation
c63
x7
#endif
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<t13"grammar_optimize_round4\n"
;
#endif
n1
grammar_optimize_round4
c63
x7
#ifdef FP_ENABLE_SHORTCUT_LOGICAL_EVALUATION
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<t13"grammar_optimize_shortcut_logical_evaluation\n"
;
#endif
n1
grammar_optimize_shortcut_logical_evaluation
c63
x7
#endif
#ifdef FP_ENABLE_IGNORE_IF_SIDEEFFECTS
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<t13"grammar_optimize_ignore_if_sideeffects\n"
;
#endif
n1
grammar_optimize_ignore_if_sideeffects
c63
x7
#endif
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<t13"grammar_optimize_abslogical\n"
;
#endif
n1
grammar_optimize_abslogical
c63
x7
#undef C
}
}
#endif
#ifdef FP_SUPPORT_OPTIMIZER
#include <algorithm>
#include <assert.h>
#include <cstring>
#include <cmath>
#include <memory> /* for auto_ptr */
nE
using
iF2
l71;using
iF2
FPoptimizer_CodeTree;using
iF2
FPoptimizer_Optimize;iF2{nM3
TestImmedConstraints
eS1
bitmask,cD2){cI3
bitmask&ValueMask
tV2
Value_AnyNum:case
ValueMask:lB
lB2:if(GetEvennessInfo(tree
iM
Value_OddInt:if(GetEvennessInfo(tree)l21
eQ1:if(GetIntegerInfo(tree
iM
Value_NonInteger:if(GetIntegerInfo(tree)l21
cZ1:if(!IsLogicalValue(tree))tM
SignMask
tV2
Sign_AnySign:lB
nB1:if(nE1
iM
Sign_Negative:if(nE1)l21
Sign_NoIdea:if(nE1)!=Unknown)tM
OnenessMask
tV2
Oneness_Any:case
OnenessMask:lB
Oneness_One:if(!iN
if(!fp_equal(fp_abs(tree
tN),e03
xR1
lB
cA1:if(!iN
e62
fp_abs(tree
tN),e03
tM
ConstnessMask
tV2
Constness_Any:lB
n41:if(!iN
break;}
lX3
true;}
iD2
lW3
extent
yL2
nbits,lZ3
yW2=lW3
int>yI2
nbitmap{private:static
const
lW3
bits_in_char=8;static
const
lW3
yX2=(xB3
yW2)*bits_in_char)/nbits;yW2
data[(extent+yX2-1)/yX2];cR3
void
inc
eS1
index,int
by=1){data[pos(index)]+=by*yW2(1<<x52);i4
void
dec
eS1
index){inc(index,-1);}
int
get
eS1
index
n71(data[pos(index)]>>x52)&mask()lG2
pos(y71
index/yX2
lG2
shift(y71
nbits*(index%yX2)lG2
mask(){lX3(1<<nbits)-1
lG2
mask(y71
mask()<<x52;}
}
;yI2
yU3{int
SubTrees:8;int
Others:8;int
tD1:8;int
c93:8;nbitmap<tZ2,2>SubTreesDetail;yU3(){std::memset(this,0,xB3*this));}
yU3
yI
yU3&b){std::memcpy(this,&b,xB3
b));}
yU3&cL1=yI
yU3&b){std::memcpy(this,&b,xB3
b))lC1*this;}
}
;lO
yU3
CreateNeedList_uncached
yI
t7){yU3
lD1;for(e31=0;a<params
yH;++a){const
yM2&n02=nV1
params.param_list,a);x03
lA2
const
xJ&eJ2
xJ
eK2
yO
GroupFunction)++eI3;else{++eN2;assert(param.data.subfunc_opcode<VarBegin);lD1.SubTreesDetail.inc
t02
cP);}
++lD1.tD1;break;}
case
NumConstant:case
ParamHolder:++eM2;++lD1.tD1;xF3
lX3
lD1;}
lO
yU3&CreateNeedList
yI
t7){typedef
std::map<const
iZ*,yU3>cG1;static
cG1
y81;cG1::nP3
i=y81.nO2&params);if(i!=y81.c51&params
n92
i
yQ2
lC1
y81.nN3,std::make_pair(&params,CreateNeedList_uncached
x5(params)))yQ2;}
lO
tW
c83
yI
yM2&n02,const
y93){x03
NumConstant:{const
yL&eJ2
yL*)n02
cZ3
lC1
CodeTreeImmed
t02
iM2);}
case
l92
i02&eJ2
xI*)n02
cZ3
lC1
info.GetParamHolderValueIfFound
t02
index);nB3
const
xJ&eJ2
xJ
eK2
tW
cQ3;cQ3
y7
nZ3
cP);cQ3.tM1).reserve
t02
data
yH);for(e31=0;a<nZ3
data
yH;++a){tW
tmp(c83(nV1
nZ3
data.param_list,a),info));cQ3
lE1
tmp);}
cQ3.Rehash()lC1
yS3}
lX3
tW();}
}
iF2
FPoptimizer_Optimize{nM3
IsLogisticallyPlausibleParamsMatch
yI
t7,cD2){yU3
lD1(CreateNeedList
x5(params))tW3
eH3=y6
if(eH3<size_t(lD1.tD1)){xD1
for
n62
eH3;++a){lW3
opcode=lR1
a)t83();cI3
opcode
tV2
cImmed:if(eI3>0)--eI3;else--eM2;lB
tZ2:case
cFCall:case
cPCall:--eM2;break;default:assert(opcode<VarBegin);if(eN2>0&&lD1.SubTreesDetail.get(opcode)>0){--eN2;lD1.SubTreesDetail.dec(opcode);}
else--eM2;}
}
if(eI3>0||eN2>0||eM2>0){xD1
if(params.match_type!=AnyParams
xX3
0||eN2<0||eM2<0){xD1}
lX3
true;}
lO
n21
TestParam
yI
yM2&n02
c73){x03
NumConstant:{const
yL&eJ2
yL
eK2
if(!iN
iQ
imm=tree
tN;switch
t02
modulo
tV2
Modulo_None:lB
Modulo_Radians:imm=fp_mod(imm,yA
imm<0)imm
yT
if(imm>fp_const_pi
x5())imm-=fp_const_twopi
x5(cH2
lX3
fp_equal(imm,nZ3
iM2);}
case
l92
i02&eJ2
xI
eK2
if(!x2
lX3
info.SaveOrTestParamHolder
t02
index
c63);nB3
const
xJ&eJ2
xJ
eK2
yO
GroupFunction
xX3!x2
tW
xG1=c83(n02,info);
#ifdef DEBUG_SUBSTITUTIONS
DumpHashes(xG1)iJ*yI
void**)&xG1
tN
iJ"\n"
iJ*yI
void**)&tree
tN
iJ"\n"
;DumpHashes(tree)iJ"Comparing "
;nX3(xG1)iJ" and "
;nX3(tree)iJ": "
iJ(xG1.IsIdenticalTo(tree)?"true"
:"false"
)iJ"\n"
;
#endif
lX3
xG1.IsIdenticalTo(tree);}
else{if(!&*start_at
xX3!x2
if(xC1!=nZ3
cP)xD1
lX3
TestParams
t02
data
c63,start_at,info,false);}
}
}
xD1
lO
yI2
iX
lV2
MatchInfo
x5
info;iX()i12,info(){}
}
;n53
MatchPositionSpec_PositionalParams
xF1
iX
x5>{cR3
i32
MatchPositionSpec_PositionalParams
cB1
n):xC3
iX
x5>(n){}
}
;yI2
tE1
lV2
tE1()i12{}
}
;class
yP
xF1
tE1>{cR3
lW3
trypos;i32
yP
cB1
n):xC3
tE1>(n),trypos(0){}
}
;lO
n21
TestParam_AnyWhere
yI
yM2&n02
c73,xD3&used,bool
yY2{xM<yP>xB;e51
yP
lJ2
a=xB->trypos;goto
retry_anywhere_2;}
cF2
yP(i42;a=0;}
for(;a<y6++a
xX3
used[a])xU1
retry_anywhere
e13
TestParam(n02,lR1
a),tX,info);tX=r.l93
used[a]=true
lX1
a);xB->trypos=a
lC1
n21(true,&*xB);}
}
retry_anywhere_2:y13
goto
retry_anywhere;}
}
xD1
lO
yI2
xV1
lV2
MatchInfo
x5
info;xD3
used;i32
xV1
cB1
eH3)i12,info(),used(eH3){}
}
;n53
MatchPositionSpec_AnyParams
xF1
xV1
x5>{cR3
i32
MatchPositionSpec_AnyParams
cB1
n,size_t
m):xC3
xV1
x5>(n,xV1
x5(m)){}
}
;lO
n21
TestParams
yI
iZ&nO
c73,bool
yY2{if(nO.match_type!=AnyParams
xX3
nO
yH!=i42
xD1
if(!IsLogisticallyPlausibleParamsMatch(nO
c63)){xD1
cI3
nO.match_type
tV2
PositionalParams:{xM<cG>xB;e51
cG
lJ2
a=nO
yH-1;goto
lN1;}
cF2
cG(nO
yH);a=0;}
for(;a
cG2{(xE3
t22
retry_positionalparams
e13
TestParam(cS
a),lR1
a),tX,info);tX=r.l93
xU1}
}
lN1:y13
lS3
a].info;goto
retry_positionalparams;}
if(a>0){--a;goto
lN1;}
lS3
0].info
lC1
false;}
if(yY2
for(e31=0;a
cG2
info.SaveMatchedParamIndex(a)lC1
n21(true,&*xB);}
case
SelectedParams:case
AnyParams:{xM<t4>xB;xD3
used(i42;yC<lW3>i52(nO
yH);yC<lW3>x62(nO
yH)e41{const
yM2
n02=cS
a);i52[a]=ParamSpec_GetDepCode(n02);}
{lW3
b=0
e41
if(i52[a]!=0)x62[b++]=a
e41
if(i52[a]==0)x62[b++]=a;}
e51
t4
lJ2
if(nO
yH==0){a=0;goto
retry_anyparams_4;}
a=nO
yH-1;goto
cH1;}
cF2
t4(nO
yH,i42;a=0;if(nO
yH!=0){(*xB)[0].t22(*xB)[0].used=used;}
}
for(;a
cG2{if(a>0){(xE3
t22(xE3
used=used;}
retry_anyparams
e13
TestParam_AnyWhere
x5(cS
x62[a])c63,tX,info,used,yY2;tX=r.l93
xU1}
}
cH1:y13
lS3
a].info;used
l83
a].used;goto
retry_anyparams;}
cI1:if(a>0){--a;goto
cH1;}
lS3
0].info
lC1
false;}
retry_anyparams_4:if(nO.lY!=0
xX3!TopLevel||!info.HasRestHolder(nO.lY)){yC<tW>y12;y12.nE3
i42;for
eS1
b=0;b<y6++b
xX3
used[b])xU1
y12
yX
lR1
b));used[b]=true
lX1
b);}
if(!info.SaveOrTestRestHolder(nO.lY,y12)){goto
cI1;}
}
else{t01&y12=info.GetRestHolderValues(nO.lY)lJ1
y12
e22
a){bool
found
e23
for
eS1
b=0;b<y6++b
xX3
used[b])xU1
if(y12[a].IsIdenticalTo(lR1
b))){used[b]=true
lX1
b);found=true;xF3
if(!found){goto
cI1;}
}
}
}
lX3
n21(true,nO
yH?&*xB:0);}
case
GroupFunction:break;}
xD1}
#endif
#ifdef FP_SUPPORT_OPTIMIZER
#include <algorithm>
#include <assert.h>
using
iF2
FPoptimizer_CodeTree;using
iF2
FPoptimizer_Optimize;iF2{lO
tW
y91
yI
yM2&n02,y93,bool
inner=true){x03
NumConstant:{const
yL&eJ2
yL*)n02
cZ3
lC1
CodeTreeImmed
t02
iM2);}
case
l92
i02&eJ2
xI*)n02
cZ3
lC1
info.GetParamHolderValue
t02
index);nB3
const
xJ&eJ2
xJ
eK2
tW
tree;tree
y7
nZ3
cP);for(e31=0;a<nZ3
data
yH;++a){tW
nparam=y91(nV1
nZ3
data.param_list,a
tM3
true)c7
nparam);}
if
t02
data.lY!=0){yC<tW>trees(info.GetRestHolderValues
t02
data.lY))lR3
AddParamsMove(trees);if(tree
iK==1){assert(tree t83()==cAdd t03()==cMul t03()==cMin t03()==cMax t03()==cAnd t03()==cOr t03()==cAbsAnd t03()==cAbsOr)lR3
nQ3
lR1
0));}
t41
tree
iK==0){cI3
xC1
tV2
cAdd:case
cOr
e83=c61
0));lB
cMul:case
cAnd
e83=c61
1));default:xF3}
if(inner)tree.Rehash()lC1
tree;}
}
lX3
tW();}
}
iF2
FPoptimizer_Optimize{nD3
SynthesizeRule
yI
cQ2
tW&tree,y93){cI3
rule.ruletype
tV2
ProduceNewTree:{tree.nQ3
y91(nV1
rule.e61
0
tM3
false)cH2
case
ReplaceParams:default:{yC<lW3>list=info.GetMatchedParamIndexes();std::sort(list.tT2
list.end()e52
list
e72);a-->0;)tree.eI1
list[a]);for(e31=0;a<rule.repl_param_count;++a){tW
nparam=y91(nV1
rule.e61
a
tM3
true)c7
nparam);}
xF3}
}
#endif
#ifdef DEBUG_SUBSTITUTIONS
#include <sstream>
#include <cstring>
nE
using
iF2
l71;using
iF2
FPoptimizer_CodeTree;using
iF2
FPoptimizer_Optimize;iF2
l71{nD3
DumpMatch
yI
cQ2
cD2,const
y93,bool
DidMatch,std::ostream&o){DumpMatch(rule
c63,info,DidMatch?tG3"match"
:tG3"mismatch"
,o);}
nD3
DumpMatch
yI
cQ2
cD2,const
y93,const
char*eJ3,std::ostream&o){static
const
char
ParamHolderNames[][2]={"%"
,"&"
,"x"
,"y"
,"z"
,"a"
,"b"
,"c"
}
;o<<eJ3<<" (rule "
<<(&rule-grammar_rules)<<")"
<<":\n  Pattern    : "
;{yM2
tmp;tmp.first=SubFunction;xJ
tmp2;tmp2.data=rule
nI2;tmp
cZ3=yI
void*)&tmp2;DumpParam
x5(tmp,o);}
o<<"\n  Replacement: "
;DumpParams
x5(rule.e61
rule.repl_param_count
cR2
o<<"  Tree       : "
;nX3(tree
cR2
if(!std::strcmp(eJ3,tG3"match"
))DumpHashes(tree,o)lJ1
tN3
e22
a
xX3!tN3[a].IsDefined())xU1
o<<"           "
<<ParamHolderNames[a]<<" = "
;nX3(tN3[a]cR2}
c91
info.lS
e22
b
xX3!cK2
first)continue
lJ1
cK2
second
e22
a){o<<"         <"
<<b<<"> = "
;nX3(cK2
second[a],o);o<<std::endl;}
}
o<<std::flush;}
}
#endif
#include <list>
#include <algorithm>
#ifdef FP_SUPPORT_OPTIMIZER
nE
iF2{nM3
MarkIncompletes(x3
xX3
tree.Is_Incompletely_Hashed()cA2
bool
tF1=false
lJ1
y6++a)tF1|=MarkIncompletes(lR1
a));if(tF1)tree.Mark_Incompletely_Hashed()lC1
tF1;}
nD3
FixIncompletes(x3
xX3
tree.Is_Incompletely_Hashed()){for
n62
y6++a)FixIncompletes(lR1
a));tree
lL1}
}
}
iF2
FPoptimizer_CodeTree{lA
Sort(){data->Sort();}
lA
Rehash(bool
constantfolding
xX3
constantfolding)ConstantFolding(*this);else
Sort();data->Recalculate_Hash_NoRecursion();}
nD3
nA2
x5::Recalculate_Hash_NoRecursion(){nC2
xW1(eY
Opcode)<<56,Opcode*t43(0x1131462E270012B));Depth=1;cI3
Opcode
tV2
cImmed
y22=0;
#if 0
long
double
value=Value;lY1=crc32::calc(yI
lW3
char*)&value,xB3
value));key^=(key<<24);
#elif 0
union{yI2{lW3
char
filler1[16];iQ
v
xQ2
char
filler2[16];}
buf2;yI2{lW3
char
filler3[xB3
iQ)+16-xB3
nY1)];lY1;}
buf1;}
data;memset(&data,0,xB3
data));data.buf2.v=Value;lY1=data.buf1.key;
#else
int
tK2;iQ
iT1=std::frexp(Value,&tB2
lY1=eS1
nG2+0x8000)&0xFFFF);if(iT1<0){iT1=-iT1;key=key^0xFFFF;}
else
key+=0x10000;iT1-=iQ(0.5);key<<=39;key|=eY(iT1+iT1)*iQ(1u<<31))<<8;
#endif
xW1.hash1|=key;nY1
crc=(key>>10)|(key<<(64-10))n72((~eY
crc))*3)^1234567;break;}
case
tZ2
y22|=eY
yZ1<<48
n72((eY
yZ1)*11)^t43(0x3A83A83A83A83A0);break;}
case
cFCall:case
cPCall
y22|=eY
yZ1<<48
n72((~eY
yZ1)*7)^3456789;}
default:{size_t
e71=0
lJ1
yV3
e22
a
xX3
yV3[a].nX2>e71)e71=yV3[a].nX2;xW1.hash1+=((yV3[a].t32
hash1*(a+1))>>12)n72
yV3[a].t32
hash1
n72(3)*t43(0x9ABCD801357);xW1.hash2*=t43(0xECADB912345)n72(~yV3[a].t32
hash2)^4567890;}
Depth+=e71;}
}
if(Hash!=xW1){Hash=xW1;tP1=0;}
}
lA
FixIncompleteHashes(){MarkIncompletes(*this);FixIncompletes(*this);}
}
#endif
#include <cmath>
#include <list>
#include <cassert>
#ifdef FP_SUPPORT_OPTIMIZER
nE
iF2{using
iF2
FPoptimizer_CodeTree;nM3
xH1
yG,long
count,const
FPoptimizer_ByteCode::SequenceOpCode
x5&eN,FPoptimizer_ByteCode::iZ1
x5&synth,size_t
max_bytecode_grow_length);static
const
yI2
SinCosTanDataType{OPCODE
whichopcode;OPCODE
inverse_opcode;enum{nominator,denominator,inverse_nominator,inverse_denominator}
;OPCODE
codes[4];}
SinCosTanData[12]={{cTan,cCot,{cSin,cCos,cCsc,cSec}
}
,{cCot,cCot,{cCos,cSin,cSec,cCsc}
}
,{cCos,cSec,{cSin,cTan,cCsc,cCot}
}
,{cSec,cCos,{cTan,cSin,cCot,cCsc}
}
,{cSin,cCsc,{cCos,cCot,cSec,cTan}
}
,{cCsc,cSin,{cCot,cCos,cTan,cSec}
}
,{y42{cSinh,cCosh,y52,{cSinh,cNop,{y42
cNop,cCosh}
}
,{cCosh,cNop,{cSinh,y42
cNop}
}
,{cNop,cTanh,{cCosh,cSinh,y52,{cNop,cSinh,{cNop,cTanh,cCosh,cNop}
}
,{cNop,cCosh,{cTanh,cSinh,y52}
;}
iF2
FPoptimizer_CodeTree{lA
SynthesizeByteCode(yC<lW3>&nR,yC
x5&Immed,size_t&stacktop_max){
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<"Making bytecode for:\n"
;iV
#endif
while(RecreateInversionsAndNegations()){
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<"One change issued, produced:\n"
;iV
#endif
FixIncompleteHashes();}
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<"Actually synthesizing, after recreating inv/neg:\n"
;iV
#endif
FPoptimizer_ByteCode::iZ1
x5
synth;SynthesizeByteCode(synth,false);iP
Pull(nR,Immed,stacktop_max);}
lA
SynthesizeByteCode(FPoptimizer_ByteCode::iZ1
x5&synth,bool
MustPopTemps
cE3{cJ1*this)){tC
for
n62
12;++a){const
SinCosTanDataType&data=SinCosTanData[a];if(data.whichopcode!=cNop
xX3
lK2
data.whichopcode)xU1
tY
lA3;lA3.lT1
lA3
xI3
inverse_opcode);lA3.lL2;cJ1
lA3)){iP
l31
else{if(lK2
cInv)xU1
if(GetParam(0).lK2
data.inverse_opcode)xU1
cJ1
GetParam(0))){iP
l31
size_t
found[4];c91
4;++b){tY
tree;if(data.eK3]==cNop){tree
y7
cInv);tY
lB3;lB3.lT1
lB3
xI3
eK3^2]);lB3.lL2
c7
lB3);}
else{tree.lT1
tree
xI3
eK3]);}
tree.lL2;found[b]=iP
cT3(tree);}
if(found[data.x72!=tA
tG1
y5
x72);l41
tG1
yT1
cDiv
nF1
x72!=tA
iY
y5
x72);l41
iY
yT1
cMul
nF1
lZ1!=tA
iY
y5
lZ1);l41
iY
yT1
cRDiv
nF1
lZ1!=tA
tG1
y5
lZ1);l41
tG1
yT1
cMul,2,1);iP
l31
size_t
n_subexpressions_synthesized=SynthCommonSubExpressions(synth);cI3
GetOpcode()tV2
tZ2:iP
PushVar(GetVar());lB
cImmed
lM2
GetImmed());lB
cAdd:case
cMul:case
cMin:case
cMax:case
cAnd:case
cOr:case
cAbsAnd:case
cAbsOr:{if(GetOpcode()==cMul){bool
xJ3
e23
yR
tP
yK1&&isLongInteger(tP
tN)){yD1=makeLongInteger(tP
tN);tY
tmp(*this,lZ3
tY::CloneTag());tmp.eI1
a);tmp
lL1
if(xH1(tmp,value,FPoptimizer_ByteCode::eL1
x5::AddSequence,synth,MAX_MULI_BYTECODE_LENGTH)){xJ3=true;xF3}
if(xJ3)break;}
int
xX1=0;xD3
done(GetParamCount(),false);tY
i9;i9
y7
e7
for(;;){bool
found
e23
yR
done[a])xU1
if(iP
IsStackTop(tP)){found=true;done[a]=true;tP.n9
i9
iO
tP);if(++xX1>1){iP
c1
2);i9.lL2;iP
y62
i9);xX1=xX1-2+1;}
}
}
if(!found)break;}
yR
done[a])xU1
tP.n9
i9
iO
tP);if(++xX1>1){iP
c1
2);i9.lL2;iP
y62
i9);xX1=xX1-2+1;}
}
if(xX1==0){cI3
GetOpcode()tV2
cAdd:case
cOr:case
cAbsOr
lM2
0);lB
cMul:case
cAnd:case
cAbsAnd
lM2
1);lB
cMin:case
cMax
lM2
0);break;default:break;}
++xX1;}
assert(n_stacked==1);break;}
case
cPow:{const
tY&p0
xH3
0);const
tY&p1
xH3
1);if(!p1
yK1||!isLongInteger(p1
tN)||!xH1(p0,makeLongInteger(p1
tN),FPoptimizer_ByteCode::eL1
x5::MulSequence,synth,MAX_POWI_BYTECODE_LENGTH)){p0.n9
p1.n9
iP
c1
2);eL2
cIf:case
cAbsIf:{lZ3
FPoptimizer_ByteCode::iZ1
x5::IfData
xN2;GetParam(0).n9
iP
SynthIfStep1(xN2,e7
GetParam(1).n9
iP
SynthIfStep2(xN2);GetParam(2).n9
iP
SynthIfStep3(xN2
cH2
case
cFCall:case
cPCall:{for
n62
GetParamCount();++a)tP.n9
iP
c1
eS1)GetParamCount()yT1
yE|GetFuncNo(),0,0
cH2
default:{for
n62
GetParamCount();++a)tP.n9
iP
c1
eS1)GetParamCount()cH2}
iP
y62*this);if(MustPopTemps&&n_subexpressions_synthesized>0){size_t
top=iP
GetStackTop();iP
DoPopNMov(top-1-n_subexpressions_synthesized,top-1);}
}
}
iF2{nM3
xH1
yG,long
count,const
FPoptimizer_ByteCode::SequenceOpCode
x5&eN,FPoptimizer_ByteCode::iZ1
x5&synth,size_t
max_bytecode_grow_length
xX3
count!=0){FPoptimizer_ByteCode::iZ1
x5
backup=synth
lR3
n9
size_t
bytecodesize_backup=iP
GetByteCodeSize();FPoptimizer_ByteCode::xH1
yX3
xT2
size_t
bytecode_grow_amount=iP
GetByteCodeSize()-bytecodesize_backup;if(bytecode_grow_amount>max_bytecode_grow_length){synth=backup
lC1
false;}
lX3
true;}
else{FPoptimizer_ByteCode::xH1
yX3,eN,synth)tQ1}
}
#endif
#include <cmath>
#include <cassert>
#ifdef FP_SUPPORT_OPTIMIZER
nE
iF2{using
iF2
FPoptimizer_CodeTree;
#define FactorStack yC
const
yI2
PowiMuliType{lW3
opcode_square
xQ2
opcode_cumulate
xQ2
opcode_invert
xQ2
opcode_half
xQ2
opcode_invhalf;}
iseq_powi={cSqr,cMul,cInv,cSqrt,cRSqrt}
,iseq_muli={tC1,cAdd,cNeg,tC1,tC1}
;lO
iQ
e81
yI
PowiMuliType&xK3,const
yC
n82,iE1&stack){iQ
cL3
1);while(IP<limit
xX3
xL3
xK3.opcode_square
xX3!eP2
cM3
2;cY
opcode_invert){lN3-cQ3;cY
opcode_half
xX3
cQ3>iQ(0)&&isEvenInteger(cM3
iQ(0.5);cY
opcode_invhalf
xX3
cQ3>iQ(0)&&isEvenInteger(cM3
iQ(-0.5);++IP;xU1}
size_t
lO2=IP;iQ
lhs(1);if(xL3
cFetch){lW3
index=yE3;if(index<xZ||size_t(index-xZ)>=xS3){IP=lO2;break;}
lhs=stack[index-xZ];goto
x82;}
if(xL3
cDup){lhs=cQ3;goto
x82;x82:xQ3
cQ3);++IP;iQ
subexponent=e81(xK3
lU1
if(IP>=limit||nR[IP]!=xK3.opcode_cumulate){IP=lO2;break;}
++IP;stack.pop_back();cQ3+=lhs*subexponent;xU1}
break;}
lX3
yS3
lO
iQ
ParsePowiSequence
yI
yC
n82){iE1
stack;xQ3
iQ(1))lC1
e81(iseq_powi
lU1}
lO
iQ
ParseMuliSequence
yI
yC
n82){iE1
stack;xQ3
iQ(1))lC1
e81(iseq_muli
lU1}
n53
CodeTreeParserData{cR3
i32
CodeTreeParserData(bool
k_powi):stack(),clones(),keep_powi(k_powi){}
void
Eat
cB1
eH3,OPCODE
opcode){tW
xK;xK
y7
opcode);yC<tW>params=Pop(eH3
yR1
params);if(!keep_powi)cI3
opcode
tV2
cTanh:{tW
sinh,cosh;sinh
y7
cSinh);sinh
iO
xK
xM3
sinh
lL1
cosh
y7
cCosh);cosh
lE1
xK
xM3
cosh
tW2
pow;pow
y7
cPow);pow
lE1
cosh);pow
yV
iQ(-1)));pow
lL1
xK
y03
xK.n91
0,sinh);xK
lE1
pow
cH2
case
cTan:{tW
sin,cos;sin
y7
cSin);sin
iO
xK
xM3
sin
lL1
cos
y7
cCos);cos
lE1
xK
xM3
cos
tW2
pow;pow
y7
cPow);pow
lE1
cos);pow
yV
iQ(-1)));pow
lL1
xK
y03
xK.n91
0,sin);xK
lE1
pow
cH2
case
cPow:{tM2&p0=xK
lN
0);tM2&p1=xK
lN
1);if(p1
nD
cAdd){yC<tW>lY3(p1
iK)lJ1
p1
iK;++a){tW
pow;pow
y7
cPow);pow
iO
p0);pow
iO
p1
lF3
pow
lL1
lY3[a].swap(pow);}
xK
y7
cMul
yR1
lY3);}
break;}
default:break;}
xK.Rehash(!keep_powi);tH1,false);
#ifdef DEBUG_SUBSTITUTIONS
eT1<<eH3<<", "
<<FP_GetOpcodeName(opcode)<<"->"
<<FP_GetOpcodeName(xK
t83())<<": "
t93
xK)iL
xK);
#endif
xQ3
xK)iH2
EatFunc
cB1
eH3,OPCODE
opcode
yL2
funcno){tW
xK=CodeTreeFuncOp
x5(opcode,funcno);yC<tW>params=Pop(eH3
yR1
params);xK.lL2;
#ifdef DEBUG_SUBSTITUTIONS
eT1<<eH3<<", "
t93
xK)iL
xK);
#endif
tH1);xQ3
xK)iH2
AddConst
eO2){tW
xK=CodeTreeImmed
cC3);tH1);Push(xK)iH2
AddVar
eS1
varno){tW
xK=CodeTreeVar
x5(varno);tH1);Push(xK)iH2
SwapLastTwoInStack(){xR3
1].swap(xR3
2])iH2
Dup(){Fetch(xS3-1)iH2
Fetch
cB1
which){Push(stack[which]);}
nL1
T>void
Push(T
tree){
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<t93
tree)iL
tree);
#endif
xQ3
tree)iH2
PopNMov
cB1
target,size_t
source){stack[target]=stack[source];stack
n93
target+1);}
tW
PullResult(){clones.clear();tW
cL3
stack.back());stack
n93
xS3-1)lC1
yS3
yC<tW>Pop
cB1
n_pop){yC<tW>cL3
n_pop);eR1
0;n<n_pop;++n)cP3.swap(xR3
n_pop+n]);
#ifdef DEBUG_SUBSTITUTIONS
eR1
n_pop;n-->0;){eT1;nX3(cP3)iL
cP3);}
#endif
stack
n93
xS3-n_pop)lC1
yS3
size_t
GetStackTop(n71
xS3;}
private:void
FindClone(tW&,bool=true){tC
private:yC<tW>stack;std::multimap<nC2,tW>clones;bool
keep_powi;private:CodeTreeParserData
yI
CodeTreeParserData&);CodeTreeParserData&cL1=yI
CodeTreeParserData&);}
;lO
yI2
IfInfo{tW
yZ2;tW
thenbranch
tW3
endif_location;IfInfo():yZ2(),thenbranch(),endif_location(){}
}
;}
iF2
FPoptimizer_CodeTree{lA
GenerateFrom
yI
yC<lW3>&nR,const
yC
x5&Immed,const
lZ3
FunctionParserBase
x5::Data&cA3,bool
keep_powi){yC<tW>n22;n22.nE3
cA3.numVariables);eR1
0;n<cA3.numVariables;++n){n22
yX
CodeTreeVar
x5(n+tZ2));}
GenerateFrom(nR,Immed,cA3,n22,keep_powi);}
lA
GenerateFrom
yI
yC<lW3>&nR,const
yC
x5&Immed,const
lZ3
FunctionParserBase
x5::Data&cA3,const
nE2
n22,bool
keep_powi){
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<"ENTERS GenerateFrom()\n"
;
#endif
CodeTreeParserData
x5
sim(keep_powi);yC<IfInfo
x5>eJ;for
cB1
IP=0,DP=0;;++IP){t42:while(!eJ
xT3&&(eJ.e8==IP||(IP<nR
e72)&&xL3
cJump&&eJ
yU1.IsDefined()))){tY
elsebranch=sim
lP2
t52
eJ.back().yZ2)t52
eJ
yU1)t52
elsebranch);n6
3,cIf);eJ.pop_back();}
if(IP>=nR
e72))break
xQ2
opcode=nR[IP];if((opcode==cSqr||opcode==cDup||opcode==cInv||opcode==cNeg||opcode==cSqrt||opcode==cRSqrt||opcode==cFetch)){size_t
was_ip=IP;iQ
tK2=ParsePowiSequence
x5(nR,IP,eJ
xT3?nR
e72):eJ.e8,sim.nP
1);if
nG2!=1.0){n3
tK2)iW1
goto
t42;}
if(opcode==cDup||opcode==cFetch||opcode==cNeg){iQ
xO2=ParseMuliSequence
x5(nR,IP,eJ
xT3?nR
e72):eJ.e8,sim.nP
1);if
lY2!=1.0){n3
xO2);n6
2
eL3
goto
t42;}
}
IP=was_ip;}
if(iG1>=tZ2){sim.Push(n22[opcode-tZ2]);}
else{cI3
iG1
tV2
cIf:case
cAbsIf:{eJ
n93
eJ
e72)+1);tY
res(sim
lP2);eJ.back().yZ2.swap(res);eJ.e8=nR
e72);IP+=2;xU1}
case
cJump:{tY
res(sim
lP2);eJ
yU1.swap(res);eJ.e8=nR[IP+1]+1;IP+=2;xU1}
case
cImmed:n3
Immed[DP++]);lB
cDup:sim.Dup();lB
cNop:lB
cFCall:{lW3
funcno=yE3;assert(funcno<fpdata.FuncPtrs.size())xQ2
params=cA3.FuncPtrs[funcno].params;sim.EatFunc(params,iG1,funcno
cH2
case
cPCall:{lW3
funcno=yE3;assert(funcno<fpdata.tB3.size());const
FunctionParserBase
x5&p=*cA3.tB3[funcno].parserPtr
xQ2
params=cA3.tB3[funcno].params;yC<tY>paramlist=sim.Pop(params);tY
t62;t62.GenerateFrom(p.data->nR,p.data->Immed,*p.data,paramlist)t52
t62
cH2
case
cInv:n3
1
nH2
cDiv);lB
cNeg:xU3
cNeg);break;n3
0
nH2
cSub);lB
cSqr:n3
2)iW1
lB
cSqrt:n3
yL1
cRSqrt:n3-yL1
cCbrt:n3
lU
3))iW1
lB
cDeg:n3
fp_const_rad_to_deg
x5
yS1
cRad:n3
fp_const_deg_to_rad
x5
yS1
cExp:xY1
goto
xW3;n3
fp_const_e
x5()nH2
cPow);lB
cExp2:xY1
goto
xW3;n3
2.0
nH2
cPow);lB
cCot:xU3
cTan);if
yS
cCsc:xU3
cSin);if
yS
cSec:xU3
cCos);if
yS
cInt:
#ifndef __x86_64
xY1{xU3
cInt
cH2
#endif
n3
0.5);xV3
xU3
cFloor);lB
cLog10:xU3
yA3
fp_const_log10inv
x5
yS1
cLog2:xU3
yA3
fp_const_log2inv
x5
yS1
yQ3:tK3
xU3
yA3
fp_const_log2inv
x5());n6
3
eL3
lB
cHypot:n3
2)iW1
tK3
n3
2)iW1
xV3
n3
yL1
cSinCos:sim.Dup();xU3
cSin);tK3
xU3
cCos);lB
cRSub:tK3
case
cSub:xY1{n6
2,cSub
cH2
n3-1);n6
2
eL3
xV3
lB
cRDiv:tK3
case
cDiv:xY1{n6
2,cDiv
cH2
n3-1)iW1
n6
2
eL3
lB
cAdd:case
cMul:case
cMod:case
cPow:case
cEqual:case
cLess:case
cGreater:case
cNEqual:case
cLessOrEq:case
cGreaterOrEq:case
cAnd:case
cOr:case
cAbsAnd:case
cAbsOr:n6
2,xI1
lB
cNot:case
cNotNot:case
lQ2:case
cAbsNotNot:xU3
xI1
lB
cFetch:sim.Fetch(yE3);lB
cPopNMov:{lW3
stackOffs_target=yE3
xQ2
stackOffs_source=yE3;sim.PopNMov(stackOffs_target,stackOffs_source
cH2
#ifndef FP_DISABLE_EVAL
case
cEval:{size_t
paramcount=cA3.numVariables;n6
paramcount,xI1
break;}
#endif
default:xW3:xQ2
funcno=opcode-cAbs;assert(funcno<FUNC_AMOUNT);const
FuncDefinition&func=Functions[funcno];n6
func.params,xI1
xF3}
nQ3
sim
lP2);
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<"Produced tree:\n"
;iV
#endif
}
}
#endif
#include <algorithm>
#ifdef FP_SUPPORT_OPTIMIZER
#include <assert.h>
#define FP_MUL_COMBINE_EXPONENTS
iF2{nE
using
iF2
FPoptimizer_CodeTree;lO
static
void
AdoptChildrenWithSameOpcode(eH2
#ifdef DEBUG_SUBSTITUTIONS
bool
iU1
e23
#endif
e92
if(lR1
a)nD
xC1){
#ifdef DEBUG_SUBSTITUTIONS
if(!iU1){std::cout<<"Before assimilation: "
c8
iU1=true;}
#endif
tree.AddParamsMove(lR1
a).GetUniqueRef().tM1),a);}
#ifdef DEBUG_SUBSTITUTIONS
if(iU1){std::cout<<"After assimilation:   "
c8}
#endif
}
}
iF2
FPoptimizer_CodeTree{nD3
ConstantFolding(eH2
tree.Sort();
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<"Runs ConstantFolding for: "
c8
DumpHashes(tree);
#endif
if(false){redo:lR3
Sort();
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<"Re-runs ConstantFolding: "
c8
DumpHashes(tree);
#endif
}
if(xC1!=cImmed){iG
p=l01(tree);if(p
tL3
l22&&p.min==p.max)lM
p.min);tC}
if(false){ReplaceTreeWithOne
e83.ReplaceWithImmed(iQ(1))lC1;ReplaceTreeWithZero
e83.ReplaceWithImmed(iQ(0))lC1;ReplaceTreeWithParam0:
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<"Before replace: "
iJ
std::hex<<'['<<cE2.hash1<<','<<cE2.hash2<<']'<<std::dec
c8
#endif
tree.nQ3
lR1
0));
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<"After replace: "
iJ
std::hex<<'['<<cE2.hash1<<','<<cE2.hash2<<']'<<std::dec
c8
#endif
c9
i62
xC1
tV2
cImmed:lB
tZ2:lB
cAnd:case
cAbsAnd:cT
bool
c5
e23
e92{if(!tN2
a)))c5=true;lC3
a),x92
cAbsAnd)tV2
IsNever
e4
c02:nD1);lB
n01
i62
tree
iK
tV2
0:e5
1
e83
y7
x92
cAnd
tU3
c9
default:if(x92
cAnd||!c5)if(ConstantFolding_AndLogic(cP2
eL2
cOr:case
cAbsOr:cT
bool
c5
e23
e92{if(!tN2
a)))c5=true;lC3
a),x92
cAbsOr)){iI1
e5
iB2
nD1);lB
n01
i62
tree
iK
tV2
0
e4
1
e83
y7
x92
cOr
tU3
c9
default:if(x92
cOr||!c5)if(ConstantFolding_OrLogic(cP2
eL2
cNot:case
lQ2:{lW3
n51
0;cI3
lE
GetOpcode()tV2
cEqual:n51
cNEqual;lB
cNEqual:n51
cEqual;lB
cLess:n51
cGreaterOrEq;lB
cGreater:n51
cLessOrEq;lB
cLessOrEq:n51
cGreater;lB
cGreaterOrEq:n51
cLess;lB
cNotNot:n51
cNot;lB
cNot:n51
cNotNot;lB
lQ2:n51
cAbsNotNot;lB
cAbsNotNot:n51
lQ2;break;default:break;}
if(opposite){tree
y7
OPCODE(opposite))lR3
SetParamsMove(lE
GetUniqueRef().tM1));c9
i62
tB
0),x92
lQ2)tV2
c02
e4
iB2
e5
n01
if(x92
cNot&&GetPositivityInfo(lR1
0))==c02)tree
y7
lQ2);if(x71
cIf||x71
nS3{tW
i72=lR1
0);tM2&ifp1=i72
lN
1);tM2&ifp2=i72
lN
2);if(ifp1
c12
ifp1
c22
ifp1
nD
cNot
tU3
t72
xM3
t82;tW
p2;p2
cW1
p2
yG3)e3
if(ifp2
c12
ifp2
c22
tree.e7
t72);t82;tW
p2;p2
y7
ifp2
nD
cNot
tU3
p2
yG3
lN
0))e3
eL2
cNotNot:case
cAbsNotNot:{if(tN2
0)))iN2
lC3
0),x92
cAbsNotNot)tV2
IsNever
e4
c02:e5
n01
if(x92
cNotNot&&GetPositivityInfo(lR1
0))==c02)tree
y7
cAbsNotNot);if(x71
cIf||x71
nS3{tW
i72=lR1
0);tM2&ifp1=i72
lN
1);tM2&ifp2=i72
lN
2);if(ifp1
c12
ifp1
nD
lQ2){tree.SetParam(0,i72
xM3
tree
iO
ifp1);tW
p2;p2
cW1
p2
yG3)e3
if(ifp2
c12
ifp2
c22
tree.e7
t72);t82;tree
yG3);tree
y7
i72.e7
c9}
eL2
cIf:case
cAbsIf:{if(ConstantFolding_IfOperations(cP2
break;}
case
cMul:{NowWeAreMulGroup:;AdoptChildrenWithSameOpcode(tree);iQ
nG1=iQ(1)tW3
tI1=0;bool
nH1=false
iV1
if(!lR1
a)yK1)xU1
iQ
immed=lR1
a)tN;if(immed==iQ(0))goto
ReplaceTreeWithZero;nG1*=immed;++tI1;}
if(tI1>1||(tI1==1&&FloatEqual(nG1,e03)nH1=true;if(nH1){
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<"cMul: Will add new "
tA3
nG1<<"\n"
;
#endif
e92
if(lR1
a)yK1){
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<" - For that, deleting "
tA3
lR1
a)tN
iJ"\n"
;
#endif
lD3!FloatEqual(nG1,e03
tree
yV
nG1));i62
tree
iK
tV2
0:e5
1:iN2
default:if(ConstantFolding_MulGrouping(cP2
if(ConstantFolding_MulLogicItems(cP2
eL2
cAdd:cT
iQ
iH1=0.0
tW3
tI1=0;bool
nH1=false
iV1
if(!lR1
a)yK1)xU1
iQ
immed=lR1
a)tN;iH1+=immed;++tI1;}
if(tI1>1||(tI1==1&&iH1==0.0))nH1=true;if(nH1){
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<"cAdd: Will add new "
tA3
iH1<<"\n"
iJ"In: "
c8
#endif
e92
if(lR1
a)yK1){
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<" - For that, deleting "
tA3
lR1
a)tN
iJ"\n"
;
#endif
lD3!(iH1==0.0))tree
yV
iH1));i62
tree
iK
tV2
0
e4
1:iN2
default:if(ConstantFolding_AddGrouping(cP2
if(ConstantFolding_AddLogicItems(cP2
eL2
cMin:cT
size_t
xA2=0;iG
e0
iV1
while(a+1<tree
iK&&lR1
iS
lR1
a+1)))nD1+1);e91
l22&&(!e0.l22||(p.max)<e0.max)){e0.max=p.max;e0.l22=true;xA2=a;}
}
if(e0.l22)e92{e91
n32
a!=xA2&&p.min>=e0.max)lD3
tree
iK==1){iN2
eL2
cMax:cT
size_t
xA2=0;iG
eZ
iV1
while(a+1<tree
iK&&lR1
iS
lR1
a+1)))nD1+1);e91
n32(!eZ.has_min||p.min>eZ.min)){eZ.min=p.min;eZ.xE2
true;xA2=a;}
}
if(eZ.yJ3{e92{e91
l22&&a!=xA2&&(p.max)<eZ.min){nD1);}
}
}
if(tree
iK==1){iN2
eL2
cEqual:case
cNEqual:case
cLess:case
cGreater:case
cLessOrEq:case
cGreaterOrEq:if(ConstantFolding_Comparison(cP2
lB
cAbs:{iG
p0=y3
0));if
eA2
iN2
if
eB2{tree
y03
tree
yV
e03;goto
NowWeAreMulGroup;}
if(x71
cMul){tM2&p=lR1
0);yC<tW>lE3;yC<tW>y72
lJ1
p
iK;++a){p0=l01(p
lF3
if
eA2{lE3
yX
p
lF3}
if
eB2{y72
yX
p
lF3}
}
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<"Abs: mul group has "
<<lE3
e72)<<" pos, "
<<y72
e72)<<"neg\n"
;
#endif
if(!lE3
xT3||!y72
xT3){
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<"AbsReplace-Before: "
;nX3(tree)iJ"\n"
<<std::flush;DumpHashes(tree
y82;
#endif
tW
cB3;cB3
y7
cMul)lJ1
p
iK;++a){p0=l01(p
lF3
if(eA2||eB2){}
else
cB3
iO
p
lF3}
cB3
tW2
lG3;lG3
y7
cAbs);lG3
lE1
cB3);lG3
tW2
xL1
cMul);lY3
lE1
lG3);xM1
AddParamsMove(lE3);if(!y72
xT3
xX3
y72
e72)%2)lY3
yV
iQ(-1)));xM1
AddParamsMove(y72);}
tree.Become
cL2);
#ifdef DEBUG_SUBSTITUTIONS
std::cout<<"AbsReplace-After: "
;nX3(tree
y82
iJ"\n"
<<std::flush;DumpHashes(tree
y82;
#endif
goto
NowWeAreMulGroup;}
}
break;}
#define HANDLE_UNARY_CONST_FUNC(funcname) x8)lM funcname yD));tC
case
cLog:eZ3(fp_log);if(x71
cPow){tW
pow=lR1
0);if(GetPositivityInfo(pow
lN
0))==c02)tH3
pow
lV1
tree.eU1
if(GetEvennessInfo(pow
lN
1))==c02)tH3
tW
abs;abs
y7
cAbs);abs
lE1
pow
xM3
abs
lL1
pow
lV1
pow.n91
0,abs)lR3
eU1}
t41
x71
cAbs){tW
pow=nW2
0);if(pow
nD
cPow)tH3
tW
abs;abs
y7
cAbs);abs
lE1
pow
xM3
abs
lL1
pow
lV1
pow.n91
0,abs)lR3
eU1}
lB
cAcosh:eZ3(fp_acosh);lB
cAsinh:eZ3(fp_asinh);lB
cAtanh:eZ3(fp_atanh);lB
cAcos:eZ3(fp_acos);lB
cAsin:eZ3(fp_asin);lB
cAtan:eZ3(fp_atan);lB
cCosh:eZ3(fp_cosh);lB
cSinh:eZ3(fp_sinh);lB
cTanh:eZ3(fp_tanh);lB
cSin:eZ3(fp_sin);lB
cCos:eZ3(fp_cos);lB
cTan:eZ3(fp_tan);lB
cCeil:if(n4
eZ3(fp_ceil);lB
cTrunc:if(n4
eZ3(fp_trunc);lB
cFloor:if(n4
eZ3(fp_floor);lB
cInt:if(n4
eZ3(fp_int);lB
cCbrt:eZ3(fp_cbrt);lB
cSqrt:eZ3(fp_sqrt);lB
cExp:eZ3(fp_exp);lB
cLog2:eZ3(fp_log2);lB
cLog10:eZ3(fp_log10);lB
yQ3:y0)lM
fp_log2
yD)*y1)t12
cMod:y0)lM
fp_mod
tO3
t12
cAtan2:{iG
p0=y2
p1=y3
1));x8&&fp_equal
yD,iQ(0))xX3
p1
eM3
p1.max)<0)lM
fp_const_pi
yH3
if(p1.n32
p1.min>=0.0)lM
iQ(0));tC}
if(xW2
fp_equal(y1,iQ(0))xX3
p0
eM3
p0.max)<0)lM-fp_const_pihalf
yH3
if(p0.n32
p0.min>0)lM
fp_const_pihalf
yH3}
y0)lM
fp_atan2
tO3;tC
if((p1.n32
p1.min>0.0)||(p1
eM3
p1.max)<fp_const_negativezero
x5())){tW
xB2;xB2
y7
cPow);xB2
lE1
tU2);xB2
yV
iQ(-1)));xB2
tW2
xD2;xD2
y03
xD2.l11
xD2
lE1
xB2);xD2
lL1
tree
y7
cAtan)nT3
0,xD2)l33
1);eL2
cPow:{if(ConstantFolding_PowOperations(cP2
break;}
case
cDiv:y0&&y1!=0.0)lM
lE
GetImmed()/y1)t12
cInv:x8&&lE
GetImmed()!=0.0)lM
iQ(1)/lE
GetImmed())t12
cSub:y0)lM
lE
GetImmed()-y1)t12
cNeg:x8)lM-lE
GetImmed())t12
cRad:x8)lM
RadiansToDegrees
yD))t12
cDeg:x8)lM
DegreesToRadians
yD))t12
cSqr:x8)lM
lE
GetImmed()*lE
GetImmed())t12
cExp2:eZ3(fp_exp2);lB
cRSqrt:x8)lM
iQ(1)/fp_sqrt
yD))t12
cCot:x8)yI3
fp_tan
yD);if(xC
cSec:x8)yI3
fp_cos
yD);if(xC
cCsc:x8)yI3
fp_sin
yD);if(xC
cHypot:y0)lM
fp_hypot
tO3
t12
cRDiv:case
cRSub:case
cDup:case
cFetch:case
cPopNMov:case
cSinCos:case
cNop:case
cJump:lB
cPCall:case
cFCall:case
cEval:xF3}
#endif
#ifdef FP_SUPPORT_OPTIMIZER
nE
using
iF2
FPoptimizer_CodeTree;iF2
yM1
lW3
Compare>yI2
Comp{}
;iD2>yI2
Comp<cLess
tQ3<yM
cLessOrEq
tQ3<=yM
cGreater
tQ3>yM
cGreaterOrEq
tQ3>=yM
cEqual
tQ3==yM
cNEqual
tQ3!=b;}
}
;}
iF2
FPoptimizer_CodeTree{lO
nQ
set_abs(xX3!n32!l22){tP3}
t41!n32
max<iQ(0)){xE2
true;min=-max;iC}
t41!yJ3{tP3
iC}
t41
min>=iQ(0)n92;t41!l22){tP3}
t41
max<iQ(0)){iQ
tmp(-max);max=-min;min=tmp;}
t41-min>=max){max=-min;min=iQ(0);}
else{min=iQ(0);}
}
lO
nQ
set_neg(){std::swap(has_min,l22);std::swap(min,max);min=-min;max=-max;tD
set_min_if
lR2
if(n32
Comp<Compare>()(min,v))min=func(min);else{xE2
i82
has_min;min=i82
min;}
tD
set_max_if
lR2
if(l22&&Comp<Compare>()(max,v))max=func(max);else{l22=i82
l22;max=i82
max;}
tD
set_min_max_if
lR2
set_min_if<Compare>(v,func,yK3
set_max_if<Compare>(v,func,yK3}
lO
nQ
set_min(iQ(lW
if(yJ3
min=func(min);else{xE2
i82
has_min;min=i82
min;}
}
lO
nQ
set_max(iQ(lW
if(l22)max=func(max);else{l22=i82
l22;max=i82
max;}
}
lO
nQ
xF2
iQ(lW
set_min(func,yK3
set_max(func,yK3}
lO
iG
l01
yG)
#ifdef DEBUG_SUBSTITUTIONS_extra_verbose
{iG
tmp=CalculateResultBoundaries_do(tree)iJ"Estimated boundaries: "
;if(tmp.yJ3
std::cout<<tmp.min;else
std::cout<<"-inf"
iJ" .. "
;if(tmp.l22)std::cout<<tmp.max;else
std::cout<<"+inf"
iJ": "
;nX3(tree)iJ
std::endl
lC1
tmp;}
lO
iG
nC
CalculateResultBoundaries_do
yG)
#endif
{eW1
eA1(-fp_const_pihalf
x5(),fp_const_pihalf
x5());eW1
pi_limits(-fp_const_pi
x5(),fp_const_pi
x5());eW1
abs_pi_limits(iQ(0),fp_const_pi
x5());using
iF2
std;cI3
xC1
tV2
cImmed:n5
tree
tN
c63
tN);case
cAnd:case
cAbsAnd:case
cOr:case
cAbsOr:case
cNot:case
lQ2:case
cNotNot:case
cAbsNotNot:case
cEqual:case
cNEqual:case
cLess:case
cLessOrEq:case
cGreater:case
cGreaterOrEq:{n5
iQ(0),iQ(1));}
case
cAbs:l9
m.set_abs(cD
cLog:l9
m.y9
fp_log
cD
cLog2:l9
m.y9
fp_log2
cD
cLog10:l9
m.y9
fp_log10
cD
cAcosh:l9
m.iE2
set_min_max_if<cGreaterOrEq
yM3
fp_acosh
cD
cAsinh:l9
m.xF2
fp_asinh
cD
cAtanh:l9
m.iE2
set_min_if<cGreater>(iQ(-1),fp_atanh);m.iE2
set_max_if<cLess
yM3
fp_atanh
cD
cAcos:l9
n5(m
eM3
m.max)<iQ(1))?fp_acos(m.max):iQ(0),(m.n32(m.min)>=iQ(-1))?fp_acos(m.min):fp_const_pi
x5());}
case
cAsin:l9
m.iE2
set_min_if<cGreater>(iQ(-1),fp_asin,eA1);m.iE2
set_max_if<cLess
yM3
fp_asin,eA1
cD
cAtan:l9
m.xF2
fp_atan,eA1
cD
cAtan2:{iG
p0=y2
p1=y3
1));x8&&fp_equal
yD,iQ(0))){lX3
abs_pi_limits;}
if(xW2
fp_equal(y1,iQ(0))){lX3
eA1;}
lX3
pi_limits;}
case
cSin:l9
bool
nR1=!iA2||!m.l22||(m.max-m.min)>=(yA
nR1)cU
iQ
min=c32
min,yA
min<iQ(0))min
yT
iQ
max=c32
max,yA
max<iQ(0))max
yT
if(max<min)max
yT
bool
xJ1=(min<=fp_const_pihalf
x5()&&max>=fp_const_pihalf
x5());bool
nI1=(min<=iQ(iA&&max>=iQ(iA);if(xJ1&&nI1)cU
if(nI1)n5
iQ(-1),iX1
if(xJ1)n5
xG2
iQ(1));n5
xG2
iX1}
case
cCos:l9
if(iA2)m.min+=fp_const_pihalf
x5();if(m.l22)m.max+=fp_const_pihalf
x5();bool
nR1=!iA2||!m.l22||(m.max-m.min)>=(yA
nR1)cU
iQ
min=c32
min,yA
min<iQ(0))min
yT
iQ
max=c32
max,yA
max<iQ(0))max
yT
if(max<min)max
yT
bool
xJ1=(min<=fp_const_pihalf
x5()&&max>=fp_const_pihalf
x5());bool
nI1=(min<=iQ(iA&&max>=iQ(iA);if(xJ1&&nI1)cU
if(nI1)n5
iQ(-1),iX1
if(xJ1)n5
xG2
iQ(1));n5
xG2
iX1}
case
cTan:{n5);}
case
cCeil:l9
m.yN1
cFloor
n23
cD
cTrunc
n23);m.yN1
cInt
n23);m.yN1
cSinh:l9
m.xF2
fp_sinh
cD
cTanh:l9
m.xF2
fp_tanh,iG(iQ(-1),iQ(1))cD
cCosh:l9
if(iA2
xX3
m.l22
xX3
m.min>=iQ(0)&&m.max
i92{m.min
cH}
t41(m.min)<iQ(0)&&m.max
i92{iQ
tmp
cH
if(tmp>m.max)m.max=tmp;m.min=iQ(1);}
eN3
min
cH
std::swap(m.min,m.max);}
}
else{if(m.min
i92{m.iC
m.min=fp_cosh(m.min);}
eN3
iC
m.min=iQ(1);}
}
}
eN3
xE2
true;m.min=iQ(1);if(m.l22){m.min=fp_cosh(m.max);m.iC}
else
m.iC}
lX3
m;}
case
cIf:case
cAbsIf:{iG
res1=y3
1));iG
res2=y3
2));if(!res2.yJ3
res1.xE2
false;t41
res1.n32(res2.min)<res1.min)res1.min=res2.min;if(!res2.l22)res1.iC
t41
res1
eM3
res2.max)>res1.max)res1.max=res2.max
lC1
res1;}
case
cMin:{bool
iD
e23
bool
iE
e23
y23;nX
m
y33!iA2)iD=true;xK1
has_min||(m.min)<y43
eP3=m.min;if(!m.l22)iE=true;xK1
l22||(m.max)<y53
eQ3=m.max;}
if(iD
c01
xE2
false;if(iE
c01
iC
lX3
y63
cMax:{bool
iD
e23
bool
iE
e23
y23;nX
m
y33!iA2)iD=true;xK1
has_min||m.min>y43
eP3=m.min;if(!m.l22)iE=true;xK1
l22||m.max>y53
eQ3=m.max;}
if(iD
c01
xE2
false;if(iE
c01
iC
lX3
y63
cAdd:{y23(iQ(0),iQ(0));nX
item
y33
item.has_min
eP3+=item.min
xC2
xE2
false;if(item.l22
eQ3+=item.max
xC2
iC
if(!cQ3.n32!cQ3.l22)break;}
if
iZ2
n32
cQ3.l22&&y43>y53)std::swap
iZ2
min,y53)lC1
y63
cMul:{yI2
Value{enum
lH3{t92,tJ1,tA2}
;lH3
e9;iQ
value;Value(lH3
t):e9(t),value(0){}
Value(iQ
v):e9(t92),value(v){}
bool
y92
n71
e9==tJ1||(e9==t92&&value<iQ(0))iH2
cL1*=yI
Value&rhs
xX3
e9==t92&&rhs.e9==t92)value*=rhs.value;else
e9=(y92)!=rhs.y92)?tJ1:tA2);}
tL2<yI
Value&rhs
n71(e9==tJ1&&rhs.e9!=tJ1)||(e9==t92&&(rhs.e9==tA2||(rhs.e9==t92&&value<rhs.value)));}
}
;yI2
xZ1{Value
xH2,xI2;xZ1():xH2(Value::tA2),xI2(Value::tJ1){}
void
n52
Value
yO3,const
Value&value2){yO3*=value2;if(yO3<xH2)xH2=yO3;if(xI2<yO3)xI2=yO3;}
}
;y23(iQ(y73
nX
item
y33!item.n32!item.l22)n5
tV3
lI3=cQ3.has_min?Value
iZ2
min
lF2
tJ1
tV3
lJ3=cQ3.l22?Value
iZ2
max
lF2
tA2
tV3
lK3=item.has_min?Value(item.min
lF2
tJ1
tV3
lL3=item.l22?Value(item.max
lF2
tA2);xZ1
range
xN3
lI3,lK3)xN3
lI3,lL3)xN3
lJ3,lK3)xN3
lJ3,lL3);if(range.xH2.e9==Value::t92
eP3=range.xH2.value
xC2
xE2
false;if(range.xI2.e9==Value::t92
eQ3=range.xI2.value
xC2
iC
if(!cQ3.n32!cQ3.l22)break;}
if
iZ2
n32
cQ3.l22&&y43>y53)std::swap
iZ2
min,y53)lC1
y63
cMod:{iG
x=y2
y=y3
1));if(y.l22
xX3
y.max
i92{if(!x.has_min||(x.min)<0)n5-y.max,y.max);eR3
iQ(0),y.max);}
else{if(!x.l22||(x.max)>=0)n5
y.max,-y.max);eR3
y.max,fp_const_negativezero
x5());}
}
eR3);}
case
cPow:{if(xW2
y1==iQ(0
xP3
y73}
x8&&lE
GetImmed()==iQ(0
xP3
0),iQ(0));}
x8&&fp_equal
yD,iQ(1)xP3
y73}
if(xW2
y1>0&&GetEvennessInfo(tU2)==c02
lH2
y1;iG
tmp=y2
cQ3;cQ3.xE2
true;y43=0;if(tmp.n32
tmp.min>=0
eP3
eO3(tmp.min,tB2
t41
tmp.l22&&tmp.max<=0
eP3
eO3(tmp.max,tB2
cQ3.iC
if(tmp.n32
tmp.l22){cQ3.l22=true;y53=std::max(fp_abs(tmp.min),fp_abs(tmp.max));y53
eO3
iZ2
max,tB2}
lX3
yS3
iG
p0=y2
p1=y3
1));cN1
p0_positivity=(p0.n32
tZ3)i92?c02:(p0
eM3
p0.max)<iQ(0)?iB2
Unknown);cN1
yA2=GetEvennessInfo(tU2);cN1
t0=Unknown;cI3
p0_positivity){iI1
t0=c02;lB
iB2{t0=yA2;break;}
default:cI3
yA2){iI1
t0=c02;lB
iB2
lB
Unknown:{if(xW2!eP2
y1)&&y1
i92{t0=c02;}
xF3
i62
t0){iI1{iQ
min=iQ(0);if(p0.n32
p1.yJ3{min
eO3
tZ3,p1.min);if
tZ3<iQ(0)&&(!p1.l22||p1.max
i92&&min
i92
min=iQ(0);}
if(p0.n32
p0.min>=iQ(0)&&p0.l22&&p1.l22){iQ
max=pow(p0.max,p1.max);if(min>max)std::swap(min,max);n5
min,max);}
n5
min,false);}
case
iB2{n5
false,fp_const_negativezero
x5());}
default:{break;}
eL2
cNeg:l9
m.set_neg(cD
cSub:{nL
cNeg
e82
1));tmp
y7
cAdd);tmp.nK
0));tmp
lO3
c42
cInv:lV-1)))c42
cDiv:{nL
cInv
e82
1))iR
lO3
c42
cRad:{tW
tmp
iR
yV
fp_const_rad_to_deg
x5()))c42
cDeg:{tW
tmp
iR
yV
fp_const_deg_to_rad
x5()))c42
cSqr:lV
2)))c42
cExp:eC2
cPow);tmp
yV
fp_const_e
x5()));tmp.nK
0))c42
cExp2:eC2
cPow);tmp
yV
iQ(2)));tmp.nK
0))c42
cCbrt:l9
m.xF2
fp_cbrt
cD
cSqrt:l9
if(iA2)m.min=(m.min)<0?0:fp_sqrt(m.min);if(m.l22)m.max=(m.max)<0?0:fp_sqrt(m.max
cD
cRSqrt:lV-0.5)))c42
cHypot:{tW
xsqr,ysqr,add,sqrt;xsqr.nK
0));xsqr
yV
iQ(2)));ysqr.nK
1));ysqr
yV
iQ(2)));xsqr
y7
cPow);ysqr
y7
cPow);add
lE1
xsqr);add
lE1
ysqr);add
y7
cAdd);sqrt
lE1
add);sqrt
y7
cSqrt)lC1
l01(sqrt);}
case
yQ3:{nL
cLog2
e82
0));tmp
y03
tmp
lO3;tmp.nK
1))c42
cCot:{nL
cTan
e82
0
xU
lI
cSec:{nL
cCos
e82
0
xU
lI
cCsc:{nL
cSin
e82
0
xU
l01(tmp);}
lB
cRDiv:case
cRSub:case
cDup:case
cFetch:case
cPopNMov:case
cSinCos:case
cNop:case
cJump:case
tZ2:lB
cPCall:lB
cFCall:lB
cEval:break;}
n5);}
lO
cN1
GetIntegerInfo
yG){cI3
xC1
tV2
cImmed:lX3
eP2
tree
tN)?c02:IsNever;case
cFloor:case
cCeil:case
cTrunc:case
cInt:lX3
c02;case
cAnd:case
cOr:case
cNot:case
cNotNot:case
cEqual:case
cNEqual:case
cLess:case
cLessOrEq:case
cGreater:case
cGreaterOrEq:lX3
c02;case
cIf:{cN1
a=GetIntegerInfo(tU2);cN1
b=GetIntegerInfo(tI3);if(a==b
n92
a
lC1
Unknown;}
case
cAdd:case
cMul:{e92
if(GetIntegerInfo(lR1
a))!=c02
n92
Unknown
lC1
c02;}
default:break;}
lX3
Unknown;}
nM3
IsLogicalValue
yG){cI3
xC1
tV2
cImmed:lX3
FloatEqual(tree
tN,iQ(0))||FloatEqual(tree
tN,iQ(1));case
cAnd:case
cOr:case
cNot:case
cNotNot:case
cAbsAnd:case
cAbsOr:case
lQ2:case
cAbsNotNot:case
cEqual:case
cNEqual:case
cLess:case
cLessOrEq:case
cGreater:case
cGreaterOrEq:nZ
cMul:{e92
if(!tN2
a))n92
false
tQ1
case
cIf:case
cAbsIf:{lX3
tN2
1))nN1
tI3);}
default:break;}
xD1}
#endif
#ifdef FP_SUPPORT_OPTIMIZER
nE
#if defined(__x86_64) || !defined(FP_SUPPORT_CBRT)
# define CBRT_IS_SLOW
#endif
#if defined(DEBUG_POWI) || defined(DEBUG_SUBSTITUTIONS)
#include <cstdio>
#endif
t2{extern
const
lW3
char
powi_table[256];}
iF2{using
iF2
FPoptimizer_CodeTree;nM3
IsOptimizableUsingPowi(long
immed,long
penalty=0){FPoptimizer_ByteCode::iZ1
x5
synth;iP
PushVar(tZ2)tW3
bytecodesize_backup=iP
GetByteCodeSize();FPoptimizer_ByteCode::xH1(immed,FPoptimizer_ByteCode::eL1
x5::MulSequence,synth)tW3
bytecode_grow_amount=iP
GetByteCodeSize()-bytecodesize_backup
lC1
bytecode_grow_amount<size_t(MAX_POWI_BYTECODE_LENGTH-penalty);}
nD3
ChangeIntoRootChain(tW&tree,bool
iC2,long
tC2,long
tD2){while(tD2>0)eC2
cCbrt);eE2
tmp
lL1
tree
yB2--tD2;}
while(tC2>0)eC2
cSqrt);if(iC2){tmp
y7
cRSqrt);iC2
e23}
eE2
tmp
lL1
tree
yB2--tC2;}
if(iC2)eC2
cInv);eE2
tree
yB2}
}
lO
yI2
RootPowerTable{static
const
iQ
RootPowers[(1+4)*(1+3)];}
;lO
const
iQ
t5(1+4)*(1+3)]={iQ(1),lU
2),lU
2*2),lU
2*2*2),lU
2*2*2*2),lU
3
l42
2
l42
2*2
l42
2*2*2
l42
2*2*2*2
l42
3
l42
3*2
l42
3*2*2
l42
3*2*2*2
l42
3*2*2*2*2
l42
3*3
l42
3*3*2
l42
3*3*2*2
l42
3*3*2*2*2
l42
3*3*2*2*2*2)}
;yI2
PowiResolver{static
const
lW3
MaxSep=4;static
lP3
MaxOp=5;typedef
int
yP3;typedef
long
lV3;typedef
long
eD;yI2
xJ2{xJ2():n_int_sqrt(0),n_int_cbrt(0),sep_list(),n11(0){}
int
n_int_sqrt;int
n_int_cbrt;int
eK1
MaxSep];eD
n11;}
;lO
static
xJ2
CreatePowiResult(iQ
tK2){xJ2
cQ3;yP3
cE=FindIntegerFactor
nG2);if(cE==0){
#ifdef DEBUG_POWI
tE2"no factor found for %Lg\n"
yC2);
#endif
lX3
yS3
cQ3.n11=lO1
nG2,cE);lV3
c52=EvaluateFactorCost(cE,0,0,0)+cA
cQ3.n11);int
eS3=0;int
eT3=0;int
lM3=0;
#ifdef DEBUG_POWI
tE2"orig = %Lg\n"
yC2);tE2"plain factor = "
tC3"%ld\n"
,(int)cE,(long)c52);
#endif
for
eS1
n_s=0;n_s<MaxSep;++n_s){int
xE=0;lV3
y01=c52;yP3
yA1=cE;for(int
s=1;s<MaxOp*4;++s){
#ifdef CBRT_IS_SLOW
if(s>=MaxOp)break;
#endif
int
n_sqrt=s%MaxOp;int
n_cbrt=s/MaxOp;if(n_sqrt+n_cbrt>4)xU1
iQ
lP1=tK2;lP1-=t5
s];eX1=FindIntegerFactor(lP1);if
lY2!=0){eD
xL=lO1(lP1,xO2);lV3
cost=EvaluateFactorCost
lY2,eS3+n_sqrt,eT3+n_cbrt,lM3+1)+cA
xL);
#ifdef DEBUG_POWI
tE2"Candidate sep %u (%d*sqrt %d*cbrt)factor = "
tC3"%ld (for %Lg to %ld)\n"
,s,n_sqrt,n_cbrt,xO2,(long)cost
i1
lP1,(long)xL);
#endif
if(cost<y01){xE=s;yA1=xO2;y01=cost;}
}
}
if(!xE)break;
#ifdef DEBUG_POWI
tE2"CHOSEN sep %u (%d*sqrt %d*cbrt)factor = "
tC3"%ld, exponent %Lg->%Lg\n"
,xE,xE%MaxOp,xE/MaxOp,yA1,y01
i1
nG2)i1
nG2-t5
xE]));
#endif
cQ3.eK1
n_s]=xE
n42-=t5
xE];eS3+=xE%MaxOp;eT3+=xE/MaxOp;c52=y01;cE=yA1;lM3+=1;}
cQ3.n11=lO1
nG2,cE);
#ifdef DEBUG_POWI
tE2"resulting exponent is %ld (from exponent=%Lg, best_factor=%Lg)\n"
,cQ3.n11
yC2
i1
cE);
#endif
while(cE%2==0){++cQ3
eG2;cE/=2;}
while(cE%3==0){++cQ3.n_int_cbrt;cE/=3;}
lX3
yS3
private:static
lV3
cA
eD
xL){static
std::map
yD2
i6;if(xL<0){lV3
cost=22
lC1
cost+cA-xL);}
std::map
yD2::nP3
i=i6.nO2
xL);if(i!=i6.c51
xL
n92
i
yQ2;std::pair
yD2
cL3
xL,0.0);lV3&cost=cQ3
cZ3;while(xL>1){int
xO2=0;if(xL<256){xO2=FPoptimizer_ByteCode::powi_table[xL];if
lY2&128)xO2&=127;else
xO2=0;if
lY2&64)xO2=-lY2&63)-1;}
if
lY2){cost+=cA
xO2);xL/=xO2;xU1}
if(!(xL&1)){xL/=2;cost+=6;}
else{cost+=7;xL-=1;}
}
i6.nN3,cQ3)lC1
cost;cN3
eD
lO1
eO2,eX1){lX3
makeLongInteger
cC3*iQ
lY2));cN3
bool
c11
eO2,eX1){iQ
v=value*iQ
lY2)lC1
isLongInteger(v);cN3
yP3
FindIntegerFactor
eO2){eX1=(2*2*2*2);
#ifdef CBRT_IS_SLOW
#else
xO2*=(3*3*3);
#endif
yP3
lN3
0;if(c11
cC3,xO2)){lN3
xO2;while(lY2%2)==0&&c11
cC3,xO2/2))lN3
xO2/=2;while(lY2%3)==0&&c11
cC3,xO2/3))lN3
xO2/=3;}
#ifdef CBRT_IS_SLOW
if(cQ3==0
xX3
c11
cC3,3)n92
3;}
#endif
lX3
yS3
static
int
EvaluateFactorCost(int
xO2,int
s,int
c,int
nmuls){lP3
lQ3=6;
#ifdef CBRT_IS_SLOW
lP3
c62=25;
#else
lP3
c62=8;
#endif
int
lN3
s*lQ3+c*c62;while
lY2%2==0){xO2/=2;cQ3+=lQ3;}
while
lY2%3==0){xO2/=3;cQ3+=c62;}
cQ3+=nmuls
lC1
yS3}
;}
iF2
FPoptimizer_CodeTree{nM3
nC
RecreateInversionsAndNegations(bool
prefer_base2){bool
changed=false
lJ1
GetParamCount();cJ3
tP.RecreateInversionsAndNegations(prefer_base2))lT2
if(changed){exit_changed:Mark_Incompletely_Hashed()tQ1
cI3
GetOpcode()tV2
cMul:{yC<tW>iJ1;tW
iK1,y21;if(true){bool
nJ1
e23
iQ
lS2=0;lK1
nW
yE2
0)tF2
tE
eU3
nJ1=true;lS2=tE
1)tN;xF3
if(nJ1){iQ
immeds=1.0;lK1
nW
yK1){immeds*=powgroup
tN;y11}
lK1-->0;){tW&powgroup=tP;if(powgroup
yE2
0)tF2
tE
eU3
tW&log2=tE
0);log2.lH1
log2
y7
yQ3);log2
yV
fp_pow(immeds,iQ(1)/lS2)));log2
lL1
xF3}
}
lK1
nW
yE2
eU3
tM2&exp_param=tE
1);iQ
tK2=exp_param
tN;if(fp_equal
nG2,iQ(-1))){lH1
iJ1
yX
tP
xM3
y11
t41
tK2<0&&isInteger
nG2)){tW
iF;iF
y7
cPow);iF
iO
tE
0));iF
yV-tK2));iF
lL1
iJ1
yX
iF);lH1
y11}
t41
powgroup
tF2!iK1.eC1
iK1=tE
0);lH1
y11
t41
powgroup
nD
yQ3&&!y21.eC1
y21=powgroup;lH1
y11}
if(!iJ1
xT3){lT2
tW
xN1;xN1
y03
xN1
c33
iJ1);xN1
tW2
xL1
cMul);xM1
SetParamsMove
eU
if(xM1
IsImmed()&&fp_equal
cL2
tN,e03{iL1
cInv)cM
xN1);}
else{if(xM1
nX2>=xN1.nX2){iL1
cDiv
lU2
cM
xN1);}
else{iL1
cRDiv)cM
xN1
lU2;}
}
}
if(iK1.eC1
tW
xL1
e7
xM1
SetParamsMove
eU
while(xM1
RecreateInversionsAndNegations(prefer_base2))xM1
FixIncompleteHashes();iL1
yQ3)cM
iK1
lU2;lT2}
if(y21.eC1
tW
xL1
cMul);lY3
lE1
y21
lN
1));xM1
AddParamsMove
eU
while(xM1
RecreateInversionsAndNegations(prefer_base2))xM1
FixIncompleteHashes();DelParams();iL1
yQ3)cM
y21
lN
0)lU2;lT2
eL2
cAdd:{yC<tW>tG2;for
cB1
a=eT
tP
nD
cMul){iM1
xO1:;tW&lY3=tP
tT
for
cB1
b=lY3
iK;b-->0;){if
cL2
lN
b
yB3
xO2=lY3
lN
b)tN;if(fp_equal
lY2
xW
xO1;}
xM1
lH1
xM1
eI1
b)c72
t41
fp_equal
lY2,iQ(-2))xX3
tV
xO1;}
xM1
lH1
xM1
eI1
b);lY3
yV
iQ(2)))c72}
}
if(t1){lY3
cK1
lY3);y11}
t41
tP
nD
cDiv){iM1
xP1:;tW&xN1=tP
tT
if(xN1
lN
0)yK1){e62
xN1
lN
0)tN
xW
xP1;}
xN1.lH1
xN1.eI1
0);xN1
y7
cInv)c72}
if(t1
xX3
tV
xP1;}
xN1
cK1
xN1);y11}
t41
tP
nD
cRDiv){iM1
nW1:;tW&xN1=tP
tT
if(xN1
lN
eU3
e62
xN1
lN
1)tN
xW
nW1;}
xN1.lH1
xN1.eI1
1);xN1
y7
cInv)c72}
if(t1
xX3
tV
nW1;}
xN1
cK1
xN1);y11}
if(!tG2
xT3){
#ifdef DEBUG_SUBSTITUTIONS
tE2"Will make a Sub conversion in:\n"
);fflush(stdout);iV
#endif
tW
yB1;yB1
y7
cAdd);yB1
c33
tG2);yB1
tW2
tJ3;tJ3
y7
cAdd);tJ3
c33
tM1));tJ3
lL1
if(tJ3
yK1&&fp_equal(tJ3
tN,iQ(0))){iL1
cNeg)cG3);}
else{if(tJ3.nX2==1){iL1
cRSub)cG3)cH3;}
t41
yB1
nD
cAdd){iL1
cSub)cH3
cG3
lN
0)e52
1;a<yB1
iK;++a){tW
c82;c82
y7
cSub);c82
c33
tM1));c82.lL2
cM
c82)cG3
lF3}
}
else{iL1
cSub)cH3
cG3);}
}
#ifdef DEBUG_SUBSTITUTIONS
tE2"After Sub conversion:\n"
);fflush(stdout);iV
#endif
eL2
cPow:{tM2&p0
xH3
0);tM2&p1
xH3
1);if(p1
yK1
xX3
p1
tN!=iQ(0)&&!eP2
p1
tN)){eE
xJ2
r=eE
CreatePowiResult(fp_abs(p1
tN));if(r.n11!=0){bool
tK1
e23
if(p1
tN<0&&r.eK1
0]==0&&r
eG2>0){tK1=true;}
#ifdef DEBUG_POWI
tE2"Will resolve powi %Lg as powi(chain(%d,%d),%ld)"
i1
fp_abs(p1
tN),r
eG2,r.n_int_cbrt,r.n11);eR1
0;n<eE
MaxSep;++n
xX3
r
yD3==0)break;int
n_sqrt=r
yD3%eE
MaxOp;int
n_cbrt=r
yD3/eE
MaxOp;tE2"*chain(%d,%d)"
,n_sqrt,n_cbrt);}
tE2"\n"
);
#endif
tW
yF2
xH3
0);tW
xK2=yF2;xK2.lH1
ChangeIntoRootChain(xK2,tK1,r
eG2,r.n_int_cbrt);xK2
tW2
pow;if(r.n11!=1){pow
y7
cPow);pow
lE1
xK2);pow
yV
iQ(r.n11)));}
else
pow.swap(xK2);tW
mul;mul
y03
mul
lE1
pow);eR1
0;n<eE
MaxSep;++n
xX3
r
yD3==0)break;int
n_sqrt=r
yD3%eE
MaxOp;int
n_cbrt=r
yD3/eE
MaxOp;tW
c92=yF2;c92.lH1
ChangeIntoRootChain(c92,false,n_sqrt,n_cbrt);c92
lL1
mul
lE1
c92);}
if(p1
tN<0&&!tK1){mul
lL1
iL1
cInv);n91
0,mul);eI1
1);}
else{iL1
cMul);SetParamsMove(mul.tM1));}
#ifdef DEBUG_POWI
iV
#endif
lT2
xF3}
if(GetOpcode()==cPow&&(!p1
yK1||!isLongInteger(p1
tN)||!IsOptimizableUsingPowi
x5(makeLongInteger(p1
tN)))xX3
p0
yK1&&p0
tN>0.0
xX3
prefer_base2){iQ
xL2=fp_log2(p0
tN);e62
xL2,e03{eI1
0);}
else{lX
CodeTreeImmed(xL2))n42
iO
p1)n42.Rehash()iT}
iL1
cExp2);lT2}
else{iQ
xL2=fp_log(p0
tN);e62
xL2,e03{eI1
0);}
else{lX
CodeTreeImmed(xL2))n42
iO
p1)n42.Rehash()iT}
iL1
cExp);lT2}
}
t41
GetPositivityInfo(p0)==c02
xX3
prefer_base2){tW
log;log
y7
cLog2);log
iO
p0);log
lL1
lX
p1)n42
lE1
log)n42
lL1
iL1
cExp2)iT
lT2}
else{tW
log;log
y7
cLog);log
iO
p0);log
lL1
lX
p1)n42
lE1
log)n42
lL1
iL1
cExp)iT
lT2}
}
}
break;}
default:break;}
if(changed)goto
exit_changed
lC1
changed;}
}
#endif
#ifdef FP_SUPPORT_OPTIMIZER
nE
iF2{using
iF2
FPoptimizer_CodeTree;class
eD1{size_t
y31
tW3
eF
tW3
eG
tW3
lQ1;cR3
eD1():y31(0),eF(0),eG(0),lQ1(0){}
void
yR3
OPCODE
op){y31+=1;tP2
cCos)++eF;tP2
cSin)++eG;tP2
cSec)++eF;tP2
cCsc)++eG;tP2
cTan)++lQ1;tP2
cCot)++lQ1;}
size_t
GetCSEscore
cD3
size_t
lN3
y31
lC1
yS3
int
NeedsSinCos
cD3
bool
always_sincostan=(y31==(eF+eG+lQ1));if((lQ1&&(eG||eF))||(eG&&eF)xX3
always_sincostan
n92
1
lC1
2;}
lX3
0;}
size_t
MinimumDepth
cD3
size_t
n_sincos=std::min(eF,eG);if(n_sincos==0
n92
2
lC1
1;}
}
;n53
TreeCountType:public
std::multimap<nC2,std::pair<eD1,tW> >{}
;nD3
FindTreeCounts(i0&iN1,cD2,OPCODE
iO1{nX1
i=iN1.nO2
cE2);bool
found
e23
for(;i!=iN1.c51
cE2;++i
xX3
tree.IsIdenticalTo(i
yQ2
eI2){i
yQ2.first.yR3
iO1;found=true;xF3
if(!found){eD1
count;count.yR3
iO1;iN1.nN3,std::make_pair(cE2,std::make_pair
yX3
c63)));}
for
n62
y6++a)FindTreeCounts(iN1,lR1
a)c63.e7}
yI2
yZ{bool
BalanceGood;bool
cB;}
;lO
yZ
e33
yZ3
root,tM2&child
xX3
root.IsIdenticalTo(child)){yZ
lN3{true,true}
lC1
yS3
yZ
lN3{true,false}
;if(root
nD
cIf||root
nD
nS3{yZ
cond
iW
0
xO3
yZ
cJ
iW
1
xO3
yZ
cC
iW
2
xO3
if(cond.cB||cJ.cB||cC.cB){cQ3.cB=true;}
cQ3
eA=((cJ.cB==cC.cB)cT2&&(cond
eA||(cJ.cB&&cC.cB))&&(cJ
eA
cT2&&(cC
eA
cT2;}
else{bool
eY1
e23
bool
nK1
e23
for
cB1
b=root
iK,a=0;a<b;++a){yZ
tmp
iW
a
xO3
if(tmp.cB
c01
cB=true;if(tmp
eA
eX3)eY1=true;t41
tmp.cB)nK1=true;}
if(eY1&&!nK1)cQ3
eA
e23}
lX3
yS3
nM3
eH1
yZ3
eY3
cD2,const
FPoptimizer_ByteCode::iZ1
x5&synth,const
i0&iN1){for
cB1
b=tree
iK,a=0;a<b;++a){tM2&leaf=lR1
a);nX1
iY1;cI2
i0::const_iterator
i=iN1
nA3
i!=iN1.end();++i
xX3
i->eG3
leaf.GetHash())continue
l61
i->lW2
size_t
score=occ.GetCSEscore();tM2&candidate=i->lX2
cN2
candidate))xU1
if(leaf.nX2<occ.MinimumDepth())xU1
if(score<2)xU1
if(e33(eY3
leaf)eA
eX3)continue
tQ1
if(eH1(eY3
leaf,synth,iN1)cA2}
xD1
nM3
iP1
yZ3
nO3,tM2&expr){y8
nO3
lN
iS
expr)cA2
y8
iP1(nO3
lN
a),expr)n92
true
lC1
false;}
nM3
GoodMomentForCSE
yZ3
nO3,tM2&expr
xX3
nO3
nD
cIf
cA2
y8
nO3
lN
iS
expr)cA2
size_t
tH2=0;y8
iP1(nO3
lN
a),expr))++tH2
lC1
tH2!=1;}
}
iF2
FPoptimizer_CodeTree{lO
size_t
nC
SynthCommonSubExpressions(FPoptimizer_ByteCode::iZ1
x5&synth
cE3{size_t
stacktop_before=iP
GetStackTop();i0
iN1;FindTreeCounts(iN1,*this,e7
#ifdef DEBUG_SUBSTITUTIONS_CSE
DumpHashes(*this);
#endif
for(;;){size_t
xM2=0;nX1
iY1;for(nX1
j,i=iN1
nA3
i!=iN1.end();i=j){j=i;++j
l61
i->lW2
size_t
score=occ.GetCSEscore();cD2=i->lX2
#ifdef DEBUG_SUBSTITUTIONS_CSE
std::cout<<"Score "
<<score<<":\n"
;DumpTreeWithIndent(tree);
#endif
cN2
tree))xX
if(tree.nX2<occ.MinimumDepth())xX
if(score<2)xX
if(e33
xZ3)eA
eX3)xX
if(eH1
xZ3,synth,iN1)){xU1}
if(!GoodMomentForCSE
xZ3))xX
score*=tree.nX2;if(score>xM2){xM2=score;iY1=i;}
}
if(xM2<=0)break
l61
iY1->lW2
cD2=iY1->lX2
#ifdef DEBUG_SUBSTITUTIONS_CSE
std::cout<<tG3"Common Subexpression:"
;nX3
x5(tree)iJ"\n"
;
#endif
int
xQ1=occ.NeedsSinCos();tW
tI2,tJ2;if(xQ1){tI2
iO
tree);tI2
y7
cSin);tI2
lL1
tJ2
iO
tree);tJ2
y7
cCos);tJ2
lL1
cN2
tI2)||iP
Find(tJ2)xX3
xQ1==2){iN1.erase(iY1);xU1}
xQ1=0;}
}
tree.SynthesizeByteCode(synth,false);iN1.erase(iY1);
#ifdef DEBUG_SUBSTITUTIONS_CSE
std::cout<<"Done with Common Subexpression:"
;nX3
x5(tree)iJ"\n"
;
#endif
if(xQ1
xX3
xQ1==2){yP2
yE1);}
iP
l12
cSinCos,1,2);iP
y62
tI2,1);iP
y62
tJ2,0);}
}
lX3
iP
nP
stacktop_before;}
}
#endif
#ifdef FP_SUPPORT_OPTIMIZER
nD3
FunctionParserBase
x5::Optimize(){using
iF2
FPoptimizer_CodeTree;lH1
tW
tree
lR3
GenerateFrom(data->nR,data->Immed,*data);FPoptimizer_Optimize::ApplyGrammars(tree);yC<lW3>yT3;yC
x5
immed
tW3
stacktop_max=0
lR3
SynthesizeByteCode(yT3,immed,stacktop_max);if(data->StackSize!=stacktop_max){data->StackSize=lW3(stacktop_max);
#ifndef FP_USE_THREAD_SAFE_EVAL
data->Stack
n93
stacktop_max);
#endif
}
data->nR.swap(yT3);data->Immed.swap(immed);}
#ifdef FP_SUPPORT_LONG_INT_TYPE
cZ
long
eZ1
#endif
#ifdef FP_SUPPORT_MPFR_FLOAT_TYPE
cZ
MpfrFloat
eZ1
#endif
#ifdef FP_SUPPORT_GMP_INT_TYPE
cZ
GmpInt
eZ1
#endif
FUNCTIONPARSER_INSTANTIATE_TYPES
#endif

#endif
