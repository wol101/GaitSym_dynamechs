GaitSym Basic Documentation 08/07/2004

This document describes the command line options and the parameter file keywords understood by ga and objective that can be used to alter the way the program functions. It also includes a description of the XML file format used.

*******************************

ga

This program runs the genetic algorithm optimisation. It writes genome based data files to disk and runs the simulator. It then reads the fitness value produced by the simulator. It is controlled by command line options and a parameter file.

Command Line Options

Supported options:

(-p) --parameterFile file_name
(-r) --restartDirectory directory_name
(-b) --bestGenome file_name
(-x) --bestGenomeFileXML file_name
(-l) --bestPopulation file_name
(-o) --outputDirectory directory_name
(-d) --debug n
(-g) --maxGenerationsOveride n
(-p) --port n (default 8085)

Note: --parameterFile or --restartDirectory required
Short form is shown in brackets.

Usually only the -p option is needed. This will automatically create an output directory and produce summary data as specified in the parameter file. If a run is aborted this directory can be used as the restart directory using the -r option and the program will continue from where it stopped. This can be used in combination with the -g option to add extra generations to a completed run.

-d produces a lot of debug information. Most of this is not very useful but the values that are understood are as follows:

#define Debug_GA 1
#define Debug_Genome 2
#define Debug_HillClimb 3
#define Debug_Mating 4
#define Debug_Objective 5
#define Debug_ParameterFile 6
#define Debug_Population 7
#define Debug_Random 8
#define Debug_StartupScreen 9
#define Debug_Statistics 10
#define Debug_TestObjective 11
#define Debug_TestRandom 12
#define Debug_Utility 13

Parameter File

This is a typical parameter file:

logNotes
"Standard run configuration"
genomeLength    21
populationSize  49
parentsToKeep   20
immigrations    0
maxGenerations  1000
lowBound        -1
highBound       1
gaussianMutationChance  0.15
frameShiftMutationChance        0
duplicationMutationChance       0
crossoverChance 0.01
defaultGaussianSD       0.1
saveBestEvery   1
savePopEvery    100
parallelObjectives      1
improvementGenerations  100
improvementThreshold    0.1
removeDuplicates        1
multipleGaussian        1
normalStats     0
fitnessTimeLimit        1200
fitnessRestartOnError   1
steadyState     0
startingPopulation      "Population_0000099.txt"
extraDataFile   "OriginalHumanSubstitutionModel.xml"
objectiveCommand        "objective"
parentSelection RouletteWheelSelection
crossoverType   Average
genomeType      IndividualRanges
conversionType  SmartSubstitution

Most of these values are self explanatory and will only rarely need to be altered. However there are alternatives for some values:

parentSelection can be UniformSelection, RankBasedSelection, SqrtBasedSelection, or RouletteWheelSelection. These effect the change of an individual being a parent. The last two require positive fitness values so are not always usable.

crossoverType can be OnePoint or Average.

conversionType can be SixMuscle, EightMuscle, TwelveMuscleQuad, TwentyFourMuscleQuadKinematic, TwelveMuscleQuadKinematic, or SmartSubstitution. It is recommended that SmartSubstitution is used for all new simulations.

modelGenome and startingPopulation both specify a starting file (either a genome or a population) that must match the size specified. This is used instead of a completely random population. If a modelGenome is specified incrementalMutation can be set to 1 to increase the variability of the population based on the genome. If randomiseModel is set to 1 then the model genome is simply used as a way of specifying individual bounds and Gaussian SDs for individual genes.

genealogyFile can be specified if genealogy data is required (warning these files can get very large).

SmartSubstitution

In SmartSubstitution mode the extraDataFile is used as a model for the XML simulation file. SmartSubstitution refers to expressions enclosed in [[ and ]]. These expressions are parsed and used to insert numerical values into the file based on the values in the genome. Individual genes are specified as g(n) where n is the gene number and standard operators are supported + - / * as well as boolean operators ! = > < >= <= & | and an if(test_condition,value_if_non_zero,value_if_zero) function. The gene number can also be the result of an expression and multiple nesting of brackets are supported. Evaluation proceeds left to right with no operator precedence unless brackets are used.

Examples:

[[g(5)]] inserts the numerical value of gene 5 (note gene counting starts at 0).

[[if(g(14)<0,-1*g(14),g(14))]] inserts the positive value of gene 14.

[[if(g(17)<0,-1*g(17),0)]] inserts the positive value of gene 17 if it negative, otherwise 0.

Note unary minus is not supported except with numbers. Use -1*g(5) not -g(5).


*******************************

objective and objective_opengl

These programs run the simulator. The first simply does the calculations (and is therefore best used with ga or when the results are wanted) and the second one produces an interactive front end that can be used for viewing and saving animations. The command line options and XML files are the same for both programs but some of the options only make sense for the interactive version.

Command Line Options

-c filename, --config filename
Reads filename rather than the default config.xml as the config data

-s filename, --score filename
Writes filename rather than the default score.tmp as the fitness data

-g path, --graphicsRoot path
Prepends path to the graphics filenames

-d n, --debug n
Prints out a lot of extra debugging information on stderr if n is higher than 0.
Suitable values of n are defined in DebugControl.h

-w x y, --windowSize x y
Sets the initial display window size - requires separate x and y values.

-k n, --displaySkip n
Displays a new frame every n calculations (default 200)

-o filename, --hostlist filename
Set the filename for the list of servers for the socket version

-r n, --runTimeLimit n
Quits the program (approximately) after it has run n seconds

-k filename, --outputKinematics filename
Writes tab-delimited kinematic data to filename

-t x, --outputModelStateAtTime x
Writes the model state to ModelState.xml at time x

-h, -?, --help
Prints this message!

This time the -d option is very important since it is often the only way of getting numerical data out of the program. It tends to produce very large files that can be quite awkward to analyse although they are usually tab delimited so they can be read into Excel and grep can be used to extract the lines of interest beforehand. The values are as follows:

const int ContactDebug = 1;
const int FitnessDebug = 2;
const int MainDebug = 3;
const int StrapForceDebug = 4;
const int StrapForceAnchorDebug = 5;
const int MAMuscleDebug = 6;
const int MemoryDebug = 7;
const int SideStabilizerDebug = 8;
const int SocketDebug = 9;
const int SimulationDebug = 10;
const int GLUIDebug = 11;
const int DampedSpringDebug = 12;
const int MyMobileBaseLinkDebug = 13;

Of particular interest is -d 11 since in the interactive version this enables a debug menu that can be used to select debugging options during a run. Also when debugging output is selected in objective_opengl then the --display_Skip value is used to filter the debugging output too which can be very helpful when the files are getting too large.

For example -d 1 can be used to obtain ground reaction forces and -d 6 will output muscle forces and velocities.


An example XML file before substitution

<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE GAITSYM SYSTEM "gaitsym.dtd">

<GAITSYM>

<!--
GLOBAL values are used to specify values that control the modelling process
or are used to control the environment
-->

<GLOBAL 
    IntegrationStepsize="0.0001"
    GravityVector="0.0 0.0 -9.81"
    GroundPlanarSpringConstant="40000.0"
    GroundNormalSpringConstant="40000.0"
    GroundPlanarDamperConstant="400.0"
    GroundNormalDamperConstant="400.0"
    GroundStaticFrictionCoeff="2000.0"     
    GroundKineticFrictionCoeff="1600.0"  
        
    BMR="0" 
    DefaultJointLimitSpringConstant="1000.0" 
    DefaultJointLimitDamperConstant="100.0" 
    DefaultJointFriction="0"      
    DefaultMuscleActivationK="0.17"      
    DefaultMuscleForcePerUnitArea="200000"  
    DefaultMuscleVMaxFactor="6"   
    
    TimeLimit="100"
    EnergyLimit="3000"
    FitnessType="DistanceTravelled" 
/>    

<!-- Links -->

<!--
MOBILE_BASE_LINK specified a Dynamechs dmMobileBaseLink object.
-->

<MOBILE_BASE_LINK
    Name="Torso"
    GraphicFile="pelvis.obj"
    Scale="0.519"
    Mass="60.2403"
    MOI="1.033303 0.0 0.0   
         0.0 10.33302843 0.0   
         0.0 0.0 10.33302843"
    CM="-0.324894 0.0 0.0"
    Position="0.5  0.5  -0.5  0.5  
              0.0  0.0  0.90"
    Velocity="0.0  0.0  0.0              
              0.0  0.0  0.0"
    VelocityRange="-10 10 -10 10 -10 10"
    PositionRange="-0.5 100 -0.1 0.1 0.5 2.5"        
/>


<!--
REVOLUTE_LINK specified a Dynamechs dmRevoluteLink object.
-->

<REVOLUTE_LINK
    Name="RightThigh"
    GraphicFile="right_femur.obj"
    Scale="0.447"
    Mass="8.885"
    MOI="0.051768 0.0 0.0   
         0.0 0.517678345 0.0   
         0.0 0.0 0.517678345"
    CM="0.193551 0.0 0.0"
    MDHA="0.0"
    MDHAlpha="0.0"
    MDHD="0.0"
    MDHTheta="0.0"
    InitialJointVelocity="0.0"
    JointMin="-0.175"
    JointMax="2.09"
/>

<REVOLUTE_LINK
    Name="RightLeg"
    GraphicFile="right_tib_fib.obj"
    Scale="0.437"
    Mass="4.131525"
    MOI="0.021996 0.0 0.0   
         0.0 0.21995868 0.0   
         0.0 0.0 0.21995868"
    CM="0.189221 0.0 0.0"
    MDHA="0.447"
    MDHAlpha="0.0"
    MDHD="0.0"
    MDHTheta="0.0"
    InitialJointVelocity="0.0"
    JointMin="-1.57"
    JointMax="0.0"
/>

<REVOLUTE_LINK
    Name="RightFoot"
    GraphicFile="right_foot.obj"
    Scale="0.148"
    Mass="1.288325"
    MOI="0.01343529 0.0 0.0   
         0.0 0.001344 0.0   
         0.0 0.0 0.01343529"
    CM="0.0 0.074 0.0"
    MDHA="0.437"
    MDHAlpha="0.0"
    MDHD="0.0"
    MDHTheta="0.0"
    InitialJointVelocity="0.0"
    JointMin="-1.047"
    JointMax="0.576"
/>

<REVOLUTE_LINK
    Name="LeftThigh"
    GraphicFile="left_femur.obj"
    Scale="0.447"
    Mass="8.885"
    MOI="0.051768 0.0 0.0   
         0.0 0.517678345 0.0   
         0.0 0.0 0.517678345"
    CM="0.193551 0.0 0.0"
    MDHA="0.0"
    MDHAlpha="0.0"
    MDHD="0.0"
    MDHTheta="0.0"
    InitialJointVelocity="0.0"
    JointMin="-0.175"
    JointMax="2.09"
/>

<REVOLUTE_LINK
    Name="LeftLeg"
    GraphicFile="left_tib_fib.obj"
    Scale="0.437"
    Mass="4.131525"
    MOI="0.021996 0.0 0.0   
         0.0 0.21995868 0.0   
         0.0 0.0 0.21995868"
    CM="0.189221 0.0 0.0"
    MDHA="0.447"
    MDHAlpha="0.0"
    MDHD="0.0"
    MDHTheta="0.0"
    InitialJointVelocity="0.0"
    JointMin="-1.57"
    JointMax="0.0"
/>

<REVOLUTE_LINK
    Name="LeftFoot"
    GraphicFile="left_foot.obj"
    Scale="0.148"
    Mass="1.288325"
    MOI="0.01343529 0.0 0.0   
         0.0 0.001344 0.0   
         0.0 0.0 0.01343529"
    CM="0.0 0.074 0.0"
    MDHA="0.437"
    MDHAlpha="0.0"
    MDHD="0.0"
    MDHTheta="0.0"
    InitialJointVelocity="0.0"
    JointMin="-1.047"
    JointMax="0.576"
/>

<!--
Robot
-->

<!--
ROBOT specifies how the predefined links are assembled
-->

<ROBOT
    Name="GaitRobot"
    RootLink="Torso"
    LinkPairs="RightThigh Torso
    RightLeg RightThigh
    RightFoot RightLeg
    LeftThigh Torso
    LeftLeg LeftThigh
    LeftFoot LeftLeg"
/>

<!--
Muscles
-->

<!--
MUSCLE specifies muscle lines of action, max force and velocity. Optionally
a MidPointLinkName and a MidPoint can be specified to allow a single
intermediate point. Alternatively a Radius value can be specified which
wraps the muscle around a circle fixed to the origin of a REVOLUTE_LINK.
-->

<MUSCLE
    Name="RightHipExtensor"
    OriginLinkName="Torso"
    Origin="0.052 -0.076 0.0"
    InsertionLinkName="RightThigh"
    Insertion="0.30 0.005 0.0"
    PCA="0.0137"
    Length="0.26"
/>

<MUSCLE
    Name="RightHipFlexor"
    OriginLinkName="Torso"
    Origin="-0.081 0.071 0.0"
    InsertionLinkName="RightThigh"
    Insertion="0.16 0.029 0.0"
    PCA="0.0075"
    Length="0.24"
/>

<MUSCLE
    Name="RightKneeExtensor"
    OriginLinkName="RightThigh"
    Origin="0.25 0.029 0.0"
    Radius="0.06"
    InsertionLinkName="RightLeg"
    Insertion="0.076 0.043 0.0"
    PCA="0.0203"
    Length="0.23"
/>

<MUSCLE
    Name="RightKneeFlexor"
    OriginLinkName="RightThigh"
    Origin="0.25 -0.0 0.0"
    Radius="-0.03"
    InsertionLinkName="RightLeg"
    Insertion="0.08 -0.01 0.0"
    PCA="0.0159"
    Length="0.23"
/>

<!--dorsiflexion-->
<MUSCLE
    Name="RightAnkleExtensor"
    OriginLinkName="RightLeg"
    Origin="0.14 0.033 0.0"
    InsertionLinkName="RightFoot"
    Insertion="0.0 0.057 0.0"
    PCA="0.0051"
    Length="0.29"
/>

<!--plantarflexion-->
<MUSCLE
    Name="RightAnkleFlexor"
    OriginLinkName="RightLeg"
    Origin="0.057 -0.029 0.0"
    InsertionLinkName="RightFoot"
    Insertion="0.038 -0.057 0.0"
    PCA="0.0264"
    Length="0.41"
/>

<MUSCLE
    Name="LeftHipExtensor"
    OriginLinkName="Torso"
    Origin="0.052 -0.076 0.0"
    InsertionLinkName="LeftThigh"
    Insertion="0.30 0.005 0.0"
    PCA="0.0137"
    Length="0.26"
/>

<MUSCLE
    Name="LeftHipFlexor"
    OriginLinkName="Torso"
    Origin="-0.081 0.071 0.0"
    InsertionLinkName="LeftThigh"
    Insertion="0.16 0.029 0.0"
    PCA="0.0075"
    Length="0.24"
/>

<MUSCLE
    Name="LeftKneeExtensor"
    OriginLinkName="LeftThigh"
    Origin="0.25 0.029 0.0"
    Radius="0.06"
    InsertionLinkName="LeftLeg"
    Insertion="0.076 0.043 0.0"
    PCA="0.0203"
    Length="0.23"
/>

<MUSCLE
    Name="LeftKneeFlexor"
    OriginLinkName="LeftThigh"
    Origin="0.25 -0.0 0.0"
    Radius="-0.03"
    InsertionLinkName="LeftLeg"
    Insertion="0.08 -0.01 0.0"
    PCA="0.0159"
    Length="0.23"
/>

<!--dorsiflexion-->
<MUSCLE
    Name="LeftAnkleExtensor"
    OriginLinkName="LeftLeg"
    Origin="0.14 0.033 0.0"
    InsertionLinkName="LeftFoot"
    Insertion="0.0 0.057 0.0"
    PCA="0.0051"
    Length="0.29"
/>

<!--plantarflexion-->
<MUSCLE
    Name="LeftAnkleFlexor"
    OriginLinkName="LeftLeg"
    Origin="0.057 -0.029 0.0"
    InsertionLinkName="LeftFoot"
    Insertion="0.038 -0.057 0.0"
    PCA="0.0264"
    Length="0.41"
/>

<!--
Contact Models
-->

<!--
CONTACT specifies ModifiedContactModel force elements
-->

<CONTACT
    Name="RightFootContact"
    LinkName="RightFoot"
    ContactPositions="0.06 -0.049284 0  
                      0.06 0.148 0"
/>

<CONTACT
    Name="LeftFootContact"
    LinkName="LeftFoot"
    ContactPositions="0.06 -0.049284 0  
                      0.06 0.148 0"
/>

<!--
Drivers
-->

<!--
CYCLIC_DRIVER specifies the activity pattern of a particular muscle.
The DurationValuePairs specify a duration followed by an activity.
The cycle time is the sum of all the durations.
-->

<!-- duration value pairs -->
<CYCLIC_DRIVER
    Name="RightHipExtensorDriver"
    Target="RightHipExtensor"
    DurationValuePairs=
    "[[if(g(0)<0,-1*g(0),g(0))]] [[if(g(1)<0,-1*g(1),0)]]
    [[if(g(7)<0,-1*g(7),g(7))]] [[if(g(8)<0,-1*g(8),0)]]
    [[if(g(14)<0,-1*g(14),g(14))]] [[if(g(15)<0,-1*g(15),0)]]
    [[if(g(0)<0,-1*g(0),g(0))]] [[if(g(4)<0,-1*g(4),0)]]
    [[if(g(7)<0,-1*g(7),g(7))]] [[if(g(11)<0,-1*g(11),0)]]
    [[if(g(14)<0,-1*g(14),g(14))]] [[if(g(18)<0,-1*g(18),0)]]"
/>

<!-- duration value pairs -->
<CYCLIC_DRIVER
    Name="RightHipFlexorDriver"
    Target="RightHipFlexor"
    DurationValuePairs=
    "[[if(g(0)<0,-1*g(0),g(0))]] [[if(g(1)>0,g(1),0)]]
    [[if(g(7)<0,-1*g(7),g(7))]] [[if(g(8)>0,g(8),0)]]
    [[if(g(14)<0,-1*g(14),g(14))]] [[if(g(15)>0,g(15),0)]]
    [[if(g(0)<0,-1*g(0),g(0))]] [[if(g(4)>0,g(4),0)]]
    [[if(g(7)<0,-1*g(7),g(7))]] [[if(g(11)>0,g(11),0)]]
    [[if(g(14)<0,-1*g(14),g(14))]] [[if(g(18)>0,g(18),0)]]"
/>

<!-- duration value pairs -->
<CYCLIC_DRIVER
    Name="RightKneeExtensorDriver"
    Target="RightKneeExtensor"
    DurationValuePairs=
    "[[if(g(0)<0,-1*g(0),g(0))]] [[if(g(2)<0,-1*g(2),0)]]
    [[if(g(7)<0,-1*g(7),g(7))]] [[if(g(9)<0,-1*g(9),0)]]
    [[if(g(14)<0,-1*g(14),g(14))]] [[if(g(16)<0,-1*g(16),0)]]
    [[if(g(0)<0,-1*g(0),g(0))]] [[if(g(5)<0,-1*g(5),0)]]
    [[if(g(7)<0,-1*g(7),g(7))]] [[if(g(12)<0,-1*g(12),0)]]
    [[if(g(14)<0,-1*g(14),g(14))]] [[if(g(19)<0,-1*g(19),0)]]"
/>

<!-- duration value pairs -->
<CYCLIC_DRIVER
    Name="RightKneeFlexorDriver"
    Target="RightKneeFlexor"
    DurationValuePairs=
    "[[if(g(0)<0,-1*g(0),g(0))]] [[if(g(2)>0,g(2),0)]]
    [[if(g(7)<0,-1*g(7),g(7))]] [[if(g(9)>0,g(9),0)]]
    [[if(g(14)<0,-1*g(14),g(14))]] [[if(g(16)>0,g(16),0)]]
    [[if(g(0)<0,-1*g(0),g(0))]] [[if(g(5)>0,g(5),0)]]
    [[if(g(7)<0,-1*g(7),g(7))]] [[if(g(12)>0,g(12),0)]]
    [[if(g(14)<0,-1*g(14),g(14))]] [[if(g(19)>0,g(19),0)]]"
/>

<!-- duration value pairs -->
<CYCLIC_DRIVER
    Name="RightAnkleExtensorDriver"
    Target="RightAnkleExtensor"
    DurationValuePairs=
    "[[if(g(0)<0,-1*g(0),g(0))]] [[if(g(3)<0,-1*g(3),0)]]
    [[if(g(7)<0,-1*g(7),g(7))]] [[if(g(10)<0,-1*g(10),0)]]
    [[if(g(14)<0,-1*g(14),g(14))]] [[if(g(17)<0,-1*g(17),0)]]
    [[if(g(0)<0,-1*g(0),g(0))]] [[if(g(6)<0,-1*g(6),0)]]
    [[if(g(7)<0,-1*g(7),g(7))]] [[if(g(13)<0,-1*g(13),0)]]
    [[if(g(14)<0,-1*g(14),g(14))]] [[if(g(20)<0,-1*g(20),0)]]"
/>

<!-- duration value pairs -->
<CYCLIC_DRIVER
    Name="RightAnkleFlexorDriver"
    Target="RightAnkleFlexor"
    DurationValuePairs=
    "[[if(g(0)<0,-1*g(0),g(0))]] [[if(g(3)>0,g(3),0)]]
    [[if(g(7)<0,-1*g(7),g(7))]] [[if(g(10)>0,g(10),0)]]
    [[if(g(14)<0,-1*g(14),g(14))]] [[if(g(17)>0,g(17),0)]]
    [[if(g(0)<0,-1*g(0),g(0))]] [[if(g(6)>0,g(6),0)]]
    [[if(g(7)<0,-1*g(7),g(7))]] [[if(g(13)>0,g(13),0)]]
    [[if(g(14)<0,-1*g(14),g(14))]] [[if(g(20)>0,g(20),0)]]"
/>


<!-- duration value pairs -->
<CYCLIC_DRIVER
    Name="LeftHipExtensorDriver"
    Target="LeftHipExtensor"
    DurationValuePairs=
    "[[if(g(0)<0,-1*g(0),g(0))]] [[if(g(4)<0,-1*g(4),0)]]
    [[if(g(7)<0,-1*g(7),g(7))]] [[if(g(11)<0,-1*g(11),0)]]
    [[if(g(14)<0,-1*g(14),g(14))]] [[if(g(18)<0,-1*g(18),0)]]
    [[if(g(0)<0,-1*g(0),g(0))]] [[if(g(1)<0,-1*g(1),0)]]
    [[if(g(7)<0,-1*g(7),g(7))]] [[if(g(8)<0,-1*g(8),0)]]
    [[if(g(14)<0,-1*g(14),g(14))]] [[if(g(15)<0,-1*g(15),0)]]"
/>

<!-- duration value pairs -->
<CYCLIC_DRIVER
    Name="LeftHipFlexorDriver"
    Target="LeftHipFlexor"
    DurationValuePairs=
    "[[if(g(0)<0,-1*g(0),g(0))]] [[if(g(4)>0,g(4),0)]]
    [[if(g(7)<0,-1*g(7),g(7))]] [[if(g(11)>0,g(11),0)]]
    [[if(g(14)<0,-1*g(14),g(14))]] [[if(g(18)>0,g(18),0)]]
    [[if(g(0)<0,-1*g(0),g(0))]] [[if(g(1)>0,g(1),0)]]
    [[if(g(7)<0,-1*g(7),g(7))]] [[if(g(8)>0,g(8),0)]]
    [[if(g(14)<0,-1*g(14),g(14))]] [[if(g(15)>0,g(15),0)]]"
/>

<!-- duration value pairs -->
<CYCLIC_DRIVER
    Name="LeftKneeExtensorDriver"
    Target="LeftKneeExtensor"
    DurationValuePairs=
    "[[if(g(0)<0,-1*g(0),g(0))]] [[if(g(5)<0,-1*g(5),0)]]
    [[if(g(7)<0,-1*g(7),g(7))]] [[if(g(12)<0,-1*g(12),0)]]
    [[if(g(14)<0,-1*g(14),g(14))]] [[if(g(19)<0,-1*g(19),0)]]
    [[if(g(0)<0,-1*g(0),g(0))]] [[if(g(2)<0,-1*g(2),0)]]
    [[if(g(7)<0,-1*g(7),g(7))]] [[if(g(9)<0,-1*g(9),0)]]
    [[if(g(14)<0,-1*g(14),g(14))]] [[if(g(16)<0,-1*g(16),0)]]"
/>

<!-- duration value pairs -->
<CYCLIC_DRIVER
    Name="LeftKneeFlexorDriver"
    Target="LeftKneeFlexor"
    DurationValuePairs=
    "[[if(g(0)<0,-1*g(0),g(0))]] [[if(g(5)>0,g(5),0)]]
    [[if(g(7)<0,-1*g(7),g(7))]] [[if(g(12)>0,g(12),0)]]
    [[if(g(14)<0,-1*g(14),g(14))]] [[if(g(19)>0,g(19),0)]]
    [[if(g(0)<0,-1*g(0),g(0))]] [[if(g(2)>0,g(2),0)]]
    [[if(g(7)<0,-1*g(7),g(7))]] [[if(g(9)>0,g(9),0)]]
    [[if(g(14)<0,-1*g(14),g(14))]] [[if(g(16)>0,g(16),0)]]"
/>

<!-- duration value pairs -->
<CYCLIC_DRIVER
    Name="LeftAnkleExtensorDriver"
    Target="LeftAnkleExtensor"
    DurationValuePairs=
    "[[if(g(0)<0,-1*g(0),g(0))]] [[if(g(6)<0,-1*g(6),0)]]
    [[if(g(7)<0,-1*g(7),g(7))]] [[if(g(13)<0,-1*g(13),0)]]
    [[if(g(14)<0,-1*g(14),g(14))]] [[if(g(20)<0,-1*g(20),0)]]
    [[if(g(0)<0,-1*g(0),g(0))]] [[if(g(3)<0,-1*g(3),0)]]
    [[if(g(7)<0,-1*g(7),g(7))]] [[if(g(10)<0,-1*g(10),0)]]
    [[if(g(14)<0,-1*g(14),g(14))]] [[if(g(17)<0,-1*g(17),0)]]"
/>

<!-- duration value pairs -->
<CYCLIC_DRIVER
    Name="LeftAnkleFlexorDriver"
    Target="LeftAnkleFlexor"
    DurationValuePairs=
    "[[if(g(0)<0,-1*g(0),g(0))]] [[if(g(6)>0,g(6),0)]]
    [[if(g(7)<0,-1*g(7),g(7))]] [[if(g(13)>0,g(13),0)]]
    [[if(g(14)<0,-1*g(14),g(14))]] [[if(g(20)>0,g(20),0)]]
    [[if(g(0)<0,-1*g(0),g(0))]] [[if(g(3)>0,g(3),0)]]
    [[if(g(7)<0,-1*g(7),g(7))]] [[if(g(10)>0,g(10),0)]]
    [[if(g(14)<0,-1*g(14),g(14))]] [[if(g(17)>0,g(17),0)]]"
/>

</GAITSYM>

*******************************

Other components that can be used when FitnessType="KinematicMatch" are:

<STEP_DRIVER Name="LeftWristExtensorDriver" Target="LeftWristExtensor" DurationValuePairs="0 0 0.017000000000000001 0 0.034000000000000002 0.1336803165299936 0.051000000000000004 0 0.068000000000000005 0 0.085000000000000006 0.25347932183717281 0.10200000000000001 0.45049019337627821 0.11900000000000001 0.69307187362083444 0.13600000000000001 0 0.15300000000000002 0.65081031025902758 0.17000000000000004 0.9762407770201772 0.18700000000000006 0.82092157131465193 0.20400000000000007 0.63305106651196008 0.22100000000000009 0.16624931708565735 0.2380000000000001 0.83243122317868645 0.25500000000000012 0.14391308697785443 0.27200000000000013 0.56345816454316833 0.28900000000000015 0.28655922462545208 0.30600000000000016 0 0.32300000000000018 0.6895165125770486 0.34000000000000019 0.21237670778252002 0.35700000000000021 0 0.37400000000000022 0 0.39100000000000024 0 0.40800000000000025 0.93689187022602982 0.42500000000000027 0 0.44200000000000028 0.33639762312681937 0.4590000000000003 0 0.47600000000000031 0 0.49300000000000033 0.82215146090161206 0.51000000000000034 0 0.52700000000000036 0 "/>

This is a non-cyclic driver where the DurationValuePairs specify the simulation time when a particular activation level kicks in.

<DATA_TARGET Name="TorsoQ1Target" Target="Torso" DataType="Q1" DurationValuePairs=" 0.017000 0.706291 0.034000 0.706352 0.051000 0.706068 0.068000 0.706586 0.085000 0.706189 0.102000 0.706115 0.119000 0.706072 0.136000 0.705370 0.153000 0.704964 0.170000 0.704526 0.187000 0.704378 0.204000 0.704416 0.221000 0.704389 0.238000 0.704606 0.255000 0.704577 0.272000 0.704604 0.289000 0.704778 0.306000 0.704513 0.323000 0.704339 0.340000 0.704917 0.357000 0.705247 0.374000 0.705080 0.391000 0.704889 0.408000 0.704290 0.425000 0.704043 0.442000 0.704268 0.459000 0.704212 0.476000 0.704110 0.493000 0.703970 0.510000 0.702844 0.527000 0.701775 0.544000 0.701759 0.561000 0.702273 0.578000 0.702104 0.595000 0.702808 0.612000 0.702733 0.629000 0.702068 0.646000 0.701935 0.663000 0.701785 0.680000 0.702503 0.697000 0.704004 0.714000 0.704485 0.731000 0.705074 0.748000 0.704940 0.765000 0.704642 0.782000 0.704928 0.799000 0.705365 0.816000 0.705008 0.833000 0.705214 0.850000 0.704635 0.867000 0.703395 0.884000 0.703170 0.901000 0.703075 0.918000 0.702984 0.935000 0.703420 0.952000 0.703913 0.969000 0.704059 0.986000 0.704397 1.003000 0.704290 1.020000 0.703974 1.037000 0.703542 1.054000 0.703545 1.071000 0.703580 1.088000 0.703094 1.105000 0.703695 1.122000 0.703708 1.139000 0.703601 1.156000 0.703763 1.173000 0.704278 1.190000 0.704650 1.207000 0.703822 1.224000 0.703531 "/>

This is a list of times and values that are used to calculate the KinematicMatch. DataType can be any of the kinematic values Q0, Q1, Q2, Q3, XP, YP, ZP for MOBILE_BASE_LINK and THETA for REVOLUTE_LINK with an optional Weight value that is used as a multiplier.
