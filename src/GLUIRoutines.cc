// GLUIRoutines.cc - routine for displaying the GLUT and GLUI windows

#ifdef USE_OPENGL

#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <assert.h>

#include <glut.h>
#include <glui.h>
#include <algebra3.h>

#include <dm.h>
#include <dmEnvironment.hpp>
#include <dmArticulation.hpp>

#include "GLUIRoutines.h"
#include "Simulation.h"
#include "DebugControl.h"
#include "PGDMath.h"
#include "TIFFWrite.h"

// external functions
int ReadModel();
void WriteModel();
void OutputProgramState(ostream &out);
void OutputKinetics();

// various globals

// Simulation global
extern Simulation *gSimulation;

extern int gWindowWidth;
extern int gWindowHeight;
extern bool gFinishedFlag;
extern bool gUseSeparateWindow;
extern int gDisplaySkip;
extern float gFOV;
extern float gCameraDistance;
    
int gAxisFlag = true;
float gAxisSize = 0.1;
int gDrawMuscleForces = true;
int gDrawContactForces = true;
float gForceLineScale = 0.001; // multiply the force by this before drawing vector
int gDrawMuscles = true;
int gDrawBonesFlag = true;
int gWhiteBackground = false;
int gDrawTrianglesTwice = false;


// movie
static char gMovieDirectory[sizeof(GLUI_String)] = "movie";
static int gWriteMovie = false;
static int gUsePPM = false;

static int gOverlayFlag = false;

static int gTrackingFlag = true;
static float gCOIx = 0;
static float gCOIy = 0;
static float gCOIz = 0;
static GLUI_Rotation *gViewRotationControl;
static int gBallDamping = true;
            
static float gCameraX;
static float gCameraY;
static float gCameraZ;

static int gWindowPositionX = 10;
static int gWindowPositionY = 20;

static int gMainWindow; // stores the id of the main display window
static int gStepFlag = false;
static int gSlowFlag = false;
static int gRunFlag = false;

// rule definition
static float gRulerMin = -500;
static float gRulerMax = 500;
static float gRulerTextSize = 0.1;
static float gRulerTextInterval = 1.0;
static float gRulerTickSize = 0.05;
static float gRulerTickInterval = 0.5;

static GLfloat gLight0Ambient[] =  {0.1f, 0.1f, 0.3f, 1.0f};
static GLfloat gLight0Diffuse[] =  {.6f, .6f, 1.0f, 1.0f};
static GLfloat gLight0Specular[] =  {.6f, .6f, 1.0f, 1.0f};
static GLfloat gLight0Position[] = {.5f, .5f, 1.0f, 0.0f};

static GLfloat gLight1Ambient[] =  {0.1f, 0.1f, 0.3f, 1.0f};
static GLfloat gLight1Diffuse[] =  {.9f, .6f, 0.0f, 1.0f};
static GLfloat gLight1Specular[] =  {.9f, .6f, 0.0f, 1.0f};
static GLfloat gLight1Position[] = {-1.0f, -1.0f, 1.0f, 0.0f};

static GLfloat gLightsRotation[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };

static float gXYAspect;
static GLUI *gGLUIControlWindow;
static float gViewRotation[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
static float gScale = 1.0;

// limits
static GLUI_Spinner *gTimeLimitSpinner;
static GLUI_Spinner *gMechanicalEnergyLimitSpinner;
static GLUI_Spinner *gMetabolicEnergyLimitSpinner;

// info panel
static GLUI_EditText *gGLUIEditTextCameraX;
static GLUI_EditText *gGLUIEditTextCameraY;
static GLUI_EditText *gGLUIEditTextCameraZ;
static GLUI_EditText *gGLUIEditTextSimulationTime;
static GLUI_EditText *gGLUIEditTextMechanicalEnergy;
static GLUI_EditText *gGLUIEditTextMetabolicEnergy;
static GLUI_EditText *gGLUIEditTextFitness;

// debug panel
static int gDebugListItem;
static int gDebugWrite = false;

static void myGlutKeyboard(unsigned char Key, int x, int y);
static void myGlutIdle( void );
static void myGlutMouse(int button, int button_state, int x, int y );
static void myGlutMotion(int x, int y );
static void myGlutReshape( int x, int y );
static void myGlutDisplay( void );
static void WritePPM(char *pathname, int gWidth, int gHeight, unsigned char *rgb);
static void WriteTIFF(char *pathname, int gWidth, int gHeight, unsigned char *rgb);   
static void GLOutputText(GLfloat x, GLfloat y, GLfloat z, char *text, double size, int plane);
static void ButtonCallback( int control );

// UI ID defines
const int STEP_BUTTON = 1;
const int STOP_BUTTON = 2;
const int START_BUTTON = 3;
const int BALL_RIGHT_BUTTON = 5;
const int BALL_FRONT_BUTTON = 6;
const int BALL_TOP_BUTTON = 7;
const int MOVIE_CHECKBOX = 11;
const int BALL_DAMP_CHECKBOX = 12;
const int DEBUG_CHECKBOX = 21;

const int DEFAULT_TEXT_WIDTH = 40;  


/**************************************** myGlutKeyboard() **********/

void myGlutKeyboard(unsigned char Key, int x, int y)
{
    switch(Key)
    {
        case 27:
        case 'q':
            exit(0);
            break;
    }

    glutPostRedisplay();
}


/***************************************** myGlutIdle() ***********/

void myGlutIdle( void )
{
    /* According to the GLUT specification, the current window is
    undefined during an idle callback.  So we need to explicitly change
    it if necessary */
    if ( glutGetWindow() != gMainWindow )
        glutSetWindow(gMainWindow);

    int i;
    static double cameraCOIX = 0;
    bool newPicture = false;
    int debugStore;

    // before we do anything else, check that we have a model

    if (gFinishedFlag)
    {
        if (ReadModel() == 0)
        {
            gFinishedFlag = false;
#if ! defined(USE_SOCKETS)
            gTimeLimitSpinner->set_float_val((float)gSimulation->GetTimeLimit());
            gMechanicalEnergyLimitSpinner->set_float_val((float)gSimulation->GetMechanicalEnergyLimit());
            gMetabolicEnergyLimitSpinner->set_float_val((float)gSimulation->GetMetabolicEnergyLimit());
#endif
#ifdef USE_SOCKETS
            gRunFlag = true;
#endif
        }
        else
        {
            usleep(1000); // slight pause on read failure
            glutPostRedisplay();
            return;
        }
    }

    if (gRunFlag)
    {
        for (i = 0; i < gDisplaySkip; i++)
        {
            if (gSimulation->ShouldQuit() == true)
            {
                gFinishedFlag = true;
                gRunFlag = false;
                break;
            }
            if (gDebug != NoDebug && i != (gDisplaySkip - 1))
            {
                debugStore = gDebug;
                gDebug = NoDebug;
                gSimulation->UpdateSimulation();
                gDebug = debugStore;
            }
            else gSimulation->UpdateSimulation();

            newPicture = true;

            if (gSimulation->TestForCatastrophy())
            {
                gFinishedFlag = true;
                gRunFlag = false;
                break;
            }
        }

        if (gSlowFlag) usleep(100000); // 0.1 s
    }
    else
    {
        if (gStepFlag)
        {
            gStepFlag = false;
            for (i = 0; i < gDisplaySkip; i++)
            {
                if (gDebug != NoDebug && i != (gDisplaySkip - 1))
                {
                    debugStore = gDebug;
                    gDebug = NoDebug;
                    gSimulation->UpdateSimulation();
                    gDebug = debugStore;
                }
                else gSimulation->UpdateSimulation();
            }
            newPicture = true;
        }

    }

    // update the live variables
    if (gTrackingFlag && gSimulation->GetTime() > 0.0)
    {
        const dmABForKinStruct *theDmABForKinStruct = gSimulation->GetRobot()->getForKinStruct(0);
        if (theDmABForKinStruct->p_ICS[0] != cameraCOIX)
        {
            cameraCOIX = theDmABForKinStruct->p_ICS[0];
            gCOIx = cameraCOIX;
        }
    }

    // set the info values by hand
    gGLUIEditTextCameraX->set_float_val(gCameraX);
    gGLUIEditTextCameraY->set_float_val(gCameraY);
    gGLUIEditTextCameraZ->set_float_val(gCameraZ);
    gGLUIEditTextSimulationTime->set_float_val((float)gSimulation->GetTime());
    gGLUIEditTextMechanicalEnergy->set_float_val((float)gSimulation->GetMechanicalEnergy());
    gGLUIEditTextMetabolicEnergy->set_float_val((float)gSimulation->GetMetabolicEnergy());
    gGLUIEditTextFitness->set_float_val((float)gSimulation->CalculateInstantaneousFitness());

    // get any new limits
#if ! defined(USE_SOCKETS)
    gSimulation->SetTimeLimit((double)gTimeLimitSpinner->get_float_val());
    gSimulation->SetMechanicalEnergyLimit((double)gMechanicalEnergyLimitSpinner->get_float_val());
    gSimulation->SetMetabolicEnergyLimit((double)gMetabolicEnergyLimitSpinner->get_float_val());
#endif
    
    gGLUIControlWindow->sync_live();
    glutPostRedisplay();

    static unsigned int frameCount = 1;
    if (newPicture && gWriteMovie)
    {
        int tx, ty, tw, th;
        GLUI_Master.get_viewport_area( &tx, &ty, &tw, &th );
        unsigned char *rgb = new unsigned char[tw * th * 3];
        glReadBuffer(GL_FRONT);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(0, 0, tw, th, GL_RGB, GL_UNSIGNED_BYTE, rgb);
        char pathname[sizeof(gMovieDirectory) + 256];
        if (gUsePPM)
        {
            sprintf(pathname, "%s/Frame%05d.ppm", gMovieDirectory, frameCount);
            WritePPM(pathname, tw, th, (unsigned char *)rgb);
            frameCount++;
        }
        else
        {
            sprintf(pathname, "%s/Frame%010.4f.tif", gMovieDirectory, gSimulation->GetTime());
            WriteTIFF(pathname, tw, th, (unsigned char *)rgb);
        }
        delete [] rgb;
    }

    // enforce a control update
    // shouldn't be necessary but I'm not getting proper update of live controls on a Mac
    // but I get flickering on Linux and SGI - sigh!
#ifdef UPDATE_CONTROLS_ON_IDLE
    glutSetWindow(gGLUIControlWindow->get_glut_window_id());
    glutPostRedisplay();
#endif
    
    if (gFinishedFlag)
    {
        WriteModel();
    }
}


/***************************************** myGlutMouse() **********/

void myGlutMouse(int button, int button_state, int x, int y )
{
}


/***************************************** myGlutMotion() **********/

void myGlutMotion(int x, int y )
{
    glutPostRedisplay(); 
}

/**************************************** myGlutReshape() *************/

void myGlutReshape( int x, int y )
{
    if (gUseSeparateWindow)
    {
        glViewport( 0, 0, x, y );
        gXYAspect = (float)x / (float)y;
    }
    else
    {
        int tx, ty, tw, th;
        GLUI_Master.get_viewport_area( &tx, &ty, &tw, &th );
        glViewport( tx, ty, tw, th );
        gXYAspect = (float)tw / (float)th;
    }

    glutPostRedisplay();
}

/**************************************** StartGLUT() *************/

void StartGlut(void)
{
    /****************************************/
    /*   Initialize GLUT and create window  */
    /****************************************/

    glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
    glutInitWindowPosition( gWindowPositionX, gWindowPositionY);
    glutInitWindowSize( gWindowWidth, gWindowHeight );

    gMainWindow = glutCreateWindow( "DynaMechs Simulation" );

    glutDisplayFunc( myGlutDisplay );
    glutMotionFunc( myGlutMotion );
    GLUI_Master.set_glutReshapeFunc( myGlutReshape );
    GLUI_Master.set_glutKeyboardFunc( myGlutKeyboard );
    GLUI_Master.set_glutSpecialFunc( NULL );
    GLUI_Master.set_glutMouseFunc( myGlutMouse );

    /****************************************/
    /*       Set up OpenGL lights           */
    /****************************************/

    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);

    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, gLight0Ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, gLight0Diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, gLight0Specular);
    glLightfv(GL_LIGHT0, GL_POSITION, gLight0Position);

    glEnable(GL_LIGHT1);
    glLightfv(GL_LIGHT1, GL_AMBIENT, gLight1Ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, gLight1Diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, gLight1Specular);
    glLightfv(GL_LIGHT1, GL_POSITION, gLight1Position);

    /****************************************/
    /*          Enable z-buffering          */
    /****************************************/

    glEnable(GL_DEPTH_TEST);

    /****************************************/
    /*          Flat Shading                */
    /****************************************/

    glShadeModel(GL_FLAT);

    /****************************************/
    /*          Backface Cull               */
    /****************************************/

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    /****************************************/
    /*         Here's the GLUI code         */
    /****************************************/

    /*** Create the side subwindow ***/
    if (gUseSeparateWindow)
    {
        gGLUIControlWindow = GLUI_Master.create_glui("Controls", 0,
                                                     gWindowPositionX + gWindowWidth + 20, gWindowPositionY);
    }
    else
    {
        gGLUIControlWindow = GLUI_Master.create_glui_subwindow(gMainWindow, GLUI_SUBWINDOW_RIGHT);
    }


    /*** Add the view controls ***/

    // column 1

    // Camera Panel
    GLUI_Panel *cameraPanel = gGLUIControlWindow->add_panel("Camera");
    gViewRotationControl =
        gGLUIControlWindow->add_rotation_to_panel(cameraPanel, "Rotate", gViewRotation);
    gViewRotationControl->set_spin( 0 );
    /* GLUI_Checkbox *dampingCheckBox = */ gGLUIControlWindow->add_checkbox_to_panel(cameraPanel, "Damping", &gBallDamping,
                                                                               BALL_DAMP_CHECKBOX, ButtonCallback);
    /* GLUI_Button *rightButton = */ gGLUIControlWindow->add_button_to_panel(cameraPanel, "Right", BALL_RIGHT_BUTTON, ButtonCallback);
    /* GLUI_Button *topButton = */ gGLUIControlWindow->add_button_to_panel(cameraPanel, "Top", BALL_TOP_BUTTON, ButtonCallback);
    /* GLUI_Button *frontButton = */ gGLUIControlWindow->add_button_to_panel(cameraPanel, "Front", BALL_FRONT_BUTTON, ButtonCallback);

#ifdef USE_TRANSLATION_WIDGETS
    GLUI_Translation *dist =
        gGLUIControlWindow->add_translation_to_panel(cameraPanel,
                                                     "Distance", GLUI_TRANSLATION_Z, &gCameraDistance );
    dist->set_speed( .1 );
#endif
    GLUI_Spinner *cameraDistanceSpinner = gGLUIControlWindow->add_spinner_to_panel(cameraPanel,
                                                                                   "Distance", GLUI_SPINNER_FLOAT, &gCameraDistance);
    cameraDistanceSpinner->set_w(DEFAULT_TEXT_WIDTH);
    cameraDistanceSpinner->set_alignment(GLUI_ALIGN_RIGHT);
    GLUI_Spinner *fovSpiner = gGLUIControlWindow->add_spinner_to_panel(cameraPanel,
                                                                       "FoV", GLUI_SPINNER_FLOAT, &gFOV);
    fovSpiner->set_float_limits(1.0, 170.0);
    fovSpiner->set_w(DEFAULT_TEXT_WIDTH);
    fovSpiner->set_alignment(GLUI_ALIGN_RIGHT);

    // Centre of Interest Panel
    GLUI_Panel *coiPanel = gGLUIControlWindow->add_panel("COI");
#ifdef USE_TRANSLATION_WIDGETS
    GLUI_Translation *trans_x =
        gGLUIControlWindow->add_translation_to_panel(coiPanel,
                                                     "X", GLUI_TRANSLATION_X, &gCOIx );
    trans_x->set_speed( .1 );
    GLUI_Translation *trans_y =
        gGLUIControlWindow->add_translation_to_panel(coiPanel,
                                                     "Y", GLUI_TRANSLATION_Z, &gCOIy );
    trans_y->set_speed( .1 );
    GLUI_Translation *trans_z =
        gGLUIControlWindow->add_translation_to_panel(coiPanel,
                                                     "Z", GLUI_TRANSLATION_Y, &gCOIz );
    trans_z->set_speed( .1 );
#endif
    GLUI_Spinner *coiXSpinner = gGLUIControlWindow->add_spinner_to_panel(coiPanel,
                                                                         "COI X", GLUI_SPINNER_FLOAT, &gCOIx);
    coiXSpinner->set_w(DEFAULT_TEXT_WIDTH);
    coiXSpinner->set_alignment(GLUI_ALIGN_RIGHT);
    GLUI_Spinner *coiYSpinner = gGLUIControlWindow->add_spinner_to_panel(coiPanel,
                                                                         "COI Y", GLUI_SPINNER_FLOAT, &gCOIy);
    coiYSpinner->set_w(DEFAULT_TEXT_WIDTH);
    coiYSpinner->set_alignment(GLUI_ALIGN_RIGHT);
    GLUI_Spinner *coiZSpinner = gGLUIControlWindow->add_spinner_to_panel(coiPanel,
                                                                         "COI Z", GLUI_SPINNER_FLOAT, &gCOIz);
    coiZSpinner->set_w(DEFAULT_TEXT_WIDTH);
    coiZSpinner->set_alignment(GLUI_ALIGN_RIGHT);


#if defined USE_TRANSLATION_WIDGETS
    // column 2
    gGLUIControlWindow->add_column( true );
#endif

    // Display Panel
    GLUI_Panel *displayPanel = gGLUIControlWindow->add_panel("Display");
    gGLUIControlWindow->add_checkbox_to_panel(displayPanel, "Tracking", &gTrackingFlag);
    gGLUIControlWindow->add_checkbox_to_panel(displayPanel, "Overlay", &gOverlayFlag);
    gGLUIControlWindow->add_checkbox_to_panel(displayPanel, "Draw Contact Forces", &gDrawContactForces);
    gGLUIControlWindow->add_checkbox_to_panel(displayPanel, "Draw Muscle Forces", &gDrawMuscleForces);
    gGLUIControlWindow->add_checkbox_to_panel(displayPanel, "Draw Muscles", &gDrawMuscles);
    gGLUIControlWindow->add_checkbox_to_panel(displayPanel, "Draw Bones", &gDrawBonesFlag);
    gGLUIControlWindow->add_checkbox_to_panel(displayPanel, "Draw Axes", &gAxisFlag);
    gGLUIControlWindow->add_checkbox_to_panel(displayPanel, "White Background", &gWhiteBackground);
    gGLUIControlWindow->add_checkbox_to_panel(displayPanel, "2-way Triangles", &gDrawTrianglesTwice);
    GLUI_Spinner *forceScaleSpiner = gGLUIControlWindow->add_spinner_to_panel(displayPanel,
                                                                              "Force Scale", GLUI_SPINNER_FLOAT, &gForceLineScale);
    forceScaleSpiner->set_float_limits(0.0, 0.1);
    forceScaleSpiner->set_w(DEFAULT_TEXT_WIDTH);
    forceScaleSpiner->set_alignment(GLUI_ALIGN_RIGHT);
    GLUI_Spinner *axisSizeSpiner = gGLUIControlWindow->add_spinner_to_panel(displayPanel,
                                                                            "Axis Size", GLUI_SPINNER_FLOAT, &gAxisSize);
    axisSizeSpiner->set_float_limits(0.0, 1.0);
    axisSizeSpiner->set_w(DEFAULT_TEXT_WIDTH);
    axisSizeSpiner->set_alignment(GLUI_ALIGN_RIGHT);

#if ! defined USE_TRANSLATION_WIDGETS && ! defined(USE_SOCKETS)
    // column 2
    gGLUIControlWindow->add_column( true );
#endif

#if ! defined(USE_SOCKETS)
    // Movie Panel
    GLUI_Panel *moviePanel = gGLUIControlWindow->add_panel("Movie");
    GLUI_EditText *directoryNameEditText = gGLUIControlWindow->add_edittext_to_panel(moviePanel, "Directory Name",
                                                                                     GLUI_EDITTEXT_TEXT, gMovieDirectory);
    directoryNameEditText->set_w(DEFAULT_TEXT_WIDTH);
    directoryNameEditText->set_alignment(GLUI_ALIGN_RIGHT);
    gGLUIControlWindow->add_checkbox_to_panel(moviePanel, "Record Movie", &gWriteMovie,
                                              MOVIE_CHECKBOX, ButtonCallback);
    gGLUIControlWindow->add_checkbox_to_panel(moviePanel, "PPM Output", &gUsePPM);

    // Animation Panel
    GLUI_Panel *animatePanel = gGLUIControlWindow->add_panel("Animate");
    gGLUIControlWindow->add_button_to_panel(animatePanel, "Start", START_BUTTON, ButtonCallback);
    gGLUIControlWindow->add_button_to_panel(animatePanel, "Stop", STOP_BUTTON, ButtonCallback);
    gGLUIControlWindow->add_button_to_panel(animatePanel, "Step", STEP_BUTTON, ButtonCallback);
    gGLUIControlWindow->add_statictext_to_panel(animatePanel, "" );
    gGLUIControlWindow->add_checkbox_to_panel(animatePanel, "Slow", &gSlowFlag);
    GLUI_Spinner *frameSkipSpiner = gGLUIControlWindow->add_spinner_to_panel(animatePanel,
                                                                             "Frame Skip", GLUI_SPINNER_INT, &gDisplaySkip);
    frameSkipSpiner->set_int_limits(1, INT_MAX);
    frameSkipSpiner->set_w(DEFAULT_TEXT_WIDTH);
    frameSkipSpiner->set_alignment(GLUI_ALIGN_RIGHT);
    gTimeLimitSpinner = gGLUIControlWindow->add_spinner_to_panel(animatePanel,
                                                                 "Time Limit", GLUI_SPINNER_FLOAT);
    gTimeLimitSpinner->set_w(DEFAULT_TEXT_WIDTH);
    gTimeLimitSpinner->set_alignment(GLUI_ALIGN_RIGHT);
    gMechanicalEnergyLimitSpinner = gGLUIControlWindow->add_spinner_to_panel(animatePanel,
                                                                   "Mech. Energy Limit", GLUI_SPINNER_FLOAT);
    gMechanicalEnergyLimitSpinner->set_w(DEFAULT_TEXT_WIDTH);
    gMechanicalEnergyLimitSpinner->set_alignment(GLUI_ALIGN_RIGHT);
    gMetabolicEnergyLimitSpinner = gGLUIControlWindow->add_spinner_to_panel(animatePanel,
                                                                   "Met. Energy Limit", GLUI_SPINNER_FLOAT);
    gMetabolicEnergyLimitSpinner->set_w(DEFAULT_TEXT_WIDTH);
    gMetabolicEnergyLimitSpinner->set_alignment(GLUI_ALIGN_RIGHT);
#endif

    // Information Panel
    GLUI_Panel *infoPanel = gGLUIControlWindow->add_panel("Info Only");
    gGLUIEditTextCameraX = gGLUIControlWindow->add_edittext_to_panel(infoPanel,
                                                                     "Camera X", GLUI_EDITTEXT_FLOAT);
    gGLUIEditTextCameraX->set_w(DEFAULT_TEXT_WIDTH);
    gGLUIEditTextCameraX->set_alignment(GLUI_ALIGN_RIGHT);
    gGLUIEditTextCameraY = gGLUIControlWindow->add_edittext_to_panel(infoPanel,
                                                                     "Camera Y", GLUI_EDITTEXT_FLOAT);
    gGLUIEditTextCameraY->set_w(DEFAULT_TEXT_WIDTH);
    gGLUIEditTextCameraY->set_alignment(GLUI_ALIGN_RIGHT);
    gGLUIEditTextCameraZ = gGLUIControlWindow->add_edittext_to_panel(infoPanel,
                                                                     "Camera Z", GLUI_EDITTEXT_FLOAT);
    gGLUIEditTextCameraZ->set_w(DEFAULT_TEXT_WIDTH);
    gGLUIEditTextCameraZ->set_alignment(GLUI_ALIGN_RIGHT);
    gGLUIControlWindow->add_statictext_to_panel(infoPanel, "" );
    gGLUIEditTextSimulationTime = gGLUIControlWindow->add_edittext_to_panel(infoPanel, "Sim. Time", GLUI_EDITTEXT_FLOAT);
    gGLUIEditTextSimulationTime->set_w(DEFAULT_TEXT_WIDTH);
    gGLUIEditTextSimulationTime->set_alignment(GLUI_ALIGN_RIGHT);
    gGLUIEditTextMechanicalEnergy = gGLUIControlWindow->add_edittext_to_panel(infoPanel, "Mech. Energy", GLUI_EDITTEXT_FLOAT);
    gGLUIEditTextMechanicalEnergy->set_w(DEFAULT_TEXT_WIDTH);
    gGLUIEditTextMechanicalEnergy->set_alignment(GLUI_ALIGN_RIGHT);
    gGLUIEditTextMetabolicEnergy = gGLUIControlWindow->add_edittext_to_panel(infoPanel, "Met. Energy", GLUI_EDITTEXT_FLOAT);
    gGLUIEditTextMetabolicEnergy->set_w(DEFAULT_TEXT_WIDTH);
    gGLUIEditTextMetabolicEnergy->set_alignment(GLUI_ALIGN_RIGHT);
    gGLUIEditTextFitness = gGLUIControlWindow->add_edittext_to_panel(infoPanel, "Fitness", GLUI_EDITTEXT_FLOAT);
    gGLUIEditTextFitness->set_w(DEFAULT_TEXT_WIDTH);
    gGLUIEditTextFitness->set_alignment(GLUI_ALIGN_RIGHT);

    // debug panel
    if (gDebug == GLUIDebug)
    {
        GLUI_Panel *debugPanel = gGLUIControlWindow->add_panel("Debug");
        GLUI_Listbox *debugListBox =
            gGLUIControlWindow->add_listbox_to_panel
            (debugPanel, "Level", &gDebugListItem);
        for (int i = 0; i < 13; i++)
            debugListBox->add_item(i , (char *)gDebugLabels[i]);
        gGLUIControlWindow->add_checkbox_to_panel(debugPanel, "Debug to File", &gDebugWrite,
                                                  DEBUG_CHECKBOX, ButtonCallback);
    }

#ifdef QUIT_BUTTON_REQUIRED
    gGLUIControlWindow->add_statictext( "" );

    /****** A 'quit' button *****/
    gGLUIControlWindow->add_button( "Quit", 0,(GLUI_Update_CB)exit );
#endif

    // tell the gGLUIControlWindow window which the main window is
    gGLUIControlWindow->set_main_gfx_window(gMainWindow);

    /**** We register the idle callback with GLUI, *not* with GLUT ****/
    GLUI_Master.set_glutIdleFunc( myGlutIdle );

}

/***************************************** myGlutDisplay() *****************/

void myGlutDisplay( void )
{
    static bool neverClearedFlag = true;

    if (gWhiteBackground) glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
    else glClearColor( .0f, .0f, .0f, 1.0f );

    if (neverClearedFlag)
    {
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        neverClearedFlag = false;
    }
    else
    {
        if (gOverlayFlag == false)
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    }


    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective(gFOV, gXYAspect, 0.01, 1000);

    glMatrixMode( GL_MODELVIEW );

    glLoadIdentity();
    glMultMatrixf( gLightsRotation );
    glLightfv(GL_LIGHT0, GL_POSITION, gLight0Position);

    glLoadIdentity();

    // note rotation - need to swap Y and Z because of different up vector
    // for the main view and the rotation widget
    mat4 myRotation(
                    gViewRotation[0], gViewRotation[1], gViewRotation[2], gViewRotation[3],
                    gViewRotation[4], gViewRotation[5], gViewRotation[6], gViewRotation[7],
                    gViewRotation[8], gViewRotation[9], gViewRotation[10], gViewRotation[11],
                    gViewRotation[12], gViewRotation[13], gViewRotation[14], gViewRotation[15]);

    vec4 myEye(0.0, 0.0, gCameraDistance, 1.0);
    vec3 myEyeTransformed = (vec3)(myRotation * myEye);

    gCameraX = gCOIx + myEyeTransformed[0];
    gCameraY = gCOIy + myEyeTransformed[2];
    gCameraZ = gCOIz + myEyeTransformed[1];
pgd::Vector viewDirection(gCOIx - gCameraX, gCOIy - gCameraY, gCOIz - gCameraZ);
    viewDirection.Normalize();
    double angle = fabs(pgd::RadiansToDegrees(acos(viewDirection * pgd::Vector(0, 0, 1))));
    if (angle < 5 || angle > 175)
        gluLookAt( gCameraX, gCameraY, gCameraZ,
                   gCOIx, gCOIy, gCOIz,
                   0.0, 1.0, 0);
    else
        gluLookAt( gCameraX, gCameraY, gCameraZ,
                   gCOIx, gCOIy, gCOIz,
                   0.0, 0.0, 1.0);
    
    glScalef( gScale, gScale, gScale );

    // ===============================================================

    if (gFinishedFlag == false)
    {
        gSimulation->Draw();
    }

    // ===============================================================
    // draw axes
    glDisable(GL_LIGHTING);

    glBegin(GL_LINES);
    glColor3f(1.0, 0.0, 0.0);
    glVertex3f(1.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glEnd();

    glBegin(GL_LINES);
    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(0.0, 1.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glEnd();

    glBegin(GL_LINES);
    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(0.0, 0.0, 1.0);
    glVertex3f(0.0, 0.0, 0.0);
    glEnd();

    // draw ruler
    glColor3f(0.5, 0.5, 1.0);
    char buffer[256];
    GLfloat v;
    for (v = gRulerMin; v <= gRulerMax; v += gRulerTextInterval)
    {
        sprintf(buffer, "%.1f", v);
        GLOutputText(v, 0, -2 * gRulerTextSize, buffer, gRulerTextSize, 1);
    }

    for (v = gRulerMin; v <= gRulerMax; v += gRulerTickInterval)
    {
        glBegin(GL_LINES);
        glVertex3f(v, 0, 0);
        glVertex3f(v, 0, -gRulerTickSize);
        glEnd();
    }
    glBegin(GL_LINES);
    glVertex3f(gRulerMin, 0, 0);
    glVertex3f(gRulerMax, 0, 0);
    glEnd();

    glEnable(GL_LIGHTING);

    glutSwapBuffers();
}  


// write a PPM file (need to invert the y axis)
void WritePPM(char *pathname, int width, int height, unsigned char *rgb)
{
    FILE *out;
    int i;

    out = fopen(pathname, "wb");

    // need to invert write order
    fprintf(out, "P6\n%d %d\n255\n", width, height);
    for (i = height - 1; i >= 0; i--)
        fwrite(rgb + (i * width * 3), width * 3, 1, out);

    fclose(out);
}

// write a TIFF file 
void WriteTIFF(char *pathname, int width, int height, unsigned char *rgb)
{
    TIFFWrite tiff;
    int i;

    tiff.initialiseImage(width, height, 72, 72, 3);
    // need to invert write order
    for (i = 0; i < height; i ++)
        tiff.copyRow(height - i - 1, rgb + (i * width * 3));

    tiff.writeToFile(pathname);
}

// write some 3D text at specified location x, y, z
// plane = 0; draw in x = 0 plane
// plane = 1; draw in y = 0 plane
// plane = 2; draw in z = 0 plane  
void GLOutputText(GLfloat x, GLfloat y, GLfloat z, char *text, double size, int plane)
{
    char *p;
    assert(plane >=0 && plane <= 2);

    glPushMatrix();
    glTranslatef(x, y, z);
    // characters are roughly 100 units so gScale accordingly
    glScalef( size / 100, size / 100, size / 100);
    // with no rotation, text is in the z = 0 plane
    if (plane == 0) glRotatef(90, 0, 1, 0); // x = 0 plane
    else if (plane == 1) glRotatef(90, 1, 0, 0); // y = 0 plane
    for (p = text; *p; p++)
        glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, *p);
    glPopMatrix();
}

// button callback function
void ButtonCallback( int control )
{
    int i;
    switch (control)
    {
        case STEP_BUTTON:
            gStepFlag = true;
            gRunFlag = false;
            break;
        case START_BUTTON:
            gRunFlag = true;
            break;
        case STOP_BUTTON:
            gRunFlag = false;
            break;
        case BALL_RIGHT_BUTTON:
            gViewRotationControl->reset();
            break;
        case BALL_TOP_BUTTON:
            const static float topRotation[16] = {1,0,0,0,  0,0,-1,0,  0,1,0,0,  0,0,0,1};
            for (i = 0; i < 16; i++) gViewRotation[i] = topRotation[i];
            break;
        case BALL_FRONT_BUTTON:
            const static float frontRotation[16] = {0,0,-1,0,  0,1,0,0,  1,0,0,0,  0,0,0,1};
            for (i = 0; i < 16; i++) gViewRotation[i] = frontRotation[i];
            break;
        case BALL_DAMP_CHECKBOX:
            if (gBallDamping)
                gViewRotationControl->set_spin( 0 );
            else
                gViewRotationControl->set_spin( 1.0 );
            break;
        case MOVIE_CHECKBOX:
            if (gWriteMovie)
            {
                struct stat sb;
                int err = stat(gMovieDirectory, &sb);
                if (err) // file doesn't exist
                {
                    char command[sizeof(gMovieDirectory) + 16];
                    sprintf(command, "mkdir %s", gMovieDirectory);
                    system(command);
                    err = stat(gMovieDirectory, &sb);
                }
                if (err) // something wrong
                {
                    gWriteMovie = false;
                    break;
                }
                if ((sb.st_mode & S_IFDIR) == 0)
                {
                    gWriteMovie = false;
                    break;
                }
            }
            break;
        case DEBUG_CHECKBOX:
        {
            if (gDebugWrite)
            {
                char filename[256];
                time_t theTime = time(0);
                struct tm *theLocalTime = localtime(&theTime);
                sprintf(filename, "%s_%04d-%02d-%02d_%02d.%02d.%02d.log",
                        gDebugLabels[gDebugListItem],
                        theLocalTime->tm_year + 1900, theLocalTime->tm_mon + 1,
                        theLocalTime->tm_mday,
                        theLocalTime->tm_hour, theLocalTime->tm_min,
                        theLocalTime->tm_sec);
                ofstream *file = new ofstream();
                file->open(filename, ofstream::out | ofstream::app);
                gDebugStream = file;
                gDebug = gDebugListItem;
            }
            else
            {
                ofstream *file = dynamic_cast<ofstream *>(gDebugStream);
                if (file != (ostream *)&cerr && file != 0)
                {
                    file->close();
                    delete file;
                }
                gDebugStream = &cerr;
                gDebug = 0; // we don't want to debug to stderr
            }
        }
            break;
    }
}

#endif // USE_OPENGL
