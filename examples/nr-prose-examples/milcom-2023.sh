#!/bin/bash

 #/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
 #
 # NIST-developed software is provided by NIST as a public
 # service. You may use, copy and distribute copies of the software in
 # any medium, provided that you keep intact this entire notice. You
 # may improve, modify and create derivative works of the software or
 # any portion of the software, and you may copy and distribute such
 # modifications or works. Modified works should carry a notice
 # stating that you changed the software and should note the date and
 # nature of any such change. Please explicitly acknowledge the
 # National Institute of Standards and Technology as the source of the
 # software.
 #
 # NIST-developed software is expressly provided "AS IS." NIST MAKES
 # NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY
 # OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 # WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
 # NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR
 # WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED
 # OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT
 # WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE
 # SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE
 # CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
 #
 # You are solely responsible for determining the appropriateness of
 # using and distributing the software and you assume all risks
 # associated with its use, including but not limited to the risks and
 # costs of program errors, compliance with applicable laws, damage to
 # or loss of data, programs or equipment, and the unavailability or
 # interruption of operation. This software is not intended to be used
 # in any situation where a failure could cause risk of injury or
 # damage to property. The software developed by NIST employees is not
 # subject to copyright protection within the United States.

#-------------Usage:-------------------------------------------------------------#
# Each evaluation is a string with the format:                                   #
#     {evalnameRoot}_{evalnameValue}_{nSquads}_{nSoldiersPerSquad}               #
#     _{platoonSideLength}_{squadRadius}_                                        #
#     _{durationTraffic}_{ulTraffic}_{dlTraffic}                                 #
#     _{enableSensing}_{numerologySl}_{rri}_{maxNumTx}                           #
#     _{ulPercentage}_{dlPercentage}_{mcs}                                       #
# To run several evaluations separate them with a space -->" "<--                # 
# e.g.:                                                                          #
#     evaluations="Evaluation1 Evaluation2 EvaluationN"                          #
# To run this script it needs to be copied to the root folder of the ns3         #
# distrubution and run_args                                                      #                              
#   ./milcom-2023.sh                                                             # 
#--------------------------------------------------------------------------------#

#evaluations=""
#for ((squadSideLength=100; squadSideLength<=1000; squadSideLength+=50))
#do
#  evaluations=${evaluations}" ulSquadSideLenght_${squadSideLength}_4_5_1000_${squadSideLength}_900_false_20_true_false"
#done

#1-6: SPS, mu=2, Changing traffic direction
evaluations1=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations1=${evaluations1}" NineRemoteMuTwoSpsUlNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_false_false_2_20_5"
done
evaluations2=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations2=${evaluations2}" NineRemoteMuTwoSpsUlSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_false_true_2_20_5"
done
evaluations3=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations3=${evaluations3}" NineRemoteMuTwoSpsDlNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_false_true_false_2_20_5"
done
evaluations4=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations4=${evaluations4}" NineRemoteMuTwoSpsDlSensing_${squadRadius}_1_9_50_${squadRadius}_10_false_true_true_2_20_5"
done
evaluations5=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations5=${evaluations5}" NineRemoteMuTwoSpsUlDlNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_true_false_2_20_5"
done
evaluations6=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations6=${evaluations6}" NineRemoteMuTwoSpsUlDlSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_true_true_2_20_5"
done

#7-12: Dyn, mu=2, Changing traffic direction
evaluations7=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations7=${evaluations7}" NineRemoteMuTwoDynUlNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_false_false_2_0_5"
done
evaluations8=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations8=${evaluations8}" NineRemoteMuTwoDynUlSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_false_true_2_0_5"
done
evaluations9=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations9=${evaluations9}" NineRemoteMuTwoDynDlNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_false_true_false_2_0_5"
done
evaluations10=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations10=${evaluations10}" NineRemoteMuTwoDynDlSensing_${squadRadius}_1_9_50_${squadRadius}_10_false_true_true_2_0_5"
done
evaluations11=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations11=${evaluations11}" NineRemoteMuTwoDynUlDlNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_true_false_2_0_5"
done
evaluations12=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations12=${evaluations12}" NineRemoteMuTwoDynUlDlSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_true_true_2_0_5"
done

#13-15: 1 UE, UL, NoSensing, changing mu
evaluations13=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations13=${evaluations13}" OneRemoteMuTwoSpsUlNoSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_false_false_2_20_5"
done
evaluations14=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations14=${evaluations14}" OneRemoteMuOneSpsUlNoSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_false_false_1_20_5"
done
evaluations15=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations15=${evaluations15}" OneRemoteMuZeroSpsUlNoSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_false_false_0_20_5"
done
#16-18: 5 UE, UL, NoSensing, changing mu
evaluations16=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations16=${evaluations16}" FiveRemoteMuTwoSpsUlNoSensing_${squadRadius}_1_5_50_${squadRadius}_10_true_false_false_2_20_5"
done
evaluations17=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations17=${evaluations17}" FiveRemoteMuOneSpsUlNoSensing_${squadRadius}_1_5_50_${squadRadius}_10_true_false_false_1_20_5"
done
evaluations18=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations18=${evaluations18}" FiveRemoteMuZeroSpsUlNoSensing_${squadRadius}_1_5_50_${squadRadius}_10_true_false_false_0_20_5"
done
#19-21: 9 UE, UL, NoSensing changing mu
evaluations19=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations19=${evaluations19}" NineRemoteMuOneSpsUlNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_false_false_1_20_5"
done
evaluations20=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations20=${evaluations20}" NineRemoteMuZeroSpsUlNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_false_false_0_20_5"
done

#evalArray=("$evaluations1" "$evaluations2" "$evaluations3" "$evaluations4" "$evaluations5" "$evaluations6" "$evaluations7" "$evaluations8" "$evaluations9" "$evaluations10" "$evaluations11" "$evaluations12" "$evaluations13" "$evaluations14" "$evaluations15" "$evaluations16" "$evaluations17" "$evaluations18" "$evaluations19" "$evaluations20")

#22-24: 1 UE, UL, Sensing, changing mu
evaluations22=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations22=${evaluations22}" OneRemoteMuTwoSpsUlSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_false_true_2_20_5"
done
evaluations23=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations23=${evaluations23}" OneRemoteMuOneSpsUlSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_false_true_1_20_5"
done
evaluations24=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations24=${evaluations24}" OneRemoteMuZeroSpsUlSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_false_true_0_20_5"
done
#25-27: 5 UE, UL, Sensing, changing mu
evaluations25=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations25=${evaluations25}" FiveRemoteMuTwoSpsUlSensing_${squadRadius}_1_5_50_${squadRadius}_10_true_false_true_2_20_5"
done
evaluations26=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations26=${evaluations26}" FiveRemoteMuOneSpsUlSensing_${squadRadius}_1_5_50_${squadRadius}_10_true_false_true_1_20_5"
done
evaluations27=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations27=${evaluations27}" FiveRemoteMuZeroSpsUlSensing_${squadRadius}_1_5_50_${squadRadius}_10_true_false_true_0_20_5"
done
#28-29: 9 UE, UL, Sensing changing mu
evaluations28=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations28=${evaluations28}" NineRemoteMuOneSpsUlSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_false_true_1_20_5"
done
evaluations29=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations29=${evaluations29}" NineRemoteMuZeroSpsUlSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_false_true_0_20_5"
done

#evalArray=("$evaluations22" "$evaluations23" "$evaluations24" "$evaluations25" "$evaluations26" "$evaluations27" "$evaluations28" "$evaluations29")

#Changing percentages 
##30-31 Full UL Half DL
evaluations30=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations30=${evaluations30}" NineRemoteMuTwoSpsUlDlFullUlHalfDlNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_true_false_2_20_5_100_50"
done
evaluations31=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations31=${evaluations31}" NineRemoteMuTwoSpsUlDlFullUlHalfDlSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_true_true_2_20_5_100_50"
done
##32-33 Full UL Quarter DL
evaluations32=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations32=${evaluations32}" NineRemoteMuTwoSpsUlDlFullUlQuarterDlNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_true_false_2_20_5_100_25"
done
evaluations33=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations33=${evaluations33}" NineRemoteMuTwoSpsUlDlFullUlQuarterDlSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_true_true_2_20_5_100_25"
done
##34-35 Half UL Full DL
evaluations34=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations34=${evaluations34}" NineRemoteMuTwoSpsUlDlHalfUlFullDlNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_true_false_2_20_5_50_100"
done
evaluations35=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations35=${evaluations35}" NineRemoteMuTwoSpsUlDlHalfUlFullDlSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_true_true_2_20_5_50_100"
done
##36-37 Quarter UL Full DL
evaluations36=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations36=${evaluations36}" NineRemoteMuTwoSpsUlDlQuarterUlFullDlNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_true_false_2_20_5_25_100"
done
evaluations37=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations37=${evaluations37}" NineRemoteMuTwoSpsUlDlQuarterUlFullDlSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_true_true_2_20_5_25_100"
done
##38-39 Half UL Half DL
evaluations38=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations38=${evaluations38}" NineRemoteMuTwoSpsUlDlHalfUlHalfDlNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_true_false_2_20_5_50_50"
done
evaluations39=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations39=${evaluations39}" NineRemoteMuTwoSpsUlDlHalfUlHalfDlSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_true_true_2_20_5_50_50"
done

#evalArray=("$evaluations30" "$evaluations31" "$evaluations32" "$evaluations33" "$evaluations34" "$evaluations35" "$evaluations36" "$evaluations37" "$evaluations38" "$evaluations39")

##Changing maxNumTx
##40-41  maxNumTx=1
evaluations40=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations40=${evaluations40}" NineRemoteMuTwoSpsUlDlMaxNumTxOneNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_true_false_2_20_1_100_100"
done
evaluations41=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations41=${evaluations41}" NineRemoteMuTwoSpsUlDlMaxNumTxOneSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_true_true_2_20_1_100_100"
done

##42-43  maxNumTx=2
evaluations42=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations42=${evaluations42}" NineRemoteMuTwoSpsUlDlMaxNumTxTwoNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_true_false_2_20_2_100_100"
done
evaluations43=""
for ((squadRadius=50; squadRadius<=1000; squadRadius+=50))
do
  evaluations43=${evaluations43}" NineRemoteMuTwoSpsUlDlMaxNumTxTwoSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_true_true_2_20_2_100_100"
done

#evalArray=("$evaluations40" "$evaluations41" "$evaluations42" "$evaluations43")


###########################################
#MCS 5 EVAL
###########################################

#nUEs=1 Mu=0 nTx=1

evaluations44=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations44=${evaluations44}" OneRemoteMuZeroNtxOneULNoSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_false_false_0_20_1_100_100_5"
done
evaluations45=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations45=${evaluations45}" OneRemoteMuZeroNtxOneULSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_false_true_0_20_1_100_100_5"
done

evaluations46=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations46=${evaluations46}" OneRemoteMuZeroNtxOneDLNoSensing_${squadRadius}_1_1_50_${squadRadius}_10_false_true_false_0_20_1_100_100_5"
done
evaluations47=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations47=${evaluations47}" OneRemoteMuZeroNtxOneDLSensing_${squadRadius}_1_1_50_${squadRadius}_10_false_true_true_0_20_1_100_100_5"
done

evaluations48=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations48=${evaluations48}" OneRemoteMuZeroNtxOneULDlNoSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_true_false_0_20_1_100_100_5"
done
evaluations49=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations49=${evaluations49}" OneRemoteMuZeroNtxOneULDlSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_true_true_0_20_1_100_100_5"
done

#nUEs=1 Mu=0 nTx=2

evaluations50=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations50=${evaluations50}" OneRemoteMuZeroNtxTwoULNoSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_false_false_0_20_2_100_100_5"
done
evaluations51=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations51=${evaluations51}" OneRemoteMuZeroNtxTwoULSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_false_true_0_20_2_100_100_5"
done

evaluations52=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations52=${evaluations52}" OneRemoteMuZeroNtxTwoDLNoSensing_${squadRadius}_1_1_50_${squadRadius}_10_false_true_false_0_20_2_100_100_5"
done
evaluations53=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations53=${evaluations53}" OneRemoteMuZeroNtxTwoDLSensing_${squadRadius}_1_1_50_${squadRadius}_10_false_true_true_0_20_2_100_100_5"
done

evaluations54=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations54=${evaluations54}" OneRemoteMuZeroNtxTwoULDlNoSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_true_false_0_20_2_100_100_5"
done
evaluations55=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations55=${evaluations55}" OneRemoteMuZeroNtxTwoULDlSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_true_true_0_20_2_100_100_5"
done


#nUEs=1 Mu=0 nTx=5

evaluations56=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations56=${evaluations56}" OneRemoteMuZeroNtxFiveULNoSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_false_false_0_20_5_100_100_5"
done
evaluations57=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations57=${evaluations57}" OneRemoteMuZeroNtxFiveULSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_false_true_0_20_5_100_100_5"
done

evaluations58=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations58=${evaluations58}" OneRemoteMuZeroNtxFiveDLNoSensing_${squadRadius}_1_1_50_${squadRadius}_10_false_true_false_0_20_5_100_100_5"
done
evaluations59=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations59=${evaluations59}" OneRemoteMuZeroNtxFiveDLSensing_${squadRadius}_1_1_50_${squadRadius}_10_false_true_true_0_20_5_100_100_5"
done

evaluations60=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations60=${evaluations60}" OneRemoteMuZeroNtxFiveULDlNoSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_true_false_0_20_5_100_100_5"
done
evaluations61=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations61=${evaluations61}" OneRemoteMuZeroNtxFiveULDlSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_true_true_0_20_5_100_100_5"
done


##
#nUEs=1 Mu=1 nTx=1

evaluations62=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations62=${evaluations62}" OneRemoteMuOneNtxOneULNoSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_false_false_1_20_1_100_100_5"
done
evaluations63=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations63=${evaluations63}" OneRemoteMuOneNtxOneULSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_false_true_1_20_1_100_100_5"
done

evaluations64=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations64=${evaluations64}" OneRemoteMuOneNtxOneDLNoSensing_${squadRadius}_1_1_50_${squadRadius}_10_false_true_false_1_20_1_100_100_5"
done
evaluations65=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations65=${evaluations65}" OneRemoteMuOneNtxOneDLSensing_${squadRadius}_1_1_50_${squadRadius}_10_false_true_true_1_20_1_100_100_5"
done

evaluations66=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations66=${evaluations66}" OneRemoteMuOneNtxOneULDlNoSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_true_false_1_20_1_100_100_5"
done
evaluations67=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations67=${evaluations67}" OneRemoteMuOneNtxOneULDlSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_true_true_1_20_1_100_100_5"
done

#nUEs=1 Mu=1 nTx=2

evaluations68=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations68=${evaluations68}" OneRemoteMuOneNtxTwoULNoSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_false_false_1_20_2_100_100_5"
done
evaluations69=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations69=${evaluations69}" OneRemoteMuOneNtxTwoULSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_false_true_1_20_2_100_100_5"
done

evaluations70=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations70=${evaluations70}" OneRemoteMuOneNtxTwoDLNoSensing_${squadRadius}_1_1_50_${squadRadius}_10_false_true_false_1_20_2_100_100_5"
done
evaluations71=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations71=${evaluations71}" OneRemoteMuOneNtxTwoDLSensing_${squadRadius}_1_1_50_${squadRadius}_10_false_true_true_1_20_2_100_100_5"
done

evaluations72=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations72=${evaluations72}" OneRemoteMuOneNtxTwoULDlNoSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_true_false_1_20_2_100_100_5"
done
evaluations73=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations73=${evaluations73}" OneRemoteMuOneNtxTwoULDlSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_true_true_1_20_2_100_100_5"
done


#nUEs=1 Mu=1 nTx=5

evaluations74=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations74=${evaluations74}" OneRemoteMuOneNtxFiveULNoSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_false_false_1_20_5_100_100_5"
done
evaluations75=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations75=${evaluations75}" OneRemoteMuOneNtxFiveULSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_false_true_1_20_5_100_100_5"
done

evaluations76=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations76=${evaluations76}" OneRemoteMuOneNtxFiveDLNoSensing_${squadRadius}_1_1_50_${squadRadius}_10_false_true_false_1_20_5_100_100_5"
done
evaluations77=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations77=${evaluations77}" OneRemoteMuOneNtxFiveDLSensing_${squadRadius}_1_1_50_${squadRadius}_10_false_true_true_1_20_5_100_100_5"
done

evaluations78=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations78=${evaluations78}" OneRemoteMuOneNtxFiveULDlNoSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_true_false_1_20_5_100_100_5"
done
evaluations79=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations79=${evaluations79}" OneRemoteMuOneNtxFiveULDlSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_true_true_1_20_5_100_100_5"
done

##
#nUEs=1 Mu=2 nTx=1

evaluations80=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations80=${evaluations80}" OneRemoteMuTwoNtxOneULNoSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_false_false_2_20_1_100_100_5"
done
evaluations81=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations81=${evaluations81}" OneRemoteMuTwoNtxOneULSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_false_true_2_20_1_100_100_5"
done

evaluations82=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations82=${evaluations82}" OneRemoteMuTwoNtxOneDLNoSensing_${squadRadius}_1_1_50_${squadRadius}_10_false_true_false_2_20_1_100_100_5"
done
evaluations83=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations83=${evaluations83}" OneRemoteMuTwoNtxOneDLSensing_${squadRadius}_1_1_50_${squadRadius}_10_false_true_true_2_20_1_100_100_5"
done

evaluations84=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations84=${evaluations84}" OneRemoteMuTwoNtxOneULDlNoSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_true_false_2_20_1_100_100_5"
done
evaluations85=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations85=${evaluations85}" OneRemoteMuTwoNtxOneULDlSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_true_true_2_20_1_100_100_5"
done

#nUEs=1 Mu=2 nTx=2

evaluations86=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations86=${evaluations86}" OneRemoteMuTwoNtxTwoULNoSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_false_false_2_20_2_100_100_5"
done
evaluations87=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations87=${evaluations87}" OneRemoteMuTwoNtxTwoULSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_false_true_2_20_2_100_100_5"
done

evaluations88=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations88=${evaluations88}" OneRemoteMuTwoNtxTwoDLNoSensing_${squadRadius}_1_1_50_${squadRadius}_10_false_true_false_2_20_2_100_100_5"
done
evaluations89=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations89=${evaluations89}" OneRemoteMuTwoNtxTwoDLSensing_${squadRadius}_1_1_50_${squadRadius}_10_false_true_true_2_20_2_100_100_5"
done

evaluations90=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations90=${evaluations90}" OneRemoteMuTwoNtxTwoULDlNoSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_true_false_2_20_2_100_100_5"
done
evaluations91=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations91=${evaluations91}" OneRemoteMuTwoNtxTwoULDlSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_true_true_2_20_2_100_100_5"
done


#nUEs=1 Mu=2 nTx=5

evaluations92=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations92=${evaluations92}" OneRemoteMuTwoNtxFiveULNoSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_false_false_2_20_5_100_100_5"
done
evaluations93=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations93=${evaluations93}" OneRemoteMuTwoNtxFiveULSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_false_true_2_20_5_100_100_5"
done

evaluations94=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations94=${evaluations94}" OneRemoteMuTwoNtxFiveDLNoSensing_${squadRadius}_1_1_50_${squadRadius}_10_false_true_false_2_20_5_100_100_5"
done
evaluations95=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations95=${evaluations95}" OneRemoteMuTwoNtxFiveDLSensing_${squadRadius}_1_1_50_${squadRadius}_10_false_true_true_2_20_5_100_100_5"
done

evaluations96=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations96=${evaluations96}" OneRemoteMuTwoNtxFiveULDlNoSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_true_false_2_20_5_100_100_5"
done
evaluations97=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations97=${evaluations97}" OneRemoteMuTwoNtxFiveULDlSensing_${squadRadius}_1_1_50_${squadRadius}_10_true_true_true_2_20_5_100_100_5"
done

#####

#nUEs=9 Mu=0 nTx=1

evaluations98=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations98=${evaluations98}" NineRemoteMuZeroNtxOneULNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_false_false_0_20_1_100_100_5"
done
evaluations99=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations99=${evaluations99}" NineRemoteMuZeroNtxOneULSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_false_true_0_20_1_100_100_5"
done

evaluations100=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations100=${evaluations100}" NineRemoteMuZeroNtxOneDLNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_false_true_false_0_20_1_100_100_5"
done
evaluations101=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations101=${evaluations101}" NineRemoteMuZeroNtxOneDLSensing_${squadRadius}_1_9_50_${squadRadius}_10_false_true_true_0_20_1_100_100_5"
done

evaluations102=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations102=${evaluations102}" NineRemoteMuZeroNtxOneULDlNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_true_false_0_20_1_100_100_5"
done
evaluations103=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations103=${evaluations103}" NineRemoteMuZeroNtxOneULDlSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_true_true_0_20_1_100_100_5"
done

#nUEs=9 Mu=0 nTx=2

evaluations104=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations104=${evaluations104}" NineRemoteMuZeroNtxTwoULNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_false_false_0_20_2_100_100_5"
done
evaluations105=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations105=${evaluations105}" NineRemoteMuZeroNtxTwoULSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_false_true_0_20_2_100_100_5"
done

evaluations106=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations106=${evaluations106}" NineRemoteMuZeroNtxTwoDLNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_false_true_false_0_20_2_100_100_5"
done
evaluations107=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations107=${evaluations107}" NineRemoteMuZeroNtxTwoDLSensing_${squadRadius}_1_9_50_${squadRadius}_10_false_true_true_0_20_2_100_100_5"
done

evaluations108=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations108=${evaluations108}" NineRemoteMuZeroNtxTwoULDlNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_true_false_0_20_2_100_100_5"
done
evaluations109=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations109=${evaluations109}" NineRemoteMuZeroNtxTwoULDlSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_true_true_0_20_2_100_100_5"
done


#nUEs=9 Mu=0 nTx=5

evaluations110=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations110=${evaluations110}" NineRemoteMuZeroNtxFiveULNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_false_false_0_20_5_100_100_5"
done
evaluations111=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations111=${evaluations111}" NineRemoteMuZeroNtxFiveULSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_false_true_0_20_5_100_100_5"
done

evaluations112=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations112=${evaluations112}" NineRemoteMuZeroNtxFiveDLNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_false_true_false_0_20_5_100_100_5"
done
evaluations113=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations113=${evaluations113}" NineRemoteMuZeroNtxFiveDLSensing_${squadRadius}_1_9_50_${squadRadius}_10_false_true_true_0_20_5_100_100_5"
done

evaluations114=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations114=${evaluations114}" NineRemoteMuZeroNtxFiveULDlNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_true_false_0_20_5_100_100_5"
done
evaluations115=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations115=${evaluations115}" NineRemoteMuZeroNtxFiveULDlSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_true_true_0_20_5_100_100_5"
done

##
#nUEs=9 Mu=1 nTx=1

evaluations116=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations116=${evaluations116}" NineRemoteMuOneNtxOneULNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_false_false_1_20_1_100_100_5"
done
evaluations117=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations117=${evaluations117}" NineRemoteMuOneNtxOneULSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_false_true_1_20_1_100_100_5"
done

evaluations118=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations118=${evaluations118}" NineRemoteMuOneNtxOneDLNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_false_true_false_1_20_1_100_100_5"
done
evaluations119=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations119=${evaluations119}" NineRemoteMuOneNtxOneDLSensing_${squadRadius}_1_9_50_${squadRadius}_10_false_true_true_1_20_1_100_100_5"
done

evaluations120=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations120=${evaluations120}" NineRemoteMuOneNtxOneULDlNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_true_false_1_20_1_100_100_5"
done
evaluations121=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations121=${evaluations121}" NineRemoteMuOneNtxOneULDlSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_true_true_1_20_1_100_100_5"
done

#nUEs=9 Mu=1 nTx=2

evaluations122=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations122=${evaluations122}" NineRemoteMuOneNtxTwoULNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_false_false_1_20_2_100_100_5"
done
evaluations123=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations123=${evaluations123}" NineRemoteMuOneNtxTwoULSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_false_true_1_20_2_100_100_5"
done

evaluations124=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations124=${evaluations124}" NineRemoteMuOneNtxTwoDLNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_false_true_false_1_20_2_100_100_5"
done
evaluations125=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations125=${evaluations125}" NineRemoteMuOneNtxTwoDLSensing_${squadRadius}_1_9_50_${squadRadius}_10_false_true_true_1_20_2_100_100_5"
done

evaluations126=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations126=${evaluations126}" NineRemoteMuOneNtxTwoULDlNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_true_false_1_20_2_100_100_5"
done
evaluations127=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations127=${evaluations127}" NineRemoteMuOneNtxTwoULDlSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_true_true_1_20_2_100_100_5"
done


#nUEs=9 Mu=1 nTx=5

evaluations128=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations128=${evaluations128}" NineRemoteMuOneNtxFiveULNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_false_false_1_20_5_100_100_5"
done
evaluations129=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations129=${evaluations129}" NineRemoteMuOneNtxFiveULSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_false_true_1_20_5_100_100_5"
done

evaluations130=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations130=${evaluations130}" NineRemoteMuOneNtxFiveDLNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_false_true_false_1_20_5_100_100_5"
done
evaluations131=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations131=${evaluations131}" NineRemoteMuOneNtxFiveDLSensing_${squadRadius}_1_9_50_${squadRadius}_10_false_true_true_1_20_5_100_100_5"
done

evaluations132=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations132=${evaluations132}" NineRemoteMuOneNtxFiveULDlNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_true_false_1_20_5_100_100_5"
done
evaluations133=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations133=${evaluations133}" NineRemoteMuOneNtxFiveULDlSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_true_true_1_20_5_100_100_5"
done

##
#nUEs=9 Mu=2 nTx=1

evaluations134=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations134=${evaluations134}" NineRemoteMuTwoNtxOneULNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_false_false_2_20_1_100_100_5"
done
evaluations135=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations135=${evaluations135}" NineRemoteMuTwoNtxOneULSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_false_true_2_20_1_100_100_5"
done

evaluations136=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations136=${evaluations136}" NineRemoteMuTwoNtxOneDLNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_false_true_false_2_20_1_100_100_5"
done
evaluations137=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations137=${evaluations137}" NineRemoteMuTwoNtxOneDLSensing_${squadRadius}_1_9_50_${squadRadius}_10_false_true_true_2_20_1_100_100_5"
done

evaluations138=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations138=${evaluations138}" NineRemoteMuTwoNtxOneULDlNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_true_false_2_20_1_100_100_5"
done
evaluations139=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations139=${evaluations139}" NineRemoteMuTwoNtxOneULDlSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_true_true_2_20_1_100_100_5"
done

#nUEs=9 Mu=2 nTx=2

evaluations140=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations140=${evaluations140}" NineRemoteMuTwoNtxTwoULNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_false_false_2_20_2_100_100_5"
done
evaluations141=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations141=${evaluations141}" NineRemoteMuTwoNtxTwoULSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_false_true_2_20_2_100_100_5"
done

evaluations142=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations142=${evaluations142}" NineRemoteMuTwoNtxTwoDLNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_false_true_false_2_20_2_100_100_5"
done
evaluations143=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations143=${evaluations143}" NineRemoteMuTwoNtxTwoDLSensing_${squadRadius}_1_9_50_${squadRadius}_10_false_true_true_2_20_2_100_100_5"
done

evaluations144=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations144=${evaluations144}" NineRemoteMuTwoNtxTwoULDlNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_true_false_2_20_2_100_100_5"
done
evaluations145=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations145=${evaluations145}" NineRemoteMuTwoNtxTwoULDlSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_true_true_2_20_2_100_100_5"
done


#nUEs=9 Mu=2 nTx=5

evaluations146=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations146=${evaluations146}" NineRemoteMuTwoNtxFiveULNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_false_false_2_20_5_100_100_5"
done
evaluations147=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations147=${evaluations147}" NineRemoteMuTwoNtxFiveULSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_false_true_2_20_5_100_100_5"
done

evaluations148=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations148=${evaluations148}" NineRemoteMuTwoNtxFiveDLNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_false_true_false_2_20_5_100_100_5"
done
evaluations149=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations149=${evaluations149}" NineRemoteMuTwoNtxFiveDLSensing_${squadRadius}_1_9_50_${squadRadius}_10_false_true_true_2_20_5_100_100_5"
done

evaluations150=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations150=${evaluations150}" NineRemoteMuTwoNtxFiveULDlNoSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_true_false_2_20_5_100_100_5"
done
evaluations151=""
for ((squadRadius=75; squadRadius<=1500; squadRadius+=75))
do
  evaluations151=${evaluations151}" NineRemoteMuTwoNtxFiveULDlSensing_${squadRadius}_1_9_50_${squadRadius}_10_true_true_true_2_20_5_100_100_5"
done


##44 to 151
evalArray=("$evaluations44" "$evaluations45" "$evaluations46" "$evaluations47" "$evaluations48" "$evaluations49" "$evaluations50" "$evaluations51" "$evaluations52"  "$evaluations53"  "$evaluations54"  "$evaluations55"  "$evaluations56"  "$evaluations57"  "$evaluations58"  "$evaluations59" "$evaluations60" "$evaluations61" "$evaluations62"  "$evaluations63"  "$evaluations64"  "$evaluations65"  "$evaluations66"  "$evaluations67"  "$evaluations68"  "$evaluations69" "$evaluations70" "$evaluations71" "$evaluations72"  "$evaluations73"  "$evaluations74"  "$evaluations75"  "$evaluations76"  "$evaluations77"  "$evaluations78"  "$evaluations79" "$evaluations80" "$evaluations81" "$evaluations82"  "$evaluations83"  "$evaluations84"  "$evaluations85"  "$evaluations86"  "$evaluations87"  "$evaluations88"  "$evaluations89" "$evaluations90" "$evaluations91" "$evaluations92"  "$evaluations93"  "$evaluations94"  "$evaluations95"  "$evaluations96"  "$evaluations97"  "$evaluations98"  "$evaluations99" "$evaluations100" "$evaluations101" "$evaluations102"  "$evaluations103"  "$evaluations104"  "$evaluations105"  "$evaluations106"  "$evaluations107"  "$evaluations108"  "$evaluations109" "$evaluations110" "$evaluations111" "$evaluations112"  "$evaluations113"  "$evaluations114"  "$evaluations115"  "$evaluations116"  "$evaluations117"  "$evaluations118"  "$evaluations119" "$evaluations120" "$evaluations121" "$evaluations122"  "$evaluations123"  "$evaluations124"  "$evaluations125"  "$evaluations126"  "$evaluations127"  "$evaluations128"  "$evaluations129" "$evaluations130" "$evaluations131" "$evaluations132"  "$evaluations133"  "$evaluations134"  "$evaluations135"  "$evaluations136"  "$evaluations137"  "$evaluations138"  "$evaluations139" "$evaluations140" "$evaluations141" "$evaluations142"  "$evaluations143"  "$evaluations144"  "$evaluations145"  "$evaluations146"  "$evaluations147"  "$evaluations148"  "$evaluations149" "$evaluations150" "$evaluations151")

#evalArray=("testTwo_1000_1_1_50_1000_10_true_false_false_2_20_5_100_100_5 testTwo_1500_1_1_50_1500_10_true_false_false_2_20_5_100_100_5")

runSims=2; #Set >0 to run simulations and evaluation, set to 0 to run just final graph
  
for ((i=0; i < ${#evalArray[@]}; i++)) 
do
  evaluations="${evalArray[$i]}"
  echo -e "============================================================================"
        echo "$evaluations"
  echo -e "============================================================================"



for e in $evaluations
do
  echo -e "----------------------------------------------------------------------------"
        echo $e
  echo -e "----------------------------------------------------------------------------"

        IFS='_' read -r -a array <<< "$e"

        #Evaluation parameters 

        evalnameRoot=${array[0]}
        evalnameValue=${array[1]}
        scenario="milcom-2023"  
        nSquads=${array[2]} #
        nSoldiersPerSquad=${array[3]} #
        platoonRadius=${array[4]} #
        squadRadius=${array[5]}
        durationTraffic=${array[6]}
        ulTraffic=${array[7]}
        dlTraffic=${array[8]}
        enableSensing=${array[9]}
        numerologySl=${array[10]}
        rri=${array[11]}
		    maxNumTx=${array[12]}
        ulPercentage=${array[13]}
        dlPercentage=${array[14]}
        mcs=${array[15]}
        
        
        
        nThreads=40 #Number of simultaneous runs to be executed for each evaluation
        
        STARTRUN=1 #Run number to start serie
        MAXRUNS=200 #Number of runs used for evaluations

	evalname=${evalnameRoot}"_"${evalnameValue}
        ver="run" #Version for logging run output
        basedir=${scenario}"_"${evalname}"/"

if [ $runSims -gt 0 ];then

        arguments=" --nSquads=$nSquads --nSoldiersPerSquad=$nSoldiersPerSquad --platoonRadius=$platoonRadius --squadRadius=$squadRadius --durationTraffic=$durationTraffic --ulTraffic=$ulTraffic --dlTraffic=$dlTraffic --enableSensing=$enableSensing --numerologySl=$numerologySl --rri=$rri --maxNumTx=$maxNumTx --ulPercentage=$ulPercentage --dlPercentage=$dlPercentage --mcs=$mcs"

        ./ns3 #First compilation to avoid problems of simultaneous compilation when using simultaneous execution

        echo "The experiment ($STARTRUN - $MAXRUNS runs) : $evalname" 
        echo $arguments
        current_time=$(date "+%Y-%m-%d %H:%M:%S")
        echo "Simulations Start Time : $current_time"

        for ((run=$STARTRUN; run<=$MAXRUNS; run++))
        do
                newdir="${basedir}${ver}${run}"
                mkdir -p $newdir
                OUTFILE="${newdir}/log-${ver}${run}.txt"
                rm -f $OUTFILE
                runinfo="$scenario, RUN: ${ver}${run}"
           echo -e " $runinfo saved to dir: ${newdir}"
                run_args="--RngSeed=$run $arguments"
                echo -e "$runinfo, $run_args " >> $OUTFILE

           ./ns3 run "$scenario $run_args" --cwd=$newdir >> $OUTFILE 2>&1 &

                 n=$(($run % $nThreads)) 
                 if [ "$n" -eq 0 ];then
                    wait
                 fi

        done

            wait

            current_time=$(date "+%Y-%m-%d %H:%M:%S")
            echo -e "\nSimulations End Time : $current_time"
            echo -e "Simulations Duration: $SECONDS seconds"

      for ((run=$STARTRUN; run<=$MAXRUNS; run++))
      do
                dir="${basedir}${ver}${run}"
                dirIn=$dir
                dirOut=$dir/Processed
                mkdir -p $dirOut

                echo -e " Processing run $run... Processed statistic saved to dir: $dirOut"
                
                ##Calculate percentage of traffic flows satisfied 
                for ((targetPercentage=50; targetPercentage<=90; targetPercentage+=10))
                do
                   targetPdr="0."$targetPercentage
                   awk -v targetPDR=${targetPdr} 'BEGIN{nFlows=0; nSatisfiedFlows=0;}{if ($1 != "srcAddr->dstAddr") {nFlows++; if ($4 >= targetPDR){nSatisfiedFlows++;} } }END { if (nFlows > 0) {print nSatisfiedFlows/nFlows;} }' ${dirIn}/${scenario}-stats_perFlow.txt > ${dirOut}/System_ratioSatisfiedFlows_${targetPercentage}.txt

                   awk -v targetPDR=${targetPdr} 'BEGIN{nFlows=0; nSatisfiedFlows=0;}{if ($1 != "srcAddr->dstAddr" && $3 == "UL") {nFlows++; if ($4 >= targetPDR){nSatisfiedFlows++;} } }END { if (nFlows > 0) {print nSatisfiedFlows/nFlows;} }' ${dirIn}/${scenario}-stats_perFlow.txt > ${dirOut}/UL_ratioSatisfiedFlows_${targetPercentage}.txt

                   awk -v targetPDR=${targetPdr} 'BEGIN{nFlows=0; nSatisfiedFlows=0;}{if ($1 != "srcAddr->dstAddr" && $3 == "DL") {nFlows++; if ($4 >= targetPDR){nSatisfiedFlows++;} } }END { if (nFlows > 0) {print nSatisfiedFlows/nFlows;} }' ${dirIn}/${scenario}-stats_perFlow.txt > ${dirOut}/DL_ratioSatisfiedFlows_${targetPercentage}.txt
                 done

     
                n=$(($run % $nThreads)) 
                if [ "$n" -eq 0 ];then
                  wait
                fi

      done

      wait

      if [[ $(($MAXRUNS - $STARTRUN)) -gt 0 ]];then
         echo -e "\nGenerating aggregated stat (averages + confidence intervals)..."

         dir="${basedir%?}/AggregatedStats"
         mkdir -p $dir

        #Calculate confidenceInterval of the system packet delivery ratio ($1)
        awk 'BEGIN{ FS="\t";} {if($1 != "SysPDR" && $1 != "-nan" && $1 != "nan" ) {ln++; avg[ln]=$1; sum=sum+avg[ln];}}END{ average=sum/ln; for(i=1;i<=ln;i++){ gap = avg[i]-average; sum2=sum2+gap*gap;} variance=sqrt(sum2/(ln-1)); c_i=1.96*variance/sqrt(ln); print average, c_i; }'  ${basedir}${ver}*/${scenario}-stats_system.txt > "$dir/System_PDR-CI.txt"


        max=`awk ' BEGIN{ max =0;} { if ($1>max) {max = $1; } } END {print max}' $dir/System_PDR-CI.txt`
        echo " reset 
                set terminal png nocrop enhanced size 800,600
                set style fill solid 0.5 
                set style histogram errorbars gap 2 lw 2
                set style data histograms
                set yrange [0:1]
                set title \"System Packet delivery ratio \"
                set output '$dir/System_PDR-CI.png'
                plot '$dir/System_PDR-CI.txt' notitle, '$dir/System_PDR-CI.txt' using (0.125):($max-(0.025*$max)):1 with labels offset 2 notitle" | gnuplot

        #Calculate confidenceInterval of the system average packet delay ($2)
        awk 'BEGIN{ FS="\t";} {if($1 != "SysPDR" && $2 != "-nan" && $2 != "nan") {ln++; avg[ln]=$2; sum=sum+avg[ln];}}END{ average=sum/ln; for(i=1;i<=ln;i++){ gap = avg[i]-average; sum2=sum2+gap*gap;} variance=sqrt(sum2/(ln-1)); c_i=1.96*variance/sqrt(ln); print average, c_i; }'  ${basedir}${ver}*/${scenario}-stats_system.txt > "$dir/System_AvgDelay-CI.txt"


        max=`awk ' BEGIN{ max =0;} { if ($1>max) {max = $1; } } END {print max}' $dir/System_AvgDelay-CI.txt`
        echo " reset 
                set terminal png nocrop enhanced size 800,600
                set style fill solid 0.5 
                set style histogram errorbars gap 2 lw 2
                set style data histograms
                set yrange [0:2*$max]
                set title \"System Average packet delay \"
                set output '$dir/System_AvgDelay-CI.png'
                plot '$dir/System_AvgDelay-CI.txt' notitle, '$dir/System_AvgDelay-CI.txt' using (0.125):($max-(0.025*$max)):1 with labels offset 2 notitle" | gnuplot

        #Calculate confidenceInterval of the system average jitter ($3)
        awk 'BEGIN{ FS="\t";} {if($1 != "SysPDR" && $3 != "-nan" && $3 != "nan") {ln++; avg[ln]=$3; sum=sum+avg[ln];}}END{ average=sum/ln; for(i=1;i<=ln;i++){ gap = avg[i]-average; sum2=sum2+gap*gap;} variance=sqrt(sum2/(ln-1)); c_i=1.96*variance/sqrt(ln); print average, c_i; }'  ${basedir}${ver}*/${scenario}-stats_system.txt > "$dir/System_AvgJitter-CI.txt"


        max=`awk ' BEGIN{ max =0;} { if ($1>max) {max = $1; } } END {print max}' $dir/System_AvgJitter-CI.txt`
        echo " reset 
                set terminal png nocrop enhanced size 800,600
                set style fill solid 0.5 
                set style histogram errorbars gap 2 lw 2
                set style data histograms
                set yrange [0:2*$max]
                set title \"System Average jitter \"
                set output '$dir/System_AvgJitter-CI.png'
                plot '$dir/System_AvgJitter-CI.txt' notitle, '$dir/System_AvgJitter-CI.txt' using (0.125):($max-(0.025*$max)):1 with labels offset 2 notitle" | gnuplot

        #Calculate confidenceInterval of the system ratio of connected users ($4)
        awk 'BEGIN{ FS="\t";} {if($1 != "SysPDR" && $4 != "-nan" && $4 != "nan" ) {ln++; avg[ln]=$4; sum=sum+avg[ln];}}END{ average=sum/ln; for(i=1;i<=ln;i++){ gap = avg[i]-average; sum2=sum2+gap*gap;} variance=sqrt(sum2/(ln-1)); c_i=1.96*variance/sqrt(ln); print average, c_i; }'  ${basedir}${ver}*/${scenario}-stats_system.txt > "$dir/System_RatioConn-CI.txt"


        echo " reset 
                set terminal png nocrop enhanced size 800,600
                set style fill solid 0.5 
                set style histogram errorbars gap 2 lw 2
                set style data histograms
                set yrange [0:1]
                set title \"Ratio of connected remote UEs \"
                set output '$dir/System_RatioConn-CI.png'
                plot '$dir/System_RatioConn-CI.txt' notitle, '$dir/System_RatioConn-CI.txt' using (0.125):($max-(0.025*$max)):1 with labels offset 2 notitle" | gnuplot



        #Calculate confidenceInterval of the UL packet delivery ratio ($5)
        awk 'BEGIN{ FS="\t";} {if($1 != "SysPDR" && $5 != "-nan" && $5 != "nan" ) {ln++; avg[ln]=$5; sum=sum+avg[ln];}}END{ average=sum/ln; for(i=1;i<=ln;i++){ gap = avg[i]-average; sum2=sum2+gap*gap;} variance=sqrt(sum2/(ln-1)); c_i=1.96*variance/sqrt(ln); print average, c_i; }'  ${basedir}${ver}*/${scenario}-stats_system.txt > "$dir/UL_PDR-CI.txt"


        max=`awk ' BEGIN{ max =0;} { if ($1>max) {max = $1; } } END {print max}' $dir/UL_PDR-CI.txt`
        echo " reset 
                set terminal png nocrop enhanced size 800,600
                set style fill solid 0.5 
                set style histogram errorbars gap 2 lw 2
                set style data histograms
                set yrange [0:1]
                set title \"Uplink Packet delivery ratio \"
                set output '$dir/UL_PDR-CI.png'
                plot '$dir/UL_PDR-CI.txt' notitle, '$dir/UL_PDR-CI.txt' using (0.125):($max-(0.025*$max)):1 with labels offset 2 notitle" | gnuplot

        #Calculate confidenceInterval of the UL average packet delay ($6)
        awk 'BEGIN{ FS="\t";} {if($1 != "SysPDR" && $6 != "-nan" && $6 != "nan") {ln++; avg[ln]=$6; sum=sum+avg[ln];}}END{ average=sum/ln; for(i=1;i<=ln;i++){ gap = avg[i]-average; sum2=sum2+gap*gap;} variance=sqrt(sum2/(ln-1)); c_i=1.96*variance/sqrt(ln); print average, c_i; }'  ${basedir}${ver}*/${scenario}-stats_system.txt > "$dir/UL_AvgDelay-CI.txt"


        max=`awk ' BEGIN{ max =0;} { if ($1>max) {max = $1; } } END {print max}' $dir/UL_AvgDelay-CI.txt`
        echo " reset 
                set terminal png nocrop enhanced size 800,600
                set style fill solid 0.5 
                set style histogram errorbars gap 2 lw 2
                set style data histograms
                set yrange [0:2*$max]
                set title \"Uplink Average packet delay \"
                set output '$dir/UL_AvgDelay-CI.png'
                plot '$dir/UL_AvgDelay-CI.txt' notitle, '$dir/UL_AvgDelay-CI.txt' using (0.125):($max-(0.025*$max)):1 with labels offset 2 notitle" | gnuplot

        #Calculate confidenceInterval of the system average jitter ($7)
        awk 'BEGIN{ FS="\t";} {if($1 != "SysPDR" && $7 != "-nan" && $7 != "nan") {ln++; avg[ln]=$7; sum=sum+avg[ln];}}END{ average=sum/ln; for(i=1;i<=ln;i++){ gap = avg[i]-average; sum2=sum2+gap*gap;} variance=sqrt(sum2/(ln-1)); c_i=1.96*variance/sqrt(ln); print average, c_i; }'  ${basedir}${ver}*/${scenario}-stats_system.txt > "$dir/UL_AvgJitter-CI.txt"


        max=`awk ' BEGIN{ max =0;} { if ($1>max) {max = $1; } } END {print max}' $dir/UL_AvgJitter-CI.txt`
        echo " reset 
                set terminal png nocrop enhanced size 800,600
                set style fill solid 0.5 
                set style histogram errorbars gap 2 lw 2
                set style data histograms
                set yrange [0:2*$max]
                set title \"Uplink Average jitter \"
                set output '$dir/UL_AvgJitter-CI.png'
                plot '$dir/UL_AvgJitter-CI.txt' notitle, '$dir/UL_AvgJitter-CI.txt' using (0.125):($max-(0.025*$max)):1 with labels offset 2 notitle" | gnuplot

        #Calculate confidenceInterval of the DL packet delivery ratio ($8)
        awk 'BEGIN{ FS="\t";} {if($1 != "SysPDR" && $8 != "-nan" && $8 != "nan" ) {ln++; avg[ln]=$8; sum=sum+avg[ln];}}END{ average=sum/ln; for(i=1;i<=ln;i++){ gap = avg[i]-average; sum2=sum2+gap*gap;} variance=sqrt(sum2/(ln-1)); c_i=1.96*variance/sqrt(ln); print average, c_i; }'  ${basedir}${ver}*/${scenario}-stats_system.txt > "$dir/DL_PDR-CI.txt"


        max=`awk ' BEGIN{ max =0;} { if ($1>max) {max = $1; } } END {print max}' $dir/DL_PDR-CI.txt`
        echo " reset 
                set terminal png nocrop enhanced size 800,600
                set style fill solid 0.5 
                set style histogram errorbars gap 2 lw 2
                set style data histograms
                set yrange [0:1]
                set title \"Downlink Packet delivery ratio \"
                set output '$dir/DL_PDR-CI.png'
                plot '$dir/DL_PDR-CI.txt' notitle, '$dir/DL_PDR-CI.txt' using (0.125):($max-(0.025*$max)):1 with labels offset 2 notitle" | gnuplot

        #Calculate confidenceInterval of the DL average packet delay ($9)
        awk 'BEGIN{ FS="\t";} {if($1 != "SysPDR" && $9 != "-nan" && $9 != "nan") {ln++; avg[ln]=$9; sum=sum+avg[ln];}}END{ average=sum/ln; for(i=1;i<=ln;i++){ gap = avg[i]-average; sum2=sum2+gap*gap;} variance=sqrt(sum2/(ln-1)); c_i=1.96*variance/sqrt(ln); print average, c_i; }'  ${basedir}${ver}*/${scenario}-stats_system.txt > "$dir/DL_AvgDelay-CI.txt"


        max=`awk ' BEGIN{ max =0;} { if ($1>max) {max = $1; } } END {print max}' $dir/DL_AvgDelay-CI.txt`
        echo " reset 
                set terminal png nocrop enhanced size 800,600
                set style fill solid 0.5 
                set style histogram errorbars gap 2 lw 2
                set style data histograms
                set yrange [0:2*$max]
                set title \"Downlink Average packet delay \"
                set output '$dir/DL_AvgDelay-CI.png'
                plot '$dir/DL_AvgDelay-CI.txt' notitle, '$dir/DL_AvgDelay-CI.txt' using (0.125):($max-(0.025*$max)):1 with labels offset 2 notitle" | gnuplot

        #Calculate confidenceInterval of the system average jitter ($10)
        awk 'BEGIN{ FS="\t";} {if($1 != "SysPDR" && $10 != "-nan" && $10 != "nan") {ln++; avg[ln]=$10; sum=sum+avg[ln];}}END{ average=sum/ln; for(i=1;i<=ln;i++){ gap = avg[i]-average; sum2=sum2+gap*gap;} variance=sqrt(sum2/(ln-1)); c_i=1.96*variance/sqrt(ln); print average, c_i; }'  ${basedir}${ver}*/${scenario}-stats_system.txt > "$dir/DL_AvgJitter-CI.txt"


        max=`awk ' BEGIN{ max =0;} { if ($1>max) {max = $1; } } END {print max}' $dir/DL_AvgJitter-CI.txt`
        echo " reset 
                set terminal png nocrop enhanced size 800,600
                set style fill solid 0.5 
                set style histogram errorbars gap 2 lw 2
                set style data histograms
                set yrange [0:2*$max]
                set title \"Downlink Average jitter \"
                set output '$dir/DL_AvgJitter-CI.png'
                plot '$dir/DL_AvgJitter-CI.txt' notitle, '$dir/DL_AvgJitter-CI.txt' using (0.125):($max-(0.025*$max)):1 with labels offset 2 notitle" | gnuplot


 
        ##Calculate confidenceInterval of the percentage of traffic flows satisfied 
        for ((targetPercentage=50; targetPercentage<=90; targetPercentage+=10))
        do
            awk 'BEGIN{ FS="\t";} {if($1 != "-nan" && $1 != "nan" ) {ln++; avg[ln]=$1; sum=sum+avg[ln];}}END{ average=sum/ln; for(i=1;i<=ln;i++){ gap = avg[i]-average; sum2=sum2+gap*gap;} variance=sqrt(sum2/(ln-1)); c_i=1.96*variance/sqrt(ln); print average, c_i; }'  ${basedir}${ver}*/Processed/System_ratioSatisfiedFlows_${targetPercentage}.txt > ${dir}/System_ratioSatisfiedFlows_${targetPercentage}-CI.txt
			
            awk 'BEGIN{ FS="\t";} {if($1 != "-nan" && $1 != "nan" ) {ln++; avg[ln]=$1; sum=sum+avg[ln];}}END{ average=sum/ln; for(i=1;i<=ln;i++){ gap = avg[i]-average; sum2=sum2+gap*gap;} variance=sqrt(sum2/(ln-1)); c_i=1.96*variance/sqrt(ln); print average, c_i; }'  ${basedir}${ver}*/Processed/UL_ratioSatisfiedFlows_${targetPercentage}.txt > ${dir}/UL_ratioSatisfiedFlows_${targetPercentage}-CI.txt			

            awk 'BEGIN{ FS="\t";} {if($1 != "-nan" && $1 != "nan" ) {ln++; avg[ln]=$1; sum=sum+avg[ln];}}END{ average=sum/ln; for(i=1;i<=ln;i++){ gap = avg[i]-average; sum2=sum2+gap*gap;} variance=sqrt(sum2/(ln-1)); c_i=1.96*variance/sqrt(ln); print average, c_i; }'  ${basedir}${ver}*/Processed/DL_ratioSatisfiedFlows_${targetPercentage}.txt > ${dir}/DL_ratioSatisfiedFlows_${targetPercentage}-CI.txt			
   
        done
     else
              echo "Can not generate aggregated stats. Total runs should be greater than $MAXRUNS"

     fi

        echo -e "\nEvaluation End Time : $current_time"
        echo -e "Evaluation Duration: $SECONDS seconds"


       echo -e "\nExperiment $evalname (simulation + evaluation) duration: $SECONDS seconds"
       echo -e "----------------------------------------------------------------------------"
fi
   done

   echo -e "\nAll experiments duration: $SECONDS seconds \n\n"

   echo -e "\nGenerating final plots..."
   
   #System PDR
   fPlotFilename=${scenario}"_"${evalnameRoot}
   awk '{split(FILENAME,b,"/"); split(b[1],a,"_"); print a[2]"\t"a[3]"\t"$1"\t"$2;}' ${scenario}_${evalnameRoot}_*/AggregatedStats/System_PDR-CI.txt | sort -k1,1 -k2,2n | awk '{print$0;};' > "${fPlotFilename}_System_PDR-CI.txt"
   max=`awk 'BEGIN{ max =0;} { if ($2>max) {max = $2; } } END {print max}' "${fPlotFilename}_System_PDR-CI.txt"`

   echo "  reset  
    set terminal png nocrop enhanced size 800,600
    set output '${fPlotFilename}_System_PDR-CI.png'
    set offset 0.5,0.5,0,0
    set xlabel '${evalnameRoot}'
    set xtics rotate by -90 offset 0,0.5 font \", 10\"
    set yrange [0:1]
    set title 'System Packet delivery ratio'
    plot '${fPlotFilename}_System_PDR-CI.txt' u 0:3:4:xticlabels(2) w yerrorbars lt rgb \"black\" notitle, '${fPlotFilename}_System_PDR-CI.txt' u 3:xticlabels(2) w points lt 3 notitle"  | gnuplot

    echo -e "Final plot generated: ${fPlotFilename}_System_PDR-CI.png"
    
    #System average delay
   fPlotFilename=${scenario}"_"${evalnameRoot}
   awk '{split(FILENAME,b,"/"); split(b[1],a,"_"); print a[2]"\t"a[3]"\t"$1"\t"$2;}' ${scenario}_${evalnameRoot}_*/AggregatedStats/System_AvgDelay-CI.txt | sort -k1,1 -k2,2n | awk '{print$0;};' > "${fPlotFilename}_System_AvgDelay-CI.txt"
   max=`awk 'BEGIN{ max =0;} { if ($2>max) {max = $2; } } END {print max}' "${fPlotFilename}_System_AvgDelay-CI.txt"`

   echo "  reset  
    set terminal png nocrop enhanced size 800,600
    set output '${fPlotFilename}_System_AvgDelay-CI.png'
    set offset 0.5,0.5,0,0
    set xlabel '${evalnameRoot}'
    set xtics rotate by -90 offset 0,0.5 font \", 10\"
    set yrange [0:]
    set title 'System Average Delay (ms)'
    plot '${fPlotFilename}_System_AvgDelay-CI.txt' u 0:3:4:xticlabels(2) w yerrorbars lt rgb \"black\" notitle, '${fPlotFilename}_System_AvgDelay-CI.txt' u 3:xticlabels(2) w points lt 3 notitle"  | gnuplot

    echo -e "Final plot generated: ${fPlotFilename}_System_AvgDelay-CI.png"    
    
    #System average jitter
   fPlotFilename=${scenario}"_"${evalnameRoot}
   awk '{split(FILENAME,b,"/"); split(b[1],a,"_"); print a[2]"\t"a[3]"\t"$1"\t"$2;}' ${scenario}_${evalnameRoot}_*/AggregatedStats/System_AvgJitter-CI.txt | sort -k1,1 -k2,2n | awk '{print$0;};' > "${fPlotFilename}_System_AvgJitter-CI.txt"
   max=`awk 'BEGIN{ max =0;} { if ($2>max) {max = $2; } } END {print max}' "${fPlotFilename}_System_AvgJitter-CI.txt"`

   echo "  reset  
    set terminal png nocrop enhanced size 800,600
    set output '${fPlotFilename}_System_AvgJitter-CI.png'
    set offset 0.5,0.5,0,0
    set xlabel '${evalnameRoot}'
    set xtics rotate by -90 offset 0,0.5 font \", 10\"
    set yrange [0:]
    set title 'System Average Jitter (ms)'
    plot '${fPlotFilename}_System_AvgJitter-CI.txt' u 0:3:4:xticlabels(2) w yerrorbars lt rgb \"black\" notitle, '${fPlotFilename}_System_AvgJitter-CI.txt' u 3:xticlabels(2) w points lt 3 notitle"  | gnuplot

    echo -e "Final plot generated: ${fPlotFilename}_System_AvgJitter-CI.png"    
	
	
   #System Ratio of connected remote UEs
   fPlotFilename=${scenario}"_"${evalnameRoot}
   awk '{split(FILENAME,b,"/"); split(b[1],a,"_"); print a[2]"\t"a[3]"\t"$1"\t"$2;}' ${scenario}_${evalnameRoot}_*/AggregatedStats/System_RatioConn-CI.txt | sort -k1,1 -k2,2n | awk '{print$0;};' > "${fPlotFilename}_System_RatioConn-CI.txt"
   max=`awk 'BEGIN{ max =0;} { if ($2>max) {max = $2; } } END {print max}' "${fPlotFilename}_System_RatioConn-CI.txt"`

   echo "  reset  
    set terminal png nocrop enhanced size 800,600
    set output '${fPlotFilename}_System_RatioConn-CI.png'
    set offset 0.5,0.5,0,0
    set xlabel '${evalnameRoot}'
    set xtics rotate by -90 offset 0,0.5 font \", 10\"
    set yrange [0:1]
    set title 'System Ratio of connected remote UEs'
    plot '${fPlotFilename}_System_RatioConn-CI.txt' u 0:3:4:xticlabels(2) w yerrorbars lt rgb \"black\" notitle, '${fPlotFilename}_System_RatioConn-CI.txt' u 3:xticlabels(2) w points lt 3 notitle"  | gnuplot

    echo -e "Final plot generated: ${fPlotFilename}_System_RatioConn-CI.png"
	
	
   #UL PDR
   fPlotFilename=${scenario}"_"${evalnameRoot}
   awk '{split(FILENAME,b,"/"); split(b[1],a,"_"); print a[2]"\t"a[3]"\t"$1"\t"$2;}' ${scenario}_${evalnameRoot}_*/AggregatedStats/UL_PDR-CI.txt | sort -k1,1 -k2,2n | awk '{print$0;};' > "${fPlotFilename}_UL_PDR-CI.txt"
   max=`awk 'BEGIN{ max =0;} { if ($2>max) {max = $2; } } END {print max}' "${fPlotFilename}_UL_PDR-CI.txt"`

   echo "  reset  
    set terminal png nocrop enhanced size 800,600
    set output '${fPlotFilename}_UL_PDR-CI.png'
    set offset 0.5,0.5,0,0
    set xlabel '${evalnameRoot}'
    set xtics rotate by -90 offset 0,0.5 font \", 10\"
    set yrange [0:1]
    set title 'Uplink Packet delivery ratio'
    plot '${fPlotFilename}_UL_PDR-CI.txt' u 0:3:4:xticlabels(2) w yerrorbars lt rgb \"black\" notitle, '${fPlotFilename}_UL_PDR-CI.txt' u 3:xticlabels(2) w points lt 3 notitle"  | gnuplot

    echo -e "Final plot generated: ${fPlotFilename}_UL_PDR-CI.png"
    
    #UL average delay
   fPlotFilename=${scenario}"_"${evalnameRoot}
   awk '{split(FILENAME,b,"/"); split(b[1],a,"_"); print a[2]"\t"a[3]"\t"$1"\t"$2;}' ${scenario}_${evalnameRoot}_*/AggregatedStats/UL_AvgDelay-CI.txt | sort -k1,1 -k2,2n | awk '{print$0;};' > "${fPlotFilename}_UL_AvgDelay-CI.txt"
   max=`awk 'BEGIN{ max =0;} { if ($2>max) {max = $2; } } END {print max}' "${fPlotFilename}_UL_AvgDelay-CI.txt"`

   echo "  reset  
    set terminal png nocrop enhanced size 800,600
    set output '${fPlotFilename}_UL_AvgDelay-CI.png'
    set offset 0.5,0.5,0,0
    set xlabel '${evalnameRoot}'
    set xtics rotate by -90 offset 0,0.5 font \", 10\"
    set yrange [0:]
    set title 'Uplink Average Delay (ms)'
    plot '${fPlotFilename}_UL_AvgDelay-CI.txt' u 0:3:4:xticlabels(2) w yerrorbars lt rgb \"black\" notitle, '${fPlotFilename}_UL_AvgDelay-CI.txt' u 3:xticlabels(2) w points lt 3 notitle"  | gnuplot

    echo -e "Final plot generated: ${fPlotFilename}_UL_AvgDelay-CI.png"    
    
    #UL average jitter
   fPlotFilename=${scenario}"_"${evalnameRoot}
   awk '{split(FILENAME,b,"/"); split(b[1],a,"_"); print a[2]"\t"a[3]"\t"$1"\t"$2;}' ${scenario}_${evalnameRoot}_*/AggregatedStats/UL_AvgJitter-CI.txt | sort -k1,1 -k2,2n | awk '{print$0;};' > "${fPlotFilename}_UL_AvgJitter-CI.txt"
   max=`awk 'BEGIN{ max =0;} { if ($2>max) {max = $2; } } END {print max}' "${fPlotFilename}_UL_AvgJitter-CI.txt"`

   echo "  reset  
    set terminal png nocrop enhanced size 800,600
    set output '${fPlotFilename}_UL_AvgJitter-CI.png'
    set offset 0.5,0.5,0,0
    set xlabel '${evalnameRoot}'
    set xtics rotate by -90 offset 0,0.5 font \", 10\"
    set yrange [0:]
    set title 'Uplink Average Jitter (ms)'
    plot '${fPlotFilename}_UL_AvgJitter-CI.txt' u 0:3:4:xticlabels(2) w yerrorbars lt rgb \"black\" notitle, '${fPlotFilename}_UL_AvgJitter-CI.txt' u 3:xticlabels(2) w points lt 3 notitle"  | gnuplot

    echo -e "Final plot generated: ${fPlotFilename}_UL_AvgJitter-CI.png"    
	
	
	
	
   #DL PDR
   fPlotFilename=${scenario}"_"${evalnameRoot}
   awk '{split(FILENAME,b,"/"); split(b[1],a,"_"); print a[2]"\t"a[3]"\t"$1"\t"$2;}' ${scenario}_${evalnameRoot}_*/AggregatedStats/DL_PDR-CI.txt | sort -k1,1 -k2,2n | awk '{print$0;};' > "${fPlotFilename}_DL_PDR-CI.txt"
   max=`awk 'BEGIN{ max =0;} { if ($2>max) {max = $2; } } END {print max}' "${fPlotFilename}_DL_PDR-CI.txt"`

   echo "  reset  
    set terminal png nocrop enhanced size 800,600
    set output '${fPlotFilename}_DL_PDR-CI.png'
    set offset 0.5,0.5,0,0
    set xlabel '${evalnameRoot}'
    set xtics rotate by -90 offset 0,0.5 font \", 10\"
    set yrange [0:1]
    set title 'Downlink Packet delivery ratio'
    plot '${fPlotFilename}_DL_PDR-CI.txt' u 0:3:4:xticlabels(2) w yerrorbars lt rgb \"black\" notitle, '${fPlotFilename}_DL_PDR-CI.txt' u 3:xticlabels(2) w points lt 3 notitle"  | gnuplot

    echo -e "Final plot generated: ${fPlotFilename}_DL_PDR-CI.png"
    
    #UL average delay
   fPlotFilename=${scenario}"_"${evalnameRoot}
   awk '{split(FILENAME,b,"/"); split(b[1],a,"_"); print a[2]"\t"a[3]"\t"$1"\t"$2;}' ${scenario}_${evalnameRoot}_*/AggregatedStats/DL_AvgDelay-CI.txt | sort -k1,1 -k2,2n | awk '{print$0;};' > "${fPlotFilename}_DL_AvgDelay-CI.txt"
   max=`awk 'BEGIN{ max =0;} { if ($2>max) {max = $2; } } END {print max}' "${fPlotFilename}_DL_AvgDelay-CI.txt"`

   echo "  reset  
    set terminal png nocrop enhanced size 800,600
    set output '${fPlotFilename}_DL_AvgDelay-CI.png'
    set offset 0.5,0.5,0,0
    set xlabel '${evalnameRoot}'
    set xtics rotate by -90 offset 0,0.5 font \", 10\"
    set yrange [0:]
    set title 'Dowlink Average Delay (ms)'
    plot '${fPlotFilename}_DL_AvgDelay-CI.txt' u 0:3:4:xticlabels(2) w yerrorbars lt rgb \"black\" notitle, '${fPlotFilename}_DL_AvgDelay-CI.txt' u 3:xticlabels(2) w points lt 3 notitle"  | gnuplot

    echo -e "Final plot generated: ${fPlotFilename}_DL_AvgDelay-CI.png"    
    
    #UL average jitter
   fPlotFilename=${scenario}"_"${evalnameRoot}
   awk '{split(FILENAME,b,"/"); split(b[1],a,"_"); print a[2]"\t"a[3]"\t"$1"\t"$2;}' ${scenario}_${evalnameRoot}_*/AggregatedStats/DL_AvgJitter-CI.txt | sort -k1,1 -k2,2n | awk '{print$0;};' > "${fPlotFilename}_DL_AvgJitter-CI.txt"
   max=`awk 'BEGIN{ max =0;} { if ($2>max) {max = $2; } } END {print max}' "${fPlotFilename}_DL_AvgJitter-CI.txt"`

   echo "  reset  
    set terminal png nocrop enhanced size 800,600
    set output '${fPlotFilename}_DL_AvgJitter-CI.png'
    set offset 0.5,0.5,0,0
    set xlabel '${evalnameRoot}'
    set xtics rotate by -90 offset 0,0.5 font \", 10\"
    set yrange [0:]
    set title 'Uplink Average Jitter (ms)'
    plot '${fPlotFilename}_DL_AvgJitter-CI.txt' u 0:3:4:xticlabels(2) w yerrorbars lt rgb \"black\" notitle, '${fPlotFilename}_DL_AvgJitter-CI.txt' u 3:xticlabels(2) w points lt 3 notitle"  | gnuplot

    echo -e "Final plot generated: ${fPlotFilename}_DL_AvgJitter-CI.png"    
	
	
	
	
	
	
	
    #Percentage of traffic flows satisfied 
    for ((targetPercentage=50; targetPercentage<=90; targetPercentage+=10))
    do
    
		awk '{split(FILENAME,b,"/"); split(b[1],a,"_"); print a[2]"\t"a[3]"\t"$1"\t"$2;}' ${scenario}_${evalnameRoot}_*/AggregatedStats/System_ratioSatisfiedFlows_${targetPercentage}-CI.txt | sort -k1,1 -k2,2n | awk '{print$0;};' > "${fPlotFilename}_System_ratioSatisfiedFlows_${targetPercentage}-CI.txt"
		max=`awk 'BEGIN{ max =0;} { if ($2>max) {max = $2; } } END {print max}' "${fPlotFilename}_System_ratioSatisfiedFlows_${targetPercentage}-CI.txt"`

		echo "  reset  
          set terminal png nocrop enhanced size 800,600
          set output '${fPlotFilename}_System_ratioSatisfiedFlows_${targetPercentage}-CI.png'
          set offset 0.5,0.5,0,0
          set xlabel '${evalnameRoot}'
          set xtics rotate by -90 offset 0,0.5 font \", 10\"
          set yrange [0:1]
          set title 'Ratio of traffic flows with PDR > 0.${targetPercentage}'
          plot '${fPlotFilename}_System_ratioSatisfiedFlows_${targetPercentage}-CI.txt' u 0:3:4:xticlabels(2) w yerrorbars lt rgb \"black\" notitle, '${fPlotFilename}_System_ratioSatisfiedFlows_${targetPercentage}-CI.txt' u 3:xticlabels(2) w points lt 3 notitle"  | gnuplot
      
        echo -e "Final plot generated: ${fPlotFilename}_System_ratioSatisfiedFlows_${targetPercentage}-CI.txt.png"
		  
		  
		awk '{split(FILENAME,b,"/"); split(b[1],a,"_"); print a[2]"\t"a[3]"\t"$1"\t"$2;}' ${scenario}_${evalnameRoot}_*/AggregatedStats/UL_ratioSatisfiedFlows_${targetPercentage}-CI.txt | sort -k1,1 -k2,2n | awk '{print$0;};' > "${fPlotFilename}_UL_ratioSatisfiedFlows_${targetPercentage}-CI.txt"
		max=`awk 'BEGIN{ max =0;} { if ($2>max) {max = $2; } } END {print max}' "${fPlotFilename}_UL_ratioSatisfiedFlows_${targetPercentage}-CI.txt"`

		echo "  reset  
          set terminal png nocrop enhanced size 800,600
          set output '${fPlotFilename}_UL_ratioSatisfiedFlows_${targetPercentage}-CI.png'
          set offset 0.5,0.5,0,0
          set xlabel '${evalnameRoot}'
          set xtics rotate by -90 offset 0,0.5 font \", 10\"
          set yrange [0:1]
          set title 'Ratio of UL traffic flows with PDR > 0.${targetPercentage}'
          plot '${fPlotFilename}_UL_ratioSatisfiedFlows_${targetPercentage}-CI.txt' u 0:3:4:xticlabels(2) w yerrorbars lt rgb \"black\" notitle, '${fPlotFilename}_UL_ratioSatisfiedFlows_${targetPercentage}-CI.txt' u 3:xticlabels(2) w points lt 3 notitle"  | gnuplot
      
          echo -e "Final plot generated: ${fPlotFilename}_UL_ratioSatisfiedFlows_${targetPercentage}-CI.txt.png"
 
		awk '{split(FILENAME,b,"/"); split(b[1],a,"_"); print a[2]"\t"a[3]"\t"$1"\t"$2;}' ${scenario}_${evalnameRoot}_*/AggregatedStats/DL_ratioSatisfiedFlows_${targetPercentage}-CI.txt | sort -k1,1 -k2,2n | awk '{print$0;};' > "${fPlotFilename}_DL_ratioSatisfiedFlows_${targetPercentage}-CI.txt"
		max=`awk 'BEGIN{ max =0;} { if ($2>max) {max = $2; } } END {print max}' "${fPlotFilename}_DL_ratioSatisfiedFlows_${targetPercentage}-CI.txt"`

		echo "  reset  
          set terminal png nocrop enhanced size 800,600
          set output '${fPlotFilename}_DL_ratioSatisfiedFlows_${targetPercentage}-CI.png'
          set offset 0.5,0.5,0,0
          set xlabel '${evalnameRoot}'
          set xtics rotate by -90 offset 0,0.5 font \", 10\"
          set yrange [0:1]
          set title 'Ratio of DL traffic flows with PDR > 0.${targetPercentage}'
          plot '${fPlotFilename}_DL_ratioSatisfiedFlows_${targetPercentage}-CI.txt' u 0:3:4:xticlabels(2) w yerrorbars lt rgb \"black\" notitle, '${fPlotFilename}_DL_ratioSatisfiedFlows_${targetPercentage}-CI.txt' u 3:xticlabels(2) w points lt 3 notitle"  | gnuplot
      
          echo -e "Final plot generated: ${fPlotFilename}_DL_ratioSatisfiedFlows_${targetPercentage}-CI.txt.png"
 
    done
    
done
