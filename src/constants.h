#ifndef CONSTANTS_H
#define CONSTANTS_H

static const unsigned int BASE_APP_WINDOW_WIDTH = 1920;

static const float MINIMUM_SCROLL_SPEED  = 0.01f;
static const float MAXIMUM_SCROLL_SPEED  = 10.0f;

static const float JACKET_HUD_SIZE  = 200.0f;
static const float INFO_PANEL_SCALE = 0.5f;
static const float KOHRA_SIZE       = 3.0f;

static const float CAMERA_Y            = 9.0f;
static const float CAMERA_Z            = 9.0f;
static const float CAMERA_Z_TABLET     = 8.0f;
static const float CAMERA_ROT_X        = 26.565f;
static const float CAMERA_ROT_X_TABLET = 27.378f;

static const float TRACK_SIZE_X          = 10.24f;
static const float TRACK_SIZE_Y          = 200.0f;
static const float LANE_WIDTH            = 2.38f;
static const float LANE_DIV_SIZE_X       = 0.12f;
static const float LANE_DIV_SIZE_Y       = 200.0f;
static const float CRITICAL_LINE_SIZE_X  = 2.38f;
static const float CRITICAL_LINE_SIZE_Y  = 1.0f;
static const float SKY_INPUT_LINE_SIZE_X = 50.0f;
static const float SKY_INPUT_LINE_SIZE_Y = 1.0f;
static const float SKY_LABEL_SIZE_X      = 9.0f;
static const float SKY_LABEL_SIZE_Y      = 1.0f;
static const float SINGLE_LINE_SIZE_X    = 1.28f;
static const float SINGLE_LINE_SIZE_Y    = 100.0f;

static const float MINIMUM_ARC_RES    = 1.0f;
static const float MAXIMUM_ARC_RES    = 10.0f;
static const float ARC_SEGMENT_LENGTH = 1000.0f / 14.0f;
static const float TRACE_MESH_SCALE   = 2.0f;
static const float ARC_MESH_SCALE     = 11.5f;
static const float ARCCAP_TRACE_SCALE = 7.0f;
static const float ARCCAP_ARC_SCALE   = 12.5f;
static const float ARC_Y0             = 1.0f;
static const float ARC_Y1             = 5.5f;

static const unsigned char PST_DIFF_COLOR[4] = { 58 , 107, 120, 255 };
static const unsigned char PRS_DIFF_COLOR[4] = { 86 , 105, 71 , 255 };
static const unsigned char FTR_DIFF_COLOR[4] = { 72 , 43 , 84 , 255 };
static const unsigned char BYD_DIFF_COLOR[4] = { 124, 28 , 48 , 255 };
static const unsigned char ETR_DIFF_COLOR[4] = { 67 , 52 , 85 , 255 };

static const unsigned char LIGHT_CONNECTOR_CL[4]   = { 104, 189, 211, 255 };
static const unsigned char CONFICT_CONNECTOR_CL[4] = { 150, 85 , 142, 255 };

static const unsigned char ARC_BLUE_LOW_CL[4]   = { 25 , 160, 235, 160 };
static const unsigned char ARC_BLUE_HIGH_CL[4]  = { 12 , 212, 212, 160 };
static const unsigned char ARC_PINK_LOW_CL[4]   = { 240, 105, 155, 160 };
static const unsigned char ARC_PINK_HIGH_CL[4]  = { 255, 150, 220, 160 };
static const unsigned char ARC_GREEN_LOW_CL[4]  = { 40 , 200, 30 , 160 };
static const unsigned char ARC_GREEN_HIGH_CL[4] = { 35 , 255, 108, 160 };
static const unsigned char TRACE_CL[4]          = { 145, 120, 170, 122 };
static const unsigned char NOTE_SHADOW_CL[4]    = { 90 , 90 , 90 , 40  };

static const unsigned short int CMD_PLT_Y          = 150;
static const unsigned short int CMD_PLT_SIZE_X     = 450;
static const unsigned short int CMD_PLT_SIZE_X_BIG = 600;
static const unsigned short int CMD_PLT_SIZE_Y     = 40;
static const unsigned short int CMD_PLT_PADDING    = 8;

#endif // CONSTANTS_H
