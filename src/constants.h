#ifndef CONSTANTS_H
#define CONSTANTS_H

static const unsigned int BASE_APP_WINDOW_WIDTH = 1920;
static const unsigned int SUPERSAMPLE_SCALE     = 2;

static const float JACKET_HUD_SIZE  = 200.0f;
static const float INFO_PANEL_SCALE = 0.5f;
static const float KOHRA_SIZE       = 3.0f;

static const float CAMERA_Y            = 9.0f;
static const float CAMERA_Z            = 9.0f;
static const float CAMERA_Z_TABLET     = 8.0f;
static const float CAMERA_ROT_X        = 26.565f;
static const float CAMERA_ROT_X_TABLET = 27.378f;

static const float TRACK_SIZE_X           = 10.24f;
static const float TRACK_SIZE_Y           = 200.0f;
static const float LANE_WIDTH             = 2.38f;
static const float LANE_DIV_SIZE_X        = 0.12f;
static const float LANE_DIV_SIZE_Y        = 200.0f;
static const float CRITICAL_LINE_SIZE_X   = 2.38f;
static const float CRITICAL_LINE_SIZE_Y   = 1.0f;
static const float SKY_INPUT_LINE_SIZE_X  = 50.0f;
static const float SKY_INPUT_LINE_SIZE_Y  = 1.0f;
static const float SKY_LABEL_SIZE_X       = 9.0f;
static const float SKY_LABEL_SIZE_Y       = 1.0f;
static const float SINGLE_LINE_SIZE_X     = 1.28f;
static const float SINGLE_LINE_SIZE_Y     = 100.0f;

static const float ARC_SEGMENT_LENGTH  = 1000.0f / 14.0f;
static const float TRACE_MESH_SCALE    = 2.0f;
static const float ARC_MESH_SCALE      = 11.5f;
static const float ARC_Y0              = 1.0f;
static const float ARC_Y1              = 5.5f;

static const short int ARC_WINDOW_MS_BEHIND = 25000;
static const short int ARC_WINDOW_MS_AHEAD  = 25000;

static const unsigned char PST_DIFF_COLOR[4] = { 58 , 107, 120, 255 };
static const unsigned char PRS_DIFF_COLOR[4] = { 86 , 105, 71 , 255 };
static const unsigned char FTR_DIFF_COLOR[4] = { 72 , 43 , 84 , 255 };
static const unsigned char BYD_DIFF_COLOR[4] = { 124, 28 , 48 , 255 };
static const unsigned char ETR_DIFF_COLOR[4] = { 67 , 52 , 85 , 255 };

static const unsigned char LIGHT_CONNECTOR_CL[4]   = { 104, 189, 211, 255 };
static const unsigned char CONFICT_CONNECTOR_CL[4] = { 150, 85 , 142, 255 };

static const unsigned short int CMD_PLT_Y          = 150;
static const unsigned short int CMD_PLT_SIZE_X     = 450;
static const unsigned short int CMD_PLT_SIZE_X_BIG = 600;
static const unsigned short int CMD_PLT_SIZE_Y     = 40;
static const unsigned short int CMD_PLT_PADDING    = 8;

#endif // CONSTANTS_H
