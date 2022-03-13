/*
	Copyright (C) 2012-2030

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


// need a define to toggle sse2 on and off for now
// on 64-bit systems, sse2 is supposed to come as standard
//#define _ENABLE_SSE2
//#define _ENABLE_SSE41

//#define _ENABLE_SSE2_SPRITE_NONTEMPLATE
//#define _ENABLE_SSE2_RECTANGLE_NONTEMPLATE
//#define _ENABLE_SSE2_TRIANGLE_MONO_NONTEMPLATE
//#define _ENABLE_SSE2_TRIANGLE_GRADIENT_NONTEMPLATE
//#define _ENABLE_SSE2_TRIANGLE_TEXTURE_NONTEMPLATE
//#define _ENABLE_SSE2_TRIANGLE_TEXTUREGRADIENT_NONTEMPLATE

#include "PS1_Gpu.h"
#include <math.h>
#include "PS1_Timer.h"
#include "Reciprocal.h"
#include "R3000A.h"


#include "MultiThread.h"
#include "GNUThreading_x64.h"


using namespace Playstation1;
using namespace x64Asm::Utilities;
using namespace Math::Reciprocal;



//#define USE_DIVIDE_GCC
//#define USE_MULTIPLY_CUSTOM

#ifndef PS2_COMPILE

/*
#define ALLOW_OPENCL_PS1
#define ENABLE_HWPIXEL_INPUT
//#define TEST_GPU_RENDER
*/

#endif


GLuint GPU::computeProgram;
//GLuint buffers[NUM_BUFS];       //SSBO objects, one for IMG_0, one for IMG_1, and one for commands/response
//static GLchar* computeSource;
GLuint GPU::shaderProgram;
//string computeSource;
GLuint GPU::ssbo1;
GLuint GPU::ssbo;

GLuint GPU::ssbo_precompute;
GLuint GPU::ssbo_sinputdata;
GLuint GPU::ssbo_outputdata;
GLuint GPU::ssbo_shadowvram;
GLuint GPU::ssbo_pixelbuffer;
GLuint GPU::ssbo_sinputpixels;
GLuint GPU::ssbo_inputpixels;
GLuint GPU::ssbo2;
int GPU::tex_w = 1024, GPU::tex_h = 512;
GLuint GPU::tex_output;
GLuint GPU::fboId = 0;

//#define GLSL(version,shader) "#version" #version "\n" #shader

const char* GPU::computeSource =
#include "compute.comp"
;

// templates take too long too compile and won't be needed in later versions

// the graphics can't really get faster than using templates with software rendering, so I'll leave this out for consistency
//#define ENABLE_TEMPLATE_MULTIPIXEL

// this should be defined at compilation for now
//#define USE_TEMPLATES

#ifdef USE_TEMPLATES_PS1_GPU

/*
#define USE_TEMPLATES_RECTANGLE
#define USE_TEMPLATES_RECTANGLE8
#define USE_TEMPLATES_RECTANGLE16
#define USE_TEMPLATES_SPRITE
#define USE_TEMPLATES_SPRITE8
#define USE_TEMPLATES_SPRITE16
#define USE_TEMPLATES_TRIANGLE_MONO
#define USE_TEMPLATES_RECTANGLE_MONO
#define USE_TEMPLATES_TRIANGLE_TEXTURE
#define USE_TEMPLATES_RECTANGLE_TEXTURE
#define USE_TEMPLATES_TRIANGLE_GRADIENT
#define USE_TEMPLATES_RECTANGLE_GRADIENT
#define USE_TEMPLATES_TRIANGLE_TEXTUREGRADIENT
#define USE_TEMPLATES_RECTANGLE_TEXTUREGRADIENT
#define USE_TEMPLATES_LINE_MONO
#define USE_TEMPLATES_LINE_SHADED
#define USE_TEMPLATES_POLYLINE_MONO
#define USE_TEMPLATES_POLYLINE_SHADED
*/

#endif

//#define EXCLUDE_RECTANGLE_NONTEMPLATE
//#define EXCLUDE_RECTANGLE8_NONTEMPLATE
//#define EXCLUDE_RECTANGLE16_NONTEMPLATE
//#define EXCLUDE_SPRITE_NONTEMPLATE
//#define EXCLUDE_SPRITE8_NONTEMPLATE
//#define EXCLUDE_SPRITE16_NONTEMPLATE
//#define EXCLUDE_TRIANGLE_MONO_NONTEMPLATE
//#define EXCLUDE_TRIANGLE_GRADIENT_NONTEMPLATE
//#define EXCLUDE_TRIANGLE_TEXTURE_NONTEMPLATE
//#define EXCLUDE_TRIANGLE_TEXTUREGRADIENT_NONTEMPLATE


#define USE_SCANLINE_TIMER


// allows the enabling of scanline simulation
#define ENABLE_SCANLINES_SIMULATION


//#define ENABLE_DRAW_OVERHEAD


// enable debugging

#ifdef _DEBUG_VERSION_

#define INLINE_DEBUG_ENABLE

//#define INLINE_DEBUG_SPLIT


#define INLINE_DEBUG_WRITE
#define INLINE_DEBUG_DMA_WRITE
#define INLINE_DEBUG_EXECUTE
#define INLINE_DEBUG_READ
#define INLINE_DEBUG_DMA_READ


//#define INLINE_DEBUG_RASTER_SCANLINE


//#define INLINE_DEBUG_RASTER_VBLANK

//#define INLINE_DEBUG_DRAWSTART
//#define INLINE_DEBUG_EVENT
//#define INLINE_DEBUG_VARS
#define INLINE_DEBUG_EXECUTE_NAME
//#define INLINE_DEBUG_DRAW_SCREEN


/*
//#define INLINE_DEBUG_DISPLAYAREA
//#define INLINE_DEBUG_DISPLAYMODE
//#define INLINE_DEBUG_DISPLAYENABLE
//#define INLINE_DEBUG_DISPLAYOFFSET
//#define INLINE_DEBUG_DISPLAYRANGE
//#define INLINE_DEBUG_MASK
//#define INLINE_DEBUG_WINDOW

//#define INLINE_DEBUG_TRIANGLE_MONO_PIXELSDRAWN
//#define INLINE_DEBUG_TRIANGLE_TEXTURE
//#define INLINE_DEBUG_TEXTURE_RECTANGLE
//#define INLINE_DEBUG_PARTIAL_TRIANGLE

//#define INLINE_DEBUG_RUN_MONO
//#define INLINE_DEBUG_RUN_SHADED
//#define INLINE_DEBUG_RUN_TEXTURE
//#define INLINE_DEBUG_RUN_SPRITE
//#define INLINE_DEBUG_RUN_TRANSFER

//#define INLINE_DEBUG_RASTER


//#define INLINE_DEBUG
//#define INLINE_DEBUG_TRIANGLE_MONO
//#define INLINE_DEBUG_TRIANGLE_GRADIENT
//#define INLINE_DEBUG_TRIANGLE_GRADIENT_TEST
//#define INLINE_DEBUG_TRIANGLE_TEXTURE
//#define INLINE_DEBUG_TRIANGLE_TEXTURE_GRADIENT
//#define INLINE_DEBUG_TRIANGLE_MONO_TEST
*/

#endif


#ifdef ALLOW_OPENCL_PS1

/*
cl_mem GPU::bufa, GPU::bufb, GPU::bufc, GPU::bufd, GPU::bufe, GPU::buff, GPU::bufg;
cl_program GPU::p;
int GPU::num_compute_units, GPU::num_local_workers, GPU::total_compute_units;
int GPU::num_local_workers_square;

Compute::Context *GPU::ctx;

// status of the opencl gpu
volatile u32 GPU::ulGPURunStatus;

// opencl event for callback
cl_event GPU::evtDrawDone;

u64 GPU::ullBufferMask = c_ulInputBuffer_Mask;
*/

#endif

volatile u64 GPU::ulTBufferIndex;


funcVoid GPU::UpdateInterrupts;

u32* GPU::_DebugPC;
u64* GPU::_DebugCycleCount;
u64* GPU::_SystemCycleCount;
u32* GPU::_NextEventIdx;

//u32* GPU::_Intc_Master;
u32* GPU::_Intc_Stat;
u32* GPU::_Intc_Mask;
u32* GPU::_R3000A_Status_12;
u32* GPU::_R3000A_Cause_13;
u64* GPU::_ProcStatus;

//GPU::t_InterruptCPU GPU::InterruptCPU;

s32 GPU::gx [ 4 ], GPU::gy [ 4 ];
s32 GPU::gu [ 4 ], GPU::gv [ 4 ];
s32 GPU::gr [ 4 ], GPU::gg [ 4 ], GPU::gb [ 4 ];
u32 GPU::gbgr [ 4 ];

s32 GPU::x, GPU::y, GPU::w, GPU::h;
u32 GPU::clut_x, GPU::clut_y, GPU::tpage_tx, GPU::tpage_ty, GPU::tpage_abr, GPU::tpage_tp, GPU::command_tge, GPU::command_abe, GPU::command_abr;
s32 GPU::u, GPU::v;

u32 GPU::NumberOfPixelsDrawn;


GPU* GPU::_GPU;


u64* GPU::_NextSystemEvent;


// needs to be removed sometime - no longer needed
u32* GPU::DebugCpuPC;


WindowClass::Window *GPU::DisplayOutput_Window;
WindowClass::Window *GPU::FrameBuffer_DebugWindow;

u32 GPU::MainProgramWindow_Width;
u32 GPU::MainProgramWindow_Height;


//Compute::Context *GPU::ctx;



u32 GPU::ulNumberOfThreads;
u32 GPU::ulNumberOfThreads_Created;
Api::Thread* GPU::GPUThreads [ GPU::c_iMaxThreads ];

volatile u64 GPU::ullInputBuffer_Index;
volatile u32 GPU::ulInputBuffer_Count;
u32 GPU::inputdata [ ( 1 << GPU::c_ulInputBuffer_Shift ) * GPU::c_ulInputBuffer_Size ] __attribute__ ((aligned (32)));
u32 *GPU::p_uHwInputData32;
u32 *GPU::p_uHwOutputData32;

volatile u64 GPU::ulInputBuffer_WriteIndex;
volatile u64 GPU::ulInputBuffer_TargetIndex;
volatile u64 GPU::ulInputBuffer_ReadIndex;


volatile u64 GPU::ullPixelInBuffer_WriteIndex;
volatile u64 GPU::ullPixelInBuffer_TargetIndex;
volatile u64 GPU::ullPixelInBuffer_ReadIndex;
u32 GPU::ulPixelInBuffer32 [ GPU::c_ullPixelInBuffer_Size ] __attribute__ ((aligned (32)));
u32 *GPU::p_ulHwPixelInBuffer32;


HANDLE ghEvent_PS1GPU_Update;
HANDLE ghEvent_PS1GPU_Frame;
HANDLE ghEvent_PS1GPU_Finish;


bool GPU::DebugWindow_Enabled;
//WindowClass::Window *GPU::DebugWindow;

// dimension 1 is twx/twy, dimension #2 is window tww/twh, dimension #3 is value
//u8 GPU::Modulo_LUT [ 32 ] [ 32 ] [ 256 ];


Debug::Log GPU::debug;


const u32 GPU::HBlank_X_LUT [ 8 ] = { 256, 368, 320, 0, 512, 0, 640, 0 };
const u32 GPU::VBlank_Y_LUT [ 2 ] = { 480, 576 };
const u32 GPU::Raster_XMax_LUT [ 2 ] [ 8 ] = { { 341, 487, 426, 0, 682, 0, 853, 0 }, { 340, 486, 426, 0, 681, 0, 851, 0 } };
const u32 GPU::Raster_YMax_LUT [ 2 ] = { 525, 625 };

const u32 GPU::c_iGPUCyclesPerPixel [] = { 10, 7, 8, 0, 5, 0, 4, 0 };


const s64 GPU::c_iDitherValues [] = { -4LL << 32, 0LL << 32, -3LL << 32, 1LL << 32,
									2LL << 32, -2LL << 32, 3LL << 32, -1LL << 32,
									-3LL << 32, 1LL << 32, -4LL << 32, 0LL << 32,
									3LL << 32, -1LL << 32, 2LL << 32, -2LL << 32 };

const s64 GPU::c_iDitherZero [] = { 0, 0, 0, 0,
									0, 0, 0, 0,
									0, 0, 0, 0,
									0, 0, 0, 0 };

const s64 GPU::c_iDitherValues24 [] = { -4LL << 24, 0LL << 24, -3LL << 24, 1LL << 24,
									2LL << 24, -2LL << 24, 3LL << 24, -1LL << 24,
									-3LL << 24, 1LL << 24, -4LL << 24, 0LL << 24,
									3LL << 24, -1LL << 24, 2LL << 24, -2LL << 24 };

const s32 GPU::c_iDitherValues16 [] = { -4LL << 16, 0LL << 16, -3LL << 16, 1LL << 16,
									2LL << 16, -2LL << 16, 3LL << 16, -1LL << 16,
									-3LL << 16, 1LL << 16, -4LL << 16, 0LL << 16,
									3LL << 16, -1LL << 16, 2LL << 16, -2LL << 16 };

const s32 GPU::c_iDitherValues4 [] = { -4LL << 4, 0LL << 4, -3LL << 4, 1LL << 4,
									2LL << 4, -2LL << 4, 3LL << 4, -1LL << 4,
									-3LL << 4, 1LL << 4, -4LL << 4, 0LL << 4,
									3LL << 4, -1LL << 4, 2LL << 4, -2LL << 4 };

const s16 GPU::c_viDitherValues16_add [] = { 0<<8, 0<<8, 0<<8, 1<<8, 0<<8, 0<<8, 0<<8, 1<<8, 0<<8, 0<<8, 0<<8, 1<<8, 0<<8, 0<<8, 0<<8, 1<<8,
											2<<8, 0<<8, 3<<8, 0<<8, 2<<8, 0<<8, 3<<8, 0<<8, 2<<8, 0<<8, 3<<8, 0<<8, 2<<8, 0<<8, 3<<8, 0<<8,
											0<<8, 1<<8, 0<<8, 0<<8, 0<<8, 1<<8, 0<<8, 0<<8, 0<<8, 1<<8, 0<<8, 0<<8, 0<<8, 1<<8, 0<<8, 0<<8,
											3<<8, 0<<8, 2<<8, 0<<8, 3<<8, 0<<8, 2<<8, 0<<8, 3<<8, 0<<8, 2<<8, 0<<8, 3<<8, 0<<8, 2<<8, 0<<8 };

const s16 GPU::c_viDitherValues16_sub [] = { 4<<8, 0<<8, 3<<8, 0<<8, 4<<8, 0<<8, 3<<8, 0<<8, 4<<8, 0<<8, 3<<8, 0<<8, 4<<8, 0<<8, 3<<8, 0<<8,
											0<<8, 2<<8, 0<<8, 1<<8, 0<<8, 2<<8, 0<<8, 1<<8, 0<<8, 2<<8, 0<<8, 1<<8, 0<<8, 2<<8, 0<<8, 1<<8,
											3<<8, 0<<8, 4<<8, 0<<8, 3<<8, 0<<8, 4<<8, 0<<8, 3<<8, 0<<8, 4<<8, 0<<8, 3<<8, 0<<8, 4<<8, 0<<8,
											0<<8, 1<<8, 0<<8, 2<<8, 0<<8, 1<<8, 0<<8, 2<<8, 0<<8, 1<<8, 0<<8, 2<<8, 0<<8, 1<<8, 0<<8, 2<<8 };
							


/*
GPU::GPU ()
{
	cout << "Running GPU constructor...\n";
}
*/

void GPU::Reset ()
{
	double dBusCycleRate;
	
	// zero object
	memset ( this, 0, sizeof( GPU ) );
	
#ifdef PS2_COMPILE

	dBusCycleRate = SystemBus_CyclesPerSec_PS2Mode1;

#else

	dBusCycleRate = SystemBus_CyclesPerSec_PS1Mode;
	
#endif

	NTSC_CyclesPerFrame = ( dBusCycleRate / ( 59.94005994L / 2.0L ) );
	PAL_CyclesPerFrame = ( dBusCycleRate / ( 50.0L / 2.0L ) );
	NTSC_FramesPerCycle = 1.0L / ( dBusCycleRate / ( 59.94005994L / 2.0L ) );
	PAL_FramesPerCycle = 1.0L / ( dBusCycleRate / ( 50.0L / 2.0L ) );

	NTSC_CyclesPerScanline = ( dBusCycleRate / ( 525.0L * ( 59.94005994L / 2.0L ) ) );
	PAL_CyclesPerScanline = ( dBusCycleRate / ( 625.0L * ( 50.0L / 2.0L ) ) );
	NTSC_ScanlinesPerCycle = 1.0L / ( dBusCycleRate / ( 525.0L * ( 59.94005994L / 2.0L ) ) );
	PAL_ScanlinesPerCycle = 1.0L / ( dBusCycleRate / ( 625.0L * ( 50.0L / 2.0L ) ) );

	NTSC_CyclesPerField_Even = 263.0L * ( dBusCycleRate / ( 525.0L * ( 59.94005994L / 2.0L ) ) );
	NTSC_CyclesPerField_Odd = 262.0L * ( dBusCycleRate / ( 525.0L * ( 59.94005994L / 2.0L ) ) );
	PAL_CyclesPerField_Even = 313.0L * ( dBusCycleRate / ( 625.0L * ( 50.0L / 2.0L ) ) );
	PAL_CyclesPerField_Odd = 312.0L * ( dBusCycleRate / ( 625.0L * ( 50.0L / 2.0L ) ) );
	NTSC_FieldsPerCycle_Even = 1.0L / ( 263.0L * ( dBusCycleRate / ( 525.0L * ( 59.94005994L / 2.0L ) ) ) );
	NTSC_FieldsPerCycle_Odd = 1.0L / ( 262.0L * ( dBusCycleRate / ( 525.0L * ( 59.94005994L / 2.0L ) ) ) );
	PAL_FieldsPerCycle_Even = 1.0L / ( 313.0L * ( dBusCycleRate / ( 625.0L * ( 50.0L / 2.0L ) ) ) );
	PAL_FieldsPerCycle_Odd = 1.0L / ( 312.0L * ( dBusCycleRate / ( 625.0L * ( 50.0L / 2.0L ) ) ) );

	NTSC_DisplayAreaCycles = 240.0L * ( dBusCycleRate / ( 525.0L * ( 59.94005994L / 2.0L ) ) );
	PAL_DisplayAreaCycles = 288.0L * ( dBusCycleRate / ( 625.0L * ( 50.0L / 2.0L ) ) );

	NTSC_CyclesPerVBlank_Even = ( 263.0L - 240.0L ) * ( dBusCycleRate / ( 525.0L * ( 59.94005994L / 2.0L ) ) );
	NTSC_CyclesPerVBlank_Odd = ( 262.0L - 240.0L ) * ( dBusCycleRate / ( 525.0L * ( 59.94005994L / 2.0L ) ) );
	PAL_CyclesPerVBlank_Even = ( 313.0L - 288.0L ) * ( dBusCycleRate / ( 625.0L * ( 50.0L / 2.0L ) ) );
	PAL_CyclesPerVBlank_Odd = ( 312.0L - 288.0L ) * ( dBusCycleRate / ( 625.0L * ( 50.0L / 2.0L ) ) );
	NTSC_VBlanksPerCycle_Even = 1.0L / ( ( 263.0L - 240.0L ) * ( dBusCycleRate / ( 525.0L * ( 59.94005994L / 2.0L ) ) ) );
	NTSC_VBlanksPerCycle_Odd = 1.0L / ( ( 262.0L - 240.0L ) * ( dBusCycleRate / ( 525.0L * ( 59.94005994L / 2.0L ) ) ) );
	PAL_VBlanksPerCycle_Even = 1.0L / ( ( 313.0L - 288.0L ) * ( dBusCycleRate / ( 625.0L * ( 50.0L / 2.0L ) ) ) );
	PAL_VBlanksPerCycle_Odd = 1.0L / ( ( 312.0L - 288.0L ) * ( dBusCycleRate / ( 625.0L * ( 50.0L / 2.0L ) ) ) );
		

	CyclesPerPixel_INC_Lookup [ 0 ] [ 0 ] = NTSC_CyclesPerPixelINC_256;
	CyclesPerPixel_INC_Lookup [ 0 ] [ 1 ] = NTSC_CyclesPerPixelINC_368;
	CyclesPerPixel_INC_Lookup [ 0 ] [ 2 ] = NTSC_CyclesPerPixelINC_320;
	CyclesPerPixel_INC_Lookup [ 0 ] [ 4 ] = NTSC_CyclesPerPixelINC_512;
	CyclesPerPixel_INC_Lookup [ 0 ] [ 6 ] = NTSC_CyclesPerPixelINC_640;

	CyclesPerPixel_INC_Lookup [ 1 ] [ 0 ] = PAL_CyclesPerPixelINC_256;
	CyclesPerPixel_INC_Lookup [ 1 ] [ 1 ] = PAL_CyclesPerPixelINC_368;
	CyclesPerPixel_INC_Lookup [ 1 ] [ 2 ] = PAL_CyclesPerPixelINC_320;
	CyclesPerPixel_INC_Lookup [ 1 ] [ 4 ] = PAL_CyclesPerPixelINC_512;
	CyclesPerPixel_INC_Lookup [ 1 ] [ 6 ] = PAL_CyclesPerPixelINC_640;
	
	CyclesPerPixel_Lookup [ 0 ] [ 0 ] = NTSC_CyclesPerPixel_256;
	CyclesPerPixel_Lookup [ 0 ] [ 1 ] = NTSC_CyclesPerPixel_368;
	CyclesPerPixel_Lookup [ 0 ] [ 2 ] = NTSC_CyclesPerPixel_320;
	CyclesPerPixel_Lookup [ 0 ] [ 4 ] = NTSC_CyclesPerPixel_512;
	CyclesPerPixel_Lookup [ 0 ] [ 6 ] = NTSC_CyclesPerPixel_640;

	CyclesPerPixel_Lookup [ 1 ] [ 0 ] = PAL_CyclesPerPixel_256;
	CyclesPerPixel_Lookup [ 1 ] [ 1 ] = PAL_CyclesPerPixel_368;
	CyclesPerPixel_Lookup [ 1 ] [ 2 ] = PAL_CyclesPerPixel_320;
	CyclesPerPixel_Lookup [ 1 ] [ 4 ] = PAL_CyclesPerPixel_512;
	CyclesPerPixel_Lookup [ 1 ] [ 6 ] = PAL_CyclesPerPixel_640;
	

	PixelsPerCycle_Lookup [ 0 ] [ 0 ] = NTSC_PixelsPerCycle_256;
	PixelsPerCycle_Lookup [ 0 ] [ 1 ] = NTSC_PixelsPerCycle_368;
	PixelsPerCycle_Lookup [ 0 ] [ 2 ] = NTSC_PixelsPerCycle_320;
	PixelsPerCycle_Lookup [ 0 ] [ 4 ] = NTSC_PixelsPerCycle_512;
	PixelsPerCycle_Lookup [ 0 ] [ 6 ] = NTSC_PixelsPerCycle_640;

	PixelsPerCycle_Lookup [ 1 ] [ 0 ] = PAL_PixelsPerCycle_256;
	PixelsPerCycle_Lookup [ 1 ] [ 1 ] = PAL_PixelsPerCycle_368;
	PixelsPerCycle_Lookup [ 1 ] [ 2 ] = PAL_PixelsPerCycle_320;
	PixelsPerCycle_Lookup [ 1 ] [ 4 ] = PAL_PixelsPerCycle_512;
	PixelsPerCycle_Lookup [ 1 ] [ 6 ] = PAL_PixelsPerCycle_640;



	// initialize command buffer mode
	BufferMode = MODE_NORMAL;
	
	//////////////////////////////////////////
	// mark GPU as not busy
	GPU_CTRL_Read.BUSY = 1;
	
	/////////////////////////////////////////////
	// mark GPU as ready to receive commands
	GPU_CTRL_Read.COM = 1;

	// initialize size of command buffer
	//BufferSize = 0;
	
	// GPU not currently busy
	//BusyCycles = 0;
}


#ifdef ALLOW_OPENCL_PS1

GLchar* LoadProgram (const char* fullpath)
{
    GLchar *source;
	std::ifstream in (fullpath);
	std::string result (
		(std::istreambuf_iterator<char> (in)),
		std::istreambuf_iterator<char> ());
	//return result;
    source = new GLchar[result.size() + 1];
	memcpy( source, result.c_str(), result.size() + 1 );

	return source;
}

#endif



// actually, you need to start objects after everything has been initialized
void GPU::Start ()
{
	cout << "Running GPU::Start...\n";
	
#ifdef INLINE_DEBUG_ENABLE

#ifdef INLINE_DEBUG_SPLIT
	// put debug output into a separate file
	debug.SetSplit ( true );
	debug.SetCombine ( false );
#endif

	debug.Create ( "GPU_Log.txt" );
#endif

#ifdef INLINE_DEBUG
	debug << "\r\nEntering GPU::Start";
#endif

	cout << "Resetting GPU...\n";

	Reset ();

	cout << "Preparing PS1 GPU...\n";
	
	///////////////////////////////
	// *** TESTING ***
	GPU_CTRL_Read.Value = 0x14802000;
	UpdateRaster_VARS ();
	// *** END TESTING ***
	////////////////////////////////
	
	// set as current GPU object
	_GPU = this;
	
#ifdef ALLOW_OPENCL_PS1
	// hooking in opencl //
	
	DisplayOutput_Window->OpenGL_MakeCurrentWindow ();
	

	cout << "\nLoading program...";

	//computeSource = LoadProgram("C:\\Users\\PCUser\\Desktop\\TheProject\\hpsx64\\hps1x64\\src\\gpu\\src\\compute.comp");
    //if (computeSource == NULL)
    //{
    //    return;
    //}

	cout << "\nLoaded:" << (char*)computeSource;

	cout << "\nInit glew...";

	GLenum err = glewInit();

	cout << "\nglewInit()= " << err;

	/*
	if (err != GLEW_OK)
	{
	cout << "Error: " << glewGetErrorString(err);
	//fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	//...
	}
	fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
	*/

	cout << "\nCreating compute shader...";

	
	GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);


	cout << "\nglShaderSource...";

    glShaderSource(computeShader, 1, &computeSource, NULL);

	cout << "\nglCompileShader...";

    glCompileShader(computeShader);

	cout << "\nglCreateProgram...";

    computeProgram = glCreateProgram();

	cout << "\nglAttachShader...";

    glAttachShader(computeProgram, computeShader);

	cout << "\nglLinkProgram...";

    glLinkProgram(computeProgram);

	cout << "\nglGetProgramiv...";

    GLint status;
    glGetProgramiv(computeProgram, GL_LINK_STATUS, &status);

    if (status == GL_TRUE)
    {
        printf("link good\n");
    }
    else
    {
        cout << "link bad\n";
        GLchar log[4096];
        GLsizei len;

        glGetProgramInfoLog(computeProgram, 4096, &len, log);

        cout << log << "\n";
        //return;
    }

	cout << "\nglUseProgram...";

	glUseProgram(computeProgram);
//
	// input data copy //

	// invalidate all input buffer entries
	memset( inputdata, -1, sizeof(inputdata) );

	// input command copy data map
	glGenBuffers(1, &ssbo_sinputdata);
	glBindBuffer ( GL_SHADER_STORAGE_BUFFER, ssbo_sinputdata );
	glBufferStorage( GL_SHADER_STORAGE_BUFFER, sizeof(inputdata), NULL, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT );
	p_uHwInputData32 = (u32*) glMapBufferRange ( GL_SHADER_STORAGE_BUFFER, 0, sizeof(inputdata), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT );

	// input pixel copy data map
	glGenBuffers(1, &ssbo_sinputpixels);
	glBindBuffer ( GL_SHADER_STORAGE_BUFFER, ssbo_sinputpixels );
	glBufferStorage( GL_SHADER_STORAGE_BUFFER, sizeof(ulPixelInBuffer32), NULL, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT );
	p_ulHwPixelInBuffer32 = (u32*) glMapBufferRange ( GL_SHADER_STORAGE_BUFFER, 0, sizeof(ulPixelInBuffer32), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT );



	glGenBuffers(1, &ssbo1);
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo1);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo1);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(inputdata), inputdata, GL_DYNAMIC_COPY);
	//glBufferStorage( GL_SHADER_STORAGE_BUFFER, sizeof(inputdata), NULL, GL_DYNAMIC_STORAGE_BIT );

	glGenBuffers(1, &ssbo_inputpixels);
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo1);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, ssbo_inputpixels);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ulPixelInBuffer32), NULL, GL_DYNAMIC_COPY);
	//glBufferStorage( GL_SHADER_STORAGE_BUFFER, sizeof(inputdata), NULL, GL_DYNAMIC_STORAGE_BIT );


	// internal pre-compute buffer //
	glGenBuffers(1, &ssbo_precompute);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_precompute);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 128 * ( 1 << 16 ) * 4, NULL, GL_DYNAMIC_COPY);

	// vram data copy and bind //
	glGenBuffers(1, &ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo);

	// copy 2x the size of VRAM for now, will update later
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(_GPU->VRAM) * 2, NULL, GL_DYNAMIC_COPY); //sizeof(data) only works for statically sized C/C++ arrays.
	//glBufferStorage( GL_SHADER_STORAGE_BUFFER, sizeof(_GPU->VRAM) * 2, NULL, GL_DYNAMIC_STORAGE_BIT );


	glGenBuffers(1, &ssbo_shadowvram);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssbo_shadowvram);

	// copy 2x the size of VRAM for now, will update later
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(_GPU->VRAM) * 2, NULL, GL_DYNAMIC_COPY); //sizeof(data) only works for statically sized C/C++ arrays.
	//glBufferStorage( GL_SHADER_STORAGE_BUFFER, sizeof(_GPU->VRAM) * 2, NULL, GL_DYNAMIC_STORAGE_BIT );


	// pixel buffer
	glGenBuffers(1, &ssbo_pixelbuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo_pixelbuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(_GPU->VRAM) * 2, NULL, GL_DYNAMIC_COPY); //sizeof(data) only works for statically sized C/C++ arrays.

	// output data
	glGenBuffers(1, &ssbo_outputdata);
	glBindBuffer ( GL_SHADER_STORAGE_BUFFER, ssbo_outputdata );
	glBufferStorage( GL_SHADER_STORAGE_BUFFER, sizeof(_GPU->VRAM) * 2, NULL, GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT );
	p_uHwOutputData32 = (u32*) glMapBufferRange ( GL_SHADER_STORAGE_BUFFER, 0, sizeof(_GPU->VRAM) * 2, GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT );



	// test rendering to texture //
	glGenTextures(1, &tex_output);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_output);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8UI, tex_w, tex_h, 0, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, NULL);
	glTexStorage2D(GL_TEXTURE_2D,1,GL_RGBA8,tex_w,tex_h);
	glBindImageTexture(5, tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

	glGenFramebuffers(1, &fboId);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fboId);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_output, 0);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);  // if not already bound
	//glBlitFramebuffer( 0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR );

	DisplayOutput_Window->OpenGL_ReleaseWindow ();

	// initialize next index for gpu
	//ullGPUIndex = 0;
	
#endif

	// testing opencl
	bEnable_OpenCL = false;
	
	// 0 means on same thread, 1 or greater means on separate threads (power of 2 ONLY!!)
	ulNumberOfThreads = 0;
	

	cout << "done\n";

#ifdef INLINE_DEBUG
	debug << "->Exiting GPU::Start";
#endif

	cout << "Exiting GPU::Start...\n";
}



void GPU::draw_screen_th( DATA_Write_Format* inputdata, u32 ulThreadNum )
{
	const int local_id = 0;	//get_local_id( 0 );
	const int group_id = 0;	//get_group_id( 0 );
	const int num_local_threads = 1;	//get_local_size ( 0 );
	const int num_global_groups = 1;	//get_num_groups( 0 );
	
//#ifdef SINGLE_SCANLINE_MODE
	const int xid = 0;
	const int yid = 0;
	
	const int xinc = num_local_threads;
	const int yinc = num_global_groups;
	const int group_yoffset = group_id;
//#endif


//inputdata format:
//0: GPU_CTRL_Read
//1: DisplayRange_Horizontal
//2: DisplayRange_Vertical
//3: ScreenArea_TopLeft
//4: bEnableScanline
//5: Y_Pixel
//6: -----------
//7: Command



	const int c_iVisibleArea_StartX_Cycle = 584;
	const int c_iVisibleArea_EndX_Cycle = 3192;
	const int c_iVisibleArea_StartY_Pixel_NTSC = 15;
	const int c_iVisibleArea_EndY_Pixel_NTSC = 257;
	const int c_iVisibleArea_StartY_Pixel_PAL = 34;
	const int c_iVisibleArea_EndY_Pixel_PAL = 292;

	const int c_iVisibleArea_StartY [] = { c_iVisibleArea_StartY_Pixel_NTSC, c_iVisibleArea_StartY_Pixel_PAL };
	const int c_iVisibleArea_EndY [] = { c_iVisibleArea_EndY_Pixel_NTSC, c_iVisibleArea_EndY_Pixel_PAL };

	const u32 c_iGPUCyclesPerPixel [] = { 10, 7, 8, 0, 5, 0, 4, 0 };


	

	u32 GPU_CTRL_Read;
	u32 DisplayRange_X1;
	u32 DisplayRange_X2;
	u32 DisplayRange_Y1;
	u32 DisplayRange_Y2;
	u32 ScreenArea_TopLeftX;
	u32 ScreenArea_TopLeftY;
	u32 bEnableScanline;
	u32 Y_Pixel;

	
	// so the max viewable width for PAL is 3232/4-544/4 = 808-136 = 672
	// so the max viewable height for PAL is 292-34 = 258
	
	// actually, will initially start with a 1 pixel border based on screen width/height and then will shift if something is off screen

	// need to know visible range of screen for NTSC and for PAL (each should be different)
	// NTSC visible y range is usually from 16-256 (0x10-0x100) (height=240)
	// PAL visible y range is usually from 35-291 (0x23-0x123) (height=256)
	// NTSC visible x range is.. I don't know. start with from about gpu cycle#544 to about gpu cycle#3232 (must use gpu cycles since res changes)
	s32 VisibleArea_StartX, VisibleArea_EndX, VisibleArea_StartY, VisibleArea_EndY, VisibleArea_Width, VisibleArea_Height;
	
	// there the frame buffer pixel, and then there's the screen buffer pixel
	u32 Pixel16, Pixel32_0, Pixel32_1;
	//u64 Pixel64;
	
	
	// this allows you to calculate horizontal pixels
	u32 GPU_CyclesPerPixel;
	
	
	Pixel_24bit_Format Pixel24;
	
	
	s32 FramePixel_X, FramePixel_Y;
	
	// need to know where to draw the actual image at
	s32 Draw_StartX, Draw_StartY, Draw_EndX, Draw_EndY, Draw_Width, Draw_Height;
	
	s32 Source_Height;
	
	
	u16* ptr_vram16;
	u32* ptr_pixelbuffer32;
	
	s32 TopBorder_Height, BottomBorder_Height, LeftBorder_Width, RightBorder_Width;
	s32 current_x, current_y;
	s32 current_xmax, current_ymax;

	
	u32 GPU_CTRL_Read_ISINTER;
	u32 GPU_CTRL_Read_HEIGHT;
	u32 GPU_CTRL_Read_WIDTH;
	u32 GPU_CTRL_Read_DEN;
	u32 GPU_CTRL_Read_ISRGB24;
	u32 GPU_CTRL_Read_VIDEO;

		
	// don't do this if it is running on a different thread
	if ( ( !ulThreadNum ) && _GPU->ulNumberOfThreads )
	{
		return;
	}
	
	
	//if ( !local_id )
	//{
		GPU_CTRL_Read = inputdata [ 0 ].Value;
		DisplayRange_X1 = inputdata [ 1 ].Value & 0xfff;
		DisplayRange_X2 = ( inputdata [ 1 ].Value >> 12 ) & 0xfff;
		DisplayRange_Y1 = inputdata [ 2 ].Value & 0x3ff;
		DisplayRange_Y2 = ( inputdata [ 2 ].Value >> 10 ) & 0x7ff;
		ScreenArea_TopLeftX = inputdata [ 3 ].Value & 0x3ff;
		ScreenArea_TopLeftY = ( inputdata [ 3 ].Value >> 10 ) & 0x1ff;
		bEnableScanline = inputdata [ 4 ].Value;
		Y_Pixel = inputdata [ 5 ].Value;

		
		// bits 16-18
		GPU_CTRL_Read_WIDTH = ( GPU_CTRL_Read >> 16 ) & 7;
		
		// bit 19
		GPU_CTRL_Read_HEIGHT = ( GPU_CTRL_Read >> 19 ) & 1;
		
		// bit 20
		GPU_CTRL_Read_VIDEO = ( GPU_CTRL_Read >> 20 ) & 1;
		
		// bit 21
		GPU_CTRL_Read_ISRGB24 = ( GPU_CTRL_Read >> 21 ) & 1;
		
		// bit 22
		GPU_CTRL_Read_ISINTER = ( GPU_CTRL_Read >> 22 ) & 1;
		
		// bit 23
		GPU_CTRL_Read_DEN = ( GPU_CTRL_Read >> 23 ) & 1;
		
		
		// GPU cycles per pixel depends on width
		GPU_CyclesPerPixel = c_iGPUCyclesPerPixel [ GPU_CTRL_Read_WIDTH ];

		// get the pixel to start and stop drawing at
		Draw_StartX = DisplayRange_X1 / GPU_CyclesPerPixel;
		Draw_EndX = DisplayRange_X2 / GPU_CyclesPerPixel;
		Draw_StartY = DisplayRange_Y1;
		Draw_EndY = DisplayRange_Y2;

		Draw_Width = Draw_EndX - Draw_StartX;
		Draw_Height = Draw_EndY - Draw_StartY;
		// get the pixel to start and stop at for visible area
		VisibleArea_StartX = c_iVisibleArea_StartX_Cycle / GPU_CyclesPerPixel;
		VisibleArea_EndX = c_iVisibleArea_EndX_Cycle / GPU_CyclesPerPixel;

		// visible area start and end y depends on pal/ntsc
		VisibleArea_StartY = c_iVisibleArea_StartY [ GPU_CTRL_Read_VIDEO ];
		VisibleArea_EndY = c_iVisibleArea_EndY [ GPU_CTRL_Read_VIDEO ];

		VisibleArea_Width = VisibleArea_EndX - VisibleArea_StartX;
		VisibleArea_Height = VisibleArea_EndY - VisibleArea_StartY;


		Source_Height = Draw_Height;

		if ( GPU_CTRL_Read_ISINTER && GPU_CTRL_Read_HEIGHT )
		{
			// 480i mode //
			
			// if not simulating scanlines, then the draw height should double too
			if ( !bEnableScanline )
			{
				VisibleArea_EndY += Draw_Height;
				VisibleArea_Height += Draw_Height;
				
				Draw_EndY += Draw_Height;
				
				Draw_Height <<= 1;
			}
			
			Source_Height <<= 1;
		}
	
#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << "\r\nGPU::Draw_Screen; GPU_CyclesPerPixel=" << dec << GPU_CyclesPerPixel << " Draw_StartX=" << Draw_StartX << " Draw_EndX=" << Draw_EndX;
	debug << "\r\nDraw_StartY=" << Draw_StartY << " Draw_EndY=" << Draw_EndY << " VisibleArea_StartX=" << VisibleArea_StartX;
	debug << "\r\nVisibleArea_EndX=" << VisibleArea_EndX << " VisibleArea_StartY=" << VisibleArea_StartY << " VisibleArea_EndY=" << VisibleArea_EndY;
#endif

		
		
		if ( !GPU_CTRL_Read_DEN )
		{
			BottomBorder_Height = VisibleArea_EndY - Draw_EndY;
			LeftBorder_Width = Draw_StartX - VisibleArea_StartX;
			TopBorder_Height = Draw_StartY - VisibleArea_StartY;
			RightBorder_Width = VisibleArea_EndX - Draw_EndX;
			
			if ( BottomBorder_Height < 0 ) BottomBorder_Height = 0;
			if ( LeftBorder_Width < 0 ) LeftBorder_Width = 0;
			
			//cout << "\n(before)Left=" << dec << LeftBorder_Width << " Bottom=" << BottomBorder_Height << " Draw_Width=" << Draw_Width << " VisibleArea_Width=" << VisibleArea_Width;
			
			
			current_ymax = Draw_Height + BottomBorder_Height;
			current_xmax = Draw_Width + LeftBorder_Width;
			
			// make suree that ymax and xmax are not greater than the size of visible area
			if ( current_xmax > VisibleArea_Width )
			{
				// entire image is not on the screen, so take from left border and recalc xmax //

				LeftBorder_Width -= ( current_xmax - VisibleArea_Width );
				if ( LeftBorder_Width < 0 ) LeftBorder_Width = 0;
				current_xmax = Draw_Width + LeftBorder_Width;
				
				// make sure again we do not draw past the edge of screen
				if ( current_xmax > VisibleArea_Width ) current_xmax = VisibleArea_Width;
			}
			
			if ( current_ymax > VisibleArea_Height )
			{
				BottomBorder_Height -= ( current_ymax - VisibleArea_Height );
				if ( BottomBorder_Height < 0 ) BottomBorder_Height = 0;
				current_ymax = Draw_Height + BottomBorder_Height;
				
				// make sure again we do not draw past the edge of screen
				if ( current_ymax > VisibleArea_Height ) current_ymax = VisibleArea_Height;
			}
			
		}	// end if ( !GPU_CTRL_Read_DEN )
		
	//}	// end if ( !local_id )

	
	// synchronize variables across workers
	//barrier ( CLK_LOCAL_MEM_FENCE );
	

	// *** new stuff *** //

	//FramePixel = 0;
	ptr_pixelbuffer32 = _GPU->PixelBuffer;
	//ptr_pixelbuffer64 = (u64*) PixelBuffer;

	
		//cout << "\n(after)Left=" << dec << LeftBorder_Width << " Bottom=" << BottomBorder_Height << " Draw_Width=" << Draw_Width << " VisibleArea_Width=" << VisibleArea_Width;
		//cout << "\n(after2)current_xmax=" << current_xmax << " current_ymax=" << current_ymax;
		
#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << "\r\nGPU::Draw_Screen; Drawing bottom border";
#endif

	// no need to do this part if rendering on gpu
	if ( ! _GPU->bEnable_OpenCL )
	{

	if ( !GPU_CTRL_Read_DEN )
	{
		// current_y should start at zero for even field and one for odd
		//current_y = 0;
		current_y = group_yoffset + yid;
		
		// added for opencl, need to start in pixel buffer on the right line
		ptr_pixelbuffer32 += ( VisibleArea_Width * ( group_yoffset + yid ) );
		
		// put in bottom border //
		
		
		// check if scanlines simulation is enabled
		if ( bEnableScanline )
		{
			// spread out workers on every other line
			ptr_pixelbuffer32 += ( VisibleArea_Width * ( group_yoffset + yid ) );
			
			// if this is an odd field, then start writing on the next line
			if ( Y_Pixel & 1 )
			{
				// odd field //
				
				ptr_pixelbuffer32 += VisibleArea_Width;
			}
		}
		

		while ( current_y < BottomBorder_Height )
		{
			//current_x = 0;
			current_x = xid;
			
			while ( current_x < VisibleArea_Width )
			{
				// *ptr_pixelbuffer32++ = 0;
				ptr_pixelbuffer32 [ current_x ] = 0;
				
				//current_x++;
				current_x += xinc;
			}
			
			//current_y++;
			current_y += yinc;
			
			// added for opencl, update pixel buffer multiple lines
			ptr_pixelbuffer32 += ( VisibleArea_Width * yinc );
			
			// check if scanline simulation is enabled
			if ( bEnableScanline )
			{
				// update again since doing every other line
				//ptr_pixelbuffer32 += VisibleArea_Width;
				ptr_pixelbuffer32 += ( VisibleArea_Width * yinc );
			}
		}

#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << "\r\nGPU::Draw_Screen; Putting in screen";
	debug << " current_ymax=" << dec << current_ymax;
	debug << " current_xmax=" << current_xmax;
	debug << " VisibleArea_Width=" << VisibleArea_Width;
	debug << " VisibleArea_Height=" << VisibleArea_Height;
	debug << " LeftBorder_Width=" << LeftBorder_Width;
#endif
		
		// put in screen
		
		
		FramePixel_Y = ScreenArea_TopLeftY + Source_Height - 1;
		FramePixel_X = ScreenArea_TopLeftX;


//if ( !__global_id )
//{
//	printf( "FramePixel_X= %i FramePixel_Y= %i", FramePixel_X, FramePixel_Y );
//}

		
		// for opencl, spread the workers across the lines
		//FramePixel_Y -= group_yoffset + yid;
		FramePixel_Y -= ( current_y - BottomBorder_Height );
		
		// check if simulating scanlines
		if ( bEnableScanline )
		{
			// check if 480i
			if ( GPU_CTRL_Read_ISINTER && GPU_CTRL_Read_HEIGHT )
			{
				// 480i //
				
				// for opencl, spread interlace mode to every other line
				//FramePixel_Y -= group_yoffset + yid;
				FramePixel_Y -= ( current_y - BottomBorder_Height );
				
				// check if in odd field or even field
				if ( Y_Pixel & 1 )
				{
					// odd field //
					
					// if the height is even, then it is ok
					// if the height is odd, need to compensate
					if ( ! ( Source_Height & 1 ) )
					{
						FramePixel_Y--;
					}
				}
				else
				{
					// even field //
					
					// if the height is odd, then it is ok
					// if the height is even, need to compensate
					if ( Source_Height & 1 )
					{
						FramePixel_Y--;
					}
				}
				
			} // end if ( GPU_CTRL_Read.ISINTER && GPU_CTRL_Read.HEIGHT )
		}
		
#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << "\r\nGPU::Draw_Screen; drawing screen pixels";
	debug << " current_x=" << dec << current_x;
	debug << " FramePixel_X=" << FramePixel_X;
	debug << " FramePixel_Y=" << FramePixel_Y;
#endif
#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << "\r\ncheck: current_x=" << current_x;
	debug << " current_xmax=" << current_xmax;
	debug << " ptr_vram32=" << ( (u64) ptr_vram32 );
	debug << " VRAM=" << ( (u64) VRAM );
	debug << " ptr_pixelbuffer64=" << ( (u64) ptr_pixelbuffer64 );
	debug << " PixelBuffer=" << ( (u64) PixelBuffer );
#endif




	
		while ( current_y < current_ymax )
		{
#ifdef INLINE_DEBUG_DRAW_SCREEN_2
	debug << "\r\nGPU::Draw_Screen; drawing left border";
	debug << " current_y=" << dec << current_y;
#endif
			// put in the left border
			//current_x = 0;
			current_x = xid;

			while ( current_x < LeftBorder_Width )
			{
				// *ptr_pixelbuffer32++ = 0;
				ptr_pixelbuffer32 [ current_x ] = 0;
				
				//current_x++;
				current_x += xinc;
			}
			
#ifdef INLINE_DEBUG_DRAW_SCREEN_2
	debug << "\r\nGPU::Draw_Screen; drawing screen pixels";
	debug << " current_x=" << dec << current_x;
	debug << " FramePixel_X=" << FramePixel_X;
	debug << " FramePixel_Y=" << FramePixel_Y;
#endif

			// *** important note *** this wraps around the VRAM
			ptr_vram16 = & (_GPU->VRAM [ FramePixel_X + ( ( FramePixel_Y & FrameBuffer_YMask ) << 10 ) ]);
			
#ifdef INLINE_DEBUG_DRAW_SCREEN_2
	debug << "\r\nGPU::Draw_Screen; got vram ptr";
#endif

			// put in screeen pixels
			if ( !GPU_CTRL_Read_ISRGB24 )
			{
#ifdef INLINE_DEBUG_DRAW_SCREEN_2
	debug << "\r\nGPU::Draw_Screen; !ISRGB24";
#endif

				while ( current_x < current_xmax )
				{
//#ifdef INLINE_DEBUG_DRAW_SCREEN
//	debug << "\r\ndrawx1; current_x=" << current_x;
//#endif

					//Pixel16 = *ptr_vram16++;
					Pixel16 = ptr_vram16 [ current_x - LeftBorder_Width ];
					
					// the previous pixel conversion is wrong
					Pixel32_0 = ( ( Pixel16 & 0x1f ) << 3 ) | ( ( Pixel16 & 0x3e0 ) << 6 ) | ( ( Pixel16 & 0x7c00 ) << 9 );
					
					// *ptr_pixelbuffer32++ = Pixel32_0;
					ptr_pixelbuffer32 [ current_x ] = Pixel32_0;
					
					
					//current_x++;
					current_x += xinc;
				}
			}
			else
			{
				while ( current_x < current_xmax )
				{
					//Pixel24.Pixel0 = *ptr_vram16++;
					//Pixel24.Pixel1 = *ptr_vram16++;
					//Pixel24.Pixel2 = *ptr_vram16++;
					Pixel24.Pixel0 = ptr_vram16 [ ( ( ( current_x - LeftBorder_Width ) >> 1 ) * 3 ) + 0 ];
					Pixel24.Pixel1 = ptr_vram16 [ ( ( ( current_x - LeftBorder_Width ) >> 1 ) * 3 ) + 1 ];
					Pixel24.Pixel2 = ptr_vram16 [ ( ( ( current_x - LeftBorder_Width ) >> 1 ) * 3 ) + 2 ];
					
					// draw first pixel
					Pixel32_0 = ( ((u32)Pixel24.Red0) ) | ( ((u32)Pixel24.Green0) << 8 ) | ( ((u32)Pixel24.Blue0) << 16 );
					
					// draw second pixel
					Pixel32_1 = ( ((u32)Pixel24.Red1) ) | ( ((u32)Pixel24.Green1) << 8 ) | ( ((u32)Pixel24.Blue1) << 16 );
					
					// *ptr_pixelbuffer32++ = Pixel32_0;
					// *ptr_pixelbuffer32++ = Pixel32_1;
					ptr_pixelbuffer32 [ current_x ] = Pixel32_0;
					ptr_pixelbuffer32 [ current_x + 1 ] = Pixel32_1;
					
					//current_x += 2;
					current_x += ( xinc << 1 );
				}
			}
			
#ifdef INLINE_DEBUG_DRAW_SCREEN_2
	debug << "\r\nGPU::Draw_Screen; drawing right border";
	debug << " current_x=" << dec << current_x;
#endif

			// put in right border
			while ( current_x < VisibleArea_Width )
			{
				// *ptr_pixelbuffer32++ = 0;
				ptr_pixelbuffer32 [ current_x ] = 0;
				
				//current_x++;
				current_x += xinc;
			}
			
			
			//current_y++;
			current_y += yinc;
			
			// for opencl, update pixel buffer to next line
			ptr_pixelbuffer32 += ( VisibleArea_Width * yinc );
			
			if ( bEnableScanline )
			{
				// check if this is 480i
				if ( GPU_CTRL_Read_ISINTER && GPU_CTRL_Read_HEIGHT )
				{
					// 480i mode //
					
					// jump two lines in source image
					//FramePixel_Y -= 2;
					FramePixel_Y -= ( yinc << 1 );
				}
				else
				{
					// go to next line in frame buffer
					//FramePixel_Y--;
					FramePixel_Y -= yinc;
				}
				
				// also go to next line in destination buffer
				//ptr_pixelbuffer32 += VisibleArea_Width;
				ptr_pixelbuffer32 += ( VisibleArea_Width * yinc );
			}
			else
			{
				// go to next line in frame buffer
				//FramePixel_Y--;
				FramePixel_Y -= yinc;
			}
			
			
		} // end while ( current_y < current_ymax )
		
#ifdef INLINE_DEBUG_DRAW_SCREEN_2
	debug << "\r\nGPU::Draw_Screen; Drawing top border";
#endif

		// put in top border //
		

		while ( current_y < VisibleArea_Height )
		{
			//current_x = 0;
			current_x = xid;
			
			while ( current_x < VisibleArea_Width )
			{
				// *ptr_pixelbuffer32++ = 0;
				ptr_pixelbuffer32 [ current_x ] = 0;
				
				//current_x++;
				current_x += xinc;
			} // end while ( current_x < VisibleArea_Width )
				
			//current_y++;
			current_y += yinc;
				
			// for opencl, update pixel buffer to next line
			ptr_pixelbuffer32 += ( VisibleArea_Width * yinc );
			
			// check if scanline simulation is enabled
			if ( bEnableScanline )
			{
				// also go to next line in destination buffer
				//ptr_pixelbuffer32 += VisibleArea_Width;
				ptr_pixelbuffer32 += ( VisibleArea_Width * yinc );
			}
			
		} // end while ( current_y < current_ymax )
	}
	else
	{
		// display disabled //
		

		//current_y = 0;
		current_y = group_yoffset + yid;
		
		// set initial row for pixel buffer pointer
		ptr_pixelbuffer32 += ( VisibleArea_Width * current_y );
		
		if ( bEnableScanline )
		{
			// space out to every other line
			ptr_pixelbuffer32 += ( VisibleArea_Width * current_y );
			
			if ( Y_Pixel & 1 )
			{
				// odd field //
				
				ptr_pixelbuffer32 += VisibleArea_Width;
			}
		}
		
		while ( current_y < VisibleArea_Height )
		{
			//current_x = 0;
			current_x = xid;
			
			while ( current_x < VisibleArea_Width )
			{
				// *ptr_pixelbuffer32++ = 0;
				ptr_pixelbuffer32 [ current_x ] = 0;
				
				//current_x++;
				current_x += xinc;
			}
			
			//current_y++;
			current_y += yinc;
			
			// for opencl, update pixel buffer to next line
			ptr_pixelbuffer32 += ( VisibleArea_Width * yinc );
			
			if ( bEnableScanline )
			{
				//ptr_pixelbuffer32 += VisibleArea_Width;
				ptr_pixelbuffer32 += ( VisibleArea_Width * yinc );
			}
		}

	}	// end if else if ( !GPU_CTRL_Read_DEN )

	}	// end if ( _GPU->bEnable_OpenCL )
	
	
	// *** output of pixel buffer to screen *** //

	// make this the current window we are drawing to
	DisplayOutput_Window->OpenGL_MakeCurrentWindow ();

#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << "\r\nGPU::Draw_Screen; glPixelZoom";
#endif


	// check if simulating scanlines
	if ( bEnableScanline )
	{
		// the visible height is actually times 2 in the buffer for odd and even fields
		VisibleArea_Height <<= 1;
		
		// but, its actually times 2 and then minus one
		VisibleArea_Height--;
	}
	
	if ( _GPU->bEnable_OpenCL )
	{
		glBlitFramebuffer( 0, 0, VisibleArea_Width, VisibleArea_Height, 0, 0, MainProgramWindow_Width, MainProgramWindow_Height, GL_COLOR_BUFFER_BIT, GL_NEAREST );
	}
	else
	{
		glPixelZoom ( (float)MainProgramWindow_Width / (float)VisibleArea_Width, (float)MainProgramWindow_Height / (float)VisibleArea_Height );
		glDrawPixels ( VisibleArea_Width, VisibleArea_Height, GL_RGBA, GL_UNSIGNED_BYTE, (const GLvoid*) _GPU->PixelBuffer );
	}

#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << "\r\nGPU::Draw_Screen; DisplayOutput_Window->FlipScreen";
#endif
	
	// update screen
	DisplayOutput_Window->FlipScreen ();

#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << "\r\nGPU::Draw_Screen; DisplayOutput_Window->OpenGL_ReleaseWindow";
#endif
	
	// this is no longer the current window we are drawing to
	DisplayOutput_Window->OpenGL_ReleaseWindow ();
	
}


// returns number of pixels drawn (as all or nothing, no clipping)
u64 GPU::DrawTriangle_Mono_th ( DATA_Write_Format* inputdata, u32 ulThreadNum )
{
	const int local_id = 0;
	const int group_id = 0;
	const int num_local_threads = 1;
	const int num_global_groups = 1;
	
//#ifdef SINGLE_SCANLINE_MODE
	const int xid = 0;
	const int yid = 0;
	
	const int xinc = num_local_threads;
	const int yinc = num_global_groups;
	int group_yoffset = group_id;
//#endif

//inputdata format:
//0: GPU_CTRL_Read
//1: DrawArea_TopLeft
//2: DrawArea_BottomRight
//3: DrawArea_Offset
//4: (TextureWindow)(not used here)
//5: ------------
//6: ------------
//7: GetBGR24 ( Buffer [ 0 ] );
//8: GetXY0 ( Buffer [ 1 ] );
//9: GetXY1 ( Buffer [ 2 ] );
//10: GetXY2 ( Buffer [ 3 ] );



	u32 GPU_CTRL_Read, GPU_CTRL_Read_ABR;
	s32 DrawArea_BottomRightX, DrawArea_TopLeftX, DrawArea_BottomRightY, DrawArea_TopLeftY;
	s32 DrawArea_OffsetX, DrawArea_OffsetY;
	
	s32 Temp;
	s32 LeftMostX, RightMostX;
	
	
	// the y starts and ends at the same place, but the x is different for each line
	s32 StartY, EndY;
	
	
	//s64 r10, r20, r21;
	
	// new variables
	s32 x0, x1, x2, y0, y1, y2;
	s32 dx_left, dx_right;
	u32 bgr;
	s32 t0, t1, denominator;
	
	u32 Coord0, Coord1, Coord2;
	
	u32 PixelMask, SetPixelMask;
	
	s32 gx [ 3 ], gy [ 3 ], gbgr [ 3 ];
	u32 Command_ABE;
	
	s32 x_left, x_right;
	
	//s32 group_yoffset;
	
	
	s32 StartX, EndX;
	s32 x_across;
	u32 xoff, yoff;
	s32 xoff_left, xoff_right;
	u32 DestPixel;
	u32 bgr_temp;
	s32 Line;
	u16 *ptr;
	
	u32 NumPixels;
	
//debug << "\nDrawTriangle_Mono_th";

	// setup vars
	//if ( !local_id )
	//{
		// no bitmaps in opencl ??
		GPU_CTRL_Read = inputdata [ 0 ].Value;
		DrawArea_TopLeftX = inputdata [ 1 ].Value & 0x3ff;
		DrawArea_TopLeftY = ( inputdata [ 1 ].Value >> 10 ) & 0x3ff;
		DrawArea_BottomRightX = inputdata [ 2 ].Value & 0x3ff;
		DrawArea_BottomRightY = ( inputdata [ 2 ].Value >> 10 ) & 0x3ff;
		DrawArea_OffsetX = ( ( (s32) inputdata [ 3 ].Value ) << 21 ) >> 21;
		DrawArea_OffsetY = ( ( (s32) inputdata [ 3 ].Value ) << 10 ) >> 21;
		
		gx [ 0 ] = (s32) ( ( inputdata [ 8 ].x << 5 ) >> 5 );
		gy [ 0 ] = (s32) ( ( inputdata [ 8 ].y << 5 ) >> 5 );
		gx [ 1 ] = (s32) ( ( inputdata [ 11 ].x << 5 ) >> 5 );
		gy [ 1 ] = (s32) ( ( inputdata [ 11 ].y << 5 ) >> 5 );
		gx [ 2 ] = (s32) ( ( inputdata [ 14 ].x << 5 ) >> 5 );
		gy [ 2 ] = (s32) ( ( inputdata [ 14 ].y << 5 ) >> 5 );
		
		Coord0 = 0;
		Coord1 = 1;
		Coord2 = 2;
		
		// initialize number of pixels drawn
		//NumberOfPixelsDrawn = 0;
		
		
		
		///////////////////////////////////
		// put top coordinates in x0,y0
		//if ( y1 < y0 )
		if ( gy [ Coord1 ] < gy [ Coord0 ] )
		{
			//Swap ( y0, y1 );
			//Swap ( Coord0, Coord1 );
			Temp = Coord0;
			Coord0 = Coord1;
			Coord1 = Temp;
		}
		
		//if ( y2 < y0 )
		if ( gy [ Coord2 ] < gy [ Coord0 ] )
		{
			//Swap ( y0, y2 );
			//Swap ( Coord0, Coord2 );
			Temp = Coord0;
			Coord0 = Coord2;
			Coord2 = Temp;
		}
		
		///////////////////////////////////////
		// put middle coordinates in x1,y1
		//if ( y2 < y1 )
		if ( gy [ Coord2 ] < gy [ Coord1 ] )
		{
			//Swap ( y1, y2 );
			//Swap ( Coord1, Coord2 );
			Temp = Coord1;
			Coord1 = Coord2;
			Coord2 = Temp;
		}
		
		// get x-values
		x0 = gx [ Coord0 ];
		x1 = gx [ Coord1 ];
		x2 = gx [ Coord2 ];
		
		// get y-values
		y0 = gy [ Coord0 ];
		y1 = gy [ Coord1 ];
		y2 = gy [ Coord2 ];
		
		//////////////////////////////////////////
		// get coordinates on screen
		x0 += DrawArea_OffsetX;
		y0 += DrawArea_OffsetY;
		x1 += DrawArea_OffsetX;
		y1 += DrawArea_OffsetY;
		x2 += DrawArea_OffsetX;
		y2 += DrawArea_OffsetY;
		
		
		// get the left/right most x
		LeftMostX = ( ( x0 < x1 ) ? x0 : x1 );
		LeftMostX = ( ( x2 < LeftMostX ) ? x2 : LeftMostX );
		RightMostX = ( ( x0 > x1 ) ? x0 : x1 );
		RightMostX = ( ( x2 > RightMostX ) ? x2 : RightMostX );

		
		// check for some important conditions
		if ( DrawArea_BottomRightX < DrawArea_TopLeftX )
		{
			//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightX < DrawArea_TopLeftX.\n";
			return 0;
		}
		
		if ( DrawArea_BottomRightY < DrawArea_TopLeftY )
		{
			//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightY < DrawArea_TopLeftY.\n";
			return 0;
		}

		// check if sprite is within draw area
		if ( RightMostX <= ((s32)DrawArea_TopLeftX) || LeftMostX > ((s32)DrawArea_BottomRightX) || y2 <= ((s32)DrawArea_TopLeftY) || y0 > ((s32)DrawArea_BottomRightY) ) return 0;
		
		// skip drawing if distance between vertices is greater than max allowed by GPU
		if ( ( _Abs( x1 - x0 ) > c_MaxPolygonWidth ) || ( _Abs( x2 - x1 ) > c_MaxPolygonWidth ) || ( y1 - y0 > c_MaxPolygonHeight ) || ( y2 - y1 > c_MaxPolygonHeight ) )
		{
			// skip drawing polygon
			return 0;
		}
		
		
		/////////////////////////////////////////////////
		// draw top part of triangle
		
		// denominator is negative when x1 is on the left, positive when x1 is on the right
		t0 = y1 - y2;
		t1 = y0 - y2;
		denominator = ( ( x0 - x2 ) * t0 ) - ( ( x1 - x2 ) * t1 );
		
		
		NumPixels = _Abs( denominator ) >> 1;
		
		// if this is being drawn on same thread, set number of pixels, else return them
		if ( ( !ulThreadNum ) && _GPU->ulNumberOfThreads )
		{
			// return number of pixels that will be drawn
			return NumPixels;
		}
		
		if ( _GPU->bEnable_OpenCL )
		{
			return NumPixels;
		}
		
		gbgr [ 0 ] = inputdata [ 7 ].Value & 0x00ffffff;
		
		GPU_CTRL_Read_ABR = ( GPU_CTRL_Read >> 5 ) & 3;
		Command_ABE = inputdata [ 7 ].Command & 2;
		
		// ME is bit 12
		//if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
		PixelMask = ( GPU_CTRL_Read & 0x1000 ) << 3;
		
		// MD is bit 11
		//if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;
		SetPixelMask = ( GPU_CTRL_Read & 0x0800 ) << 4;
		
		// get color(s)
		bgr = gbgr [ 0 ];
		
		// ?? convert to 16-bit ?? (or should leave 24-bit?)
		//bgr = ( ( bgr & ( 0xf8 << 0 ) ) >> 3 ) | ( ( bgr & ( 0xf8 << 8 ) ) >> 6 ) | ( ( bgr & ( 0xf8 << 16 ) ) >> 9 );
		bgr = ( ( bgr >> 9 ) & 0x7c00 ) | ( ( bgr >> 6 ) & 0x3e0 ) | ( ( bgr >> 3 ) & 0x1f );
		

		// get reciprocals
		// *** todo ***
		//if ( y1 - y0 ) r10 = ( 1LL << 48 ) / ((s64)( y1 - y0 ));
		//if ( y2 - y0 ) r20 = ( 1LL << 48 ) / ((s64)( y2 - y0 ));
		//if ( y2 - y1 ) r21 = ( 1LL << 48 ) / ((s64)( y2 - y1 ));
		
		///////////////////////////////////////////
		// start at y0
		//Line = y0;
		
		
		
		//if ( denominator < 0 )
		//{
			// x1 is on the left and x0 is on the right //
			
			////////////////////////////////////
			// get slopes
			
			if ( y1 - y0 )
			{
				/////////////////////////////////////////////
				// init x on the left and right
				x_left = ( x0 << 16 );
				x_right = x_left;
				
				if ( denominator < 0 )
				{
					dx_left = ( ( x1 - x0 ) << 16 ) / ( y1 - y0 );
					dx_right = ( ( x2 - x0 ) << 16 ) / ( y2 - y0 );
					//dx_left = ( ((s64)( x1 - x0 )) * r10 ) >> 32;
					//dx_right = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
				}
				else
				{
					dx_right = ( ( x1 - x0 ) << 16 ) / ( y1 - y0 );
					dx_left = ( ( x2 - x0 ) << 16 ) / ( y2 - y0 );
					//dx_right = ( ((s64)( x1 - x0 )) * r10 ) >> 32;
					//dx_left = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
				}
			}
			else
			{
				if ( denominator < 0 )
				{
					// change x_left and x_right where y1 is on left
					x_left = ( x1 << 16 );
					x_right = ( x0 << 16 );
					
					if ( y2 - y1 )
					{
						dx_left = ( ( x2 - x1 ) << 16 ) / ( y2 - y1 );
						dx_right = ( ( x2 - x0 ) << 16 ) / ( y2 - y0 );
						//dx_left = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
						//dx_right = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
					}
				}
				else
				{
					x_right = ( x1 << 16 );
					x_left = ( x0 << 16 );
				
					if ( y2 - y1 )
					{
						dx_right = ( ( x2 - x1 ) << 16 ) / ( y2 - y1 );
						dx_left = ( ( x2 - x0 ) << 16 ) / ( y2 - y0 );
						//dx_right = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
						//dx_left = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
					}
				}
			}
		//}
		


	
		////////////////
		// *** TODO *** at this point area of full triangle can be calculated and the rest of the drawing can be put on another thread *** //
		
		//x_left += 0xffff;
		//x_right -= 1;
		
		
		StartY = y0;
		EndY = y1;

		if ( StartY < ((s32)DrawArea_TopLeftY) )
		{
			
			if ( EndY < ((s32)DrawArea_TopLeftY) )
			{
				Temp = EndY - StartY;
				StartY = EndY;
			}
			else
			{
				Temp = DrawArea_TopLeftY - StartY;
				StartY = DrawArea_TopLeftY;
			}
			
			x_left += dx_left * Temp;
			x_right += dx_right * Temp;
		}
		
		if ( EndY > ((s32)DrawArea_BottomRightY) )
		{
			EndY = DrawArea_BottomRightY + 1;
		}

		
		// offset to get to this compute unit's scanline
		//group_yoffset = group_id - ( StartY % num_global_groups );
		//if ( group_yoffset < 0 )
		//{
		//	group_yoffset += num_global_groups;
		//}

	//}	// end if ( !local_id )
	
	

	// synchronize variables across workers
	//barrier ( CLK_LOCAL_MEM_FENCE );

	
	
	
//debug << "\nStarting to draw mono triangle top. StartY=" << dec << StartY << " EndY=" << EndY;

	if ( EndY > StartY )
	{
	
	// in opencl, each worker could be on a different line
	xoff_left = x_left + ( dx_left * (group_yoffset + yid) );
	xoff_right = x_right + ( dx_right * (group_yoffset + yid) );
	
	//////////////////////////////////////////////
	// draw down to y1
	//for ( Line = StartY; Line < EndY; Line++ )
	for ( Line = StartY + group_yoffset + yid; Line < EndY; Line += yinc )
	{
		
		// left point is included if points are equal
		StartX = ( xoff_left + 0xffff ) >> 16;
		EndX = ( xoff_right - 1 ) >> 16;
		//StartX = xoff_left >> 16;
		//EndX = xoff_right >> 16;
		

		if ( StartX <= ((s32)DrawArea_BottomRightX) && EndX >= ((s32)DrawArea_TopLeftX) && EndX >= StartX )
		{
		
			if ( StartX < ((s32)DrawArea_TopLeftX) )
			{
				StartX = DrawArea_TopLeftX;
			}
			
			if ( EndX > ((s32)DrawArea_BottomRightX) )
			{
				EndX = DrawArea_BottomRightX;
			}
			
			ptr = & ( _GPU->VRAM [ StartX + xid + ( Line << 10 ) ] );
			
			
			/////////////////////////////////////////////////////
			// update number of cycles used to draw polygon
			//NumberOfPixelsDrawn += EndX - StartX + 1;
			
//debug << "\nStarting to draw mono triangle across. StartX=" << StartX << " EndX=" << EndX;

			// draw horizontal line
			// x_left and x_right need to be rounded off
			//for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize )
			for ( x_across = StartX + xid; x_across <= EndX; x_across += xinc )
			{
				// read pixel from frame buffer if we need to check mask bit
				DestPixel = *ptr;
				
				bgr_temp = bgr;
	
				// semi-transparency
				if ( Command_ABE )
				{
					bgr_temp = SemiTransparency16 ( DestPixel, bgr_temp, GPU_CTRL_Read_ABR );
				}
				
				// check if we should set mask bit when drawing
				//bgr_temp |= SetPixelMask;

				// draw pixel if we can draw to mask pixels or mask bit not set
				if ( ! ( DestPixel & PixelMask ) ) *ptr = ( bgr_temp | SetPixelMask );
				//DestPixel = ( ! ( DestPixel & PixelMask ) ) ? bgr_temp : DestPixel;
				// *ptr = DestPixel;
				
				//ptr += c_iVectorSize;
				ptr += xinc;
			}
			
		}
		
		
		/////////////////////////////////////
		// update x on left and right
		//x_left += dx_left;
		//x_right += dx_right;
		xoff_left += ( dx_left * yinc );
		xoff_right += ( dx_right * yinc );
	}

	} // end if ( EndY > StartY )

	
	// synchronize variables across workers
	//barrier ( CLK_LOCAL_MEM_FENCE );

	
	////////////////////////////////////////////////
	// draw bottom part of triangle

	/////////////////////////////////////////////
	// init x on the left and right
	
	//if ( !local_id )
	//{
	
		//////////////////////////////////////////////////////
		// check if y1 is on the left or on the right
		if ( denominator < 0 )
		{
			// y1 is on the left //
			
			x_left = ( x1 << 16 );
			
			// need to recalculate the other side when doing this in parallel with this algorithm
			x_right = ( x0 << 16 ) + ( ( y1 - y0 ) * dx_right );
			
			if ( y2 - y1 )
			{
				dx_left = ( ( x2 - x1 ) << 16 ) / ( y2 - y1 );
				//dx_left = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
			}
		}
		else
		{
			// y1 is on the right //
			
			x_right = ( x1 << 16 );
			
			// need to recalculate the other side when doing this in parallel with this algorithm
			x_left = ( x0 << 16 ) + ( ( y1 - y0 ) * dx_left );
			
			if ( y2 - y1 )
			{
				dx_right = ( ( x2 - x1 ) << 16 ) / ( y2 - y1 );
				//dx_right = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
			}
		}
		
		
		//x_left += 0xffff;
		//x_right -= 1;
		
		// the line starts at y1 from here
		//Line = y1;

		StartY = y1;
		EndY = y2;

		if ( StartY < ((s32)DrawArea_TopLeftY) )
		{
			
			if ( EndY < ((s32)DrawArea_TopLeftY) )
			{
				Temp = EndY - StartY;
				StartY = EndY;
			}
			else
			{
				Temp = DrawArea_TopLeftY - StartY;
				StartY = DrawArea_TopLeftY;
			}
			
			x_left += dx_left * Temp;
			x_right += dx_right * Temp;
		}
		
		if ( EndY > ((s32)DrawArea_BottomRightY) )
		{
			EndY = DrawArea_BottomRightY + 1;
		}
		
		
		// offset to get to this compute unit's scanline
		//group_yoffset = group_id - ( StartY % num_global_groups );
		//if ( group_yoffset < 0 )
		//{
		//	group_yoffset += num_global_groups;
		//}
	//}

	
	// synchronize variables across workers
	//barrier ( CLK_LOCAL_MEM_FENCE );

//cout << "\nStarting to draw mono triangle bottom";
	
	if ( EndY > StartY )
	{
	
	// in opencl, each worker could be on a different line
	xoff_left = x_left + ( dx_left * (group_yoffset + yid) );
	xoff_right = x_right + ( dx_right * (group_yoffset + yid) );
	
	//////////////////////////////////////////////
	// draw down to y2
	for ( Line = StartY + group_yoffset + yid; Line < EndY; Line += yinc )
	{
		
		// left point is included if points are equal
		StartX = ( xoff_left + 0xffff ) >> 16;
		EndX = ( xoff_right - 1 ) >> 16;
		//StartX = xoff_left >> 16;
		//EndX = xoff_right >> 16;
		
		
		if ( StartX <= ((s32)DrawArea_BottomRightX) && EndX >= ((s32)DrawArea_TopLeftX) && EndX >= StartX )
		{
			if ( StartX < ((s32)DrawArea_TopLeftX) )
			{
				StartX = DrawArea_TopLeftX;
			}
			
			if ( EndX > ((s32)DrawArea_BottomRightX) )
			{
				EndX = DrawArea_BottomRightX;
			}
			
			ptr = & ( _GPU->VRAM [ StartX + xid + ( Line << 10 ) ] );
			
			
			/////////////////////////////////////////////////////
			// update number of cycles used to draw polygon
			//NumberOfPixelsDrawn += EndX - StartX + 1;
			

			// draw horizontal line
			// x_left and x_right need to be rounded off
			//for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize )
			for ( x_across = StartX + xid; x_across <= EndX; x_across += xinc )
			{
				// read pixel from frame buffer if we need to check mask bit
				DestPixel = *ptr;
				
				bgr_temp = bgr;
	
				// semi-transparency
				if ( Command_ABE )
				{
					bgr_temp = SemiTransparency16 ( DestPixel, bgr_temp, GPU_CTRL_Read_ABR );
				}
				
				// check if we should set mask bit when drawing
				//bgr_temp |= SetPixelMask;

				// draw pixel if we can draw to mask pixels or mask bit not set
				if ( ! ( DestPixel & PixelMask ) ) *ptr = ( bgr_temp | SetPixelMask );
				//DestPixel = ( ! ( DestPixel & PixelMask ) ) ? bgr_temp : DestPixel;
				// *ptr = DestPixel;
				
				//ptr += c_iVectorSize;
				ptr += xinc;
			}
			
		}
		
		/////////////////////////////////////
		// update x on left and right
		//x_left += dx_left;
		//x_right += dx_right;
		xoff_left += ( dx_left * yinc );
		xoff_right += ( dx_right * yinc );
	}
	
	} // end if ( EndY > StartY )

	// return the number of pixels drawn
	return NumPixels;
}



u64 GPU::DrawTriangle_Gradient_th ( DATA_Write_Format* inputdata, u32 ulThreadNum )
{
	const int local_id = 0;
	const int group_id = 0;
	const int num_local_threads = 1;
	const int num_global_groups = 1;
	
//#ifdef SINGLE_SCANLINE_MODE
	const int xid = 0;
	const int yid = 0;
	
	const int xinc = num_local_threads;
	const int yinc = num_global_groups;
	int group_yoffset = 0;
//#endif


//inputdata format:
//0: GPU_CTRL_Read
//1: DrawArea_TopLeft
//2: DrawArea_BottomRight
//3: DrawArea_Offset
//4: (TextureWindow)(not used here)
//5: ------------
//6: ------------
//7: GetBGR0_8 ( Buffer [ 0 ] );
//8: GetXY0 ( Buffer [ 1 ] );
//9: GetBGR1_8 ( Buffer [ 2 ] );
//10: GetXY1 ( Buffer [ 3 ] );
//11: GetBGR2_8 ( Buffer [ 4 ] );
//12: GetXY2 ( Buffer [ 5 ] );

	
	u32 GPU_CTRL_Read, GPU_CTRL_Read_ABR;
	s32 DrawArea_BottomRightX, DrawArea_TopLeftX, DrawArea_BottomRightY, DrawArea_TopLeftY;
	s32 DrawArea_OffsetX, DrawArea_OffsetY;
	u32 Command_ABE;
	u32 GPU_CTRL_Read_DTD;
	
	u16 *ptr;
	
	s32 Temp;
	s32 LeftMostX, RightMostX;
	
	s32 StartX, EndX;
	s32 StartY, EndY;

	//s64 r10, r20, r21;
	
	//s32* DitherArray;
	//s32* DitherLine;
	s32 DitherValue;

	// new variables
	s32 x0, x1, x2, y0, y1, y2;
	s32 dx_left, dx_right;
	s32 x_left, x_right;
	s32 x_across;
	u32 bgr, bgr_temp;
	s32 Line;
	s32 t0, t1, denominator;

	// more variables for gradient triangle
	s32 dR_left, dG_left, dB_left;
	s32 dR_across, dG_across, dB_across;
	s32 iR, iG, iB;
	s32 R_left, G_left, B_left;
	s32 Roff_left, Goff_left, Boff_left;
	s32 r0, r1, r2, g0, g1, g2, b0, b1, b2;
	//s32 gr [ 3 ], gg [ 3 ], gb [ 3 ];

	s32 gx [ 3 ], gy [ 3 ], gbgr [ 3 ];
	
	s32 xoff_left, xoff_right;
	
	s32 Red, Green, Blue;
	u32 DestPixel;
	u32 PixelMask, SetPixelMask;

	u32 Coord0, Coord1, Coord2;
	//s32 group_yoffset;
	
	u32 NumPixels;

	// setup vars
	//if ( !local_id )
	//{
		
		// no bitmaps in opencl ??
		GPU_CTRL_Read = inputdata [ 0 ].Value;
		DrawArea_TopLeftX = inputdata [ 1 ].Value & 0x3ff;
		DrawArea_TopLeftY = ( inputdata [ 1 ].Value >> 10 ) & 0x3ff;
		DrawArea_BottomRightX = inputdata [ 2 ].Value & 0x3ff;
		DrawArea_BottomRightY = ( inputdata [ 2 ].Value >> 10 ) & 0x3ff;
		DrawArea_OffsetX = ( ( (s32) inputdata [ 3 ].Value ) << 21 ) >> 21;
		DrawArea_OffsetY = ( ( (s32) inputdata [ 3 ].Value ) << 10 ) >> 21;
		
		gx [ 0 ] = (s32) ( ( inputdata [ 8 ].x << 5 ) >> 5 );
		gy [ 0 ] = (s32) ( ( inputdata [ 8 ].y << 5 ) >> 5 );
		gx [ 1 ] = (s32) ( ( inputdata [ 11 ].x << 5 ) >> 5 );
		gy [ 1 ] = (s32) ( ( inputdata [ 11 ].y << 5 ) >> 5 );
		gx [ 2 ] = (s32) ( ( inputdata [ 14 ].x << 5 ) >> 5 );
		gy [ 2 ] = (s32) ( ( inputdata [ 14 ].y << 5 ) >> 5 );

		
		Coord0 = 0;
		Coord1 = 1;
		Coord2 = 2;
		
		///////////////////////////////////
		// put top coordinates in x0,y0
		//if ( y1 < y0 )
		if ( gy [ Coord1 ] < gy [ Coord0 ] )
		{
			//Swap ( y0, y1 );
			//Swap ( Coord0, Coord1 );
			Temp = Coord0;
			Coord0 = Coord1;
			Coord1 = Temp;
		}
		
		//if ( y2 < y0 )
		if ( gy [ Coord2 ] < gy [ Coord0 ] )
		{
			//Swap ( y0, y2 );
			//Swap ( Coord0, Coord2 );
			Temp = Coord0;
			Coord0 = Coord2;
			Coord2 = Temp;
		}
		
		///////////////////////////////////////
		// put middle coordinates in x1,y1
		//if ( y2 < y1 )
		if ( gy [ Coord2 ] < gy [ Coord1 ] )
		{
			//Swap ( y1, y2 );
			//Swap ( Coord1, Coord2 );
			Temp = Coord1;
			Coord1 = Coord2;
			Coord2 = Temp;
		}
		
		// get x-values
		x0 = gx [ Coord0 ];
		x1 = gx [ Coord1 ];
		x2 = gx [ Coord2 ];
		
		// get y-values
		y0 = gy [ Coord0 ];
		y1 = gy [ Coord1 ];
		y2 = gy [ Coord2 ];
		
		//////////////////////////////////////////
		// get coordinates on screen
		x0 += DrawArea_OffsetX;
		y0 += DrawArea_OffsetY;
		x1 += DrawArea_OffsetX;
		y1 += DrawArea_OffsetY;
		x2 += DrawArea_OffsetX;
		y2 += DrawArea_OffsetY;
		
		// get the left/right most x
		LeftMostX = ( ( x0 < x1 ) ? x0 : x1 );
		LeftMostX = ( ( x2 < LeftMostX ) ? x2 : LeftMostX );
		RightMostX = ( ( x0 > x1 ) ? x0 : x1 );
		RightMostX = ( ( x2 > RightMostX ) ? x2 : RightMostX );

		// check for some important conditions
		if ( DrawArea_BottomRightX < DrawArea_TopLeftX )
		{
			//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightX < DrawArea_TopLeftX.\n";
			return 0;
		}
		
		if ( DrawArea_BottomRightY < DrawArea_TopLeftY )
		{
			//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightY < DrawArea_TopLeftY.\n";
			return 0;
		}

		// check if sprite is within draw area
		if ( RightMostX <= ((s32)DrawArea_TopLeftX) || LeftMostX > ((s32)DrawArea_BottomRightX) || y2 <= ((s32)DrawArea_TopLeftY) || y0 > ((s32)DrawArea_BottomRightY) ) return 0;
		
		// skip drawing if distance between vertices is greater than max allowed by GPU
		if ( ( _Abs( x1 - x0 ) > c_MaxPolygonWidth ) || ( _Abs( x2 - x1 ) > c_MaxPolygonWidth ) || ( y1 - y0 > c_MaxPolygonHeight ) || ( y2 - y1 > c_MaxPolygonHeight ) )
		{
			// skip drawing polygon
			return 0;
		}
		
		
		
		/////////////////////////////////////////////////
		// draw top part of triangle
		
		// denominator is negative when x1 is on the left, positive when x1 is on the right
		t0 = y1 - y2;
		t1 = y0 - y2;
		denominator = ( ( x0 - x2 ) * t0 ) - ( ( x1 - x2 ) * t1 );

		NumPixels = _Abs ( denominator ) >> 1;
		if ( ( !ulThreadNum ) && _GPU->ulNumberOfThreads )
		{
			return NumPixels;
		}
		
		if ( _GPU->bEnable_OpenCL )
		{
			return NumPixels;
		}

		gbgr [ 0 ] = inputdata [ 7 ].Value & 0x00ffffff;
		gbgr [ 1 ] = inputdata [ 10 ].Value & 0x00ffffff;
		gbgr [ 2 ] = inputdata [ 13 ].Value & 0x00ffffff;
			
		GPU_CTRL_Read_ABR = ( GPU_CTRL_Read >> 5 ) & 3;
		Command_ABE = inputdata [ 7 ].Command & 2;
		
		// DTD is bit 9 in GPU_CTRL_Read
		GPU_CTRL_Read_DTD = ( GPU_CTRL_Read >> 9 ) & 1;
		
		// ME is bit 12
		//if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
		PixelMask = ( GPU_CTRL_Read & 0x1000 ) << 3;
		
		// MD is bit 11
		//if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;
		SetPixelMask = ( GPU_CTRL_Read & 0x0800 ) << 4;
		
		
		// initialize number of pixels drawn
		//NumberOfPixelsDrawn = 0;
		
		
		
		

		// get rgb-values
		//r0 = gr [ Coord0 ];
		//r1 = gr [ Coord1 ];
		//r2 = gr [ Coord2 ];
		//g0 = gg [ Coord0 ];
		//g1 = gg [ Coord1 ];
		///g2 = gg [ Coord2 ];
		//b0 = gb [ Coord0 ];
		//b1 = gb [ Coord1 ];
		//b2 = gb [ Coord2 ];
		r0 = gbgr [ Coord0 ] & 0xff;
		r1 = gbgr [ Coord1 ] & 0xff;
		r2 = gbgr [ Coord2 ] & 0xff;
		g0 = ( gbgr [ Coord0 ] >> 8 ) & 0xff;
		g1 = ( gbgr [ Coord1 ] >> 8 ) & 0xff;
		g2 = ( gbgr [ Coord2 ] >> 8 ) & 0xff;
		b0 = ( gbgr [ Coord0 ] >> 16 ) & 0xff;
		b1 = ( gbgr [ Coord1 ] >> 16 ) & 0xff;
		b2 = ( gbgr [ Coord2 ] >> 16 ) & 0xff;


		
		if ( denominator )
		{
			//dR_across = ( ( (s32) ( ( ( r0 - r2 ) * t0 ) - ( ( r1 - r2 ) * t1 ) ) ) << 6 ) / denominator;
			//dG_across = ( ( (s32) ( ( ( g0 - g2 ) * t0 ) - ( ( g1 - g2 ) * t1 ) ) ) << 6 ) / denominator;
			//dB_across = ( ( (s32) ( ( ( b0 - b2 ) * t0 ) - ( ( b1 - b2 ) * t1 ) ) ) << 6 ) / denominator;
			//dR_across <<= 10;
			//dG_across <<= 10;
			//dB_across <<= 10;
			
			
			dR_across = ( ( (s32) ( ( ( r0 - r2 ) * t0 ) - ( ( r1 - r2 ) * t1 ) ) ) << 8 ) / denominator;
			dG_across = ( ( (s32) ( ( ( g0 - g2 ) * t0 ) - ( ( g1 - g2 ) * t1 ) ) ) << 8 ) / denominator;
			dB_across = ( ( (s32) ( ( ( b0 - b2 ) * t0 ) - ( ( b1 - b2 ) * t1 ) ) ) << 8 ) / denominator;
			dR_across <<= 8;
			dG_across <<= 8;
			dB_across <<= 8;
			
			//printf ( "dR_across=%x top=%i bottom=%i divide=%x", dR_across, ( ( ( r0 - r2 ) * t0 ) - ( ( r1 - r2 ) * t1 ) ), denominator, ( ( ( ( r0 - r2 ) * t0 ) - ( ( r1 - r2 ) * t1 ) ) << 16 )/denominator );
			//printf ( "dG_across=%x top=%i bottom=%i divide=%x", dG_across, ( ( ( g0 - g2 ) * t0 ) - ( ( g1 - g2 ) * t1 ) ), denominator, ( ( ( ( g0 - g2 ) * t0 ) - ( ( g1 - g2 ) * t1 ) ) << 16 )/denominator );
		}
		
		
		
		
		//if ( denominator < 0 )
		//{
			// x1 is on the left and x0 is on the right //
			
			////////////////////////////////////
			// get slopes
			
			if ( y1 - y0 )
			{
				/////////////////////////////////////////////
				// init x on the left and right
				x_left = ( x0 << 16 );
				x_right = x_left;
				
				R_left = ( r0 << 16 );
				G_left = ( g0 << 16 );
				B_left = ( b0 << 16 );
				
				if ( denominator < 0 )
				{
					//dx_left = ( ((s64)( x1 - x0 )) * r10 ) >> 32;
					//dx_right = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
					dx_left = ( ( x1 - x0 ) << 16 ) / ( y1 - y0 );
					dx_right = ( ( x2 - x0 ) << 16 ) / ( y2 - y0 );
					
					
					//dx_left = divide_s32( ( ( x1 - x0 ) << 16 ), ( y1 - y0 ) );
					//dx_right = divide_s32( ( ( x2 - x0 ) << 16 ), ( y2 - y0 ) );
					
					dR_left = (( r1 - r0 ) << 16 ) / ( y1 - y0 );
					dG_left = (( g1 - g0 ) << 16 ) / ( y1 - y0 );
					dB_left = (( b1 - b0 ) << 16 ) / ( y1 - y0 );
					
					
					//dR_left = divide_s32( (( r1 - r0 ) << 16 ), ( y1 - y0 ) );
					//dG_left = divide_s32( (( g1 - g0 ) << 16 ), ( y1 - y0 ) );
					//dB_left = divide_s32( (( b1 - b0 ) << 16 ), ( y1 - y0 ) );
				}
				else
				{
					//dx_right = ( ((s64)( x1 - x0 )) * r10 ) >> 32;
					//dx_left = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
					dx_right = ( ( x1 - x0 ) << 16 ) / ( y1 - y0 );
					
					if ( y2 - y0 )
					{
						dx_left = ( ( x2 - x0 ) << 16 ) / ( y2 - y0 );
						
						
						//dx_right = divide_s32( ( ( x1 - x0 ) << 16 ), ( y1 - y0 ) );
						//dx_left = divide_s32( ( ( x2 - x0 ) << 16 ), ( y2 - y0 ) );
						
						dR_left = (( r2 - r0 ) << 16 ) / ( y2 - y0 );
						dG_left = (( g2 - g0 ) << 16 ) / ( y2 - y0 );
						dB_left = (( b2 - b0 ) << 16 ) / ( y2 - y0 );
						
						
						//dR_left = divide_s32( (( r2 - r0 ) << 16 ), ( y2 - y0 ) );
						//dG_left = divide_s32( (( g2 - g0 ) << 16 ), ( y2 - y0 ) );
						//dB_left = divide_s32( (( b2 - b0 ) << 16 ), ( y2 - y0 ) );
					}
				}
			}
			else
			{
				if ( denominator < 0 )
				{
					// change x_left and x_right where y1 is on left
					x_left = ( x1 << 16 );
					x_right = ( x0 << 16 );
					
					R_left = ( r1 << 16 );
					G_left = ( g1 << 16 );
					B_left = ( b1 << 16 );
					
					if ( y2 - y1 )
					{
						//dx_left = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
						//dx_right = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
						dx_left = ( ( x2 - x1 ) << 16 ) / ( y2 - y1 );
						dx_right = ( ( x2 - x0 ) << 16 ) / ( y2 - y0 );
						
						
						//dx_left = divide_s32( ( ( x2 - x1 ) << 16 ), ( y2 - y1 ) );
						//dx_right = divide_s32( ( ( x2 - x0 ) << 16 ), ( y2 - y0 ) );
						

						dR_left = (( r2 - r1 ) << 16 ) / ( y2 - y1 );
						dG_left = (( g2 - g1 ) << 16 ) / ( y2 - y1 );
						dB_left = (( b2 - b1 ) << 16 ) / ( y2 - y1 );
						
						
						//dR_left = divide_s32( (( r2 - r1 ) << 16 ), ( y2 - y1 ) );
						//dG_left = divide_s32( (( g2 - g1 ) << 16 ), ( y2 - y1 ) );
						//dB_left = divide_s32( (( b2 - b1 ) << 16 ), ( y2 - y1 ) );
					}
				}
				else
				{
					x_right = ( x1 << 16 );
					x_left = ( x0 << 16 );
				
					R_left = ( r0 << 16 );
					G_left = ( g0 << 16 );
					B_left = ( b0 << 16 );
					
					if ( y2 - y1 )
					{
						//dx_right = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
						//dx_left = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
						dx_right = ( ( x2 - x1 ) << 16 ) / ( y2 - y1 );
						dx_left = ( ( x2 - x0 ) << 16 ) / ( y2 - y0 );
						
						
						//dx_right = divide_s32( ( ( x2 - x1 ) << 16 ), ( y2 - y1 ) );
						//dx_left = divide_s32( ( ( x2 - x0 ) << 16 ), ( y2 - y0 ) );
						
						
						dR_left = (( r2 - r0 ) << 16 ) / ( y2 - y0 );
						dG_left = (( g2 - g0 ) << 16 ) / ( y2 - y0 );
						dB_left = (( b2 - b0 ) << 16 ) / ( y2 - y0 );
						
						
						//dR_left = divide_s32( (( r2 - r0 ) << 16 ), ( y2 - y0 ) );
						//dG_left = divide_s32( (( g2 - g0 ) << 16 ), ( y2 - y0 ) );
						//dB_left = divide_s32( (( b2 - b0 ) << 16 ), ( y2 - y0 ) );
					}
				}
			}
		//}
		


	
		////////////////
		// *** TODO *** at this point area of full triangle can be calculated and the rest of the drawing can be put on another thread *** //
		
		
		
		// r,g,b values are not specified with a fractional part, so there must be an initial fractional part
		R_left |= ( 1 << 15 );
		G_left |= ( 1 << 15 );
		B_left |= ( 1 << 15 );
		
		//x_left += 0xffff;
		//x_right -= 1;
		
		StartY = y0;
		EndY = y1;

		if ( StartY < ((s32)DrawArea_TopLeftY) )
		{
			
			if ( EndY < ((s32)DrawArea_TopLeftY) )
			{
				Temp = EndY - StartY;
				StartY = EndY;
			}
			else
			{
				Temp = DrawArea_TopLeftY - StartY;
				StartY = DrawArea_TopLeftY;
			}
			
			x_left += dx_left * Temp;
			x_right += dx_right * Temp;
			
			R_left += dR_left * Temp;
			G_left += dG_left * Temp;
			B_left += dB_left * Temp;
		}
		
		if ( EndY > ((s32)DrawArea_BottomRightY) )
		{
			EndY = DrawArea_BottomRightY + 1;
		}

		
		// offset to get to this compute unit's scanline
		//group_yoffset = group_id - ( StartY % num_global_groups );
		//if ( group_yoffset < 0 )
		//{
		//	group_yoffset += num_global_groups;
		//}
		
		//printf( "x_left=%x x_right=%x dx_left=%i dx_right=%i R_left=%x G_left=%x B_left=%x OffsetX=%i OffsetY=%i",x_left,x_right,dx_left,dx_right,R_left,G_left,B_left, DrawArea_OffsetX, DrawArea_OffsetY );
		//printf( "x0=%i y0=%i x1=%i y1=%i x2=%i y2=%i r0=%i r1=%i r2=%i g0=%i g1=%i g2=%i b0=%i b1=%i b2=%i", x0, y0, x1, y1, x2, y2, r0, r1, r2, g0, g1, g2, b0, b1, b2 );
		//printf( "dR_across=%x dG_across=%x dB_across=%x", dR_across, dG_across, dB_across );

	//}	// end if ( !local_id )
	
	

	// synchronize variables across workers
	//barrier ( CLK_LOCAL_MEM_FENCE );

	


	
	



	
	/////////////////////////////////////////////
	// init x on the left and right
	
	


	if ( EndY > StartY )
	{
	
	// in opencl, each worker could be on a different line
	xoff_left = x_left + ( dx_left * (group_yoffset + yid) );
	xoff_right = x_right + ( dx_right * (group_yoffset + yid) );
	
	Roff_left = R_left + ( dR_left * (group_yoffset + yid) );
	Goff_left = G_left + ( dG_left * (group_yoffset + yid) );
	Boff_left = B_left + ( dB_left * (group_yoffset + yid) );
	
	//////////////////////////////////////////////
	// draw down to y1
	//for ( Line = StartY; Line < EndY; Line++ )
	for ( Line = StartY + group_yoffset + yid; Line < EndY; Line += yinc )
	{
		
		// left point is included if points are equal
		StartX = ( xoff_left + 0xffff ) >> 16;
		EndX = ( xoff_right - 1 ) >> 16;
		//StartX = xoff_left >> 16;
		//EndX = xoff_right >> 16;
		
		
		if ( StartX <= ((s32)DrawArea_BottomRightX) && EndX >= ((s32)DrawArea_TopLeftX) && EndX >= StartX )
		{
			
			iR = Roff_left;
			iG = Goff_left;
			iB = Boff_left;
			
			// get the difference between x_left and StartX
			Temp = ( StartX << 16 ) - xoff_left;
			
			if ( StartX < ((s32)DrawArea_TopLeftX) )
			{
				Temp += ( DrawArea_TopLeftX - StartX ) << 16;
				StartX = DrawArea_TopLeftX;
				
				//iR += dR_across * Temp;
				//iG += dG_across * Temp;
				//iB += dB_across * Temp;
			}
			
			iR += ( dR_across >> 8 ) * ( Temp >> 8 );
			iG += ( dG_across >> 8 ) * ( Temp >> 8 );
			iB += ( dB_across >> 8 ) * ( Temp >> 8 );
			
			if ( EndX > ((s32)DrawArea_BottomRightX) )
			{
				//EndX = DrawArea_BottomRightX + 1;
				EndX = DrawArea_BottomRightX;
			}
			
			ptr = & ( _GPU->VRAM [ StartX + xid + ( Line << 10 ) ] );
			//DitherLine = & ( DitherArray [ ( Line & 0x3 ) << 2 ] );
			
			
			iR += ( dR_across * xid );
			iG += ( dG_across * xid );
			iB += ( dB_across * xid );

			// draw horizontal line
			// x_left and x_right need to be rounded off
			//for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize )
			for ( x_across = StartX + xid; x_across <= EndX; x_across += xinc )
			{
				if ( GPU_CTRL_Read_DTD )
				{
					//bgr = ( _Round( iR ) >> 32 ) | ( ( _Round( iG ) >> 32 ) << 8 ) | ( ( _Round( iB ) >> 32 ) << 16 );
					//bgr = ( _Round( iR ) >> 35 ) | ( ( _Round( iG ) >> 35 ) << 5 ) | ( ( _Round( iB ) >> 35 ) << 10 );
					//DitherValue = DitherLine [ x_across & 0x3 ];
					DitherValue = c_iDitherValues16 [ ( x_across & 3 ) + ( ( Line & 3 ) << 2 ) ];
					
					// perform dither
					//Red = iR + DitherValue;
					//Green = iG + DitherValue;
					//Blue = iB + DitherValue;
					Red = iR + DitherValue;
					Green = iG + DitherValue;
					Blue = iB + DitherValue;
					
					//Red = Clamp5 ( ( iR + DitherValue ) >> 27 );
					//Green = Clamp5 ( ( iG + DitherValue ) >> 27 );
					//Blue = Clamp5 ( ( iB + DitherValue ) >> 27 );
					
					// perform shift
					Red >>= ( 16 + 3 );
					Green >>= ( 16 + 3 );
					Blue >>= ( 16 + 3 );
					
					//Red = clamp ( Red, 0, 0x1f );
					//Green = clamp ( Green, 0, 0x1f );
					//Blue = clamp ( Blue, 0, 0x1f );
					Red = Clamp5 ( Red );
					Green = Clamp5 ( Green );
					Blue = Clamp5 ( Blue );
				}
				else
				{
					Red = iR >> ( 16 + 3 );
					Green = iG >> ( 16 + 3 );
					Blue = iB >> ( 16 + 3 );
				}
					
				
					
					// if dithering, perform signed clamp to 5 bits
					//Red = AddSignedClamp<s64,5> ( Red );
					//Green = AddSignedClamp<s64,5> ( Green );
					//Blue = AddSignedClamp<s64,5> ( Blue );
					
					bgr_temp = ( Blue << 10 ) | ( Green << 5 ) | Red;
					
					// shade pixel color
				
					// read pixel from frame buffer if we need to check mask bit
					DestPixel = *ptr;
					
					
					//bgr_temp = bgr;
		
					
					// semi-transparency
					if ( Command_ABE )
					{
						bgr_temp = SemiTransparency16 ( DestPixel, bgr_temp, GPU_CTRL_Read_ABR );
					}
					
					// check if we should set mask bit when drawing
					bgr_temp |= SetPixelMask;

					
					// draw pixel if we can draw to mask pixels or mask bit not set
					if ( ! ( DestPixel & PixelMask ) ) *ptr = bgr_temp;
						
					iR += ( dR_across * xinc );
					iG += ( dG_across * xinc );
					iB += ( dB_across * xinc );
				
				//ptr += c_iVectorSize;
				ptr += xinc;
			}
			
		}
		
		
		/////////////////////////////////////
		// update x on left and right
		xoff_left += ( dx_left * yinc );
		xoff_right += ( dx_right * yinc );
		
		Roff_left += ( dR_left * yinc );
		Goff_left += ( dG_left * yinc );
		Boff_left += ( dB_left * yinc );
	}

	} // end if ( EndY > StartY )

	
	// synchronize variables across workers
	//barrier ( CLK_LOCAL_MEM_FENCE );

	
	////////////////////////////////////////////////
	// draw bottom part of triangle

	/////////////////////////////////////////////
	// init x on the left and right
	
	//if ( !local_id )
	//{
		//////////////////////////////////////////////////////
		// check if y1 is on the left or on the right
		if ( denominator < 0 )
		{
			x_left = ( x1 << 16 );

			x_right = ( x0 << 16 ) + ( dx_right * ( y1 - y0 ) );
			
			R_left = ( r1 << 16 );
			G_left = ( g1 << 16 );
			B_left = ( b1 << 16 );
			
			
			if ( y2 - y1 )
			{
				//dx_left = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
				dx_left = (( x2 - x1 ) << 16 ) / ( y2 - y1 );
			
			
				//dx_left = divide_s32( (( x2 - x1 ) << 16 ), ( y2 - y1 ) );
				
				//dR_left = ( ((s64)( r2 - r1 )) * r21 ) >> 24;
				//dG_left = ( ((s64)( g2 - g1 )) * r21 ) >> 24;
				//dB_left = ( ((s64)( b2 - b1 )) * r21 ) >> 24;
				dR_left = (( r2 - r1 ) << 16 ) / ( y2 - y1 );
				dG_left = (( g2 - g1 ) << 16 ) / ( y2 - y1 );
				dB_left = (( b2 - b1 ) << 16 ) / ( y2 - y1 );
				
				
				//dR_left = divide_s32( (( r2 - r1 ) << 16 ), ( y2 - y1 ) );
				//dG_left = divide_s32( (( g2 - g1 ) << 16 ), ( y2 - y1 ) );
				//dB_left = divide_s32( (( b2 - b1 ) << 16 ), ( y2 - y1 ) );
			}
		}
		else
		{
			x_right = ( x1 << 16 );

			x_left = ( x0 << 16 ) + ( dx_left * ( y1 - y0 ) );
			
			R_left = ( r0 << 16 ) + ( dR_left * ( y1 - y0 ) );
			G_left = ( g0 << 16 ) + ( dG_left * ( y1 - y0 ) );
			B_left = ( b0 << 16 ) + ( dB_left * ( y1 - y0 ) );
			
			if ( y2 - y1 )
			{
				//dx_right = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
				dx_right = (( x2 - x1 ) << 16 ) / ( y2 - y1 );
			
			
				//dx_right = divide_s32( (( x2 - x1 ) << 16 ), ( y2 - y1 ) );
				
			}
		}


		R_left += ( 1 << 15 );
		G_left += ( 1 << 15 );
		B_left += ( 1 << 15 );

		//x_left += 0xffff;
		//x_right -= 1;
		

		StartY = y1;
		EndY = y2;

		if ( StartY < ((s32)DrawArea_TopLeftY) )
		{
			
			if ( EndY < ((s32)DrawArea_TopLeftY) )
			{
				Temp = EndY - StartY;
				StartY = EndY;
			}
			else
			{
				Temp = DrawArea_TopLeftY - StartY;
				StartY = DrawArea_TopLeftY;
			}
			
			x_left += dx_left * Temp;
			x_right += dx_right * Temp;
			
			R_left += dR_left * Temp;
			G_left += dG_left * Temp;
			B_left += dB_left * Temp;
			
		}
		
		if ( EndY > ((s32)DrawArea_BottomRightY) )
		{
			EndY = DrawArea_BottomRightY + 1;
		}
		
		// offset to get to this compute unit's scanline
		//group_yoffset = group_id - ( StartY % num_global_groups );
		//if ( group_yoffset < 0 )
		//{
		//	group_yoffset += num_global_groups;
		//}
	//}

	
	// synchronize variables across workers
	//barrier ( CLK_LOCAL_MEM_FENCE );

	
	if ( EndY > StartY )
	{
	
	// in opencl, each worker could be on a different line
	xoff_left = x_left + ( dx_left * (group_yoffset + yid) );
	xoff_right = x_right + ( dx_right * (group_yoffset + yid) );
	
	Roff_left = R_left + ( dR_left * (group_yoffset + yid) );
	Goff_left = G_left + ( dG_left * (group_yoffset + yid) );
	Boff_left = B_left + ( dB_left * (group_yoffset + yid) );
	
	//////////////////////////////////////////////
	// draw down to y2
	//for ( Line = StartY; Line < EndY; Line++ )
	for ( Line = StartY + group_yoffset + yid; Line < EndY; Line += yinc )
	{
		
		// left point is included if points are equal
		StartX = ( xoff_left + 0xffff ) >> 16;
		EndX = ( xoff_right - 1 ) >> 16;
		//StartX = xoff_left >> 16;
		//EndX = xoff_right >> 16;
		
		
		if ( StartX <= ((s32)DrawArea_BottomRightX) && EndX >= ((s32)DrawArea_TopLeftX) && EndX >= StartX )
		{
			iR = Roff_left;
			iG = Goff_left;
			iB = Boff_left;
			
			
			// get the difference between x_left and StartX
			Temp = ( StartX << 16 ) - xoff_left;
			
			if ( StartX < ((s32)DrawArea_TopLeftX) )
			{
				Temp += ( DrawArea_TopLeftX - StartX ) << 16;
				StartX = DrawArea_TopLeftX;
				
				//iR += dR_across * Temp;
				//iG += dG_across * Temp;
				//iB += dB_across * Temp;
			}
			
			iR += ( dR_across >> 8 ) * ( Temp >> 8 );
			iG += ( dG_across >> 8 ) * ( Temp >> 8 );
			iB += ( dB_across >> 8 ) * ( Temp >> 8 );
			
			if ( EndX > ((s32)DrawArea_BottomRightX) )
			{
				//EndX = DrawArea_BottomRightX + 1;
				EndX = DrawArea_BottomRightX;
			}
			
			ptr = & ( _GPU->VRAM [ StartX + xid + ( Line << 10 ) ] );
			//DitherLine = & ( DitherArray [ ( Line & 0x3 ) << 2 ] );
			
			
			/////////////////////////////////////////////////////
			// update number of cycles used to draw polygon
			//NumberOfPixelsDrawn += EndX - StartX + 1;
			
			iR += ( dR_across * xid );
			iG += ( dG_across * xid );
			iB += ( dB_across * xid );

			// draw horizontal line
			// x_left and x_right need to be rounded off
			//for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize )
			for ( x_across = StartX + xid; x_across <= EndX; x_across += xinc )
			{
				if ( GPU_CTRL_Read_DTD )
				{
					//bgr = ( _Round( iR ) >> 32 ) | ( ( _Round( iG ) >> 32 ) << 8 ) | ( ( _Round( iB ) >> 32 ) << 16 );
					//bgr = ( _Round( iR ) >> 35 ) | ( ( _Round( iG ) >> 35 ) << 5 ) | ( ( _Round( iB ) >> 35 ) << 10 );
					//DitherValue = DitherLine [ x_across & 0x3 ];
					DitherValue = c_iDitherValues16 [ ( x_across & 3 ) + ( ( Line & 3 ) << 2 ) ];
					
					// perform dither
					//Red = iR + DitherValue;
					//Green = iG + DitherValue;
					//Blue = iB + DitherValue;
					Red = iR + DitherValue;
					Green = iG + DitherValue;
					Blue = iB + DitherValue;
					
					//Red = Clamp5 ( ( iR + DitherValue ) >> 27 );
					//Green = Clamp5 ( ( iG + DitherValue ) >> 27 );
					//Blue = Clamp5 ( ( iB + DitherValue ) >> 27 );
					
					// perform shift
					Red >>= ( 16 + 3 );
					Green >>= ( 16 + 3 );
					Blue >>= ( 16 + 3 );
					
					//Red = clamp ( Red, 0, 0x1f );
					//Green = clamp ( Green, 0, 0x1f );
					//Blue = clamp ( Blue, 0, 0x1f );
					Red = Clamp5 ( Red );
					Green = Clamp5 ( Green );
					Blue = Clamp5 ( Blue );
				}
				else
				{
					Red = iR >> ( 16 + 3 );
					Green = iG >> ( 16 + 3 );
					Blue = iB >> ( 16 + 3 );
				}
					
					bgr = ( Blue << 10 ) | ( Green << 5 ) | Red;
					
					// shade pixel color
				
					// read pixel from frame buffer if we need to check mask bit
					DestPixel = *ptr;
					
					bgr_temp = bgr;
		
					// semi-transparency
					if ( Command_ABE )
					{
						bgr_temp = SemiTransparency16 ( DestPixel, bgr_temp, GPU_CTRL_Read_ABR );
					}
					
					// check if we should set mask bit when drawing
					bgr_temp |= SetPixelMask;

					// draw pixel if we can draw to mask pixels or mask bit not set
					if ( ! ( DestPixel & PixelMask ) ) *ptr = bgr_temp;

					
				iR += ( dR_across * xinc );
				iG += ( dG_across * xinc );
				iB += ( dB_across * xinc );
				
				//ptr += c_iVectorSize;
				ptr += xinc;
			}
			
		}
		
		/////////////////////////////////////
		// update x on left and right
		xoff_left += ( dx_left * yinc );
		xoff_right += ( dx_right * yinc );
		
		Roff_left += ( dR_left * yinc );
		Goff_left += ( dG_left * yinc );
		Boff_left += ( dB_left * yinc );
	}
	
	} // end if ( EndY > StartY )
	
	return NumPixels;
}




u64 GPU::DrawTriangle_Texture_th ( DATA_Write_Format* inputdata, u32 ulThreadNum )
{
	const int local_id = 0;
	const int group_id = 0;
	const int num_local_threads = 1;
	const int num_global_groups = 1;
	
//#ifdef SINGLE_SCANLINE_MODE
	const int xid = local_id;
	const int yid = 0;
	
	const int xinc = num_local_threads;
	const int yinc = num_global_groups;
	s32 group_yoffset = 0;
//#endif


//inputdata format:
//0: GPU_CTRL_Read
//1: DrawArea_TopLeft
//2: DrawArea_BottomRight
//3: DrawArea_Offset
//4: (TextureWindow)(not used here)
//5: ------------
//6: ------------
//7:GetBGR24 ( Buffer [ 0 ] );
//8:GetXY0 ( Buffer [ 1 ] );
//9:GetUV0 ( Buffer [ 2 ] );
//9:GetCLUT ( Buffer [ 2 ] );
//10:GetXY1 ( Buffer [ 3 ] );
//11:GetUV1 ( Buffer [ 4 ] );
//11:GetTPAGE ( Buffer [ 4 ] );
//12:GetXY2 ( Buffer [ 5 ] );
//13:GetUV2 ( Buffer [ 6 ] );

	
	u32 GPU_CTRL_Read;
	s32 DrawArea_BottomRightX, DrawArea_TopLeftX, DrawArea_BottomRightY, DrawArea_TopLeftY;
	s32 DrawArea_OffsetX, DrawArea_OffsetY;
	u32 Command_ABE;
	u32 Command_TGE;
	//u32 GPU_CTRL_Read_ABR;
	//u32 GPU_CTRL_Read_DTD;
	
	u16 *ptr;
	
	s32 Temp;
	s32 LeftMostX, RightMostX;
	
	s32 StartX, EndX;
	s32 StartY, EndY;

	//s64 r10, r20, r21;
	
	s32 DitherValue;

	// new variables
	s32 x0, x1, x2, y0, y1, y2;
	s32 dx_left, dx_right;
	s32 x_left, x_right;
	s32 x_across;
	u32 bgr, bgr_temp;
	s32 Line;
	s32 t0, t1, denominator;

	// more variables for gradient triangle
	//s32 dR_left, dG_left, dB_left;
	//s32 dR_across, dG_across, dB_across;
	//s32 iR, iG, iB;
	//s32 R_left, G_left, B_left;
	//s32 Roff_left, Goff_left, Boff_left;
	//s32 r0, r1, r2, g0, g1, g2, b0, b1, b2;
	
	// variables for texture triangle
	s32 dU_left, dV_left;
	s32 dU_across, dV_across;
	s32 iU, iV;
	s32 U_left, V_left;
	s32 Uoff_left, Voff_left;
	s32 u0, u1, u2, v0, v1, v2;
	s32 gu [ 3 ], gv [ 3 ];
	

	s32 gx [ 3 ], gy [ 3 ], gbgr [ 3 ];
	
	s32 xoff_left, xoff_right;
	
	s32 Red, Green, Blue;
	u32 DestPixel;
	u32 PixelMask, SetPixelMask;

	u32 Coord0, Coord1, Coord2;
	//s32 group_yoffset;


	u32 color_add;
	
	u16 *ptr_texture, *ptr_clut;
	u32 clut_xoffset, clut_yoffset;
	u32 clut_x, clut_y, tpage_tx, tpage_ty, tpage_abr, tpage_tp, command_tge, command_abe, command_abr;
	
	u32 TexCoordX, TexCoordY;
	u32 Shift1, Shift2, And1, And2;
	u32 TextureOffset;

	u32 TWYTWH, TWXTWW, Not_TWH, Not_TWW;
	u32 TWX, TWY, TWW, TWH;
	
	u32 NumPixels;
	
	
	// setup vars
	//if ( !local_id )
	//{
		// no bitmaps in opencl ??
		GPU_CTRL_Read = inputdata [ 0 ].Value;
		DrawArea_TopLeftX = inputdata [ 1 ].Value & 0x3ff;
		DrawArea_TopLeftY = ( inputdata [ 1 ].Value >> 10 ) & 0x3ff;
		DrawArea_BottomRightX = inputdata [ 2 ].Value & 0x3ff;
		DrawArea_BottomRightY = ( inputdata [ 2 ].Value >> 10 ) & 0x3ff;
		DrawArea_OffsetX = ( ( (s32) inputdata [ 3 ].Value ) << 21 ) >> 21;
		DrawArea_OffsetY = ( ( (s32) inputdata [ 3 ].Value ) << 10 ) >> 21;
		
		gx [ 0 ] = (s32) ( ( inputdata [ 8 ].x << 5 ) >> 5 );
		gy [ 0 ] = (s32) ( ( inputdata [ 8 ].y << 5 ) >> 5 );
		gx [ 1 ] = (s32) ( ( inputdata [ 11 ].x << 5 ) >> 5 );
		gy [ 1 ] = (s32) ( ( inputdata [ 11 ].y << 5 ) >> 5 );
		gx [ 2 ] = (s32) ( ( inputdata [ 14 ].x << 5 ) >> 5 );
		gy [ 2 ] = (s32) ( ( inputdata [ 14 ].y << 5 ) >> 5 );

		Coord0 = 0;
		Coord1 = 1;
		Coord2 = 2;

		///////////////////////////////////
		// put top coordinates in x0,y0
		//if ( y1 < y0 )
		if ( gy [ Coord1 ] < gy [ Coord0 ] )
		{
			//Swap ( y0, y1 );
			//Swap ( Coord0, Coord1 );
			Temp = Coord0;
			Coord0 = Coord1;
			Coord1 = Temp;
		}
		
		//if ( y2 < y0 )
		if ( gy [ Coord2 ] < gy [ Coord0 ] )
		{
			//Swap ( y0, y2 );
			//Swap ( Coord0, Coord2 );
			Temp = Coord0;
			Coord0 = Coord2;
			Coord2 = Temp;
		}
		
		///////////////////////////////////////
		// put middle coordinates in x1,y1
		//if ( y2 < y1 )
		if ( gy [ Coord2 ] < gy [ Coord1 ] )
		{
			//Swap ( y1, y2 );
			//Swap ( Coord1, Coord2 );
			Temp = Coord1;
			Coord1 = Coord2;
			Coord2 = Temp;
		}
		
		// get x-values
		x0 = gx [ Coord0 ];
		x1 = gx [ Coord1 ];
		x2 = gx [ Coord2 ];
		
		// get y-values
		y0 = gy [ Coord0 ];
		y1 = gy [ Coord1 ];
		y2 = gy [ Coord2 ];

		//////////////////////////////////////////
		// get coordinates on screen
		x0 += DrawArea_OffsetX;
		y0 += DrawArea_OffsetY;
		x1 += DrawArea_OffsetX;
		y1 += DrawArea_OffsetY;
		x2 += DrawArea_OffsetX;
		y2 += DrawArea_OffsetY;
		
		// get the left/right most x
		LeftMostX = ( ( x0 < x1 ) ? x0 : x1 );
		LeftMostX = ( ( x2 < LeftMostX ) ? x2 : LeftMostX );
		RightMostX = ( ( x0 > x1 ) ? x0 : x1 );
		RightMostX = ( ( x2 > RightMostX ) ? x2 : RightMostX );

		// check for some important conditions
		if ( DrawArea_BottomRightX < DrawArea_TopLeftX )
		{
			//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightX < DrawArea_TopLeftX.\n";
			return 0;
		}
		
		if ( DrawArea_BottomRightY < DrawArea_TopLeftY )
		{
			//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightY < DrawArea_TopLeftY.\n";
			return 0;
		}

		// check if sprite is within draw area
		if ( RightMostX <= ((s32)DrawArea_TopLeftX) || LeftMostX > ((s32)DrawArea_BottomRightX) || y2 <= ((s32)DrawArea_TopLeftY) || y0 > ((s32)DrawArea_BottomRightY) ) return 0;
		
		// skip drawing if distance between vertices is greater than max allowed by GPU
		if ( ( _Abs( x1 - x0 ) > c_MaxPolygonWidth ) || ( _Abs( x2 - x1 ) > c_MaxPolygonWidth ) || ( y1 - y0 > c_MaxPolygonHeight ) || ( y2 - y1 > c_MaxPolygonHeight ) )
		{
			// skip drawing polygon
			return 0;
		}

		/////////////////////////////////////////////////
		// draw top part of triangle
		
		// denominator is negative when x1 is on the left, positive when x1 is on the right
		t0 = y1 - y2;
		t1 = y0 - y2;
		denominator = ( ( x0 - x2 ) * t0 ) - ( ( x1 - x2 ) * t1 );

		NumPixels = _Abs ( denominator ) >> 1;
		if ( ( !ulThreadNum ) && _GPU->ulNumberOfThreads )
		{
			return NumPixels;
		}
		
		if ( _GPU->bEnable_OpenCL )
		{
			return NumPixels;
		}


		
		gbgr [ 0 ] = inputdata [ 7 ].Value & 0x00ffffff;
		//gbgr [ 1 ] = inputdata [ 9 ].Value & 0x00ffffff;
		//gbgr [ 2 ] = inputdata [ 11 ].Value & 0x00ffffff;

		gu [ 0 ] = inputdata [ 9 ].u;
		gu [ 1 ] = inputdata [ 12 ].u;
		gu [ 2 ] = inputdata [ 15 ].u;
		gv [ 0 ] = inputdata [ 9 ].v;
		gv [ 1 ] = inputdata [ 12 ].v;
		gv [ 2 ] = inputdata [ 15 ].v;
		
		//GPU_CTRL_Read_ABR = ( GPU_CTRL_Read >> 5 ) & 3;
		Command_ABE = inputdata [ 7 ].Command & 2;
		Command_TGE = inputdata [ 7 ].Command & 1;
		

		// bits 0-5 in upper halfword
		clut_x = ( inputdata [ 9 ].Value >> ( 16 + 0 ) ) & 0x3f;
		clut_y = ( inputdata [ 9 ].Value >> ( 16 + 6 ) ) & 0x1ff;

		TWY = ( inputdata [ 4 ].Value >> 15 ) & 0x1f;
		TWX = ( inputdata [ 4 ].Value >> 10 ) & 0x1f;
		TWH = ( inputdata [ 4 ].Value >> 5 ) & 0x1f;
		TWW = inputdata [ 4 ].Value & 0x1f;

		
		// DTD is bit 9 in GPU_CTRL_Read
		//GPU_CTRL_Read_DTD = ( GPU_CTRL_Read >> 9 ) & 1;
		
		// ME is bit 12
		//if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
		PixelMask = ( GPU_CTRL_Read & 0x1000 ) << 3;
		
		// MD is bit 11
		//if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;
		SetPixelMask = ( GPU_CTRL_Read & 0x0800 ) << 4;
		
		
		// initialize number of pixels drawn
		//NumberOfPixelsDrawn = 0;
		
		// bits 0-3
		//tpage_tx = GPU_CTRL_Read & 0xf;
		tpage_tx = ( inputdata [ 12 ].Value >> ( 16 + 0 ) ) & 0xf;
		
		// bit 4
		//tpage_ty = ( GPU_CTRL_Read >> 4 ) & 1
		tpage_ty = ( inputdata [ 12 ].Value >> ( 16 + 4 ) ) & 1;
		
		// bits 5-6
		//GPU_CTRL_Read_ABR = ( GPU_CTRL_Read >> 5 ) & 3;
		tpage_abr = ( inputdata [ 12 ].Value >> ( 16 + 5 ) ) & 3;
		
		// bits 7-8
		//tpage_tp = ( GPU_CTRL_Read >> 7 ) & 3;
		tpage_tp = ( inputdata [ 12 ].Value >> ( 16 + 7 ) ) & 3;
		
		Shift1 = 0;
		Shift2 = 0;
		And1 = 0;
		And2 = 0;


		TWYTWH = ( ( TWY & TWH ) << 3 );
		TWXTWW = ( ( TWX & TWW ) << 3 );
		
		Not_TWH = ~( TWH << 3 );
		Not_TWW = ~( TWW << 3 );

		
		
		/////////////////////////////////////////////////////////
		// Get offset into texture page
		TextureOffset = ( tpage_tx << 6 ) + ( ( tpage_ty << 8 ) << 10 );
		
		clut_xoffset = clut_x << 4;
		
		if ( tpage_tp == 0 )
		{
			And2 = 0xf;
			
			Shift1 = 2; Shift2 = 2;
			And1 = 3; And2 = 0xf;
		}
		else if ( tpage_tp == 1 )
		{
			And2 = 0xff;
			
			Shift1 = 1; Shift2 = 3;
			And1 = 1; And2 = 0xff;
		}

		
		// get uv coords
		u0 = gu [ Coord0 ];
		u1 = gu [ Coord1 ];
		u2 = gu [ Coord2 ];
		v0 = gv [ Coord0 ];
		v1 = gv [ Coord1 ];
		v2 = gv [ Coord2 ];

		// get rgb-values
		//r0 = gbgr [ Coord0 ] & 0xff;
		//r1 = gbgr [ Coord1 ] & 0xff;
		//r2 = gbgr [ Coord2 ] & 0xff;
		//g0 = ( gbgr [ Coord0 ] >> 8 ) & 0xff;
		//g1 = ( gbgr [ Coord1 ] >> 8 ) & 0xff;
		//g2 = ( gbgr [ Coord2 ] >> 8 ) & 0xff;
		//b0 = ( gbgr [ Coord0 ] >> 16 ) & 0xff;
		//b1 = ( gbgr [ Coord1 ] >> 16 ) & 0xff;
		//b2 = ( gbgr [ Coord2 ] >> 16 ) & 0xff;
		// ?? convert to 16-bit ?? (or should leave 24-bit?)
		bgr = gbgr [ 0 ];
		//bgr = ( ( bgr >> 9 ) & 0x7c00 ) | ( ( bgr >> 6 ) & 0x3e0 ) | ( ( bgr >> 3 ) & 0x1f );
		
		if ( ( bgr & 0x00ffffff ) == 0x00808080 ) Command_TGE = 1;
		
		color_add = bgr;
		
		if ( denominator )
		{
			//dR_across = divide_s32 ( ( (s32) ( ( ( r0 - r2 ) * t0 ) - ( ( r1 - r2 ) * t1 ) ) ) << 8, denominator );
			//dG_across = divide_s32 ( ( (s32) ( ( ( g0 - g2 ) * t0 ) - ( ( g1 - g2 ) * t1 ) ) ) << 8, denominator );
			//dB_across = divide_s32 ( ( (s32) ( ( ( b0 - b2 ) * t0 ) - ( ( b1 - b2 ) * t1 ) ) ) << 8, denominator );
			//dR_across <<= 8;
			//dG_across <<= 8;
			//dB_across <<= 8;
			dU_across = ( ( (s32) ( ( ( u0 - u2 ) * t0 ) - ( ( u1 - u2 ) * t1 ) ) ) << 8 ) / denominator;
			dV_across = ( ( (s32) ( ( ( v0 - v2 ) * t0 ) - ( ( v1 - v2 ) * t1 ) ) ) << 8 ) / denominator;
			dU_across <<= 8;
			dV_across <<= 8;
			
			//printf ( "dR_across=%x top=%i bottom=%i divide=%x", dR_across, ( ( ( r0 - r2 ) * t0 ) - ( ( r1 - r2 ) * t1 ) ), denominator, ( ( ( ( r0 - r2 ) * t0 ) - ( ( r1 - r2 ) * t1 ) ) << 16 )/denominator );
			//printf ( "dG_across=%x top=%i bottom=%i divide=%x", dG_across, ( ( ( g0 - g2 ) * t0 ) - ( ( g1 - g2 ) * t1 ) ), denominator, ( ( ( ( g0 - g2 ) * t0 ) - ( ( g1 - g2 ) * t1 ) ) << 16 )/denominator );
		}
		
		
		
		
		//if ( denominator < 0 )
		//{
			// x1 is on the left and x0 is on the right //
			
			////////////////////////////////////
			// get slopes
			
			if ( y1 - y0 )
			{
				/////////////////////////////////////////////
				// init x on the left and right
				x_left = ( x0 << 16 );
				x_right = x_left;
				
				//R_left = ( r0 << 16 );
				//G_left = ( g0 << 16 );
				//B_left = ( b0 << 16 );

				U_left = ( u0 << 16 );
				V_left = ( v0 << 16 );
				
				if ( denominator < 0 )
				{
					//dx_left = ( ((s64)( x1 - x0 )) * r10 ) >> 32;
					//dx_right = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
					dx_left = ( ( x1 - x0 ) << 16 ) / ( y1 - y0 );
					dx_right = ( ( x2 - x0 ) << 16 ) / ( y2 - y0 );
					//dx_left = divide_s32( ( ( x1 - x0 ) << 16 ), ( y1 - y0 ) );
					//dx_right = divide_s32( ( ( x2 - x0 ) << 16 ), ( y2 - y0 ) );
					
					//dR_left = (( r1 - r0 ) << 16 ) / ( y1 - y0 );
					//dG_left = (( g1 - g0 ) << 16 ) / ( y1 - y0 );
					//dB_left = (( b1 - b0 ) << 16 ) / ( y1 - y0 );
					//dR_left = divide_s32( (( r1 - r0 ) << 16 ), ( y1 - y0 ) );
					//dG_left = divide_s32( (( g1 - g0 ) << 16 ), ( y1 - y0 ) );
					//dB_left = divide_s32( (( b1 - b0 ) << 16 ), ( y1 - y0 ) );
					
					dU_left = ( (( u1 - u0 ) << 16 ) ) / ( y1 - y0 );
					dV_left = ( (( v1 - v0 ) << 16 ) ) / ( y1 - y0 );
				}
				else
				{
					//dx_right = ( ((s64)( x1 - x0 )) * r10 ) >> 32;
					//dx_left = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
					dx_right = ( ( x1 - x0 ) << 16 ) / ( y1 - y0 );
					dx_left = ( ( x2 - x0 ) << 16 ) / ( y2 - y0 );
					//dx_right = divide_s32( ( ( x1 - x0 ) << 16 ), ( y1 - y0 ) );
					//dx_left = divide_s32( ( ( x2 - x0 ) << 16 ), ( y2 - y0 ) );
					
					//dR_left = (( r2 - r0 ) << 16 ) / ( y2 - y0 );
					//dG_left = (( g2 - g0 ) << 16 ) / ( y2 - y0 );
					//dB_left = (( b2 - b0 ) << 16 ) / ( y2 - y0 );
					//dR_left = divide_s32( (( r2 - r0 ) << 16 ), ( y2 - y0 ) );
					//dG_left = divide_s32( (( g2 - g0 ) << 16 ), ( y2 - y0 ) );
					//dB_left = divide_s32( (( b2 - b0 ) << 16 ), ( y2 - y0 ) );
					
					dU_left = ( (( u2 - u0 ) << 16 ) ) / ( y2 - y0 );
					dV_left = ( (( v2 - v0 ) << 16 ) ) / ( y2 - y0 );
				}
			}
			else
			{
				if ( denominator < 0 )
				{
					// change x_left and x_right where y1 is on left
					x_left = ( x1 << 16 );
					x_right = ( x0 << 16 );
					
					U_left = ( u1 << 16 );
					V_left = ( v1 << 16 );
					
					if ( y2 - y1 )
					{
						//dx_left = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
						//dx_right = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
						dx_left = ( ( x2 - x1 ) << 16 ) / ( y2 - y1 );
						dx_right = ( ( x2 - x0 ) << 16 ) / ( y2 - y0 );
						//dx_left = divide_s32( ( ( x2 - x1 ) << 16 ), ( y2 - y1 ) );
						//dx_right = divide_s32( ( ( x2 - x0 ) << 16 ), ( y2 - y0 ) );
						
						//R_left = ( r1 << 16 );
						//G_left = ( g1 << 16 );
						//B_left = ( b1 << 16 );

						
						//dR_left = (( r2 - r1 ) << 16 ) / ( y2 - y1 );
						//dG_left = (( g2 - g1 ) << 16 ) / ( y2 - y1 );
						//dB_left = (( b2 - b1 ) << 16 ) / ( y2 - y1 );
						//dR_left = divide_s32( (( r2 - r1 ) << 16 ), ( y2 - y1 ) );
						//dG_left = divide_s32( (( g2 - g1 ) << 16 ), ( y2 - y1 ) );
						//dB_left = divide_s32( (( b2 - b1 ) << 16 ), ( y2 - y1 ) );
						
						dU_left = ( (( u2 - u1 ) << 16 ) ) / ( y2 - y1 );
						dV_left = ( (( v2 - v1 ) << 16 ) ) / ( y2 - y1 );
					}
				}
				else
				{
					x_right = ( x1 << 16 );
					x_left = ( x0 << 16 );
				
					U_left = ( u0 << 16 );
					V_left = ( v0 << 16 );
					
					if ( y2 - y1 )
					{
						//dx_right = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
						//dx_left = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
						dx_right = ( ( x2 - x1 ) << 16 ) / ( y2 - y1 );
						dx_left = ( ( x2 - x0 ) << 16 ) / ( y2 - y0 );
						//dx_right = divide_s32( ( ( x2 - x1 ) << 16 ), ( y2 - y1 ) );
						//dx_left = divide_s32( ( ( x2 - x0 ) << 16 ), ( y2 - y0 ) );
						
						//R_left = ( r0 << 16 );
						//G_left = ( g0 << 16 );
						//B_left = ( b0 << 16 );
						
						
						//dR_left = (( r2 - r0 ) << 16 ) / ( y2 - y0 );
						//dG_left = (( g2 - g0 ) << 16 ) / ( y2 - y0 );
						//dB_left = (( b2 - b0 ) << 16 ) / ( y2 - y0 );
						//dR_left = divide_s32( (( r2 - r0 ) << 16 ), ( y2 - y0 ) );
						//dG_left = divide_s32( (( g2 - g0 ) << 16 ), ( y2 - y0 ) );
						//dB_left = divide_s32( (( b2 - b0 ) << 16 ), ( y2 - y0 ) );
						
						dU_left = ( (( u2 - u0 ) << 16 ) ) / ( y2 - y0 );
						dV_left = ( (( v2 - v0 ) << 16 ) ) / ( y2 - y0 );
					}
				}
			}
		//}
		


	
		////////////////
		// *** TODO *** at this point area of full triangle can be calculated and the rest of the drawing can be put on another thread *** //
		
		
		
		// r,g,b values are not specified with a fractional part, so there must be an initial fractional part
		//R_left |= ( 1 << 15 );
		//G_left |= ( 1 << 15 );
		//B_left |= ( 1 << 15 );

		U_left |= ( 1 << 15 );
		V_left |= ( 1 << 15 );
		
		//x_left += 0xffff;
		//x_right -= 1;
		
		StartY = y0;
		EndY = y1;

		if ( StartY < ((s32)DrawArea_TopLeftY) )
		{
			
			if ( EndY < ((s32)DrawArea_TopLeftY) )
			{
				Temp = EndY - StartY;
				StartY = EndY;
			}
			else
			{
				Temp = DrawArea_TopLeftY - StartY;
				StartY = DrawArea_TopLeftY;
			}
			
			x_left += dx_left * Temp;
			x_right += dx_right * Temp;
			
			//R_left += dR_left * Temp;
			//G_left += dG_left * Temp;
			//B_left += dB_left * Temp;
			
			U_left += dU_left * Temp;
			V_left += dV_left * Temp;
		}
		
		if ( EndY > ((s32)DrawArea_BottomRightY) )
		{
			EndY = DrawArea_BottomRightY + 1;
		}

		
		// offset to get to this compute unit's scanline
		//group_yoffset = group_id - ( StartY % num_global_groups );
		//if ( group_yoffset < 0 )
		//{
		//	group_yoffset += num_global_groups;
		//}
		
		//printf( "x_left=%x x_right=%x dx_left=%i dx_right=%i R_left=%x G_left=%x B_left=%x OffsetX=%i OffsetY=%i",x_left,x_right,dx_left,dx_right,R_left,G_left,B_left, DrawArea_OffsetX, DrawArea_OffsetY );
		//printf( "x0=%i y0=%i x1=%i y1=%i x2=%i y2=%i r0=%i r1=%i r2=%i g0=%i g1=%i g2=%i b0=%i b1=%i b2=%i", x0, y0, x1, y1, x2, y2, r0, r1, r2, g0, g1, g2, b0, b1, b2 );
		//printf( "dR_across=%x dG_across=%x dB_across=%x", dR_across, dG_across, dB_across );

	//}	// end if ( !local_id )
	
	

	// synchronize variables across workers
	//barrier ( CLK_LOCAL_MEM_FENCE );

	


	
	
	ptr_clut = & ( _GPU->VRAM [ clut_y << 10 ] );
	ptr_texture = & ( _GPU->VRAM [ ( tpage_tx << 6 ) + ( ( tpage_ty << 8 ) << 10 ) ] );

	



	
	/////////////////////////////////////////////
	// init x on the left and right
	
	


	if ( EndY > StartY )
	{
	
	// in opencl, each worker could be on a different line
	xoff_left = x_left + ( dx_left * (group_yoffset + yid) );
	xoff_right = x_right + ( dx_right * (group_yoffset + yid) );
	
	//Roff_left = R_left + ( dR_left * (group_yoffset + yid) );
	//Goff_left = G_left + ( dG_left * (group_yoffset + yid) );
	//Boff_left = B_left + ( dB_left * (group_yoffset + yid) );
	
	Uoff_left = U_left + ( dU_left * (group_yoffset + yid) );
	Voff_left = V_left + ( dV_left * (group_yoffset + yid) );
	
	//xoff_left += 0xffff;
	//xoff_right -= 1;
	
	//////////////////////////////////////////////
	// draw down to y1
	//for ( Line = StartY; Line < EndY; Line++ )
	for ( Line = StartY + group_yoffset + yid; Line < EndY; Line += yinc )
	{
		
		// left point is included if points are equal
		StartX = ( xoff_left + 0xffff ) >> 16;
		EndX = ( xoff_right - 1 ) >> 16;
		//StartX = xoff_left >> 16;
		//EndX = xoff_right >> 16;
		
//debug << "\r\nStartX=" << dec << StartX << " EndX=" << EndX << " Line=" << Line << " StartY=" << StartY << " EndY=" << EndY;
		
		if ( StartX <= ((s32)DrawArea_BottomRightX) && EndX >= ((s32)DrawArea_TopLeftX) && EndX >= StartX )
		{
			
			//iR = Roff_left;
			//iG = Goff_left;
			//iB = Boff_left;
			
			iU = Uoff_left;
			iV = Voff_left;
			
			// get the difference between x_left and StartX
			Temp = ( StartX << 16 ) - xoff_left;
			
			if ( StartX < ((s32)DrawArea_TopLeftX) )
			{
				Temp += ( DrawArea_TopLeftX - StartX ) << 16;
				StartX = DrawArea_TopLeftX;
				
			}
			
			//iR += ( dR_across >> 8 ) * ( Temp >> 8 );
			//iG += ( dG_across >> 8 ) * ( Temp >> 8 );
			//iB += ( dB_across >> 8 ) * ( Temp >> 8 );
			
			iU += ( dU_across >> 8 ) * ( Temp >> 8 );
			iV += ( dV_across >> 8 ) * ( Temp >> 8 );
			
			if ( EndX > ((s32)DrawArea_BottomRightX) )
			{
				//EndX = DrawArea_BottomRightX + 1;
				EndX = DrawArea_BottomRightX;
			}
			
			ptr = & ( _GPU->VRAM [ StartX + xid + ( Line << 10 ) ] );
			//DitherLine = & ( DitherArray [ ( Line & 0x3 ) << 2 ] );
			
			
			//iR += ( dR_across * xid );
			//iG += ( dG_across * xid );
			//iB += ( dB_across * xid );

			iU += ( dU_across * xid );
			iV += ( dV_across * xid );
			
			// draw horizontal line
			// x_left and x_right need to be rounded off
			//for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize )
			for ( x_across = StartX + xid; x_across <= EndX; x_across += xinc )
			{
				
				/*
				if ( GPU_CTRL_Read_DTD )
				{
					//bgr = ( _Round( iR ) >> 32 ) | ( ( _Round( iG ) >> 32 ) << 8 ) | ( ( _Round( iB ) >> 32 ) << 16 );
					//bgr = ( _Round( iR ) >> 35 ) | ( ( _Round( iG ) >> 35 ) << 5 ) | ( ( _Round( iB ) >> 35 ) << 10 );
					//DitherValue = DitherLine [ x_across & 0x3 ];
					DitherValue = c_iDitherValues24 [ ( x_across & 3 ) + ( ( Line & 3 ) << 2 ) ];
					
					// perform dither
					//Red = iR + DitherValue;
					//Green = iG + DitherValue;
					//Blue = iB + DitherValue;
					Red = iR + DitherValue;
					Green = iG + DitherValue;
					Blue = iB + DitherValue;
					
					//Red = Clamp5 ( ( iR + DitherValue ) >> 27 );
					//Green = Clamp5 ( ( iG + DitherValue ) >> 27 );
					//Blue = Clamp5 ( ( iB + DitherValue ) >> 27 );
					
					// perform shift
					Red >>= ( 16 + 3 );
					Green >>= ( 16 + 3 );
					Blue >>= ( 16 + 3 );
					
					Red = clamp ( Red, 0, 0x1f );
					Green = clamp ( Green, 0, 0x1f );
					Blue = clamp ( Blue, 0, 0x1f );
				}
				else
				{
					Red = iR >> ( 16 + 3 );
					Green = iG >> ( 16 + 3 );
					Blue = iB >> ( 16 + 3 );
				}
				
				color_add = ( Blue << 10 ) | ( Green << 5 ) | Red;
				*/

				TexCoordY = (u8) ( ( ( iV >> 16 ) & Not_TWH ) | ( TWYTWH ) );
				TexCoordY <<= 10;

				//TexCoordX = (u8) ( ( iU & ~( TWW << 3 ) ) | ( ( TWX & TWW ) << 3 ) );
				TexCoordX = (u8) ( ( ( iU >> 16 ) & Not_TWW ) | ( TWXTWW ) );
				
				//bgr = ptr_texture [ ( TexCoordX >> Shift1 ) + ( TexCoordY << 10 ) ];
				bgr = ptr_texture [ ( TexCoordX >> Shift1 ) + TexCoordY ];
				
				if ( Shift1 )
				{
					//bgr = VRAM [ ( ( ( clut_x << 4 ) + TexelIndex ) & FrameBuffer_XMask ) + ( clut_y << 10 ) ];
					bgr = ptr_clut [ ( clut_xoffset + ( ( bgr >> ( ( TexCoordX & And1 ) << Shift2 ) ) & And2 ) ) & FrameBuffer_XMask ];
				}

//debug << "\r\nx_across=" << dec << x_across << " TexCoordX=" << TexCoordX << " TexCoordY=" << TexCoordY << " bgr=" << hex << bgr;				
				
				if ( bgr )
				{
					
					// shade pixel color
					
					// read pixel from frame buffer if we need to check mask bit
					DestPixel = *ptr;
					
					
					bgr_temp = bgr;
		
					if ( !Command_TGE )
					{
						// brightness calculation
						//bgr_temp = Color24To16 ( ColorMultiply24 ( Color16To24 ( bgr_temp ), color_add ) );
						bgr_temp = ColorMultiply1624 ( bgr_temp, color_add );
					}
					
					// semi-transparency
					if ( Command_ABE && ( bgr & 0x8000 ) )
					{
						bgr_temp = SemiTransparency16 ( DestPixel, bgr_temp, tpage_abr );
					}
					
					// check if we should set mask bit when drawing
					bgr_temp |= SetPixelMask | ( bgr & 0x8000 );

					// draw pixel if we can draw to mask pixels or mask bit not set
					if ( ! ( DestPixel & PixelMask ) ) *ptr = bgr_temp;
					
				}
						
				//iR += ( dR_across * xinc );
				//iG += ( dG_across * xinc );
				//iB += ( dB_across * xinc );
			
				iU += ( dU_across * xinc );
				iV += ( dV_across * xinc );
					
				//ptr += c_iVectorSize;
				ptr += xinc;
			}
			
		}
		
		
		/////////////////////////////////////
		// update x on left and right
		xoff_left += ( dx_left * yinc );
		xoff_right += ( dx_right * yinc );
		
		//Roff_left += ( dR_left * yinc );
		//Goff_left += ( dG_left * yinc );
		//Boff_left += ( dB_left * yinc );
		
		Uoff_left += ( dU_left * yinc );
		Voff_left += ( dV_left * yinc );
	}

	} // end if ( EndY > StartY )

	
	// synchronize variables across workers
	//barrier ( CLK_LOCAL_MEM_FENCE );

	
	////////////////////////////////////////////////
	// draw bottom part of triangle

	/////////////////////////////////////////////
	// init x on the left and right
	
	//if ( !local_id )
	//{
		//////////////////////////////////////////////////////
		// check if y1 is on the left or on the right
		if ( denominator < 0 )
		{
			x_left = ( x1 << 16 );

			x_right = ( x0 << 16 ) + ( dx_right * ( y1 - y0 ) );
			
			//R_left = ( r1 << 16 );
			//G_left = ( g1 << 16 );
			//B_left = ( b1 << 16 );
			
			U_left = ( u1 << 16 );
			V_left = ( v1 << 16 );
			
			if ( y2 - y1 )
			{
				//dx_left = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
				//dx_left = (( x2 - x1 ) << 16 ) / ( y2 - y1 );
				dx_left = ( (( x2 - x1 ) << 16 ) ) / ( y2 - y1 );
				
				//dR_left = (( r2 - r1 ) << 16 ) / ( y2 - y1 );
				//dG_left = (( g2 - g1 ) << 16 ) / ( y2 - y1 );
				//dB_left = (( b2 - b1 ) << 16 ) / ( y2 - y1 );
				//dR_left = divide_s32( (( r2 - r1 ) << 16 ), ( y2 - y1 ) );
				//dG_left = divide_s32( (( g2 - g1 ) << 16 ), ( y2 - y1 ) );
				//dB_left = divide_s32( (( b2 - b1 ) << 16 ), ( y2 - y1 ) );
				
				dU_left = ( (( u2 - u1 ) << 16 ) ) / ( y2 - y1 );
				dV_left = ( (( v2 - v1 ) << 16 ) ) / ( y2 - y1 );
			}
		}
		else
		{
			x_right = ( x1 << 16 );

			x_left = ( x0 << 16 ) + ( dx_left * ( y1 - y0 ) );
			
			//R_left = ( r0 << 16 ) + ( dR_left * ( y1 - y0 ) );
			//G_left = ( g0 << 16 ) + ( dG_left * ( y1 - y0 ) );
			//B_left = ( b0 << 16 ) + ( dB_left * ( y1 - y0 ) );
			
			U_left = ( u0 << 16 ) + ( dU_left * ( y1 - y0 ) );
			V_left = ( v0 << 16 ) + ( dV_left * ( y1 - y0 ) );
			
			if ( y2 - y1 )
			{
				//dx_right = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
				//dx_right = (( x2 - x1 ) << 16 ) / ( y2 - y1 );
				dx_right = ( (( x2 - x1 ) << 16 ) ) / ( y2 - y1 );
				
			}
		}


		//R_left += ( 1 << 15 );
		//G_left += ( 1 << 15 );
		//B_left += ( 1 << 15 );

		U_left += ( 1 << 15 );
		V_left += ( 1 << 15 );
		
		//x_left += 0xffff;
		//x_right -= 1;

		StartY = y1;
		EndY = y2;

		if ( StartY < ((s32)DrawArea_TopLeftY) )
		{
			
			if ( EndY < ((s32)DrawArea_TopLeftY) )
			{
				Temp = EndY - StartY;
				StartY = EndY;
			}
			else
			{
				Temp = DrawArea_TopLeftY - StartY;
				StartY = DrawArea_TopLeftY;
			}
			
			x_left += dx_left * Temp;
			x_right += dx_right * Temp;
			
			//R_left += dR_left * Temp;
			//G_left += dG_left * Temp;
			//B_left += dB_left * Temp;
			
			U_left += dU_left * Temp;
			V_left += dV_left * Temp;
		}
		
		if ( EndY > ((s32)DrawArea_BottomRightY) )
		{
			EndY = DrawArea_BottomRightY + 1;
		}
		
		// offset to get to this compute unit's scanline
		//group_yoffset = group_id - ( StartY % num_global_groups );
		//if ( group_yoffset < 0 )
		//{
		//	group_yoffset += num_global_groups;
		//}
	//}

	
	// synchronize variables across workers
	//barrier ( CLK_LOCAL_MEM_FENCE );

	
	if ( EndY > StartY )
	{
	
	// in opencl, each worker could be on a different line
	xoff_left = x_left + ( dx_left * (group_yoffset + yid) );
	xoff_right = x_right + ( dx_right * (group_yoffset + yid) );
	
	//Roff_left = R_left + ( dR_left * (group_yoffset + yid) );
	//Goff_left = G_left + ( dG_left * (group_yoffset + yid) );
	//Boff_left = B_left + ( dB_left * (group_yoffset + yid) );
	
	Uoff_left = U_left + ( dU_left * (group_yoffset + yid) );
	Voff_left = V_left + ( dV_left * (group_yoffset + yid) );
	
	//xoff_left += 0xffff;
	//xoff_right -= 1;
	
	//////////////////////////////////////////////
	// draw down to y2
	//for ( Line = StartY; Line < EndY; Line++ )
	for ( Line = StartY + group_yoffset + yid; Line < EndY; Line += yinc )
	{
		
		// left point is included if points are equal
		StartX = ( xoff_left + 0xffff ) >> 16;
		EndX = ( xoff_right - 1 ) >> 16;
		//StartX = xoff_left >> 16;
		//EndX = xoff_right >> 16;
		
		
		if ( StartX <= ((s32)DrawArea_BottomRightX) && EndX >= ((s32)DrawArea_TopLeftX) && EndX >= StartX )
		{
			//iR = Roff_left;
			//iG = Goff_left;
			//iB = Boff_left;
			
			iU = Uoff_left;
			iV = Voff_left;
			
			// get the difference between x_left and StartX
			Temp = ( StartX << 16 ) - xoff_left;
			
			if ( StartX < ((s32)DrawArea_TopLeftX) )
			{
				Temp += ( DrawArea_TopLeftX - StartX ) << 16;
				StartX = DrawArea_TopLeftX;
				
			}
			
			//iR += ( dR_across >> 8 ) * ( Temp >> 8 );
			//iG += ( dG_across >> 8 ) * ( Temp >> 8 );
			//iB += ( dB_across >> 8 ) * ( Temp >> 8 );
			
			iU += ( dU_across >> 8 ) * ( Temp >> 8 );
			iV += ( dV_across >> 8 ) * ( Temp >> 8 );
			
			if ( EndX > ((s32)DrawArea_BottomRightX) )
			{
				//EndX = DrawArea_BottomRightX + 1;
				EndX = DrawArea_BottomRightX;
			}
			
			ptr = & ( _GPU->VRAM [ StartX + xid + ( Line << 10 ) ] );
			//DitherLine = & ( DitherArray [ ( Line & 0x3 ) << 2 ] );
			
			
			
			//iR += ( dR_across * xid );
			//iG += ( dG_across * xid );
			//iB += ( dB_across * xid );

			iU += ( dU_across * xid );
			iV += ( dV_across * xid );
			
			// draw horizontal line
			// x_left and x_right need to be rounded off
			//for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize )
			for ( x_across = StartX + xid; x_across <= EndX; x_across += xinc )
			{
				
				/*
				if ( GPU_CTRL_Read_DTD )
				{
					//bgr = ( _Round( iR ) >> 32 ) | ( ( _Round( iG ) >> 32 ) << 8 ) | ( ( _Round( iB ) >> 32 ) << 16 );
					//bgr = ( _Round( iR ) >> 35 ) | ( ( _Round( iG ) >> 35 ) << 5 ) | ( ( _Round( iB ) >> 35 ) << 10 );
					//DitherValue = DitherLine [ x_across & 0x3 ];
					DitherValue = c_iDitherValues24 [ ( x_across & 3 ) + ( ( Line & 3 ) << 2 ) ];
					
					// perform dither
					//Red = iR + DitherValue;
					//Green = iG + DitherValue;
					//Blue = iB + DitherValue;
					Red = iR + DitherValue;
					Green = iG + DitherValue;
					Blue = iB + DitherValue;
					
					//Red = Clamp5 ( ( iR + DitherValue ) >> 27 );
					//Green = Clamp5 ( ( iG + DitherValue ) >> 27 );
					//Blue = Clamp5 ( ( iB + DitherValue ) >> 27 );
					
					// perform shift
					Red >>= ( 16 + 3 );
					Green >>= ( 16 + 3 );
					Blue >>= ( 16 + 3 );
					
					Red = clamp ( Red, 0, 0x1f );
					Green = clamp ( Green, 0, 0x1f );
					Blue = clamp ( Blue, 0, 0x1f );
				}
				else
				{
					Red = iR >> ( 16 + 3 );
					Green = iG >> ( 16 + 3 );
					Blue = iB >> ( 16 + 3 );
				}
				
				color_add = ( Blue << 10 ) | ( Green << 5 ) | Red;
				*/

				TexCoordY = (u8) ( ( ( iV >> 16 ) & Not_TWH ) | ( TWYTWH ) );
				TexCoordY <<= 10;

				//TexCoordX = (u8) ( ( iU & ~( TWW << 3 ) ) | ( ( TWX & TWW ) << 3 ) );
				TexCoordX = (u8) ( ( ( iU >> 16 ) & Not_TWW ) | ( TWXTWW ) );
				
				//bgr = ptr_texture [ ( TexCoordX >> Shift1 ) + ( TexCoordY << 10 ) ];
				bgr = ptr_texture [ ( TexCoordX >> Shift1 ) + TexCoordY ];
				
				if ( Shift1 )
				{
					//bgr = VRAM [ ( ( ( clut_x << 4 ) + TexelIndex ) & FrameBuffer_XMask ) + ( clut_y << 10 ) ];
					bgr = ptr_clut [ ( clut_xoffset + ( ( bgr >> ( ( TexCoordX & And1 ) << Shift2 ) ) & And2 ) ) & FrameBuffer_XMask ];
				}

				
				if ( bgr )
				{
					
					// shade pixel color
					
					// read pixel from frame buffer if we need to check mask bit
					DestPixel = *ptr;
					
					
					bgr_temp = bgr;
		
					if ( !Command_TGE )
					{
						// brightness calculation
						//bgr_temp = Color24To16 ( ColorMultiply24 ( Color16To24 ( bgr_temp ), color_add ) );
						bgr_temp = ColorMultiply1624 ( bgr_temp, color_add );
					}
					
					// semi-transparency
					if ( Command_ABE && ( bgr & 0x8000 ) )
					{
						bgr_temp = SemiTransparency16 ( DestPixel, bgr_temp, tpage_abr );
					}
					
					// check if we should set mask bit when drawing
					bgr_temp |= SetPixelMask | ( bgr & 0x8000 );

					// draw pixel if we can draw to mask pixels or mask bit not set
					if ( ! ( DestPixel & PixelMask ) ) *ptr = bgr_temp;
					
				}

					
				//iR += ( dR_across * xinc );
				//iG += ( dG_across * xinc );
				//iB += ( dB_across * xinc );
				
				iU += ( dU_across * xinc );
				iV += ( dV_across * xinc );
				
				//ptr += c_iVectorSize;
				ptr += xinc;
			}
			
		}
		
		/////////////////////////////////////
		// update x on left and right
		xoff_left += ( dx_left * yinc );
		xoff_right += ( dx_right * yinc );
		
		//Roff_left += ( dR_left * yinc );
		//Goff_left += ( dG_left * yinc );
		//Boff_left += ( dB_left * yinc );
		
		Uoff_left += ( dU_left * yinc );
		Voff_left += ( dV_left * yinc );
	}
	
	} // end if ( EndY > StartY )
		
	return NumPixels;
}




u64 GPU::DrawTriangle_TextureGradient_th ( DATA_Write_Format* inputdata, u32 ulThreadNum )
{
	const int local_id = 0;
	const int group_id = 0;
	const int num_local_threads = 1;
	const int num_global_groups = 1;
	
//#ifdef SINGLE_SCANLINE_MODE
	const int xid = local_id;
	const int yid = 0;
	
	const int xinc = num_local_threads;
	const int yinc = num_global_groups;
	s32 group_yoffset = 0;
//#endif


//inputdata format:
//0: GPU_CTRL_Read
//1: DrawArea_TopLeft
//2: DrawArea_BottomRight
//3: DrawArea_Offset
//4: (TextureWindow)(not used here)
//5: ------------
//6: ------------
//7:GetBGR0_8 ( Buffer [ 0 ] );
//8:GetXY0 ( Buffer [ 1 ] );
//9:GetCLUT ( Buffer [ 2 ] );
//9:GetUV0 ( Buffer [ 2 ] );
//10:GetBGR1_8 ( Buffer [ 3 ] );
//11:GetXY1 ( Buffer [ 4 ] );
//12:GetTPAGE ( Buffer [ 5 ] );
//12:GetUV1 ( Buffer [ 5 ] );
//13:GetBGR2_8 ( Buffer [ 6 ] );
//14:GetXY2 ( Buffer [ 7 ] );
//15:GetUV2 ( Buffer [ 8 ] );

	
	u32 GPU_CTRL_Read;
	//u32 GPU_CTRL_Read_ABR;
	s32 DrawArea_BottomRightX, DrawArea_TopLeftX, DrawArea_BottomRightY, DrawArea_TopLeftY;
	s32 DrawArea_OffsetX, DrawArea_OffsetY;
	u32 Command_ABE;
	u32 Command_TGE;
	u32 GPU_CTRL_Read_DTD;
	
	u16 *ptr;
	
	s32 Temp;
	s32 LeftMostX, RightMostX;
	
	s32 StartX, EndX;
	s32 StartY, EndY;

	s32 DitherValue;

	// new variables
	s32 x0, x1, x2, y0, y1, y2;
	s32 dx_left, dx_right;
	s32 x_left, x_right;
	s32 x_across;
	u32 bgr, bgr_temp;
	s32 Line;
	s32 t0, t1, denominator;

	// more variables for gradient triangle
	s32 dR_left, dG_left, dB_left;
	s32 dR_across, dG_across, dB_across;
	s32 iR, iG, iB;
	s32 R_left, G_left, B_left;
	s32 Roff_left, Goff_left, Boff_left;
	s32 r0, r1, r2, g0, g1, g2, b0, b1, b2;
	//s32 gr [ 3 ], gg [ 3 ], gb [ 3 ];
	
	// variables for texture triangle
	s32 dU_left, dV_left;
	s32 dU_across, dV_across;
	s32 iU, iV;
	s32 U_left, V_left;
	s32 Uoff_left, Voff_left;
	s32 u0, u1, u2, v0, v1, v2;
	s32 gu [ 3 ], gv [ 3 ];
	

	s32 gx [ 3 ], gy [ 3 ], gbgr [ 3 ];
	
	s32 xoff_left, xoff_right;
	
	s32 Red, Green, Blue;
	u32 DestPixel;
	u32 PixelMask, SetPixelMask;

	u32 Coord0, Coord1, Coord2;


	u32 color_add;
	
	u16 *ptr_texture, *ptr_clut;
	u32 clut_xoffset, clut_yoffset;
	u32 clut_x, clut_y, tpage_tx, tpage_ty, tpage_abr, tpage_tp;
	
	u32 TexCoordX, TexCoordY;
	u32 Shift1, Shift2, And1, And2;
	u32 TextureOffset;

	u32 TWYTWH, TWXTWW, Not_TWH, Not_TWW;
	u32 TWX, TWY, TWW, TWH;
	
	u32 NumPixels;
	
	
	// setup vars
	//if ( !local_id )
	//{
		// no bitmaps in opencl ??
		GPU_CTRL_Read = inputdata [ 0 ].Value;
		DrawArea_TopLeftX = inputdata [ 1 ].Value & 0x3ff;
		DrawArea_TopLeftY = ( inputdata [ 1 ].Value >> 10 ) & 0x3ff;
		DrawArea_BottomRightX = inputdata [ 2 ].Value & 0x3ff;
		DrawArea_BottomRightY = ( inputdata [ 2 ].Value >> 10 ) & 0x3ff;
		DrawArea_OffsetX = ( ( (s32) inputdata [ 3 ].Value ) << 21 ) >> 21;
		DrawArea_OffsetY = ( ( (s32) inputdata [ 3 ].Value ) << 10 ) >> 21;
		
		gx [ 0 ] = (s32) ( ( inputdata [ 8 ].x << 5 ) >> 5 );
		gy [ 0 ] = (s32) ( ( inputdata [ 8 ].y << 5 ) >> 5 );
		gx [ 1 ] = (s32) ( ( inputdata [ 11 ].x << 5 ) >> 5 );
		gy [ 1 ] = (s32) ( ( inputdata [ 11 ].y << 5 ) >> 5 );
		gx [ 2 ] = (s32) ( ( inputdata [ 14 ].x << 5 ) >> 5 );
		gy [ 2 ] = (s32) ( ( inputdata [ 14 ].y << 5 ) >> 5 );
		
		Coord0 = 0;
		Coord1 = 1;
		Coord2 = 2;
		
		///////////////////////////////////
		// put top coordinates in x0,y0
		//if ( y1 < y0 )
		if ( gy [ Coord1 ] < gy [ Coord0 ] )
		{
			//Swap ( y0, y1 );
			//Swap ( Coord0, Coord1 );
			Temp = Coord0;
			Coord0 = Coord1;
			Coord1 = Temp;
		}
		
		//if ( y2 < y0 )
		if ( gy [ Coord2 ] < gy [ Coord0 ] )
		{
			//Swap ( y0, y2 );
			//Swap ( Coord0, Coord2 );
			Temp = Coord0;
			Coord0 = Coord2;
			Coord2 = Temp;
		}
		
		///////////////////////////////////////
		// put middle coordinates in x1,y1
		//if ( y2 < y1 )
		if ( gy [ Coord2 ] < gy [ Coord1 ] )
		{
			//Swap ( y1, y2 );
			//Swap ( Coord1, Coord2 );
			Temp = Coord1;
			Coord1 = Coord2;
			Coord2 = Temp;
		}
		
		// get x-values
		x0 = gx [ Coord0 ];
		x1 = gx [ Coord1 ];
		x2 = gx [ Coord2 ];
		
		// get y-values
		y0 = gy [ Coord0 ];
		y1 = gy [ Coord1 ];
		y2 = gy [ Coord2 ];

		//////////////////////////////////////////
		// get coordinates on screen
		x0 += DrawArea_OffsetX;
		y0 += DrawArea_OffsetY;
		x1 += DrawArea_OffsetX;
		y1 += DrawArea_OffsetY;
		x2 += DrawArea_OffsetX;
		y2 += DrawArea_OffsetY;
		
		// get the left/right most x
		LeftMostX = ( ( x0 < x1 ) ? x0 : x1 );
		LeftMostX = ( ( x2 < LeftMostX ) ? x2 : LeftMostX );
		RightMostX = ( ( x0 > x1 ) ? x0 : x1 );
		RightMostX = ( ( x2 > RightMostX ) ? x2 : RightMostX );
		
		// check for some important conditions
		if ( DrawArea_BottomRightX < DrawArea_TopLeftX )
		{
			//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightX < DrawArea_TopLeftX.\n";
			return 0;
		}
		
		if ( DrawArea_BottomRightY < DrawArea_TopLeftY )
		{
			//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightY < DrawArea_TopLeftY.\n";
			return 0;
		}

		// check if sprite is within draw area
		if ( RightMostX <= ((s32)DrawArea_TopLeftX) || LeftMostX > ((s32)DrawArea_BottomRightX) || y2 <= ((s32)DrawArea_TopLeftY) || y0 > ((s32)DrawArea_BottomRightY) ) return 0;
		
		// skip drawing if distance between vertices is greater than max allowed by GPU
		if ( ( _Abs( x1 - x0 ) > c_MaxPolygonWidth ) || ( _Abs( x2 - x1 ) > c_MaxPolygonWidth ) || ( y1 - y0 > c_MaxPolygonHeight ) || ( y2 - y1 > c_MaxPolygonHeight ) )
		{
			// skip drawing polygon
			return 0;
		}
		
		/////////////////////////////////////////////////
		// draw top part of triangle
		
		// denominator is negative when x1 is on the left, positive when x1 is on the right
		t0 = y1 - y2;
		t1 = y0 - y2;
		denominator = ( ( x0 - x2 ) * t0 ) - ( ( x1 - x2 ) * t1 );

		NumPixels = _Abs ( denominator ) >> 1;
		if ( ( !ulThreadNum ) && _GPU->ulNumberOfThreads )
		{
			return NumPixels;
		}
		
		if ( _GPU->bEnable_OpenCL )
		{
			return NumPixels;
		}

		
		gbgr [ 0 ] = inputdata [ 7 ].Value & 0x00ffffff;
		gbgr [ 1 ] = inputdata [ 10 ].Value & 0x00ffffff;
		gbgr [ 2 ] = inputdata [ 13 ].Value & 0x00ffffff;
		
		gu [ 0 ] = inputdata [ 9 ].u;
		gu [ 1 ] = inputdata [ 12 ].u;
		gu [ 2 ] = inputdata [ 15 ].u;
		gv [ 0 ] = inputdata [ 9 ].v;
		gv [ 1 ] = inputdata [ 12 ].v;
		gv [ 2 ] = inputdata [ 15 ].v;
		
		//GPU_CTRL_Read_ABR = ( GPU_CTRL_Read >> 5 ) & 3;
		Command_ABE = inputdata [ 7 ].Command & 2;
		Command_TGE = inputdata [ 7 ].Command & 1;
		
		//if ( ( bgr & 0x00ffffff ) == 0x00808080 ) Command_TGE = 1;

		// bits 0-5 in upper halfword
		clut_x = ( inputdata [ 9 ].Value >> ( 16 + 0 ) ) & 0x3f;
		clut_y = ( inputdata [ 9 ].Value >> ( 16 + 6 ) ) & 0x1ff;

		TWY = ( inputdata [ 4 ].Value >> 15 ) & 0x1f;
		TWX = ( inputdata [ 4 ].Value >> 10 ) & 0x1f;
		TWH = ( inputdata [ 4 ].Value >> 5 ) & 0x1f;
		TWW = inputdata [ 4 ].Value & 0x1f;

		
		// DTD is bit 9 in GPU_CTRL_Read
		GPU_CTRL_Read_DTD = ( GPU_CTRL_Read >> 9 ) & 1;
		
		// ME is bit 12
		//if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
		PixelMask = ( GPU_CTRL_Read & 0x1000 ) << 3;
		
		// MD is bit 11
		//if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;
		SetPixelMask = ( GPU_CTRL_Read & 0x0800 ) << 4;
		
		
		// initialize number of pixels drawn
		//NumberOfPixelsDrawn = 0;
		
		// bits 0-3
		//tpage_tx = GPU_CTRL_Read & 0xf;
		tpage_tx = ( inputdata [ 12 ].Value >> ( 16 + 0 ) ) & 0xf;
		
		// bit 4
		//tpage_ty = ( GPU_CTRL_Read >> 4 ) & 1
		tpage_ty = ( inputdata [ 12 ].Value >> ( 16 + 4 ) ) & 1;
		
		// bits 5-6
		//GPU_CTRL_Read_ABR = ( GPU_CTRL_Read >> 5 ) & 3;
		tpage_abr = ( inputdata [ 12 ].Value >> ( 16 + 5 ) ) & 3;
		
		// bits 7-8
		//tpage_tp = ( GPU_CTRL_Read >> 7 ) & 3;
		tpage_tp = ( inputdata [ 12 ].Value >> ( 16 + 7 ) ) & 3;
		
		Shift1 = 0;
		Shift2 = 0;
		And1 = 0;
		And2 = 0;


		TWYTWH = ( ( TWY & TWH ) << 3 );
		TWXTWW = ( ( TWX & TWW ) << 3 );
		
		Not_TWH = ~( TWH << 3 );
		Not_TWW = ~( TWW << 3 );

		
		
		/////////////////////////////////////////////////////////
		// Get offset into texture page
		TextureOffset = ( tpage_tx << 6 ) + ( ( tpage_ty << 8 ) << 10 );
		
		clut_xoffset = clut_x << 4;
		
		if ( tpage_tp == 0 )
		{
			And2 = 0xf;
			
			Shift1 = 2; Shift2 = 2;
			And1 = 3; And2 = 0xf;
		}
		else if ( tpage_tp == 1 )
		{
			And2 = 0xff;
			
			Shift1 = 1; Shift2 = 3;
			And1 = 1; And2 = 0xff;
		}
		
		
		// get u,v coords
		u0 = gu [ Coord0 ];
		u1 = gu [ Coord1 ];
		u2 = gu [ Coord2 ];
		v0 = gv [ Coord0 ];
		v1 = gv [ Coord1 ];
		v2 = gv [ Coord2 ];

		// get rgb-values
		r0 = gbgr [ Coord0 ] & 0xff;
		r1 = gbgr [ Coord1 ] & 0xff;
		r2 = gbgr [ Coord2 ] & 0xff;
		g0 = ( gbgr [ Coord0 ] >> 8 ) & 0xff;
		g1 = ( gbgr [ Coord1 ] >> 8 ) & 0xff;
		g2 = ( gbgr [ Coord2 ] >> 8 ) & 0xff;
		b0 = ( gbgr [ Coord0 ] >> 16 ) & 0xff;
		b1 = ( gbgr [ Coord1 ] >> 16 ) & 0xff;
		b2 = ( gbgr [ Coord2 ] >> 16 ) & 0xff;
		// ?? convert to 16-bit ?? (or should leave 24-bit?)
		//bgr = gbgr [ 0 ];
		//bgr = ( ( bgr >> 9 ) & 0x7c00 ) | ( ( bgr >> 6 ) & 0x3e0 ) | ( ( bgr >> 3 ) & 0x1f );
		
		//color_add = bgr;
		
		if ( denominator )
		{
			dR_across = ( ( (s32) ( ( ( r0 - r2 ) * t0 ) - ( ( r1 - r2 ) * t1 ) ) ) << 8 ) / denominator;
			dG_across = ( ( (s32) ( ( ( g0 - g2 ) * t0 ) - ( ( g1 - g2 ) * t1 ) ) ) << 8 ) / denominator;
			dB_across = ( ( (s32) ( ( ( b0 - b2 ) * t0 ) - ( ( b1 - b2 ) * t1 ) ) ) << 8 ) / denominator;
			
			dU_across = ( ( (s32) ( ( ( u0 - u2 ) * t0 ) - ( ( u1 - u2 ) * t1 ) ) ) << 8 ) / denominator;
			dV_across = ( ( (s32) ( ( ( v0 - v2 ) * t0 ) - ( ( v1 - v2 ) * t1 ) ) ) << 8 ) / denominator;
			
			dR_across <<= 8;
			dG_across <<= 8;
			dB_across <<= 8;
			
			dU_across <<= 8;
			dV_across <<= 8;
			
			//printf ( "dR_across=%x top=%i bottom=%i divide=%x", dR_across, ( ( ( r0 - r2 ) * t0 ) - ( ( r1 - r2 ) * t1 ) ), denominator, ( ( ( ( r0 - r2 ) * t0 ) - ( ( r1 - r2 ) * t1 ) ) << 16 )/denominator );
			//printf ( "dG_across=%x top=%i bottom=%i divide=%x", dG_across, ( ( ( g0 - g2 ) * t0 ) - ( ( g1 - g2 ) * t1 ) ), denominator, ( ( ( ( g0 - g2 ) * t0 ) - ( ( g1 - g2 ) * t1 ) ) << 16 )/denominator );
		}
		
		
		
		
		//if ( denominator < 0 )
		//{
			// x1 is on the left and x0 is on the right //
			
			////////////////////////////////////
			// get slopes
			
			if ( y1 - y0 )
			{
				/////////////////////////////////////////////
				// init x on the left and right
				x_left = ( x0 << 16 );
				x_right = x_left;
				
				R_left = ( r0 << 16 );
				G_left = ( g0 << 16 );
				B_left = ( b0 << 16 );

				U_left = ( u0 << 16 );
				V_left = ( v0 << 16 );
				
				if ( denominator < 0 )
				{
					//dx_left = ( ((s64)( x1 - x0 )) * r10 ) >> 32;
					//dx_right = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
					dx_left = ( ( x1 - x0 ) << 16 ) / ( y1 - y0 );
					dx_right = ( ( x2 - x0 ) << 16 ) / ( y2 - y0 );
					//dx_left = divide_s32( ( ( x1 - x0 ) << 16 ), ( y1 - y0 ) );
					//dx_right = divide_s32( ( ( x2 - x0 ) << 16 ), ( y2 - y0 ) );
					
					dR_left = (( r1 - r0 ) << 16 ) / ( y1 - y0 );
					dG_left = (( g1 - g0 ) << 16 ) / ( y1 - y0 );
					dB_left = (( b1 - b0 ) << 16 ) / ( y1 - y0 );
					//dR_left = divide_s32( (( r1 - r0 ) << 16 ), ( y1 - y0 ) );
					//dG_left = divide_s32( (( g1 - g0 ) << 16 ), ( y1 - y0 ) );
					//dB_left = divide_s32( (( b1 - b0 ) << 16 ), ( y1 - y0 ) );
					
					dU_left = ( (( u1 - u0 ) << 16 ) ) / ( y1 - y0 );
					dV_left = ( (( v1 - v0 ) << 16 ) ) / ( y1 - y0 );
				}
				else
				{
					//dx_right = ( ((s64)( x1 - x0 )) * r10 ) >> 32;
					//dx_left = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
					dx_right = ( ( x1 - x0 ) << 16 ) / ( y1 - y0 );
					dx_left = ( ( x2 - x0 ) << 16 ) / ( y2 - y0 );
					//dx_right = divide_s32( ( ( x1 - x0 ) << 16 ), ( y1 - y0 ) );
					//dx_left = divide_s32( ( ( x2 - x0 ) << 16 ), ( y2 - y0 ) );
					
					dR_left = (( r2 - r0 ) << 16 ) / ( y2 - y0 );
					dG_left = (( g2 - g0 ) << 16 ) / ( y2 - y0 );
					dB_left = (( b2 - b0 ) << 16 ) / ( y2 - y0 );
					//dR_left = divide_s32( (( r2 - r0 ) << 16 ), ( y2 - y0 ) );
					//dG_left = divide_s32( (( g2 - g0 ) << 16 ), ( y2 - y0 ) );
					//dB_left = divide_s32( (( b2 - b0 ) << 16 ), ( y2 - y0 ) );
					
					dU_left = ( (( u2 - u0 ) << 16 ) ) / ( y2 - y0 );
					dV_left = ( (( v2 - v0 ) << 16 ) ) / ( y2 - y0 );
				}
			}
			else
			{
				if ( denominator < 0 )
				{
					// change x_left and x_right where y1 is on left
					x_left = ( x1 << 16 );
					x_right = ( x0 << 16 );
					
					R_left = ( r1 << 16 );
					G_left = ( g1 << 16 );
					B_left = ( b1 << 16 );

					U_left = ( u1 << 16 );
					V_left = ( v1 << 16 );
					
					if ( y2 - y1 )
					{
						//dx_left = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
						//dx_right = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
						dx_left = ( ( x2 - x1 ) << 16 ) / ( y2 - y1 );
						dx_right = ( ( x2 - x0 ) << 16 ) / ( y2 - y0 );
						//dx_left = divide_s32( ( ( x2 - x1 ) << 16 ), ( y2 - y1 ) );
						//dx_right = divide_s32( ( ( x2 - x0 ) << 16 ), ( y2 - y0 ) );
						
						dR_left = (( r2 - r1 ) << 16 ) / ( y2 - y1 );
						dG_left = (( g2 - g1 ) << 16 ) / ( y2 - y1 );
						dB_left = (( b2 - b1 ) << 16 ) / ( y2 - y1 );
						//dR_left = divide_s32( (( r2 - r1 ) << 16 ), ( y2 - y1 ) );
						//dG_left = divide_s32( (( g2 - g1 ) << 16 ), ( y2 - y1 ) );
						//dB_left = divide_s32( (( b2 - b1 ) << 16 ), ( y2 - y1 ) );
						
						dU_left = ( (( u2 - u1 ) << 16 ) ) / ( y2 - y1 );
						dV_left = ( (( v2 - v1 ) << 16 ) ) / ( y2 - y1 );
					}
				}
				else
				{
					x_right = ( x1 << 16 );
					x_left = ( x0 << 16 );
				
					R_left = ( r0 << 16 );
					G_left = ( g0 << 16 );
					B_left = ( b0 << 16 );
					
					U_left = ( u0 << 16 );
					V_left = ( v0 << 16 );
					
					if ( y2 - y1 )
					{
						//dx_right = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
						//dx_left = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
						dx_right = ( ( x2 - x1 ) << 16 ) / ( y2 - y1 );
						dx_left = ( ( x2 - x0 ) << 16 ) / ( y2 - y0 );
						//dx_right = divide_s32( ( ( x2 - x1 ) << 16 ), ( y2 - y1 ) );
						//dx_left = divide_s32( ( ( x2 - x0 ) << 16 ), ( y2 - y0 ) );
						
						dR_left = (( r2 - r0 ) << 16 ) / ( y2 - y0 );
						dG_left = (( g2 - g0 ) << 16 ) / ( y2 - y0 );
						dB_left = (( b2 - b0 ) << 16 ) / ( y2 - y0 );
						//dR_left = divide_s32( (( r2 - r0 ) << 16 ), ( y2 - y0 ) );
						//dG_left = divide_s32( (( g2 - g0 ) << 16 ), ( y2 - y0 ) );
						//dB_left = divide_s32( (( b2 - b0 ) << 16 ), ( y2 - y0 ) );
						
						dU_left = ( (( u2 - u0 ) << 16 ) ) / ( y2 - y0 );
						dV_left = ( (( v2 - v0 ) << 16 ) ) / ( y2 - y0 );
					}
				}
			}
		//}
		


	
		////////////////
		// *** TODO *** at this point area of full triangle can be calculated and the rest of the drawing can be put on another thread *** //
		
		
		
		// r,g,b values are not specified with a fractional part, so there must be an initial fractional part
		R_left |= ( 1 << 15 );
		G_left |= ( 1 << 15 );
		B_left |= ( 1 << 15 );

		U_left |= ( 1 << 15 );
		V_left |= ( 1 << 15 );

		//x_left += 0xffff;
		//x_right -= 1;
		
		StartY = y0;
		EndY = y1;

		if ( StartY < ((s32)DrawArea_TopLeftY) )
		{
			
			if ( EndY < ((s32)DrawArea_TopLeftY) )
			{
				Temp = EndY - StartY;
				StartY = EndY;
			}
			else
			{
				Temp = DrawArea_TopLeftY - StartY;
				StartY = DrawArea_TopLeftY;
			}
			
			x_left += dx_left * Temp;
			x_right += dx_right * Temp;
			
			R_left += dR_left * Temp;
			G_left += dG_left * Temp;
			B_left += dB_left * Temp;
			
			U_left += dU_left * Temp;
			V_left += dV_left * Temp;
		}
		
		if ( EndY > ((s32)DrawArea_BottomRightY) )
		{
			EndY = DrawArea_BottomRightY + 1;
		}

		
		// offset to get to this compute unit's scanline
		//group_yoffset = group_id - ( StartY % num_global_groups );
		//if ( group_yoffset < 0 )
		//{
		//	group_yoffset += num_global_groups;
		//}
		
		//printf( "x_left=%x x_right=%x dx_left=%i dx_right=%i R_left=%x G_left=%x B_left=%x OffsetX=%i OffsetY=%i",x_left,x_right,dx_left,dx_right,R_left,G_left,B_left, DrawArea_OffsetX, DrawArea_OffsetY );
		//printf( "x0=%i y0=%i x1=%i y1=%i x2=%i y2=%i r0=%i r1=%i r2=%i g0=%i g1=%i g2=%i b0=%i b1=%i b2=%i", x0, y0, x1, y1, x2, y2, r0, r1, r2, g0, g1, g2, b0, b1, b2 );
		//printf( "dR_across=%x dG_across=%x dB_across=%x", dR_across, dG_across, dB_across );

	//}	// end if ( !local_id )
	
	

	// synchronize variables across workers
	//barrier ( CLK_LOCAL_MEM_FENCE );

	


	
	
	ptr_clut = & ( _GPU->VRAM [ clut_y << 10 ] );
	ptr_texture = & ( _GPU->VRAM [ ( tpage_tx << 6 ) + ( ( tpage_ty << 8 ) << 10 ) ] );

	



	
	/////////////////////////////////////////////
	// init x on the left and right
	
	


	if ( EndY > StartY )
	{
	
	// in opencl, each worker could be on a different line
	xoff_left = x_left + ( dx_left * (group_yoffset + yid) );
	xoff_right = x_right + ( dx_right * (group_yoffset + yid) );
	
	Roff_left = R_left + ( dR_left * (group_yoffset + yid) );
	Goff_left = G_left + ( dG_left * (group_yoffset + yid) );
	Boff_left = B_left + ( dB_left * (group_yoffset + yid) );
	
	Uoff_left = U_left + ( dU_left * (group_yoffset + yid) );
	Voff_left = V_left + ( dV_left * (group_yoffset + yid) );
	
//debug << "\r\nDrawing Top= " << hex << color_add << " " << Command_TGE;
	//////////////////////////////////////////////
	// draw down to y1
	//for ( Line = StartY; Line < EndY; Line++ )
	for ( Line = StartY + group_yoffset + yid; Line < EndY; Line += yinc )
	{
		// left point is included if points are equal
		StartX = ( xoff_left + 0xffffLL ) >> 16;
		EndX = ( xoff_right - 1 ) >> 16;
		//StartX = xoff_left >> 16;
		//EndX = xoff_right >> 16;
		
//debug << "\r\nStartX=" << dec << StartX << " EndX=" << EndX << " Line=" << Line << " StartY=" << StartY << " EndY=" << EndY;
		
		if ( StartX <= ((s32)DrawArea_BottomRightX) && EndX >= ((s32)DrawArea_TopLeftX) && EndX >= StartX )
		{
			iR = Roff_left;
			iG = Goff_left;
			iB = Boff_left;
			
			iU = Uoff_left;
			iV = Voff_left;
			
			// get the difference between x_left and StartX
			Temp = ( StartX << 16 ) - xoff_left;
			
			if ( StartX < ((s32)DrawArea_TopLeftX) )
			{
				Temp += ( DrawArea_TopLeftX - StartX ) << 16;
				StartX = DrawArea_TopLeftX;
				
			}
			
			iR += ( dR_across >> 8 ) * ( Temp >> 8 );
			iG += ( dG_across >> 8 ) * ( Temp >> 8 );
			iB += ( dB_across >> 8 ) * ( Temp >> 8 );
			
			iU += ( dU_across >> 8 ) * ( Temp >> 8 );
			iV += ( dV_across >> 8 ) * ( Temp >> 8 );
			
			if ( EndX > ((s32)DrawArea_BottomRightX) )
			{
				//EndX = DrawArea_BottomRightX + 1;
				EndX = DrawArea_BottomRightX;
			}
			
			ptr = & ( _GPU->VRAM [ StartX + xid + ( Line << 10 ) ] );
			//DitherLine = & ( DitherArray [ ( Line & 0x3 ) << 2 ] );
			
			
			iR += ( dR_across * xid );
			iG += ( dG_across * xid );
			iB += ( dB_across * xid );

			iU += ( dU_across * xid );
			iV += ( dV_across * xid );
			
			// draw horizontal line
			// x_left and x_right need to be rounded off
			//for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize )
			for ( x_across = StartX + xid; x_across <= EndX; x_across += xinc )
			{
				TexCoordY = (u8) ( ( ( iV >> 16 ) & Not_TWH ) | ( TWYTWH ) );
				TexCoordY <<= 10;

				//TexCoordX = (u8) ( ( iU & ~( TWW << 3 ) ) | ( ( TWX & TWW ) << 3 ) );
				TexCoordX = (u8) ( ( ( iU >> 16 ) & Not_TWW ) | ( TWXTWW ) );
				
				//bgr = ptr_texture [ ( TexCoordX >> Shift1 ) + ( TexCoordY << 10 ) ];
				bgr = ptr_texture [ ( TexCoordX >> Shift1 ) + TexCoordY ];
				
				if ( Shift1 )
				{
					//bgr = VRAM [ ( ( ( clut_x << 4 ) + TexelIndex ) & FrameBuffer_XMask ) + ( clut_y << 10 ) ];
					bgr = ptr_clut [ ( clut_xoffset + ( ( bgr >> ( ( TexCoordX & And1 ) << Shift2 ) ) & And2 ) ) & FrameBuffer_XMask ];
				}

//debug << "\r\nx_across=" << dec << x_across << " TexCoordX=" << TexCoordX << " TexCoordY=" << TexCoordY << " bgr=" << hex << bgr;				
				if ( bgr )
				{
					
					// shade pixel color
					
					// read pixel from frame buffer if we need to check mask bit
					DestPixel = *ptr;
					
					
					bgr_temp = bgr;
		
					if ( !Command_TGE )
					{
						if ( GPU_CTRL_Read_DTD )
						{
							DitherValue = c_iDitherValues16 [ ( x_across & 3 ) + ( ( Line & 3 ) << 2 ) ];
							
							// perform dither
							Red = iR + DitherValue;
							Green = iG + DitherValue;
							Blue = iB + DitherValue;
							
							// perform shift
							Red >>= ( 16 + 0 );
							Green >>= ( 16 + 0 );
							Blue >>= ( 16 + 0 );
							
							//Red = clamp ( Red, 0, 0x1f );
							//Green = clamp ( Green, 0, 0x1f );
							//Blue = clamp ( Blue, 0, 0x1f );
							Red = Clamp8 ( Red );
							Green = Clamp8 ( Green );
							Blue = Clamp8 ( Blue );
						}
						else
						{
							Red = iR >> ( 16 + 0 );
							Green = iG >> ( 16 + 0 );
							Blue = iB >> ( 16 + 0 );
						}
						
						color_add = ( Blue << 16 ) | ( Green << 8 ) | Red;
//debug << "\r\nColorMultiply1624 Top= " << hex << color_add;
						// brightness calculation
						//bgr_temp = Color24To16 ( ColorMultiply24 ( Color16To24 ( bgr_temp ), color_add ) );
						bgr_temp = ColorMultiply1624 ( bgr_temp, color_add );
					}
					
					// semi-transparency
					if ( Command_ABE && ( bgr & 0x8000 ) )
					{
//debug << "\r\SemiTransparency16 Bottom";
						bgr_temp = SemiTransparency16 ( DestPixel, bgr_temp, tpage_abr );
					}
					
					// check if we should set mask bit when drawing
					//bgr_temp |= SetPixelMask | ( bgr & 0x8000 );

					// draw pixel if we can draw to mask pixels or mask bit not set
					if ( ! ( DestPixel & PixelMask ) ) *ptr = bgr_temp | SetPixelMask | ( bgr & 0x8000 );
					
				}
						
				iR += ( dR_across * xinc );
				iG += ( dG_across * xinc );
				iB += ( dB_across * xinc );
			
				iU += ( dU_across * xinc );
				iV += ( dV_across * xinc );
					
				//ptr += c_iVectorSize;
				ptr += xinc;
			}
			
		}
		
		
		/////////////////////////////////////
		// update x on left and right
		xoff_left += ( dx_left * yinc );
		xoff_right += ( dx_right * yinc );
		
		Roff_left += ( dR_left * yinc );
		Goff_left += ( dG_left * yinc );
		Boff_left += ( dB_left * yinc );
		
		Uoff_left += ( dU_left * yinc );
		Voff_left += ( dV_left * yinc );
	}

	} // end if ( EndY > StartY )

	
	// synchronize variables across workers
	//barrier ( CLK_LOCAL_MEM_FENCE );

	
	////////////////////////////////////////////////
	// draw bottom part of triangle

	/////////////////////////////////////////////
	// init x on the left and right
	
	//if ( !local_id )
	//{
		//////////////////////////////////////////////////////
		// check if y1 is on the left or on the right
		if ( denominator < 0 )
		{
			x_left = ( x1 << 16 );

			x_right = ( x0 << 16 ) + ( dx_right * ( y1 - y0 ) );
			
			R_left = ( r1 << 16 );
			G_left = ( g1 << 16 );
			B_left = ( b1 << 16 );
			
			U_left = ( u1 << 16 );
			V_left = ( v1 << 16 );
			
			if ( y2 - y1 )
			{
				//dx_left = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
				dx_left = (( x2 - x1 ) << 16 ) / ( y2 - y1 );
				//dx_left = divide_s32( (( x2 - x1 ) << 16 ), ( y2 - y1 ) );
				
				//dR_left = ( ((s64)( r2 - r1 )) * r21 ) >> 24;
				//dG_left = ( ((s64)( g2 - g1 )) * r21 ) >> 24;
				//dB_left = ( ((s64)( b2 - b1 )) * r21 ) >> 24;
				dR_left = (( r2 - r1 ) << 16 ) / ( y2 - y1 );
				dG_left = (( g2 - g1 ) << 16 ) / ( y2 - y1 );
				dB_left = (( b2 - b1 ) << 16 ) / ( y2 - y1 );
				//dR_left = divide_s32( (( r2 - r1 ) << 16 ), ( y2 - y1 ) );
				//dG_left = divide_s32( (( g2 - g1 ) << 16 ), ( y2 - y1 ) );
				//dB_left = divide_s32( (( b2 - b1 ) << 16 ), ( y2 - y1 ) );
				
				dU_left = ( (( u2 - u1 ) << 16 ) ) / ( y2 - y1 );
				dV_left = ( (( v2 - v1 ) << 16 ) ) / ( y2 - y1 );
			}
		}
		else
		{
			x_right = ( x1 << 16 );

			x_left = ( x0 << 16 ) + ( dx_left * ( y1 - y0 ) );
			
			R_left = ( r0 << 16 ) + ( dR_left * ( y1 - y0 ) );
			G_left = ( g0 << 16 ) + ( dG_left * ( y1 - y0 ) );
			B_left = ( b0 << 16 ) + ( dB_left * ( y1 - y0 ) );
			
			U_left = ( u0 << 16 ) + ( dU_left * ( y1 - y0 ) );
			V_left = ( v0 << 16 ) + ( dV_left * ( y1 - y0 ) );
			
			if ( y2 - y1 )
			{
				//dx_right = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
				dx_right = (( x2 - x1 ) << 16 ) / ( y2 - y1 );
				//dx_right = divide_s32( (( x2 - x1 ) << 16 ), ( y2 - y1 ) );
				
			}
		}


		R_left += ( 1 << 15 );
		G_left += ( 1 << 15 );
		B_left += ( 1 << 15 );

		U_left += ( 1 << 15 );
		V_left += ( 1 << 15 );
		

		//x_left += 0xffff;
		//x_right -= 1;
		
		StartY = y1;
		EndY = y2;

		if ( StartY < ((s32)DrawArea_TopLeftY) )
		{
			
			if ( EndY < ((s32)DrawArea_TopLeftY) )
			{
				Temp = EndY - StartY;
				StartY = EndY;
			}
			else
			{
				Temp = DrawArea_TopLeftY - StartY;
				StartY = DrawArea_TopLeftY;
			}
			
			x_left += dx_left * Temp;
			x_right += dx_right * Temp;
			
			R_left += dR_left * Temp;
			G_left += dG_left * Temp;
			B_left += dB_left * Temp;
			
			U_left += dU_left * Temp;
			V_left += dV_left * Temp;
		}
		
		if ( EndY > ((s32)DrawArea_BottomRightY) )
		{
			EndY = DrawArea_BottomRightY + 1;
		}
		
		// offset to get to this compute unit's scanline
		//group_yoffset = group_id - ( StartY % num_global_groups );
		//if ( group_yoffset < 0 )
		//{
		//	group_yoffset += num_global_groups;
		//}
	//}

	
	// synchronize variables across workers
	//barrier ( CLK_LOCAL_MEM_FENCE );

	
	if ( EndY > StartY )
	{
	
	// in opencl, each worker could be on a different line
	xoff_left = x_left + ( dx_left * (group_yoffset + yid) );
	xoff_right = x_right + ( dx_right * (group_yoffset + yid) );
	
	Roff_left = R_left + ( dR_left * (group_yoffset + yid) );
	Goff_left = G_left + ( dG_left * (group_yoffset + yid) );
	Boff_left = B_left + ( dB_left * (group_yoffset + yid) );
	
	Uoff_left = U_left + ( dU_left * (group_yoffset + yid) );
	Voff_left = V_left + ( dV_left * (group_yoffset + yid) );
	
//debug << "\r\nDrawing Bottom= " << hex << color_add << " " << Command_TGE;
	//////////////////////////////////////////////
	// draw down to y2
	//for ( Line = StartY; Line < EndY; Line++ )
	for ( Line = StartY + group_yoffset + yid; Line < EndY; Line += yinc )
	{
		
		// left point is included if points are equal
		StartX = ( xoff_left + 0xffffLL ) >> 16;
		EndX = ( xoff_right - 1 ) >> 16;
		//StartX = xoff_left >> 16;
		//EndX = xoff_right >> 16;
		
		
		if ( StartX <= ((s32)DrawArea_BottomRightX) && EndX >= ((s32)DrawArea_TopLeftX) && EndX >= StartX )
		{
			iR = Roff_left;
			iG = Goff_left;
			iB = Boff_left;
			
			iU = Uoff_left;
			iV = Voff_left;
			
			// get the difference between x_left and StartX
			Temp = ( StartX << 16 ) - xoff_left;
			
			if ( StartX < ((s32)DrawArea_TopLeftX) )
			{
				Temp += ( DrawArea_TopLeftX - StartX ) << 16;
				StartX = DrawArea_TopLeftX;
				
			}
			
			iR += ( dR_across >> 8 ) * ( Temp >> 8 );
			iG += ( dG_across >> 8 ) * ( Temp >> 8 );
			iB += ( dB_across >> 8 ) * ( Temp >> 8 );
			
			iU += ( dU_across >> 8 ) * ( Temp >> 8 );
			iV += ( dV_across >> 8 ) * ( Temp >> 8 );
			
			if ( EndX > ((s32)DrawArea_BottomRightX) )
			{
				//EndX = DrawArea_BottomRightX + 1;
				EndX = DrawArea_BottomRightX;
			}
			
			ptr = & ( _GPU->VRAM [ StartX + xid + ( Line << 10 ) ] );
			//DitherLine = & ( DitherArray [ ( Line & 0x3 ) << 2 ] );
			
			
			/////////////////////////////////////////////////////
			// update number of cycles used to draw polygon
			//NumberOfPixelsDrawn += EndX - StartX + 1;
			
			iR += ( dR_across * xid );
			iG += ( dG_across * xid );
			iB += ( dB_across * xid );

			iU += ( dU_across * xid );
			iV += ( dV_across * xid );
			
			// draw horizontal line
			// x_left and x_right need to be rounded off
			//for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize )
			for ( x_across = StartX + xid; x_across <= EndX; x_across += xinc )
			{
				TexCoordY = (u8) ( ( ( iV >> 16 ) & Not_TWH ) | ( TWYTWH ) );
				TexCoordY <<= 10;

				//TexCoordX = (u8) ( ( iU & ~( TWW << 3 ) ) | ( ( TWX & TWW ) << 3 ) );
				TexCoordX = (u8) ( ( ( iU >> 16 ) & Not_TWW ) | ( TWXTWW ) );
				
				//bgr = ptr_texture [ ( TexCoordX >> Shift1 ) + ( TexCoordY << 10 ) ];
				bgr = ptr_texture [ ( TexCoordX >> Shift1 ) + TexCoordY ];
				
				if ( Shift1 )
				{
					//bgr = VRAM [ ( ( ( clut_x << 4 ) + TexelIndex ) & FrameBuffer_XMask ) + ( clut_y << 10 ) ];
					bgr = ptr_clut [ ( clut_xoffset + ( ( bgr >> ( ( TexCoordX & And1 ) << Shift2 ) ) & And2 ) ) & FrameBuffer_XMask ];
				}

				
				if ( bgr )
				{
					
					// shade pixel color
					
					// read pixel from frame buffer if we need to check mask bit
					DestPixel = *ptr;
					
					
					bgr_temp = bgr;
		
					
					if ( !Command_TGE )
					{
						if ( GPU_CTRL_Read_DTD )
						{
							DitherValue = c_iDitherValues16 [ ( x_across & 3 ) + ( ( Line & 3 ) << 2 ) ];
							
							// perform dither
							Red = iR + DitherValue;
							Green = iG + DitherValue;
							Blue = iB + DitherValue;
							
							// perform shift
							Red >>= ( 16 + 0 );
							Green >>= ( 16 + 0 );
							Blue >>= ( 16 + 0 );
							
							//Red = clamp ( Red, 0, 0x1f );
							//Green = clamp ( Green, 0, 0x1f );
							//Blue = clamp ( Blue, 0, 0x1f );
							Red = Clamp8 ( Red );
							Green = Clamp8 ( Green );
							Blue = Clamp8 ( Blue );
						}
						else
						{
							Red = iR >> ( 16 + 0 );
							Green = iG >> ( 16 + 0 );
							Blue = iB >> ( 16 + 0 );
						}
						
						color_add = ( Blue << 16 ) | ( Green << 8 ) | Red;
//debug << "\r\nColorMultiply1624 Bottom= " << hex << color_add;
						// brightness calculation
						//bgr_temp = Color24To16 ( ColorMultiply24 ( Color16To24 ( bgr_temp ), color_add ) );
						bgr_temp = ColorMultiply1624 ( bgr_temp, color_add );
					}
					
					// semi-transparency
					if ( Command_ABE && ( bgr & 0x8000 ) )
					{
//debug << "\r\SemiTransparency16 Bottom";
						bgr_temp = SemiTransparency16 ( DestPixel, bgr_temp, tpage_abr );
					}
					
					// check if we should set mask bit when drawing
					//bgr_temp |= SetPixelMask | ( bgr & 0x8000 );

					// draw pixel if we can draw to mask pixels or mask bit not set
					if ( ! ( DestPixel & PixelMask ) ) *ptr = bgr_temp | SetPixelMask | ( bgr & 0x8000 );
					
				}

					
				iR += ( dR_across * xinc );
				iG += ( dG_across * xinc );
				iB += ( dB_across * xinc );
				
				iU += ( dU_across * xinc );
				iV += ( dV_across * xinc );
				
				//ptr += c_iVectorSize;
				ptr += xinc;
			}
			
		}
		
		/////////////////////////////////////
		// update x on left and right
		xoff_left += ( dx_left * yinc );
		xoff_right += ( dx_right * yinc );
		
		Roff_left += ( dR_left * yinc );
		Goff_left += ( dG_left * yinc );
		Boff_left += ( dB_left * yinc );
		
		Uoff_left += ( dU_left * yinc );
		Voff_left += ( dV_left * yinc );
	}
	
	} // end if ( EndY > StartY )
		
	return NumPixels;
}



u64 GPU::Draw_Rectangle_60_th ( DATA_Write_Format* inputdata, u32 ulThreadNum )
{
	const int local_id = 0;
	const int group_id = 0;
	const int num_local_threads = 1;
	const int num_global_groups = 1;
	
//#ifdef SINGLE_SCANLINE_MODE
	const int xid = local_id;
	const int yid = 0;
	
	const int xinc = num_local_threads;
	const int yinc = num_global_groups;
	s32 group_yoffset = 0;
//#endif

//inputdata format:
//0: GPU_CTRL_Read
//1: DrawArea_BottomRightX
//2: DrawArea_TopLeftX
//3: DrawArea_BottomRightY
//4: DrawArea_TopLeftY
//5: DrawArea_OffsetX
//6: DrawArea_OffsetY
//----------------
//0: GPU_CTRL_Read
//1: DrawArea_TopLeft
//2: DrawArea_BottomRight
//3: DrawArea_Offset
//4: (TextureWindow)(not used here)
//5: ------------
//6: ------------
//7: GetBGR24 ( Buffer [ 0 ] );
//8: GetXY ( Buffer [ 1 ] );
//9: GetCLUT ( Buffer [ 2 ] );
//9: GetUV ( Buffer [ 2 ] );
//10: GetHW ( Buffer [ 3 ] );

	//u32 Pixel;
	
	u32 GPU_CTRL_Read, GPU_CTRL_Read_ABR;
	s32 DrawArea_BottomRightX, DrawArea_TopLeftX, DrawArea_BottomRightY, DrawArea_TopLeftY;
	s32 DrawArea_OffsetX, DrawArea_OffsetY;
	u32 Command_ABE;
	
	s32 StartX, EndX, StartY, EndY;
	//u32 PixelsPerLine;
	
	// new variables
	s32 x, y;
	u32 w, h;
	s32 x0, x1, y0, y1;
	u32 bgr;
	
	u32 PixelMask, SetPixelMask;
	
	//s32 group_yoffset;
	
	u16 *ptr;
	
	u32 DestPixel;
	u32 bgr_temp;
	s32 x_across;
	s32 Line;
	
	u64 NumPixels;


	// set variables
	//if ( !local_id )
	//{
		// set bgr64
		//bgr64 = gbgr [ 0 ];
		//bgr64 |= ( bgr64 << 16 );
		//bgr64 |= ( bgr64 << 32 );
		bgr = inputdata [ 7 ].Value;
		bgr = ( ( bgr >> 9 ) & 0x7c00 ) | ( ( bgr >> 6 ) & 0x3e0 ) | ( ( bgr >> 3 ) & 0x1f );
		
		x = inputdata [ 8 ].x;
		y = inputdata [ 8 ].y;
		
		// x and y are actually 11 bits
		x = ( x << ( 5 + 16 ) ) >> ( 5 + 16 );
		y = ( y << ( 5 + 16 ) ) >> ( 5 + 16 );
		
		w = inputdata [ 10 ].w;
		h = inputdata [ 10 ].h;
	
		GPU_CTRL_Read = inputdata [ 0 ].Value;
		DrawArea_TopLeftX = inputdata [ 1 ].Value & 0x3ff;
		DrawArea_TopLeftY = ( inputdata [ 1 ].Value >> 10 ) & 0x3ff;
		DrawArea_BottomRightX = inputdata [ 2 ].Value & 0x3ff;
		DrawArea_BottomRightY = ( inputdata [ 2 ].Value >> 10 ) & 0x3ff;
		DrawArea_OffsetX = ( ( (s32) inputdata [ 3 ].Value ) << 21 ) >> 21;
		DrawArea_OffsetY = ( ( (s32) inputdata [ 3 ].Value ) << 10 ) >> 21;
		
		
		// get top left corner of sprite and bottom right corner of sprite
		x0 = x + DrawArea_OffsetX;
		y0 = y + DrawArea_OffsetY;
		x1 = x0 + w - 1;
		y1 = y0 + h - 1;
		
		//////////////////////////////////////////
		// get coordinates on screen
		//x0 = DrawArea_OffsetX + x0;
		//y0 = DrawArea_OffsetY + y0;
		//x1 = DrawArea_OffsetX + x1;
		//y1 = DrawArea_OffsetY + y1;
		
		// check for some important conditions
		if ( DrawArea_BottomRightX < DrawArea_TopLeftX )
		{
			//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightX < DrawArea_TopLeftX.\n";
			return 0;
		}
		
		if ( DrawArea_BottomRightY < DrawArea_TopLeftY )
		{
			//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightY < DrawArea_TopLeftY.\n";
			return 0;
		}

		// check if sprite is within draw area
		if ( x1 < ((s32)DrawArea_TopLeftX) || x0 > ((s32)DrawArea_BottomRightX) || y1 < ((s32)DrawArea_TopLeftY) || y0 > ((s32)DrawArea_BottomRightY) ) return 0;
		
		
		StartX = x0;
		EndX = x1;
		StartY = y0;
		EndY = y1;

		if ( StartY < ((s32)DrawArea_TopLeftY) )
		{
			StartY = DrawArea_TopLeftY;
		}
		
		if ( EndY > ((s32)DrawArea_BottomRightY) )
		{
			EndY = DrawArea_BottomRightY;
		}
		
		if ( StartX < ((s32)DrawArea_TopLeftX) )
		{
			StartX = DrawArea_TopLeftX;
		}
		
		if ( EndX > ((s32)DrawArea_BottomRightX) )
		{
			EndX = DrawArea_BottomRightX;
		}

		
		NumPixels = ( EndX - StartX + 1 ) * ( EndY - StartY + 1 );
		if ( ( !ulThreadNum ) && _GPU->ulNumberOfThreads )
		{
			return NumPixels;
		}
		
		if ( _GPU->bEnable_OpenCL )
		{
			return NumPixels;
		}

		
		GPU_CTRL_Read_ABR = ( GPU_CTRL_Read >> 5 ) & 3;
		Command_ABE = inputdata [ 7 ].Command & 2;
		
		// ME is bit 12
		//if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
		PixelMask = ( GPU_CTRL_Read & 0x1000 ) << 3;
		
		// MD is bit 11
		//if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;
		SetPixelMask = ( GPU_CTRL_Read & 0x0800 ) << 4;
		
		// initialize number of pixels drawn
		//NumberOfPixelsDrawn = 0;
		
		
		// get color(s)
		//bgr = gbgr [ 0 ];
		
		// ?? convert to 16-bit ?? (or should leave 24-bit?)
		//bgr = ( ( bgr & ( 0xf8 << 0 ) ) >> 3 ) | ( ( bgr & ( 0xf8 << 8 ) ) >> 6 ) | ( ( bgr & ( 0xf8 << 16 ) ) >> 9 );

		// offset to get to this compute unit's scanline
		//group_yoffset = group_id - ( StartY % num_global_groups );
		//if ( group_yoffset < 0 )
		//{
		//	group_yoffset += num_global_groups;
		//}
		
		
		//printf ( "x0=%i y0=%i x1=%i y1=%i StartX=%i StartY=%i EndX=%i EndY=%i", x0, y0, x1, y1, StartX, StartY, EndX, EndY );
	//}

	
	// synchronize variables across workers
	//barrier ( CLK_LOCAL_MEM_FENCE );
	
	
	// initialize number of pixels drawn
	//NumberOfPixelsDrawn = 0;
	
	

	
	//NumberOfPixelsDrawn = ( EndX - StartX + 1 ) * ( EndY - StartY + 1 );
	
	
	//for ( Line = StartY; Line <= EndY; Line++ )
	for ( Line = StartY + group_yoffset + yid; Line <= EndY; Line += yinc )
	{
		ptr = & ( _GPU->VRAM [ StartX + xid + ( Line << 10 ) ] );
		

		// draw horizontal line
		//for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize )
		for ( x_across = StartX + xid; x_across <= EndX; x_across += xinc )
		{
			// read pixel from frame buffer if we need to check mask bit
			DestPixel = *ptr;
			
			bgr_temp = bgr;

			// semi-transparency
			if ( Command_ABE )
			{
				bgr_temp = SemiTransparency16 ( DestPixel, bgr_temp, GPU_CTRL_Read_ABR );
			}
			
			// check if we should set mask bit when drawing
			bgr_temp |= SetPixelMask;

			// draw pixel if we can draw to mask pixels or mask bit not set
			//if ( ! ( DestPixel & PixelMask ) ) *ptr = bgr_temp;
			DestPixel = ( ! ( DestPixel & PixelMask ) ) ? bgr_temp : DestPixel;
			*ptr = DestPixel;
			
			// update pointer for pixel out
			//ptr += c_iVectorSize;
			ptr += xinc;
		}
	}
	
	// set the amount of time drawing used up
	//BusyCycles = NumberOfPixelsDrawn * 1;
	return NumPixels;
}


u64 GPU::DrawSprite_th ( DATA_Write_Format* inputdata, u32 ulThreadNum )
{
	const int local_id = 0;
	const int group_id = 0;
	const int num_local_threads = 1;
	const int num_global_groups = 1;
	
//#ifdef SINGLE_SCANLINE_MODE
	const int xid = local_id;
	const int yid = 0;
	
	const int xinc = num_local_threads;
	const int yinc = num_global_groups;
	s32 group_yoffset = 0;
//#endif


//inputdata format:
//0: GPU_CTRL_Read
//1: DrawArea_BottomRightX
//2: DrawArea_TopLeftX
//3: DrawArea_BottomRightY
//4: DrawArea_TopLeftY
//5: DrawArea_OffsetX
//6: DrawArea_OffsetY
//7: TextureWindow
//-------------------------
//0: GPU_CTRL_Read
//1: DrawArea_TopLeft
//2: DrawArea_BottomRight
//3: DrawArea_Offset
//4: TextureWindow
//5: ------------
//6: ------------
//7: GetBGR24 ( Buffer [ 0 ] );
//8: GetXY ( Buffer [ 1 ] );
//9: GetCLUT ( Buffer [ 2 ] );
//9: GetUV ( Buffer [ 2 ] );
//10: GetHW ( Buffer [ 3 ] );


	// notes: looks like sprite size is same as specified by w/h

	//u32 Pixel,

	u32 GPU_CTRL_Read, GPU_CTRL_Read_ABR;
	s32 DrawArea_BottomRightX, DrawArea_TopLeftX, DrawArea_BottomRightY, DrawArea_TopLeftY;
	s32 DrawArea_OffsetX, DrawArea_OffsetY;
	u32 Command_ABE;
	u32 Command_TGE;

	
	u32 TexelIndex;
	
	
	u16 *ptr;
	s32 StartX, EndX, StartY, EndY;
	s32 x, y, w, h;
	
	//s32 group_yoffset;
	
	u32 Temp;
	
	// new variables
	s32 x0, x1, y0, y1;
	s32 u0, v0;
	s32 u, v;
	u32 bgr, bgr_temp;
	s32 iU, iV;
	s32 x_across;
	s32 Line;
	
	u32 DestPixel;
	u32 PixelMask, SetPixelMask;

	
	u32 color_add;
	
	u16 *ptr_texture, *ptr_clut;
	u32 clut_xoffset, clut_yoffset;
	u32 clut_x, clut_y, tpage_tx, tpage_ty, tpage_abr, tpage_tp, command_tge, command_abe, command_abr;
	
	u32 TexCoordX, TexCoordY;
	u32 Shift1, Shift2, And1, And2;
	u32 TextureOffset;

	u32 TWYTWH, TWXTWW, Not_TWH, Not_TWW;
	u32 TWX, TWY, TWW, TWH;
	
	u32 NumPixels;
	

	// set variables
	//if ( !local_id )
	//{
		GPU_CTRL_Read = inputdata [ 0 ].Value;
		DrawArea_TopLeftX = inputdata [ 1 ].Value & 0x3ff;
		DrawArea_TopLeftY = ( inputdata [ 1 ].Value >> 10 ) & 0x3ff;
		DrawArea_BottomRightX = inputdata [ 2 ].Value & 0x3ff;
		DrawArea_BottomRightY = ( inputdata [ 2 ].Value >> 10 ) & 0x3ff;
		DrawArea_OffsetX = ( ( (s32) inputdata [ 3 ].Value ) << 21 ) >> 21;
		DrawArea_OffsetY = ( ( (s32) inputdata [ 3 ].Value ) << 10 ) >> 21;
		
		x = inputdata [ 8 ].x;
		y = inputdata [ 8 ].y;
		
		// x and y are actually 11 bits
		x = ( x << ( 5 + 16 ) ) >> ( 5 + 16 );
		y = ( y << ( 5 + 16 ) ) >> ( 5 + 16 );
		
		w = inputdata [ 10 ].w;
		h = inputdata [ 10 ].h;

		// get top left corner of sprite and bottom right corner of sprite
		x0 = x + DrawArea_OffsetX;
		y0 = y + DrawArea_OffsetY;
		x1 = x0 + w - 1;
		y1 = y0 + h - 1;

		u = inputdata [ 9 ].u;
		v = inputdata [ 9 ].v;
		
		// check for some important conditions
		if ( DrawArea_BottomRightX < DrawArea_TopLeftX )
		{
			//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightX < DrawArea_TopLeftX.\n";
			return 0;
		}
		
		if ( DrawArea_BottomRightY < DrawArea_TopLeftY )
		{
			//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightY < DrawArea_TopLeftY.\n";
			return 0;
		}
		
		// check if sprite is within draw area
		if ( x1 < ((s32)DrawArea_TopLeftX) || x0 > ((s32)DrawArea_BottomRightX) || y1 < ((s32)DrawArea_TopLeftY) || y0 > ((s32)DrawArea_BottomRightY) ) return 0;

		
		// get texture coords
		u0 = u;
		v0 = v;
		
		StartX = x0;
		EndX = x1;
		StartY = y0;
		EndY = y1;

		if ( StartY < ((s32)DrawArea_TopLeftY) )
		{
			v0 += ( DrawArea_TopLeftY - StartY );
			StartY = DrawArea_TopLeftY;
		}
		
		if ( EndY > ((s32)DrawArea_BottomRightY) )
		{
			EndY = DrawArea_BottomRightY;
		}
		
		if ( StartX < ((s32)DrawArea_TopLeftX) )
		{
			u0 += ( DrawArea_TopLeftX - StartX );
			StartX = DrawArea_TopLeftX;
		}
		
		if ( EndX > ((s32)DrawArea_BottomRightX) )
		{
			EndX = DrawArea_BottomRightX;
		}
		
		NumPixels = ( EndX - StartX + 1 ) * ( EndY - StartY + 1 );
		if ( ( !ulThreadNum ) && _GPU->ulNumberOfThreads )
		{
			return NumPixels;
		}
		
		if ( _GPU->bEnable_OpenCL )
		{
			return NumPixels;
		}

		
		// set bgr64
		//bgr64 = gbgr [ 0 ];
		//bgr64 |= ( bgr64 << 16 );
		//bgr64 |= ( bgr64 << 32 );
		bgr = inputdata [ 7 ].Value & 0x00ffffff;
		//bgr = ( ( bgr >> 9 ) & 0x7c00 ) | ( ( bgr >> 6 ) & 0x3e0 ) | ( ( bgr >> 3 ) & 0x1f );


		
		Command_ABE = inputdata [ 7 ].Command & 2;
		Command_TGE = inputdata [ 7 ].Command & 1;
		
		if ( ( bgr & 0x00ffffff ) == 0x00808080 ) Command_TGE = 1;

		
		
		
		// bits 0-5 in upper halfword
		clut_x = ( inputdata [ 9 ].Value >> ( 16 + 0 ) ) & 0x3f;
		clut_y = ( inputdata [ 9 ].Value >> ( 16 + 6 ) ) & 0x1ff;
	
		
		
		TWY = ( inputdata [ 4 ].Value >> 15 ) & 0x1f;
		TWX = ( inputdata [ 4 ].Value >> 10 ) & 0x1f;
		TWH = ( inputdata [ 4 ].Value >> 5 ) & 0x1f;
		TWW = inputdata [ 4 ].Value & 0x1f;

		
		// ME is bit 12
		//if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
		PixelMask = ( GPU_CTRL_Read & 0x1000 ) << 3;
		
		// MD is bit 11
		//if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;
		SetPixelMask = ( GPU_CTRL_Read & 0x0800 ) << 4;
		
		// bits 0-3
		tpage_tx = GPU_CTRL_Read & 0xf;
		
		// bit 4
		tpage_ty = ( GPU_CTRL_Read >> 4 ) & 1;
		
		// bits 5-6
		GPU_CTRL_Read_ABR = ( GPU_CTRL_Read >> 5 ) & 3;
		
		// bits 7-8
		tpage_tp = ( GPU_CTRL_Read >> 7 ) & 3;
		
		Shift1 = 0;
		Shift2 = 0;
		And1 = 0;
		And2 = 0;


		TWYTWH = ( ( TWY & TWH ) << 3 );
		TWXTWW = ( ( TWX & TWW ) << 3 );
		
		Not_TWH = ~( TWH << 3 );
		Not_TWW = ~( TWW << 3 );

		
		
		/////////////////////////////////////////////////////////
		// Get offset into texture page
		TextureOffset = ( tpage_tx << 6 ) + ( ( tpage_ty << 8 ) << 10 );
		

		
		//////////////////////////////////////////////////////
		// Get offset into color lookup table
		//u32 ClutOffset = ( clut_x << 4 ) + ( clut_y << 10 );
		
		clut_xoffset = clut_x << 4;
		
		if ( tpage_tp == 0 )
		{
			And2 = 0xf;
			
			Shift1 = 2; Shift2 = 2;
			And1 = 3; And2 = 0xf;
		}
		else if ( tpage_tp == 1 )
		{
			And2 = 0xff;
			
			Shift1 = 1; Shift2 = 3;
			And1 = 1; And2 = 0xff;
		}
		
		
		color_add = bgr;
		
		
		
		// initialize number of pixels drawn
		//NumberOfPixelsDrawn = 0;
		
		

		// offset to get to this compute unit's scanline
		//group_yoffset = group_id - ( StartY % num_global_groups );
		//if ( group_yoffset < 0 )
		//{
		//	group_yoffset += num_global_groups;
		//}
		
//printf( "StartX= %i EndX= %i StartY= %i EndY= %i x= %i y= %i w= %i h=%i", StartX, EndX, StartY, EndY, x, y, w, h );
	//}

	
	// synchronize variables across workers
	//barrier ( CLK_LOCAL_MEM_FENCE );
	

	// initialize number of pixels drawn
	//NumberOfPixelsDrawn = 0;
	
	
	
	//NumberOfPixelsDrawn = ( EndX - StartX + 1 ) * ( EndY - StartY + 1 );
		

	ptr_clut = & ( _GPU->VRAM [ clut_y << 10 ] );
	ptr_texture = & ( _GPU->VRAM [ ( tpage_tx << 6 ) + ( ( tpage_ty << 8 ) << 10 ) ] );

	
	//iV = v0;
	iV = v0 + group_yoffset + yid;

	//for ( Line = StartY; Line <= EndY; Line++ )
	for ( Line = StartY + group_yoffset + yid; Line <= EndY; Line += yinc )
	{
			// need to start texture coord from left again
			//iU = u0;
			iU = u0 + xid;

			TexCoordY = (u8) ( ( iV & Not_TWH ) | ( TWYTWH ) );
			TexCoordY <<= 10;

			//ptr = & ( VRAM [ StartX + ( Line << 10 ) ] );
			ptr = & ( _GPU->VRAM [ StartX + xid + ( Line << 10 ) ] );
			

			// draw horizontal line
			//for ( x_across = StartX; x_across <= EndX; x_across += xinc )
			for ( x_across = StartX + xid; x_across <= EndX; x_across += xinc )
			{
				//TexCoordX = (u8) ( ( iU & ~( TWW << 3 ) ) | ( ( TWX & TWW ) << 3 ) );
				TexCoordX = (u8) ( ( iU & Not_TWW ) | ( TWXTWW ) );
				
				//bgr = VRAM [ TextureOffset + ( TexCoordX >> Shift1 ) + ( TexCoordY << 10 ) ];
				//bgr = ptr_texture [ ( TexCoordX >> Shift1 ) + ( TexCoordY << 10 ) ];
				bgr = ptr_texture [ ( TexCoordX >> Shift1 ) + TexCoordY ];
				
				if ( Shift1 )
				{
					//TexelIndex = ( ( bgr >> ( ( TexCoordX & And1 ) << Shift2 ) ) & And2 );
					//bgr = VRAM [ ( ( ( clut_x << 4 ) + TexelIndex ) & FrameBuffer_XMask ) + ( clut_y << 10 ) ];
					bgr = ptr_clut [ ( clut_xoffset + ( ( bgr >> ( ( TexCoordX & And1 ) << Shift2 ) ) & And2 ) ) & FrameBuffer_XMask ];
				}

				
				if ( bgr )
				{
					// read pixel from frame buffer if we need to check mask bit
					DestPixel = *ptr;	//VRAM [ x_across + ( Line << 10 ) ];
					
					bgr_temp = bgr;
		
					if ( !Command_TGE )
					{
						// brightness calculation
						//bgr_temp = Color24To16 ( ColorMultiply24 ( Color16To24 ( bgr_temp ), color_add ) );
						bgr_temp = ColorMultiply1624 ( bgr_temp, color_add );
					}
					
					// semi-transparency
					if ( Command_ABE && ( bgr & 0x8000 ) )
					{
						bgr_temp = SemiTransparency16 ( DestPixel, bgr_temp, GPU_CTRL_Read_ABR );
					}
					
					// check if we should set mask bit when drawing
					bgr_temp |= SetPixelMask | ( bgr & 0x8000 );

					// draw pixel if we can draw to mask pixels or mask bit not set
					if ( ! ( DestPixel & PixelMask ) ) *ptr = bgr_temp;
				}
					
				/////////////////////////////////////////////////////////
				// interpolate texture coords across
				//iU += c_iVectorSize;
				iU += xinc;
				
				// update pointer for pixel out
				//ptr += c_iVectorSize;
				ptr += xinc;
					
			}
		
		/////////////////////////////////////////////////////////
		// interpolate texture coords down
		//iV++;	//+= dV_left;
		iV += yinc;
	}
	
	return NumPixels;
}



void GPU::Draw_Pixel_68_th ( DATA_Write_Format* inputdata, u32 ulThreadNum )
{
	const int local_id = 0;
	

//inputdata format:
//0: GPU_CTRL_Read
//1: DrawArea_TopLeft
//2: DrawArea_BottomRight
//3: DrawArea_Offset
//4: (TextureWindow)(not used here)
//5: ------------
//6: ------------
//7:GetBGR24 ( Buffer [ 0 ] );
//8:GetXY ( Buffer [ 1 ] );


	u32 GPU_CTRL_Read, GPU_CTRL_Read_ABR;
	s32 DrawArea_BottomRightX, DrawArea_TopLeftX, DrawArea_BottomRightY, DrawArea_TopLeftY;
	s32 DrawArea_OffsetX, DrawArea_OffsetY;
	u32 Command_ABE;
	
	u32 bgr;
	//s32 Absolute_DrawX, Absolute_DrawY;
	s32 x, y;
	
	u16* ptr16;
	
	u32 DestPixel, PixelMask;
	u32 SetPixelMask;
	
	
	//if ( !local_id )
	//{
		// no bitmaps in opencl ??
		GPU_CTRL_Read = inputdata [ 0 ].Value;
		DrawArea_TopLeftX = inputdata [ 1 ].Value & 0x3ff;
		DrawArea_TopLeftY = ( inputdata [ 1 ].Value >> 10 ) & 0x3ff;
		DrawArea_BottomRightX = inputdata [ 2 ].Value & 0x3ff;
		DrawArea_BottomRightY = ( inputdata [ 2 ].Value >> 10 ) & 0x3ff;
		DrawArea_OffsetX = ( ( (s32) inputdata [ 3 ].Value ) << 21 ) >> 21;
		DrawArea_OffsetY = ( ( (s32) inputdata [ 3 ].Value ) << 10 ) >> 21;

		x = (s32) ( ( inputdata [ 8 ].x << 5 ) >> 5 );
		y = (s32) ( ( inputdata [ 8 ].y << 5 ) >> 5 );
		
		x += DrawArea_OffsetX;
		y += DrawArea_OffsetY;

		// check for some important conditions
		if ( DrawArea_BottomRightX < DrawArea_TopLeftX )
		{
			//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightX < DrawArea_TopLeftX.\n";
			//return 0;
			return;
		}
		
		if ( DrawArea_BottomRightY < DrawArea_TopLeftY )
		{
			//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightY < DrawArea_TopLeftY.\n";
			//return 0;
			return;
		}

		if ( x < DrawArea_TopLeftX || y < DrawArea_TopLeftY || x > DrawArea_BottomRightX || y > DrawArea_BottomRightY )
		{
			//return 0;
			return;
		}
		
		if ( ( !ulThreadNum ) && _GPU->ulNumberOfThreads )
		{
			//return 1;
			return;
		}
		
		if ( _GPU->bEnable_OpenCL )
		{
			return;
		}

		
		GPU_CTRL_Read_ABR = ( GPU_CTRL_Read >> 5 ) & 3;
		Command_ABE = inputdata [ 7 ].Command & 2;
		
		// ME is bit 12
		//if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
		PixelMask = ( GPU_CTRL_Read & 0x1000 ) << 3;
		
		// MD is bit 11
		//if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;
		SetPixelMask = ( GPU_CTRL_Read & 0x0800 ) << 4;
		
		
		// ?? convert to 16-bit ?? (or should leave 24-bit?)
		bgr = inputdata [ 7 ].Value & 0x00ffffff;
		bgr = ( ( bgr & ( 0xf8 << 0 ) ) >> 3 ) | ( ( bgr & ( 0xf8 << 8 ) ) >> 6 ) | ( ( bgr & ( 0xf8 << 16 ) ) >> 9 );
		
		
		
		///////////////////////////////////////////////
		// set amount of time GPU will be busy for
		//BusyCycles += 1;
		
		/////////////////////////////////////////
		// Draw the pixel

		// make sure we are putting pixel within draw area
		//if ( x >= DrawArea_TopLeftX && y >= DrawArea_TopLeftY && x <= DrawArea_BottomRightX && y <= DrawArea_BottomRightY )
		//{
			ptr16 = & ( _GPU->VRAM [ x + ( y << 10 ) ] );
			
			
			// read pixel from frame buffer if we need to check mask bit
			//DestPixel = VRAM [ Absolute_DrawX + ( Absolute_DrawY << 10 ) ];
			DestPixel = *ptr16;
			
			// semi-transparency
			if ( Command_ABE )
			{
				bgr = SemiTransparency16 ( DestPixel, bgr, GPU_CTRL_Read_ABR );
			}
			
			// check if we should set mask bit when drawing
			//if ( GPU_CTRL_Read.MD ) bgr |= 0x8000;

			// draw pixel if we can draw to mask pixels or mask bit not set
			if ( ! ( DestPixel & PixelMask ) ) *ptr16 = bgr | SetPixelMask;
		//}
	//}
	
	//return 1;
	return;
}



u64 GPU::Draw_FrameBufferRectangle_02_th ( DATA_Write_Format* inputdata, u32 ulThreadNum )
{
	// goes at the top for opencl function

	const int local_id = 0;
	const int group_id = 0;
	const int num_local_threads = 1;
	const int num_global_groups = 1;
	
//#ifdef SINGLE_SCANLINE_MODE
	const int xid = local_id;
	const int yid = 0;
	
	const int xinc = num_local_threads;
	
	// the space between consecutive yid's
	const int yinc = num_global_groups;
	s32 group_yoffset = 0;
//#endif

	u32 NumPixels;

//inputdata format:
//0: -------
//1: -------
//2: -------
//3: -------
//4: -------
//5: -------
//6: -------
//7: GetBGR ( Buffer [ 0 ] );
//8: GetXY ( Buffer [ 1 ] );
//9: GetHW ( Buffer [ 2 ] );


	// ptr to vram
	u16 *ptr;
	//u16 *ptr16;
	u16 bgr16;
	u32 bgr32;
	
	u32 w, h, xmax, ymax, ymax2;
	s32 x, y;
	
	//s32 group_yoffset;
	
	u32 xoff, yoff;
	
	
	// set variables
	//if ( !local_id )
	//{
		w = inputdata [ 9 ].w;
		h = inputdata [ 9 ].h;
		
		// Xsiz=((Xsiz AND 3FFh)+0Fh) AND (NOT 0Fh)
		w = ( ( w & 0x3ff ) + 0xf ) & ~0xf;
		
		// Ysiz=((Ysiz AND 1FFh))
		h &= 0x1ff;
		
		NumPixels = w * h;
		if ( ( !ulThreadNum ) && _GPU->ulNumberOfThreads )
		{
			return NumPixels;
		}

		if ( _GPU->bEnable_OpenCL )
		{
			return NumPixels;
		}
		
		// set bgr64
		//bgr64 = gbgr [ 0 ];
		//bgr64 |= ( bgr64 << 16 );
		//bgr64 |= ( bgr64 << 32 );
		bgr32 = inputdata [ 7 ].Value;
		bgr16 = ( ( bgr32 >> 9 ) & 0x7c00 ) | ( ( bgr32 >> 6 ) & 0x3e0 ) | ( ( bgr32 >> 3 ) & 0x1f );
		
		x = inputdata [ 8 ].x;
		y = inputdata [ 8 ].y;
		
		// x and y are actually 11 bits
		// doesn't matter for frame buffer
		//x = ( x << 5 ) >> 5;
		//y = ( y << 5 ) >> 5;
		
		
		// Xpos=(Xpos AND 3F0h)
		x &= 0x3f0;
		
		// ypos & 0x1ff
		y &= 0x1ff;
		
	
		// adding xmax, ymax
		xmax = x + w;
		ymax = y + h;
		
		//printf( "\ninputdata= %x %x %x %x", inputdata [ 0 ].Value, inputdata [ 1 ].Value, inputdata [ 2 ].Value, inputdata [ 3 ].Value );
		//printf( "\nvram= %x %x %x %x", VRAM [ 0 ], VRAM [ 1 ], VRAM [ 2 ], VRAM [ 3 ] );
		
		ymax2 = 0;
		if ( ymax > FrameBuffer_Height )
		{
			ymax2 = ymax - FrameBuffer_Height;
			ymax = FrameBuffer_Height;
		}
		
		// offset to get to this compute unit's scanline
		group_yoffset = group_id - ( y % num_global_groups );
		if ( group_yoffset < 0 )
		{
			group_yoffset += num_global_groups;
		}
		
		//printf ( "local_id= %i num_global_groups= %i group_id= %i group_yoffset= %i yoff= %i", local_id, num_global_groups, group_id, group_yoffset, y + group_yoffset + ( yid * yinc ) );
	//}

	// synchronize variables across workers
	//barrier ( CLK_LOCAL_MEM_FENCE );

	
	// *** NOTE: coordinates wrap *** //
	
	//printf( "\nlocal_id#%i group_id= %i __local_size= %i group_max_size= %i x= %i y= %i w= %i h= %i", local_id, yinit, xinc, yinc, x, y, w, h );
	//printf( "\n__local_size= %i", xinc );
	//printf( "\ngroup_max_size= %i", yinc );
	
	
	// need to first make sure there is something to draw
	if ( h > 0 && w > 0 )
	{
		for ( yoff = y + group_yoffset + yid; yoff < ymax; yoff += yinc )
		{
			for ( xoff = x + xid; xoff < xmax; xoff += xinc )
			{
				_GPU->VRAM [ ( xoff & FrameBuffer_XMask ) + ( yoff << 10 ) ] = bgr16;
			}
		}
		
		for ( yoff = group_id + yid; yoff < ymax2; yoff += yinc )
		{
			for ( xoff = x + xid; xoff < xmax; xoff += xinc )
			{
				_GPU->VRAM [ ( xoff & FrameBuffer_XMask ) + ( yoff << 10 ) ] = bgr16;
			}
		}
	}
	
	///////////////////////////////////////////////
	// set amount of time GPU will be busy for
	//BusyCycles += (u32) ( ( (u64) h * (u64) w * dFrameBufferRectangle_02_CyclesPerPixel ) );
	
	return NumPixels;
}



u64 GPU::DrawLine_Mono_th ( DATA_Write_Format* inputdata, u32 ulThreadNum )
{
	const int local_id = 0;
	const int group_id = 0;
	const int num_local_threads = 1;
	const int num_global_groups = 1;
	
//#ifdef SINGLE_SCANLINE_MODE
	const int xid = 0;
	const int yid = 0;
	
	const int xinc = num_local_threads;
	const int yinc = num_global_groups;
	int group_yoffset = group_id;
//#endif

//inputdata format:
//0: GPU_CTRL_Read
//1: DrawArea_TopLeft
//2: DrawArea_BottomRight
//3: DrawArea_Offset
//4: (TextureWindow)(not used here)
//5: ------------
//6: ------------
//7: GetBGR24 ( Buffer [ 0 ] );
//8: GetXY0 ( Buffer [ 1 ] );
//9: GetXY1 ( Buffer [ 2 ] );
//10: GetXY2 ( Buffer [ 3 ] );



	u32 GPU_CTRL_Read, GPU_CTRL_Read_ABR;
	s32 DrawArea_BottomRightX, DrawArea_TopLeftX, DrawArea_BottomRightY, DrawArea_TopLeftY;
	s32 DrawArea_OffsetX, DrawArea_OffsetY;
	
	s32 Temp;
	s32 LeftMostX, RightMostX;
	
	
	// the y starts and ends at the same place, but the x is different for each line
	s32 StartY, EndY;
	
	
	//s64 r10, r20, r21;
	
	// new variables
	s32 x0, x1, y0, y1;
	s32 dx_left, dx_right;
	u32 bgr;
	s32 t0, t1, denominator;
	
	u32 Coord0, Coord1, Coord2;
	
	u32 PixelMask, SetPixelMask;
	
	s32 gx [ 3 ], gy [ 3 ], gbgr [ 3 ];
	u32 Command_ABE;
	
	s32 x_left, x_right;
	
	//s32 group_yoffset;
	
	
	s32 StartX, EndX;
	s32 x_across;
	u32 xoff, yoff;
	s32 xoff_left, xoff_right;
	u32 DestPixel;
	u32 bgr_temp;
	s32 Line;
	u16 *ptr;
	
	s32 x_distance, y_distance, line_length, incdec;
	s32 y_left, dy_left, yoff_left;
	
	s32 ix, iy, dx, dy;
	
	u32 NumPixels;
	
//debug << "\nDrawTriangle_Mono_th";

	// setup vars
	//if ( !local_id )
	//{
		
		
		
		// no bitmaps in opencl ??
		GPU_CTRL_Read = inputdata [ 0 ].Value;
		DrawArea_TopLeftX = inputdata [ 1 ].Value & 0x3ff;
		DrawArea_TopLeftY = ( inputdata [ 1 ].Value >> 10 ) & 0x3ff;
		DrawArea_BottomRightX = inputdata [ 2 ].Value & 0x3ff;
		DrawArea_BottomRightY = ( inputdata [ 2 ].Value >> 10 ) & 0x3ff;
		DrawArea_OffsetX = ( ( (s32) inputdata [ 3 ].Value ) << 21 ) >> 21;
		DrawArea_OffsetY = ( ( (s32) inputdata [ 3 ].Value ) << 10 ) >> 21;

		gx [ 0 ] = (s32) ( ( inputdata [ 8 ].x << 5 ) >> 5 );
		gy [ 0 ] = (s32) ( ( inputdata [ 8 ].y << 5 ) >> 5 );
		gx [ 1 ] = (s32) ( ( inputdata [ 10 ].x << 5 ) >> 5 );
		gy [ 1 ] = (s32) ( ( inputdata [ 10 ].y << 5 ) >> 5 );
		
		Coord0 = 0;
		Coord1 = 1;
		
		///////////////////////////////////
		// put top coordinates in x0,y0
		//if ( y1 < y0 )
		if ( gy [ Coord1 ] < gy [ Coord0 ] )
		{
			//Swap ( y0, y1 );
			//Swap ( Coord0, Coord1 );
			Temp = Coord0;
			Coord0 = Coord1;
			Coord1 = Temp;
		}
		
		
		// get x-values
		x0 = gx [ Coord0 ];
		x1 = gx [ Coord1 ];
		
		// get y-values
		y0 = gy [ Coord0 ];
		y1 = gy [ Coord1 ];
		
		//////////////////////////////////////////
		// get coordinates on screen
		x0 += DrawArea_OffsetX;
		y0 += DrawArea_OffsetY;
		x1 += DrawArea_OffsetX;
		y1 += DrawArea_OffsetY;
		
		// get the left/right most x
		LeftMostX = ( ( x0 < x1 ) ? x0 : x1 );
		//LeftMostX = ( ( x2 < LeftMostX ) ? x2 : LeftMostX );
		RightMostX = ( ( x0 > x1 ) ? x0 : x1 );
		//RightMostX = ( ( x2 > RightMostX ) ? x2 : RightMostX );

		// check for some important conditions
		if ( DrawArea_BottomRightX < DrawArea_TopLeftX )
		{
			//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightX < DrawArea_TopLeftX.\n";
			return 0;
		}
		
		if ( DrawArea_BottomRightY < DrawArea_TopLeftY )
		{
			//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightY < DrawArea_TopLeftY.\n";
			return 0;
		}

		// check if sprite is within draw area
		if ( RightMostX <= ((s32)DrawArea_TopLeftX) || LeftMostX > ((s32)DrawArea_BottomRightX) || y1 <= ((s32)DrawArea_TopLeftY) || y0 > ((s32)DrawArea_BottomRightY) ) return 0;
		
		// skip drawing if distance between vertices is greater than max allowed by GPU
		if ( ( _Abs( x1 - x0 ) > c_MaxPolygonWidth ) || ( y1 - y0 > c_MaxPolygonHeight ) )
		{
			// skip drawing polygon
			return 0;
		}
		
		x_distance = _Abs( x1 - x0 );
		y_distance = _Abs( y1 - y0 );
		

		
		gbgr [ 0 ] = inputdata [ 7 ].Value & 0x00ffffff;
		
		GPU_CTRL_Read_ABR = ( GPU_CTRL_Read >> 5 ) & 3;
		Command_ABE = inputdata [ 7 ].Command & 2;
		
		// ME is bit 12
		//if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
		PixelMask = ( GPU_CTRL_Read & 0x1000 ) << 3;
		
		// MD is bit 11
		//if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;
		SetPixelMask = ( GPU_CTRL_Read & 0x0800 ) << 4;
		
		
		// initialize number of pixels drawn
		//NumberOfPixelsDrawn = 0;
		
		
		// get color(s)
		bgr = gbgr [ 0 ];
		
		// ?? convert to 16-bit ?? (or should leave 24-bit?)
		//bgr = ( ( bgr & ( 0xf8 << 0 ) ) >> 3 ) | ( ( bgr & ( 0xf8 << 8 ) ) >> 6 ) | ( ( bgr & ( 0xf8 << 16 ) ) >> 9 );
		bgr = ( ( bgr >> 9 ) & 0x7c00 ) | ( ( bgr >> 6 ) & 0x3e0 ) | ( ( bgr >> 3 ) & 0x1f );
		
		

		
	
	StartX = x0;
	EndX = x1;
	StartY = y0;
	EndY = y1;
	
	// check if line is horizontal
	if ( x_distance > y_distance )
	{
		
		// get the largest length
		line_length = x_distance;
		
		//if ( denominator < 0 )
		//{
			// x1 is on the left and x0 is on the right //
			
			////////////////////////////////////
			// get slopes
			
		//ix = x0;
		iy = ( y0 << 16 ) + 0x8000;
		//x_right = x_left;
		
		//if ( y1 - y0 )
		if ( line_length )
		{
			/////////////////////////////////////////////
			// init x on the left and right
			
			//dx_left = ( ( x1 - x0 ) << 16 ) / ( ( y1 - y0 ) + 1 );
			//dx = ( ( x1 - x0 ) << 16 ) / line_length;
			dy = ( ( y1 - y0 ) << 16 ) / line_length;
		}

		
		// check if line is going left or right
		if ( x1 > x0 )
		{
			// line is going to the right
			incdec = 1;
			
			// clip against edge of screen
			if ( StartX < ((s32)DrawArea_TopLeftX) )
			{
				Temp = DrawArea_TopLeftX - StartX;
				
				iy += dy * Temp;
				StartX = DrawArea_TopLeftX;
			}
			
			if ( EndX > ((s32)DrawArea_BottomRightX) )
			{
				EndX = DrawArea_BottomRightX + 1;
			}
		}
		else
		{
			// line is going to the left from the right
			incdec = -1;
			
			// clip against edge of screen
			if ( StartX > ((s32)DrawArea_BottomRightX) )
			{
				Temp = StartX - DrawArea_BottomRightX;
				
				iy += dy * Temp;
				StartX = DrawArea_BottomRightX;
			}
			
			if ( EndX < ((s32)DrawArea_TopLeftX) )
			{
				EndX = DrawArea_TopLeftX - 1;
			}
		}
		
		
		if ( dy <= 0 )
		{
	
			if ( ( iy >> 16 ) < ((s32)DrawArea_TopLeftY) )
			{
				return 0;
			}
			//else
			//{
			//	// line is veering onto screen
			//	
			//	// get y value it hits screen at
			//	ix = ( ( ( y0 << 16 ) + 0x8000 ) - ( ((s32)DrawArea_TopLeftY) << 16 ) ) / ( dy >> 8 );
			//	ix -= ( x0 << 8 ) + 0xff;
			//	
			//}
			
			if ( EndY < ((s32)DrawArea_TopLeftY) )
			{
				// line is going up, so End Y would
				EndY = DrawArea_TopLeftY - 1;
			}
		}
		
		if ( dy >= 0 )
		{
			if ( ( iy >> 16 ) > ((s32)DrawArea_BottomRightY) )
			{
				// line is veering off screen
				return 0;
			}
			
			if ( EndY > ((s32)DrawArea_BottomRightY) )
			{
				// line is going down, so End Y would
				EndY = DrawArea_BottomRightY + 1;
			}
		}
		
		
		NumPixels = _Abs( StartX - EndX );
		if ( ( !ulThreadNum ) && _GPU->ulNumberOfThreads )
		{
			return NumPixels;
		}

		if ( _GPU->bEnable_OpenCL )
		{
			return NumPixels;
		}

		
		////////////////
		// *** TODO *** at this point area of full triangle can be calculated and the rest of the drawing can be put on another thread *** //
		
		// draw the line horizontally
		for ( ix = StartX; ix != EndX; ix += incdec )
		{
			Line = iy >> 16;
			
			if ( Line >= ((s32)DrawArea_TopLeftY) && Line <= ((s32)DrawArea_BottomRightY) )
			{
				ptr = & ( _GPU->VRAM [ ix + ( Line << 10 ) ] );
			
				// read pixel from frame buffer if we need to check mask bit
				DestPixel = *ptr;
				
				bgr_temp = bgr;
	
				// semi-transparency
				if ( Command_ABE )
				{
					bgr_temp = SemiTransparency16 ( DestPixel, bgr_temp, GPU_CTRL_Read_ABR );
				}
				
				// check if we should set mask bit when drawing
				//bgr_temp |= SetPixelMask;

				// draw pixel if we can draw to mask pixels or mask bit not set
				if ( ! ( DestPixel & PixelMask ) ) *ptr = ( bgr_temp | SetPixelMask );
			}
			
			iy += dy;
		}
		
	}
	else
	{
		// line is vertical //

		// get the largest length
		line_length = y_distance;
		
		//if ( denominator < 0 )
		//{
			// x1 is on the left and x0 is on the right //
			
			////////////////////////////////////
			// get slopes
			
		ix = ( x0 << 16 ) + 0x8000;
		//iy = y0;
		//x_right = x_left;
		
		//if ( y1 - y0 )
		if ( line_length )
		{
			/////////////////////////////////////////////
			// init x on the left and right
			
			//dx_left = ( ( x1 - x0 ) << 16 ) / ( ( y1 - y0 ) + 1 );
			dx = ( ( x1 - x0 ) << 16 ) / line_length;
			//dy = ( ( y1 - y0 ) << 16 ) / line_length;,
		}
		
		StartY = y0;
		EndY = y1;
		
		
		// check if line is going up or down
		if ( y1 > y0 )
		{
			// line is going to the down
			incdec = 1;
			
			// clip against edge of screen
			if ( StartY < ((s32)DrawArea_TopLeftY) )
			{
				Temp = DrawArea_TopLeftY - StartY;
				
				ix += dx * Temp;
				StartY = DrawArea_TopLeftY;
			}
			
			if ( EndY > ((s32)DrawArea_BottomRightY) )
			{
				EndY = DrawArea_BottomRightY + 1;
			}
		}
		else
		{
			// line is going to the left from the up
			incdec = -1;
			
			// clip against edge of screen
			if ( StartY > ((s32)DrawArea_BottomRightY) )
			{
				Temp = StartY - DrawArea_BottomRightY;
				
				ix += dx * Temp;
				StartY = DrawArea_BottomRightY;
			}
			
			if ( EndY < ((s32)DrawArea_TopLeftY) )
			{
				EndY = DrawArea_TopLeftY - 1;
			}
		}
	
		if ( dx <= 0 )
		{
			if ( ( ix >> 16 ) < ((s32)DrawArea_TopLeftX) )
			{
				// line is veering off screen
				return 0;
			}
			
			
			if ( EndX < ((s32)DrawArea_TopLeftX) )
			{
				EndX = DrawArea_TopLeftX - 1;
			}
		}
		
		if ( dx >= 0 )
		{
			if ( ( ix >> 16 ) > ((s32)DrawArea_BottomRightX) )
			{
				// line is veering off screen
				return 0;
			}
			
			if ( EndX > ((s32)DrawArea_BottomRightX) )
			{
				EndX = DrawArea_BottomRightX + 1;
			}
		}
		
		
		// offset to get to this compute unit's scanline
		//group_yoffset = group_id - ( StartY % num_global_groups );
		//if ( group_yoffset < 0 )
		//{
		//	group_yoffset += num_global_groups;
		//}

	//}	// end if ( !local_id )
	
		NumPixels = _Abs( StartY - EndY );
		if ( ( !ulThreadNum ) && _GPU->ulNumberOfThreads )
		{
			return NumPixels;
		}

		if ( _GPU->bEnable_OpenCL )
		{
			return NumPixels;
		}


	// synchronize variables across workers
	//barrier ( CLK_LOCAL_MEM_FENCE );

	
		// draw the line vertically
		for ( iy = StartY; iy != EndY; iy += incdec )
		{
			Line = ix >> 16;
			
			if ( Line >= ((s32)DrawArea_TopLeftX) && Line <= ((s32)DrawArea_BottomRightX) )
			{
				ptr = & ( _GPU->VRAM [ Line + ( iy << 10 ) ] );
			
				// read pixel from frame buffer if we need to check mask bit
				DestPixel = *ptr;
				
				bgr_temp = bgr;
	
				// semi-transparency
				if ( Command_ABE )
				{
					bgr_temp = SemiTransparency16 ( DestPixel, bgr_temp, GPU_CTRL_Read_ABR );
				}
				
				// check if we should set mask bit when drawing
				//bgr_temp |= SetPixelMask;

				// draw pixel if we can draw to mask pixels or mask bit not set
				if ( ! ( DestPixel & PixelMask ) ) *ptr = ( bgr_temp | SetPixelMask );
			}
			
			ix += dx;
		}


	}
	

	return NumPixels;
}



u64 GPU::DrawLine_Gradient_th ( DATA_Write_Format* inputdata, u32 ulThreadNum )
{
	const int local_id = 0;
	const int group_id = 0;
	const int num_local_threads = 1;
	const int num_global_groups = 1;
	
//#ifdef SINGLE_SCANLINE_MODE
	const int xid = 0;
	const int yid = 0;
	
	const int xinc = num_local_threads;
	const int yinc = num_global_groups;
	int group_yoffset = group_id;
//#endif

//inputdata format:
//0: GPU_CTRL_Read
//1: DrawArea_TopLeft
//2: DrawArea_BottomRight
//3: DrawArea_Offset
//4: (TextureWindow)(not used here)
//5: ------------
//6: ------------
//7: GetBGR24 ( Buffer [ 0 ] );
//8: GetXY0 ( Buffer [ 1 ] );
//9: GetXY1 ( Buffer [ 2 ] );
//10: GetXY2 ( Buffer [ 3 ] );



	u32 GPU_CTRL_Read, GPU_CTRL_Read_ABR;
	s32 DrawArea_BottomRightX, DrawArea_TopLeftX, DrawArea_BottomRightY, DrawArea_TopLeftY;
	s32 DrawArea_OffsetX, DrawArea_OffsetY;
	u32 GPU_CTRL_Read_DTD;
	s32 DitherValue;
	
	s32 Temp;
	s32 LeftMostX, RightMostX;
	
	
	// the y starts and ends at the same place, but the x is different for each line
	s32 StartY, EndY;
	
	
	//s64 r10, r20, r21;
	
	// new variables
	s32 x0, x1, y0, y1;
	s32 dx_left, dx_right;
	u32 bgr;
	s32 t0, t1, denominator;

	s32 iR, iG, iB;
	//s32 R_left, G_left, B_left;
	//s32 Roff_left, Goff_left, Boff_left;
	s32 r0, r1, r2, g0, g1, g2, b0, b1, b2;
	s32 Red, Blue, Green;
	s32 dr, dg, db;
	
	u32 Coord0, Coord1, Coord2;
	
	u32 PixelMask, SetPixelMask;
	
	s32 gx [ 3 ], gy [ 3 ], gbgr [ 3 ];
	u32 Command_ABE;
	
	s32 x_left, x_right;
	
	//s32 group_yoffset;
	
	
	s32 StartX, EndX;
	s32 x_across;
	u32 xoff, yoff;
	s32 xoff_left, xoff_right;
	u32 DestPixel;
	u32 bgr_temp;
	s32 Line;
	u16 *ptr;
	
	s32 x_distance, y_distance, line_length, incdec;
	s32 y_left, dy_left, yoff_left;
	
	s32 ix, iy, dx, dy;
	
	u32 NumPixels;
	
//debug << "\nDrawTriangle_Mono_th";

	// setup vars
	//if ( !local_id )
	//{
		
		// no bitmaps in opencl ??
		GPU_CTRL_Read = inputdata [ 0 ].Value;
		DrawArea_TopLeftX = inputdata [ 1 ].Value & 0x3ff;
		DrawArea_TopLeftY = ( inputdata [ 1 ].Value >> 10 ) & 0x3ff;
		DrawArea_BottomRightX = inputdata [ 2 ].Value & 0x3ff;
		DrawArea_BottomRightY = ( inputdata [ 2 ].Value >> 10 ) & 0x3ff;
		DrawArea_OffsetX = ( ( (s32) inputdata [ 3 ].Value ) << 21 ) >> 21;
		DrawArea_OffsetY = ( ( (s32) inputdata [ 3 ].Value ) << 10 ) >> 21;

		gx [ 0 ] = (s32) ( ( inputdata [ 8 ].x << 5 ) >> 5 );
		gy [ 0 ] = (s32) ( ( inputdata [ 8 ].y << 5 ) >> 5 );
		gx [ 1 ] = (s32) ( ( inputdata [ 10 ].x << 5 ) >> 5 );
		gy [ 1 ] = (s32) ( ( inputdata [ 10 ].y << 5 ) >> 5 );
		//gx [ 2 ] = (s32) ( ( inputdata [ 10 ].x << 5 ) >> 5 );
		//gy [ 2 ] = (s32) ( ( inputdata [ 10 ].y << 5 ) >> 5 );
		
		Coord0 = 0;
		Coord1 = 1;
		//Coord2 = 2;

		///////////////////////////////////
		// put top coordinates in x0,y0
		//if ( y1 < y0 )
		if ( gy [ Coord1 ] < gy [ Coord0 ] )
		{
			//Swap ( y0, y1 );
			//Swap ( Coord0, Coord1 );
			Temp = Coord0;
			Coord0 = Coord1;
			Coord1 = Temp;
		}
		
		// get x-values
		x0 = gx [ Coord0 ];
		x1 = gx [ Coord1 ];
		
		// get y-values
		y0 = gy [ Coord0 ];
		y1 = gy [ Coord1 ];

		//////////////////////////////////////////
		// get coordinates on screen
		x0 += DrawArea_OffsetX;
		y0 += DrawArea_OffsetY;
		x1 += DrawArea_OffsetX;
		y1 += DrawArea_OffsetY;
		
		// get the left/right most x
		LeftMostX = ( ( x0 < x1 ) ? x0 : x1 );
		//LeftMostX = ( ( x2 < LeftMostX ) ? x2 : LeftMostX );
		RightMostX = ( ( x0 > x1 ) ? x0 : x1 );
		//RightMostX = ( ( x2 > RightMostX ) ? x2 : RightMostX );

		// check for some important conditions
		if ( DrawArea_BottomRightX < DrawArea_TopLeftX )
		{
			//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightX < DrawArea_TopLeftX.\n";
			return 0;
		}
		
		if ( DrawArea_BottomRightY < DrawArea_TopLeftY )
		{
			//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightY < DrawArea_TopLeftY.\n";
			return 0;
		}

		// check if sprite is within draw area
		if ( RightMostX < ((s32)DrawArea_TopLeftX) || LeftMostX > ((s32)DrawArea_BottomRightX) || y1 < ((s32)DrawArea_TopLeftY) || y0 > ((s32)DrawArea_BottomRightY) ) return 0;
		
		// skip drawing if distance between vertices is greater than max allowed by GPU
		if ( ( _Abs( x1 - x0 ) > c_MaxPolygonWidth ) || ( y1 - y0 > c_MaxPolygonHeight ) )
		{
			// skip drawing polygon
			return 0;
		}
		
		x_distance = _Abs( x1 - x0 );
		y_distance = _Abs( y1 - y0 );

		if ( x_distance > y_distance )
		{
			NumPixels = x_distance;
			
			if ( LeftMostX < ((s32)DrawArea_TopLeftX) )
			{
				NumPixels -= ( DrawArea_TopLeftX - LeftMostX );
			}
			
			if ( RightMostX > ((s32)DrawArea_BottomRightX) )
			{
				NumPixels -= ( RightMostX - DrawArea_BottomRightX );
			}
		}
		else
		{
			NumPixels = y_distance;
			
			if ( y0 < ((s32)DrawArea_TopLeftY) )
			{
				NumPixels -= ( DrawArea_TopLeftY - y0 );
			}
			
			if ( y1 > ((s32)DrawArea_BottomRightY) )
			{
				NumPixels -= ( y1 - DrawArea_BottomRightY );
			}
		}
		
		if ( ( !ulThreadNum ) && _GPU->ulNumberOfThreads )
		{
			return NumPixels;
		}

		if ( _GPU->bEnable_OpenCL )
		{
			return NumPixels;
		}

		
		gbgr [ 0 ] = inputdata [ 7 ].Value & 0x00ffffff;
		gbgr [ 1 ] = inputdata [ 9 ].Value & 0x00ffffff;
		
		GPU_CTRL_Read_ABR = ( GPU_CTRL_Read >> 5 ) & 3;
		Command_ABE = inputdata [ 7 ].Command & 2;
		
		// DTD is bit 9 in GPU_CTRL_Read
		GPU_CTRL_Read_DTD = ( GPU_CTRL_Read >> 9 ) & 1;
		
		// ME is bit 12
		//if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
		PixelMask = ( GPU_CTRL_Read & 0x1000 ) << 3;
		
		// MD is bit 11
		//if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;
		SetPixelMask = ( GPU_CTRL_Read & 0x0800 ) << 4;
		
		
		// initialize number of pixels drawn
		//NumberOfPixelsDrawn = 0;
		
		
		// get color(s)
		//bgr = gbgr [ 0 ];
		
		// ?? convert to 16-bit ?? (or should leave 24-bit?)
		//bgr = ( ( bgr & ( 0xf8 << 0 ) ) >> 3 ) | ( ( bgr & ( 0xf8 << 8 ) ) >> 6 ) | ( ( bgr & ( 0xf8 << 16 ) ) >> 9 );
		//bgr = ( ( bgr >> 9 ) & 0x7c00 ) | ( ( bgr >> 6 ) & 0x3e0 ) | ( ( bgr >> 3 ) & 0x1f );
		
		

		// get rgb-values
		r0 = gbgr [ Coord0 ] & 0xff;
		r1 = gbgr [ Coord1 ] & 0xff;

		g0 = ( gbgr [ Coord0 ] >> 8 ) & 0xff;
		g1 = ( gbgr [ Coord1 ] >> 8 ) & 0xff;

		b0 = ( gbgr [ Coord0 ] >> 16 ) & 0xff;
		b1 = ( gbgr [ Coord1 ] >> 16 ) & 0xff;

		

		
		
		
		/////////////////////////////////////////////////
		// draw top part of triangle
		
		// denominator is negative when x1 is on the left, positive when x1 is on the right
		//t0 = y1 - y2;
		//t1 = y0 - y2;
		//denominator = ( ( x0 - x2 ) * t0 ) - ( ( x1 - x2 ) * t1 );

		// get reciprocals
		// *** todo ***
		//if ( y1 - y0 ) r10 = ( 1LL << 48 ) / ((s64)( y1 - y0 ));
		//if ( y2 - y0 ) r20 = ( 1LL << 48 ) / ((s64)( y2 - y0 ));
		//if ( y2 - y1 ) r21 = ( 1LL << 48 ) / ((s64)( y2 - y1 ));
		
		///////////////////////////////////////////
		// start at y0
		//Line = y0;
		
	
	iR = ( r0 << 16 ) + 0x8000;
	iG = ( g0 << 16 ) + 0x8000;
	iB = ( b0 << 16 ) + 0x8000;
	
	StartX = x0;
	EndX = x1;
	StartY = y0;
	EndY = y1;
	
	// check if line is horizontal
	if ( x_distance > y_distance )
	{
		
		// get the largest length
		line_length = x_distance;
		
		//if ( denominator < 0 )
		//{
			// x1 is on the left and x0 is on the right //
			
			////////////////////////////////////
			// get slopes
			
		//ix = x0;
		iy = ( y0 << 16 ) + 0x8000;
		//x_right = x_left;
		
		
		//if ( y1 - y0 )
		if ( line_length )
		{
			/////////////////////////////////////////////
			// init x on the left and right
			
			//dx_left = ( ( x1 - x0 ) << 16 ) / ( ( y1 - y0 ) + 1 );
			//dx = ( ( x1 - x0 ) << 16 ) / line_length;
			dy = ( ( y1 - y0 ) << 16 ) / line_length;
			
			dr = ( ( r1 - r0 ) << 16 ) / line_length;
			dg = ( ( g1 - g0 ) << 16 ) / line_length;
			db = ( ( b1 - b0 ) << 16 ) / line_length;
		}

		
		// check if line is going left or right
		if ( x1 > x0 )
		{
			// line is going to the right
			incdec = 1;
			
			// clip against edge of screen
			if ( StartX < ((s32)DrawArea_TopLeftX) )
			{
				Temp = DrawArea_TopLeftX - StartX;
				StartX = DrawArea_TopLeftX;
				
				iy += dy * Temp;
				iR += dr * Temp;
				iG += dg * Temp;
				iB += db * Temp;
			}
			
			if ( EndX > ((s32)DrawArea_BottomRightX) )
			{
				EndX = DrawArea_BottomRightX + 1;
			}
		}
		else
		{
			// line is going to the left from the right
			incdec = -1;
			
			// clip against edge of screen
			if ( StartX > ((s32)DrawArea_BottomRightX) )
			{
				Temp = StartX - DrawArea_BottomRightX;
				StartX = DrawArea_BottomRightX;
				
				iy += dy * Temp;
				iR += dr * Temp;
				iG += dg * Temp;
				iB += db * Temp;
			}
			
			if ( EndX < ((s32)DrawArea_TopLeftX) )
			{
				EndX = DrawArea_TopLeftX - 1;
			}
		}
		
		
		if ( dy <= 0 )
		{
	
			if ( ( iy >> 16 ) < ((s32)DrawArea_TopLeftY) )
			{
				return NumPixels;
			}
			//else
			//{
			//	// line is veering onto screen
			//	
			//	// get y value it hits screen at
			//	ix = ( ( ( y0 << 16 ) + 0x8000 ) - ( ((s32)DrawArea_TopLeftY) << 16 ) ) / ( dy >> 8 );
			//	ix -= ( x0 << 8 ) + 0xff;
			//	
			//}
			
			if ( EndY < ((s32)DrawArea_TopLeftY) )
			{
				// line is going down, so End Y would
				EndY = DrawArea_TopLeftY - 1;
			}
		}
		
		if ( dy >= 0 )
		{
			if ( ( iy >> 16 ) > ((s32)DrawArea_BottomRightY) )
			{
				// line is veering off screen
				return NumPixels;
			}
			
			if ( EndY > ((s32)DrawArea_BottomRightY) )
			{
				// line is going down, so End Y would
				EndY = DrawArea_BottomRightY + 1;
			}
		}
		
		
		////////////////
		// *** TODO *** at this point area of full triangle can be calculated and the rest of the drawing can be put on another thread *** //
		
		// draw the line horizontally
		for ( ix = StartX; ix != EndX; ix += incdec )
		{
			Line = iy >> 16;
			
			if ( Line >= ((s32)DrawArea_TopLeftY) && Line <= ((s32)DrawArea_BottomRightY) )
			{
				if ( GPU_CTRL_Read_DTD )
				{
					//bgr = ( _Round( iR ) >> 32 ) | ( ( _Round( iG ) >> 32 ) << 8 ) | ( ( _Round( iB ) >> 32 ) << 16 );
					//bgr = ( _Round( iR ) >> 35 ) | ( ( _Round( iG ) >> 35 ) << 5 ) | ( ( _Round( iB ) >> 35 ) << 10 );
					//DitherValue = DitherLine [ x_across & 0x3 ];
					DitherValue = c_iDitherValues16 [ ( ix & 3 ) + ( ( Line & 3 ) << 2 ) ];
					
					// perform dither
					//Red = iR + DitherValue;
					//Green = iG + DitherValue;
					//Blue = iB + DitherValue;
					Red = iR + DitherValue;
					Green = iG + DitherValue;
					Blue = iB + DitherValue;
					
					//Red = Clamp5 ( ( iR + DitherValue ) >> 27 );
					//Green = Clamp5 ( ( iG + DitherValue ) >> 27 );
					//Blue = Clamp5 ( ( iB + DitherValue ) >> 27 );
					
					// perform shift
					Red >>= ( 16 + 3 );
					Green >>= ( 16 + 3 );
					Blue >>= ( 16 + 3 );
					
					//Red = clamp ( Red, 0, 0x1f );
					//Green = clamp ( Green, 0, 0x1f );
					//Blue = clamp ( Blue, 0, 0x1f );
					Red = Clamp5 ( Red );
					Green = Clamp5 ( Green );
					Blue = Clamp5 ( Blue );
				}
				else
				{
					Red = iR >> ( 16 + 3 );
					Green = iG >> ( 16 + 3 );
					Blue = iB >> ( 16 + 3 );
				}
					
					
				bgr_temp = ( Blue << 10 ) | ( Green << 5 ) | Red;
				
				
				ptr = & ( _GPU->VRAM [ ix + ( Line << 10 ) ] );
			
				// read pixel from frame buffer if we need to check mask bit
				DestPixel = *ptr;
				
				//bgr_temp = bgr;
	
				// semi-transparency
				if ( Command_ABE )
				{
					bgr_temp = SemiTransparency16 ( DestPixel, bgr_temp, GPU_CTRL_Read_ABR );
				}
				
				// check if we should set mask bit when drawing
				//bgr_temp |= SetPixelMask;

				// draw pixel if we can draw to mask pixels or mask bit not set
				if ( ! ( DestPixel & PixelMask ) ) *ptr = ( bgr_temp | SetPixelMask );
			}
			
			iy += dy;
			
			iR += dr;
			iG += dg;
			iB += db;
		}
		
	}
	else
	{
		// line is vertical //

		// get the largest length
		line_length = y_distance;
		
		//if ( denominator < 0 )
		//{
			// x1 is on the left and x0 is on the right //
			
			////////////////////////////////////
			// get slopes
			
		ix = ( x0 << 16 ) + 0x8000;
		//iy = y0;
		//x_right = x_left;
		
		//if ( y1 - y0 )
		if ( line_length )
		{
			/////////////////////////////////////////////
			// init x on the left and right
			
			//dx_left = ( ( x1 - x0 ) << 16 ) / ( ( y1 - y0 ) + 1 );
			dx = ( ( x1 - x0 ) << 16 ) / line_length;
			//dy = ( ( y1 - y0 ) << 16 ) / line_length;,
			
			dr = ( ( r1 - r0 ) << 16 ) / line_length;
			dg = ( ( g1 - g0 ) << 16 ) / line_length;
			db = ( ( b1 - b0 ) << 16 ) / line_length;
		}
		
		
		
		// check if line is going up or down
		if ( y1 > y0 )
		{
			// line is going to the down
			incdec = 1;
			
			// clip against edge of screen
			if ( StartY < ((s32)DrawArea_TopLeftY) )
			{
				Temp = DrawArea_TopLeftY - StartY;
				StartY = DrawArea_TopLeftY;
				
				ix += dx * Temp;
				iR += dr * Temp;
				iG += dg * Temp;
				iB += db * Temp;
			}
			
			if ( EndY > ((s32)DrawArea_BottomRightY) )
			{
				EndY = DrawArea_BottomRightY + 1;
			}
		}
		else
		{
			// line is going to the left from the up
			incdec = -1;
			
			// clip against edge of screen
			if ( StartY > ((s32)DrawArea_BottomRightY) )
			{
				Temp = StartY - DrawArea_BottomRightY;
				StartY = DrawArea_BottomRightY;
				
				ix += dx * Temp;
				iR += dr * Temp;
				iG += dg * Temp;
				iB += db * Temp;
			}
			
			if ( EndY < ((s32)DrawArea_TopLeftY) )
			{
				EndY = DrawArea_TopLeftY - 1;
			}
		}
	
		if ( dx <= 0 )
		{
			if ( ( ix >> 16 ) < ((s32)DrawArea_TopLeftX) )
			{
				// line is veering off screen
				return NumPixels;
			}
			
			if ( EndX < ((s32)DrawArea_TopLeftX) )
			{
				EndX = DrawArea_TopLeftX - 1;
			}
		}
		
		if ( dx >= 0 )
		{
			if ( ( ix >> 16 ) > ((s32)DrawArea_BottomRightX) )
			{
				// line is veering off screen
				return NumPixels;
			}
			
			if ( EndX > ((s32)DrawArea_BottomRightX) )
			{
				EndX = DrawArea_BottomRightX + 1;
			}
		}
		
		
		// offset to get to this compute unit's scanline
		//group_yoffset = group_id - ( StartY % num_global_groups );
		//if ( group_yoffset < 0 )
		//{
		//	group_yoffset += num_global_groups;
		//}

	//}	// end if ( !local_id )
	
	

	// synchronize variables across workers
	//barrier ( CLK_LOCAL_MEM_FENCE );

	
		// draw the line vertically
		for ( iy = StartY; iy != EndY; iy += incdec )
		{
			Line = ix >> 16;
			
			if ( Line >= ((s32)DrawArea_TopLeftX) && Line <= ((s32)DrawArea_BottomRightX) )
			{
				if ( GPU_CTRL_Read_DTD )
				{
					//bgr = ( _Round( iR ) >> 32 ) | ( ( _Round( iG ) >> 32 ) << 8 ) | ( ( _Round( iB ) >> 32 ) << 16 );
					//bgr = ( _Round( iR ) >> 35 ) | ( ( _Round( iG ) >> 35 ) << 5 ) | ( ( _Round( iB ) >> 35 ) << 10 );
					//DitherValue = DitherLine [ x_across & 0x3 ];
					DitherValue = c_iDitherValues16 [ ( Line & 3 ) + ( ( iy & 3 ) << 2 ) ];
					
					// perform dither
					//Red = iR + DitherValue;
					//Green = iG + DitherValue;
					//Blue = iB + DitherValue;
					Red = iR + DitherValue;
					Green = iG + DitherValue;
					Blue = iB + DitherValue;
					
					//Red = Clamp5 ( ( iR + DitherValue ) >> 27 );
					//Green = Clamp5 ( ( iG + DitherValue ) >> 27 );
					//Blue = Clamp5 ( ( iB + DitherValue ) >> 27 );
					
					// perform shift
					Red >>= ( 16 + 3 );
					Green >>= ( 16 + 3 );
					Blue >>= ( 16 + 3 );
					
					//Red = clamp ( Red, 0, 0x1f );
					//Green = clamp ( Green, 0, 0x1f );
					//Blue = clamp ( Blue, 0, 0x1f );
					Red = Clamp5 ( Red );
					Green = Clamp5 ( Green );
					Blue = Clamp5 ( Blue );
				}
				else
				{
					Red = iR >> ( 16 + 3 );
					Green = iG >> ( 16 + 3 );
					Blue = iB >> ( 16 + 3 );
				}
					
					
				bgr_temp = ( Blue << 10 ) | ( Green << 5 ) | Red;
				
				
				ptr = & ( _GPU->VRAM [ Line + ( iy << 10 ) ] );
			
				// read pixel from frame buffer if we need to check mask bit
				DestPixel = *ptr;
				
				//bgr_temp = bgr;
	
				// semi-transparency
				if ( Command_ABE )
				{
					bgr_temp = SemiTransparency16 ( DestPixel, bgr_temp, GPU_CTRL_Read_ABR );
				}
				
				// check if we should set mask bit when drawing
				//bgr_temp |= SetPixelMask;

				// draw pixel if we can draw to mask pixels or mask bit not set
				if ( ! ( DestPixel & PixelMask ) ) *ptr = ( bgr_temp | SetPixelMask );
			}
			
			ix += dx;
			
			iR += dr;
			iG += dg;
			iB += db;
		}


	}
	
	return NumPixels;
}




u64 GPU::Transfer_MoveImage_80_th ( DATA_Write_Format* inputdata, u32 ulThreadNum )
{
	u32 SrcPixel, DstPixel;
	u32 DestPixel, PixelMask, SetPixelMask;
	
	u32 SrcStartX, SrcStartY, DstStartX, DstStartY, Height, Width, SrcXRun, DstXRun, Width1, Width2, CurX, CurY;
	u16 *SrcPtr, *DstPtr, *SrcLinePtr, *DstLinePtr;
	
	u32 GPU_CTRL_Read, sX, dX, sY, dY, w, h;
	
	u32 NumPixels;
	
	///////////////////////////////////////////////
	// set amount of time GPU will be busy for
	//BusyCycles += h * w * dMoveImage_80_CyclesPerPixel;	//CyclesPerPixelMove;
	
	GPU_CTRL_Read = inputdata [ 0 ].Value;
	w = inputdata [ 10 ].w;
	h = inputdata [ 10 ].h;
	
	// Xsiz=((Xsiz-1) AND 3FFh)+1
	Width = ( ( w - 1 ) & 0x3ff ) + 1;
	
	// Ysiz=((Ysiz-1) AND 1FFh)+1
	Height = ( ( h - 1 ) & 0x1ff ) + 1;

	NumPixels = Width * Height;
	if ( ( !ulThreadNum ) && _GPU->ulNumberOfThreads )
	{
		return NumPixels;
	}

	if ( _GPU->bEnable_OpenCL )
	{
		return NumPixels;
	}

	
	sX = inputdata [ 8 ].x;
	sY = inputdata [ 8 ].y;
	dX = inputdata [ 9 ].x;
	dY = inputdata [ 9 ].y;

	// nocash psx specifications: transfer/move vram-to-vram use masking
	// ME is bit 12
	//if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
	PixelMask = ( GPU_CTRL_Read & 0x1000 ) << 3;
	
	// MD is bit 11
	//if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;
	SetPixelMask = ( GPU_CTRL_Read & 0x0800 ) << 4;
	
	// xpos & 0x3ff
	//sX &= 0x3ff;
	SrcStartX = sX & 0x3ff;
	//dX &= 0x3ff;
	DstStartX = dX & 0x3ff;
	
	// ypos & 0x1ff
	//sY &= 0x1ff;
	SrcStartY = sY & 0x1ff;
	//dY &= 0x1ff;
	DstStartY = dY & 0x1ff;
	
	
	// *** NOTE: coordinates wrap *** //
	
	SrcXRun = FrameBuffer_Width - SrcStartX;
	SrcXRun = ( Width <= SrcXRun ) ? Width : SrcXRun;
	
	DstXRun = FrameBuffer_Width - DstStartX;
	DstXRun = ( Width <= DstXRun ) ? Width : DstXRun;
	
	Width1 = ( SrcXRun < DstXRun ) ? SrcXRun : DstXRun;
	Width2 = ( SrcXRun > DstXRun ) ? SrcXRun : DstXRun;
	
	for ( CurY = 0; CurY < Height; CurY++ )
	{
		// start Src/Dst pointers for line
		SrcLinePtr = & ( _GPU->VRAM [ ( ( SrcStartY + CurY ) & FrameBuffer_YMask ) << 10 ] );
		DstLinePtr = & ( _GPU->VRAM [ ( ( DstStartY + CurY ) & FrameBuffer_YMask ) << 10 ] );
		
		SrcPtr = & ( SrcLinePtr [ ( SrcStartX ) & FrameBuffer_XMask ] );
		DstPtr = & ( DstLinePtr [ ( DstStartX ) & FrameBuffer_XMask ] );
		
		// should always transfer this first block, since the width is always at least 1
		for ( CurX = 0; CurX < Width1; CurX++ )
		{
			SrcPixel = *SrcPtr++;
			DstPixel = *DstPtr;
			
			//SrcPixel |= SetPixelMask;
			
			if ( ! ( DstPixel & PixelMask ) ) *DstPtr++ = ( SrcPixel | SetPixelMask );
		}
		
		if ( CurX < Width2 )
		{
		
		SrcPtr = & ( SrcLinePtr [ ( SrcStartX + CurX ) & FrameBuffer_XMask ] );
		DstPtr = & ( DstLinePtr [ ( DstStartX + CurX ) & FrameBuffer_XMask ] );

		for ( ; CurX < Width2; CurX++ )
		{
			SrcPixel = *SrcPtr++;
			DstPixel = *DstPtr;
			
			//SrcPixel |= SetPixelMask;
			
			if ( ! ( DstPixel & PixelMask ) ) *DstPtr++ = ( SrcPixel | SetPixelMask );
		}
		
		} // end if ( CurX < Width2 )
	
		if ( CurX < Width )
		{
		
		SrcPtr = & ( SrcLinePtr [ ( SrcStartX + CurX ) & FrameBuffer_XMask ] );
		DstPtr = & ( DstLinePtr [ ( DstStartX + CurX ) & FrameBuffer_XMask ] );
		
		for ( ; CurX < Width; CurX++ )
		{
			SrcPixel = *SrcPtr++;
			DstPixel = *DstPtr;
			
			//SrcPixel |= SetPixelMask;
			
			if ( ! ( DstPixel & PixelMask ) ) *DstPtr++ = ( SrcPixel | SetPixelMask );
		}
		
		} // end if ( CurX < Width )
	}
	
	return NumPixels;
}




void GPU::TransferPixelPacketIn_th ( DATA_Write_Format* inputdata, u32 ulThreadNum )
{
	u32 bgr2;
	u32 pix0, pix1;
	u32 DestPixel, PixelMask = 0, SetPixelMask = 0;
	u32 Data, BS, Count = 0;
	
	u32 GPU_CTRL_Read, dX, dY, iX, iY, w, h;
	
	u32 *pData;
	
#ifdef INLINE_DEBUG_PIX_WRITE
	debug << "; TRANSFER PIX IN; h = " << dec << h << " w = " << w << " iX = " << iX << " iY = " << iY << " dX=" << dX << " dY=" << dY;
#endif


	
	///////////////////////////////////////////////
	// set amount of time GPU will be busy for
	//BusyCycles += h * w * dMoveImage_80_CyclesPerPixel;	//CyclesPerPixelMove;
	
	GPU_CTRL_Read = inputdata [ 0 ].Value;
	dX = inputdata [ 1 ].Value;
	dY = inputdata [ 2 ].Value;
	w = inputdata [ 3 ].Value;
	h = inputdata [ 4 ].Value;
	iX = inputdata [ 5 ].Value;
	iY = inputdata [ 6 ].Value;
	
	BS = inputdata [ 7 ].Value & 0xffffff;
	
	pData = & ( inputdata [ 8 ].Value );

	// nocash psx specifications: transfer/move vram-to-vram use masking
	// ME is bit 12
	//if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
	PixelMask = ( GPU_CTRL_Read & 0x1000 ) << 3;
	
	// MD is bit 11
	//if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;
	SetPixelMask = ( GPU_CTRL_Read & 0x0800 ) << 4;
	
	while ( Count < BS )
	{
		Data = *pData++;
		Count++;
		
	//////////////////////////////////////////////////////
	// transfer pixel of image to VRAM
	pix0 = Data & 0xffff;
	pix1 = ( Data >> 16 );
	
	// transfer pix0
	//if ( ( dX + iX ) < FrameBuffer_Width && ( dY + iY ) < FrameBuffer_Height )
	//{
		bgr2 = pix0;
		
		// read pixel from frame buffer if we need to check mask bit
		DestPixel = _GPU->VRAM [ ( (dX + iX) & 0x3ff ) + ( ( (dY + iY) & 0x1ff ) << 10 ) ];
		
		//VRAM [ (dX + iX) + ( (dY + iY) << 10 ) ] = pix0;
		
		// check if we should set mask bit when drawing
		//if ( GPU_CTRL_Read.MD ) bgr2 |= 0x8000;
		//bgr2 |= SetPixelMask;

		// draw pixel if we can draw to mask pixels or mask bit not set
		if ( ! ( DestPixel & PixelMask ) ) _GPU->VRAM [ ( (dX + iX) & 0x3ff ) + ( ( (dY + iY) & 0x1ff ) << 10 ) ] = ( bgr2 | SetPixelMask );
		//VRAM [ ( (dX + iX) & 0x3ff ) + ( ( (dY + iY) & 0x1ff ) << 10 ) ] = bgr2;
	//}
	//else
	//{
		//cout << "\nGPU::TransferPixelPacketIn: Error: Transferring pix0 outside of VRAM bounds. x=" << dec << (dX+iX) << " y=" << (dY+iY) << " DrawArea_OffsetX=" << DrawArea_OffsetX << " DrawArea_OffsetY=" << DrawArea_OffsetY;
	//}

	
	// update x
	iX++;
	
	// if it is at width then go to next line
	if ( iX == w )
	{
		iX = 0;
		iY++;
		
		// if this was the last pixel, then we are done
		if ( iY == h )
		{
			/////////////////////////////////////
			// set buffer mode back to normal
			//BufferMode = MODE_NORMAL;
			
			////////////////////////////////////////
			// done
			return;
		}
	}
	
	
	// transfer pix 1
	//if ( ( dX + iX ) < FrameBuffer_Width && ( dY + iY ) < FrameBuffer_Height )
	//{
		//VRAM [ (dX + iX) + ( (dY + iY) << 10 ) ] = pix1;
		bgr2 = pix1;
		
		// read pixel from frame buffer if we need to check mask bit
		DestPixel = _GPU->VRAM [ ( (dX + iX) & 0x3ff ) + ( ( (dY + iY) & 0x1ff ) << 10 ) ];
		
		//VRAM [ (dX + iX) + ( (dY + iY) << 10 ) ] = pix0;
		
		// check if we should set mask bit when drawing
		//if ( GPU_CTRL_Read.MD ) bgr2 |= 0x8000;
		//bgr2 |= SetPixelMask;

		// draw pixel if we can draw to mask pixels or mask bit not set
		if ( ! ( DestPixel & PixelMask ) ) _GPU->VRAM [ ( (dX + iX) & 0x3ff ) + ( ( (dY + iY) & 0x1ff ) << 10 ) ] = ( bgr2 | SetPixelMask );
		//VRAM [ ( (dX + iX) & 0x3ff ) + ( ( (dY + iY) & 0x1ff ) << 10 ) ] = bgr2;
	//}
	//else
	//{
		//cout << "\nGPU::TransferPixelPacketIn: Error: Transferring pix1 outside of VRAM bounds. x=" << dec << (dX+iX) << " y=" << (dY+iY) << " DrawArea_OffsetX=" << DrawArea_OffsetX << " DrawArea_OffsetY=" << DrawArea_OffsetY;
	//}

	
	// update x
	iX++;
	
	// if it is at width then go to next line
	if ( iX == w )
	{
		iX = 0;
		iY++;
		
		// if this was the last pixel, then we are done
		if ( iY == h )
		{
			/////////////////////////////////////
			// set buffer mode back to normal
			//BufferMode = MODE_NORMAL;
			
			////////////////////////////////////////
			// done
			return;
		}
	}
	
	}
	
	
}


void GPU::Start_Frame ( void )
{
	if ( ulNumberOfThreads )
	{
		// create event for triggering gpu thread UPDATE
		ghEvent_PS1GPU_Update = CreateEvent(
			NULL,			// default security
			false,			// auto-reset event (true for manually reset event)
			false,			// initially not set
			NULL	//"PS1GPU_Update"	// name of event object
		);
		
		if ( !ghEvent_PS1GPU_Update )
		{
			cout << "\nERROR: Unable to create PS1 GPU UPDATE event. " << GetLastError ();
		}
		
		// create event for triggering/allowing next FRAME
		ghEvent_PS1GPU_Frame = CreateEvent(
			NULL,
			false,
			true,
			NULL	//"PS1GPU_Frame"
		);
		
		if ( !ghEvent_PS1GPU_Frame )
		{
			cout << "\nERROR: Unable to create PS1 GPU FRAME event. " << GetLastError ();
		}
		
		
		// create event to trigger FINISH
		ghEvent_PS1GPU_Finish = CreateEvent(
			NULL,
			true,
			true,
			NULL	//"PS1GPU_Finish"
		);
		
		if ( !ghEvent_PS1GPU_Finish )
		{
			cout << "\nERROR: Unable to create PS1 GPU FINISH event. " << GetLastError ();
		}
		
		// ***todo*** reset write index
		ulInputBuffer_WriteIndex = 0;
		
		// clear read index
		ulInputBuffer_ReadIndex = 0;
		
		// transfer to target index
		Lock_Exchange64 ( (long long&) ulInputBuffer_TargetIndex, ulInputBuffer_WriteIndex );

		for ( int i = 0; i < ulNumberOfThreads; i++ )
		{
			//cout << "\nCreating GPU thread#" << dec << i;
			
			// create thread
			GPUThreads [ i ] = new Api::Thread( Start_GPUThread, (void*) inputdata, false );
			
			//cout << "\nCreated GPU thread#" << dec << i << " ThreadStarted=" << GPUThreads[ i ]->ThreadStarted;
			
			// reset index into buffer
			ullInputBuffer_Index = 0;
		}
		
		ulNumberOfThreads_Created = ulNumberOfThreads;
	}
}

void GPU::End_Frame ( void )
{
	int iRet;
	u32 *inputdata_ptr;
	
	if ( ulNumberOfThreads_Created )
	{
		inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
		
		if ( ulNumberOfThreads_Created )
		{
			// set command
			inputdata_ptr [ 7 ] = 6 << 24;

			ulInputBuffer_WriteIndex++;
		}
		
		inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
		
		// send command to kill the threads
		Select_KillGpuThreads_t ( inputdata_ptr, 0 );
		
		Flush ();
		
		for ( int i = 0; i < ulNumberOfThreads_Created; i++ )
		{
			//cout << "\nKilling GPU thread#" << dec << i << " ThreadStarted=" << GPUThreads[ i ]->ThreadStarted;
			
			// create thread
			iRet = GPUThreads [ i ]->Join();
			
			//cout << "\nThreadStarted=" << GPUThreads[ i ]->ThreadStarted;
			
			if ( iRet )
			{
				cout << "\nhps1x64: GPU: ALERT: Problem with completion of GPU thread#" << dec << i << " iRet=" << iRet;
			}
			
			delete GPUThreads [ i ];
			
			//cout << "\nKilled GPU thread#" << dec << i << " iRet=" << iRet;
		}
		
		// no more active threads after you delete them all
		ulNumberOfThreads_Created = 0;
		
		// remove the events for now
		CloseHandle ( ghEvent_PS1GPU_Finish );
		CloseHandle ( ghEvent_PS1GPU_Frame );
		CloseHandle ( ghEvent_PS1GPU_Update );
	}
	else
	{
		// get the joy pad data here if single threaded
		//Playstation1::SIO::Joy.ReadJoystick ( 0 );
		//Playstation1::SIO::Joy.ReadJoystick ( 1 );
	}
}


#ifdef ALLOW_OPENCL_PS1

/*
//void (CL_CALLBACK*pfn_event_notify)(cl_event event, cl_int event_command_exec_status,void *user_data)
void GPU::draw_complete_event(cl_event event, cl_int event_command_exec_status,void *user_data)
{
	u32 VisibleArea_Width, VisibleArea_Height;
	u32 *pData;

	// update the progress in processing the data
	Lock_Exchange64 ( (long long&) ulInputBuffer_ReadIndex, ulTBufferIndex );

	// for now, check for exit on zero. normally will have to check for code 2 exit on draw screen
	if ( ulGPURunStatus == 2 )
	{
		// convert pixels
		//for ( int x = 0; x < 1024 * 512; x++ )
		//{
		//	display_gfxbuffer [ x ] = ( ((long)( VRAM [ x ] & 0x1f )) << 3 ) | ( ((long)( VRAM [ x ] & 0x3e0 )) << 6 ) | ( ((long)( VRAM [ x ] & 0x7c00 )) << 9 );
		//}

		 pData = & inputdata [ ( ( ulTBufferIndex - 1 ) & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ];
		 VisibleArea_Width = pData [ 8 ];
		 VisibleArea_Height = pData [ 9 ];

		cout << "\nDrawing Screen: Width=" << dec << VisibleArea_Width << " Height=" << VisibleArea_Height;
		
		DisplayOutput_Window->OpenGL_MakeCurrentWindow ();

		glPixelZoom ( (float)MainProgramWindow_Width / (float)VisibleArea_Width, (float)MainProgramWindow_Height / (float)VisibleArea_Height );
		
		glDrawPixels ( VisibleArea_Width, VisibleArea_Height, GL_RGBA, GL_UNSIGNED_BYTE, (const GLvoid*) _GPU->PixelBuffer );

		// send pixels to the screen
		//glDrawPixels ( 1024, 512, GL_RGBA, GL_UNSIGNED_BYTE, (const GLvoid*) ps1gfxbuffer );
		//glDrawPixels ( 1024, 512, GL_RGBA, GL_UNSIGNED_BYTE, (const GLvoid*) _GPU->PixelBuffer );
		
		// update screen
		DisplayOutput_Window->FlipScreen ();

		// this is no longer the current window we are drawing to
		DisplayOutput_Window->OpenGL_ReleaseWindow ();


		// *** release thread waiting for screen draw here ***
		SetEvent( ghEvent_PS1GPU_Frame );
	}

	// done - gpu now idle
	ulGPURunStatus = 0;

	// now that gpu is idle, check if it is finished
	if ( ulTBufferIndex >= Lock_ExchangeAdd64( (long long&) ulInputBuffer_TargetIndex, 0 ) )
	{
		// all gpu tasks are currently complete
		SetEvent( ghEvent_PS1GPU_Finish );
	}

	// ***go ahead and release gpu thread here***
	// gpu thread will check if gpu is idle, then if index and maxindex don't match dispatch a run
	SetEvent( ghEvent_PS1GPU_Update );

	//cout << "\nEvent ran successfully\n";
}
*/

#endif

int GPU::Start_GPUThread( void* Param )
{
	//u64 ulTBufferIndex = 0;
	DATA_Write_Format *circularlist, *p_inputdata;

	u64 ullTargetTemp;
	
	ulTBufferIndex = 0;

	//circularlist = (DATA_Write_Format*) Param;
	circularlist = (DATA_Write_Format*) _GPU->inputdata;
	
	// infinite loop
	while ( 1 )
	{
		// wait for data to appear in the input buffers
		//while ( ulTBufferIndex == ulInputBuffer_TargetIndex );
		while ( ulTBufferIndex == Lock_ExchangeAdd64( (long long&) ulInputBuffer_TargetIndex, 0 ) )
		{
			if ( !SetEvent ( ghEvent_PS1GPU_Finish ) )
			{
				cout << "\nUnable to set finish event. " << GetLastError ();
			}
			
			WaitForSingleObject( ghEvent_PS1GPU_Update, INFINITE );
		}
		
		if ( !ResetEvent ( ghEvent_PS1GPU_Finish ) )
		{
			cout << "\nUnable to reset finish event. " << GetLastError ();
		}

		
#ifdef ALLOW_OPENCL_PS1
		
		if ( _GPU->bEnable_OpenCL )
		{
			while ( ulTBufferIndex < ulInputBuffer_TargetIndex )
			{
				
				p_inputdata = & ( circularlist [ ( ulTBufferIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );


				ulTBufferIndex++;
				
				
				// get the command
				switch ( p_inputdata [ 7 ].Command )
				{
					case 0xfe:
						// frame has been drawn now, or will be drawn
						SetEvent( ghEvent_PS1GPU_Frame );

						// send the commands up to this point including FRAME command
#ifdef ENABLE_HWPIXEL_INPUT
#else
						FlushToHardware ( ulInputBuffer_ReadIndex, ulTBufferIndex );
#endif
						
						// since gpu rendering is enabled, this should just basically flip the buffers
						draw_screen_th( p_inputdata, 1 );

						// now update where we are in circular draw list
						ulInputBuffer_ReadIndex = ulTBufferIndex;

						break;

					case 5:
						// Kill GPU Thread
						//ulTBufferIndex++;
						Lock_Exchange64 ( (long long&) ulInputBuffer_ReadIndex, ulTBufferIndex );
						
						if ( !SetEvent ( ghEvent_PS1GPU_Finish ) )
						{
							cout << "\nUnable to set finish event after killing GPU thread. " << GetLastError ();
						}
						
						return 0;
						break;

					default:
						//ulTBufferIndex++;
						break;
				}	// end switch ( p_inputdata [ 7 ].Command )

			}	// end while ( ulTBufferIndex != ulInputBuffer_TargetIndex )

#ifdef ENABLE_HWPIXEL_INPUT
#else
			FlushToHardware ( ulInputBuffer_ReadIndex, ulTBufferIndex );
#endif
			Lock_Exchange64 ( (long long&) ulInputBuffer_ReadIndex, ulTBufferIndex );
		}
		else
		{

#endif
		
		
		while ( ulTBufferIndex != ulInputBuffer_TargetIndex )
		{
		
		p_inputdata = & ( circularlist [ ( ulTBufferIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
		
//debug << "\nulInputBuffer_Count=" << dec << _GPU->ulInputBuffer_Count;
//debug << "\nulInputBuffer_WriteIndex=" << dec << _GPU->ulInputBuffer_WriteIndex;
//debug << "\nulTBufferIndex=" << dec << ulTBufferIndex;
		
		// get the command
		switch ( p_inputdata [ 7 ].Command )
		{
			case 0xfe:
				// frame has been drawn now, or will be drawn
				//x64ThreadSafe::Utilities::Lock_Exchange64 ( (long long&) ullFrameDrawPending, 0 );
				
				//MsgWaitForMultipleObjectsEx( NULL, NULL, 1, QS_ALLINPUT, MWMO_ALERTABLE );
				SetEvent( ghEvent_PS1GPU_Frame );
				
				
				draw_screen_th( p_inputdata, 1 );
				
				// command is now complete
				//Lock_ExchangeAdd32 ( (long&) _GPU->ulInputBuffer_Count, -1 );
				//ulTBufferIndex++;
				
				//return 0;
				break;
				
				
			
			case 0x02:
				Draw_FrameBufferRectangle_02_th( p_inputdata, 1 );
				break;
				
			
			case 5:
				// Kill GPU Thread
				ulTBufferIndex++;
				Lock_Exchange64 ( (long long&) ulInputBuffer_ReadIndex, ulTBufferIndex );
				
				if ( !SetEvent ( ghEvent_PS1GPU_Finish ) )
				{
					cout << "\nUnable to set finish event after killing GPU thread. " << GetLastError ();
				}
				
				return 0;
				break;
				
			case 6:
				// get key pad
				//Playstation1::SIO::Joy.ReadJoystick ( 0 );
				//Playstation1::SIO::Joy.ReadJoystick ( 1 );
				break;
				
				
			// monochrome triangle
			case 0x20:
			case 0x21:
			case 0x22:
			case 0x23:
#ifdef USE_TEMPLATES_PS1_TRIANGLE
				Select_Triangle_Renderer_t( p_inputdata, 1 );
#else
				DrawTriangle_Mono_th( p_inputdata, 1 );
#endif
				break;
				
			
			// textured triangle
			case 0x24:
			case 0x25:
			case 0x26:
			case 0x27:
#ifdef USE_TEMPLATES_PS1_TRIANGLE
				Select_Triangle_Renderer_t( p_inputdata, 1 );
#else
				DrawTriangle_Texture_th ( p_inputdata, 1 );
#endif
				break;
			
			
			
			//7:GetBGR24 ( Buffer [ 0 ] );
			//8:GetXY0 ( Buffer [ 1 ] );
			//9:GetXY1 ( Buffer [ 2 ] );
			//10:GetXY2 ( Buffer [ 3 ] );
			//11:GetXY3 ( Buffer [ 4 ] );
			// monochrome rectangle
			case 0x28:
			case 0x29:
			case 0x2a:
			case 0x2b:
#ifdef USE_TEMPLATES_PS1_TRIANGLE
				Select_Triangle_Renderer_t( p_inputdata, 1 );
#else
				DrawTriangle_Mono_th( p_inputdata, 1 );
#endif
				
				//if ( !local_id )
				//{
				//	p_inputdata [ 8 ].Value = p_inputdata [ 11 ].Value;
				//}
				//
				//DrawTriangle_Mono_th( p_inputdata, 1 );
				break;
				
			
			//7:GetBGR24 ( Buffer [ 0 ] );
			//8:GetXY0 ( Buffer [ 1 ] );
			//9:GetCLUT ( Buffer [ 2 ] );
			//9:GetUV0 ( Buffer [ 2 ] );
			//10:GetXY1 ( Buffer [ 3 ] );
			//11:GetTPAGE ( Buffer [ 4 ] );
			//11:GetUV1 ( Buffer [ 4 ] );
			//12:GetXY2 ( Buffer [ 5 ] );
			//13:GetUV2 ( Buffer [ 6 ] );
			//14:GetXY3 ( Buffer [ 7 ] );
			//15:GetUV3 ( Buffer [ 8 ] );
			// textured rectangle
			case 0x2c:
			case 0x2d:
			case 0x2e:
			case 0x2f:
#ifdef USE_TEMPLATES_PS1_TRIANGLE
				Select_Triangle_Renderer_t( p_inputdata, 1 );
#else
				DrawTriangle_Texture_th ( p_inputdata, 1 );
#endif
				
				//if ( !local_id )
				//{
				//	p_inputdata [ 8 ].Value = p_inputdata [ 14 ].Value;
				//	p_inputdata [ 9 ].Value = ( p_inputdata [ 9 ].Value & ~0xffff ) | ( p_inputdata [ 15 ].Value & 0xffff );
				//}
				//
				//DrawTriangle_Texture_th ( p_inputdata, 1 );
				break;
			
				
			case 0x30:
			case 0x31:
			case 0x32:
			case 0x33:
#ifdef USE_TEMPLATES_PS1_TRIANGLE
				Select_Triangle_Renderer_t( p_inputdata, 1 );
#else
				DrawTriangle_Gradient_th ( p_inputdata, 1 );
#endif
				break;
				
			
			// texture gradient triangle
			case 0x34:
			case 0x35:
			case 0x36:
			case 0x37:
#ifdef USE_TEMPLATES_PS1_TRIANGLE
				Select_Triangle_Renderer_t( p_inputdata, 1 );
#else
				DrawTriangle_TextureGradient_th ( p_inputdata, 1 );
#endif
				break;
			
				
			//7:GetBGR0_8 ( Buffer [ 0 ] );
			//8:GetXY0 ( Buffer [ 1 ] );
			//9:GetBGR1_8 ( Buffer [ 2 ] );
			//10:GetXY1 ( Buffer [ 3 ] );
			//11:GetBGR2_8 ( Buffer [ 4 ] );
			//12:GetXY2 ( Buffer [ 5 ] );
			//13:GetBGR3_8 ( Buffer [ 6 ] );
			//14:GetXY3 ( Buffer [ 7 ] );
			// gradient rectangle
			case 0x38:
			case 0x39:
			case 0x3a:
			case 0x3b:
#ifdef USE_TEMPLATES_PS1_TRIANGLE
				Select_Triangle_Renderer_t( p_inputdata, 1 );
#else
				DrawTriangle_Gradient_th( p_inputdata, 1 );
#endif
				
				//if ( !local_id )
				//{
				//	p_inputdata [ 7 ].Value = ( p_inputdata [ 7 ].Value & ~0xffffff ) | ( p_inputdata [ 13 ].Value & 0xffffff );
				//	p_inputdata [ 8 ].Value = p_inputdata [ 14 ].Value;
				//}
				//
				//DrawTriangle_Gradient_th( p_inputdata, 1 );
				break;


			
			//7:GetBGR0_8 ( Buffer [ 0 ] );
			//8:GetXY0 ( Buffer [ 1 ] );
			//9:GetCLUT ( Buffer [ 2 ] );
			//9:GetUV0 ( Buffer [ 2 ] );
			//10:GetBGR1_8 ( Buffer [ 3 ] );
			//11:GetXY1 ( Buffer [ 4 ] );
			//12:GetTPAGE ( Buffer [ 5 ] );
			//12:GetUV1 ( Buffer [ 5 ] );
			//13:GetBGR2_8 ( Buffer [ 6 ] );
			//14:GetXY2 ( Buffer [ 7 ] );
			//15:GetUV2 ( Buffer [ 8 ] );
			//GetBGR3_8 ( Buffer [ 9 ] );
			//GetXY3 ( Buffer [ 10 ] );
			//GetUV3 ( Buffer [ 11 ] );
			// texture gradient rectangle
			case 0x3c:
			case 0x3d:
			case 0x3e:
			case 0x3f:
#ifdef USE_TEMPLATES_PS1_TRIANGLE
				Select_Triangle_Renderer_t( p_inputdata, 1 );
#else
				DrawTriangle_TextureGradient_th ( p_inputdata, 1 );
#endif
				
				// I'll fit this in by putting the uv3 into uv2 and the rest in slots 5 and 6
				//p_inputdata [ 7 ].Value = ( p_inputdata [ 7 ].Value & ~0xffffff ) | ( ( p_inputdata [ 5 ].Value ) & 0xffffff );
				//p_inputdata [ 8 ].Value = p_inputdata [ 6 ].Value;
				//p_inputdata [ 9 ].Value = ( p_inputdata [ 9 ].Value & ~0xffff ) | ( ( p_inputdata [ 15 ].Value >> 16 ) & 0xffff );
				//
				//DrawTriangle_TextureGradient_th ( p_inputdata, 1 );
				break;
				
				
			
			// monochrome line
			case 0x40:
			case 0x41:
			case 0x42:
			case 0x43:
			case 0x44:
			case 0x45:
			case 0x46:
			case 0x47:
#ifdef USE_TEMPLATES_PS1_LINE
				Select_Line_Renderer_t( p_inputdata, 1 );
#else
				DrawLine_Mono_th ( p_inputdata, 1 );
#endif
				break;
				
				
			// monochrome polyline
			case 0x48:
			case 0x49:
			case 0x4a:
			case 0x4b:
			case 0x4c:
			case 0x4d:
			case 0x4e:
			case 0x4f:
#ifdef USE_TEMPLATES_PS1_LINE
				Select_Line_Renderer_t( p_inputdata, 1 );
#else
				DrawLine_Mono_th ( p_inputdata, 1 );
#endif
				break;
			
			
			// gradient line
			case 0x50:
			case 0x51:
			case 0x52:
			case 0x53:
			case 0x54:
			case 0x55:
			case 0x56:
			case 0x57:
#ifdef USE_TEMPLATES_PS1_LINE
				Select_Line_Renderer_t( p_inputdata, 1 );
#else
				DrawLine_Gradient_th ( p_inputdata, 1 );
#endif
				break;

				
			// gradient polyline
			case 0x58:
			case 0x59:
			case 0x5c:
			case 0x5d:
			case 0x5a:
			case 0x5b:
			case 0x5e:
			case 0x5f:
#ifdef USE_TEMPLATES_PS1_LINE
				Select_Line_Renderer_t( p_inputdata, 1 );
#else
				DrawLine_Gradient_th ( p_inputdata, 1 );
#endif
				break;
			

				
			//GetBGR24 ( Buffer [ 0 ] );
			//GetXY ( Buffer [ 1 ] );
			//GetHW ( Buffer [ 2 ] );
			// x by y rectangle
			case 0x60:
			case 0x61:
			case 0x62:
			case 0x63:
#ifdef USE_TEMPLATES_PS1_RECTANGLE
				Select_Sprite_Renderer_t( p_inputdata, 1 );
#else
				Draw_Rectangle_60_th( p_inputdata, 1 );
#endif
				break;
				
				
			//GetBGR24 ( Buffer [ 0 ] );
			//GetXY ( Buffer [ 1 ] );
			//GetCLUT ( Buffer [ 2 ] );
			//GetUV ( Buffer [ 2 ] );
			//GetHW ( Buffer [ 3 ] );
			// x by y sprite
			case 0x64:
			case 0x65:
			case 0x66:
			case 0x67:
#ifdef USE_TEMPLATES_PS1_RECTANGLE
				Select_Sprite_Renderer_t( p_inputdata, 1 );
#else
				DrawSprite_th( p_inputdata, 1 );
#endif
				break;

				
			case 0x68:
			case 0x69:
			case 0x6a:
			case 0x6b:
				Draw_Pixel_68_th( p_inputdata, 1 );
				break;


			//GetBGR24 ( Buffer [ 0 ] );
			//GetXY ( Buffer [ 1 ] );
			// 8x8 rectangle
			case 0x70:
			case 0x71:
			case 0x72:
			case 0x73:
				p_inputdata [ 10 ].w = 8;
				p_inputdata [ 10 ].h = 8;
#ifdef USE_TEMPLATES_PS1_RECTANGLE
				Select_Sprite_Renderer_t( p_inputdata, 1 );
#else
				Draw_Rectangle_60_th( p_inputdata, 1 );
#endif
				break;
				
				
			//GetBGR24 ( Buffer [ 0 ] );
			//GetXY ( Buffer [ 1 ] );
			//GetCLUT ( Buffer [ 2 ] );
			//GetUV ( Buffer [ 2 ] );
			// 8x8 sprite
			case 0x74:
			case 0x75:
			case 0x76:
			case 0x77:
				p_inputdata [ 10 ].w = 8;
				p_inputdata [ 10 ].h = 8;
#ifdef USE_TEMPLATES_PS1_RECTANGLE
				Select_Sprite_Renderer_t( p_inputdata, 1 );
#else
				DrawSprite_th( p_inputdata, 1 );
#endif
				break;
				


			// 16x16 rectangle
			case 0x78:
			case 0x79:
			case 0x7a:
			case 0x7b:
				p_inputdata [ 10 ].w = 16;
				p_inputdata [ 10 ].h = 16;
#ifdef USE_TEMPLATES_PS1_RECTANGLE
				Select_Sprite_Renderer_t( p_inputdata, 1 );
#else
				Draw_Rectangle_60_th( p_inputdata, 1 );
#endif
				break;

					
			// 16x16 sprite
			case 0x7c:
			case 0x7d:
			case 0x7e:
			case 0x7f:
				p_inputdata [ 10 ].w = 16;
				p_inputdata [ 10 ].h = 16;
#ifdef USE_TEMPLATES_PS1_RECTANGLE
				Select_Sprite_Renderer_t( p_inputdata, 1 );
#else
				DrawSprite_th( p_inputdata, 1 );
#endif
				break;
			

			////////////////////////////////////////
			// Transfer commands
			
			// MoveImage
			case 0x80:
			case 0x81:
			case 0x82:
			case 0x83:
			case 0x84:
			case 0x85:
			case 0x86:
			case 0x87:
			case 0x88:
			case 0x89:
			case 0x8a:
			case 0x8b:
			case 0x8c:
			case 0x8d:
			case 0x8e:
			case 0x8f:
			case 0x90:
			case 0x91:
			case 0x92:
			case 0x93:
			case 0x94:
			case 0x95:
			case 0x96:
			case 0x97:
			case 0x98:
			case 0x99:
			case 0x9a:
			case 0x9b:
			case 0x9c:
			case 0x9d:
			case 0x9e:
			case 0x9f:
				Transfer_MoveImage_80_th ( p_inputdata, 1 );
				break;
			
			case 0xa0:
			case 0xa1:
			case 0xa2:
			case 0xa3:
			case 0xa4:
			case 0xa5:
			case 0xa6:
			case 0xa7:
			case 0xa8:
			case 0xa9:
			case 0xaa:
			case 0xab:
			case 0xac:
			case 0xad:
			case 0xae:
			case 0xaf:
			case 0xb0:
			case 0xb1:
			case 0xb2:
			case 0xb3:
			case 0xb4:
			case 0xb5:
			case 0xb6:
			case 0xb7:
			case 0xb8:
			case 0xb9:
			case 0xba:
			case 0xbb:
			case 0xbc:
			case 0xbd:
			case 0xbe:
			case 0xbf:
				TransferPixelPacketIn_th ( p_inputdata, 1 );
				break;
		
			default:
				break;
		}
		
		ulTBufferIndex++;
		
		}	// end while ( ulTBufferIndex != ulInputBuffer_TargetIndex )
			
		Lock_Exchange64 ( (long long&) ulInputBuffer_ReadIndex, ulTBufferIndex );

#ifdef ALLOW_OPENCL_PS1
		}	// end if bEnable_OpenCL

#endif



	}	// while ( 1 )
}






void GPU::SetDisplayOutputWindow ( u32 width, u32 height, WindowClass::Window* DisplayOutput )
{
	MainProgramWindow_Width = width;
	MainProgramWindow_Height = height;
	DisplayOutput_Window = DisplayOutput;
}


void GPU::Update_LCF ()
{
	if ( ( Y_Pixel & ~1 ) >= VBlank_Y )
	{
		// in the vblank area //
		
		if ( GPU_CTRL_Read.ISINTER )
		{
			// screen is interlaced //
			
			// toggle LCF at vblank when interlaced
			//GPU_CTRL_Read.LCF ^= 1;
			GPU_CTRL_Read.LCF = ( Y_Pixel ^ 1 ) & 1;
		}
		else
		{
			// screen is NOT interlaced //
			
			// clear LCF at vblank when NOT interlaced
			GPU_CTRL_Read.LCF = 0;
		}
	}
	else
	{
		// NOT in VBLANK //
		
		// check if NOT interlaced //
		if ( !GPU_CTRL_Read.ISINTER )
		{
			// toggle LCF per scanline when NOT interlaced AND not in VBLANK
			GPU_CTRL_Read.LCF ^= 1;
		}
	}
}


void GPU::Run ()
{

	//u64 Scanline_Number;
	
	//if ( NextEvent_Cycle != *_DebugCycleCount ) return;
	
	// get the scanline number
	//Scanline_Number = GetScanline_Number ();

	// must be vblank or time to draw screen //
	
	
	// update scanline number
	Y_Pixel += 2;
	if ( Y_Pixel >= Raster_YMax )
	{
		// End of VBLANK //
		Y_Pixel -= Raster_YMax;
	}
	
	if ( ( Y_Pixel & ~1 ) < VBlank_Y )
	{
		// NOT in VBLANK //
		
		// check if NOT interlaced //
		if ( !GPU_CTRL_Read.ISINTER )
		{
			// toggle LCF per scanline when NOT interlaced AND not in VBLANK
			GPU_CTRL_Read.LCF ^= 1;
		}
	}

	

	// check if this is vblank or time to draw screen
	//if ( GetCycles_ToNextDrawStart () < dCyclesPerField0 )
	//{
	if ( ( Y_Pixel & ~1 ) == VBlank_Y )
	//if ( NextEvent_Cycle == NextEvent_Cycle_Vsync )
	{
		// vblank //
#ifdef INLINE_DEBUG_RASTER_VBLANK
	debug << "\r\n\r\n***VBLANK*** " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; (before) LCF=" << GPU_CTRL_Read.LCF << "; CTRL=" << hex << GPU_CTRL_Read.Value << "; BusyCycles=" << dec << BusyCycles << " Scanline_Number=" << Y_Pixel;
#endif
		
		// update count of frames for debugging
		Frame_Count++;
		
		// toggle lcf at vblank
		// *** testing ***
		// *note* only do this when screen is interlaced, otherwise do this every scanline
		if ( GPU_CTRL_Read.ISINTER )
		{
			// screen is interlaced //
			
			// toggle LCF at vblank when interlaced
			//GPU_CTRL_Read.LCF ^= 1;
			GPU_CTRL_Read.LCF = ( Y_Pixel ^ 1 ) & 1;
		}
		else
		{
			// screen is NOT interlaced //
			
			// clear LCF at vblank when NOT interlaced
			GPU_CTRL_Read.LCF = 0;
		}

// ps2 does not have ps1 GPU, so on PS2 Vsync probably comes from GS
#ifndef PS2_COMPILE
		// send vblank signal
		SetInterrupt_Vsync ();
#endif

	//}
	//else
	//{
//#ifdef INLINE_DEBUG_RASTER_VBLANK
//	debug << "\r\n\r\n***SCREEN DRAW*** " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; (before) LCF=" << GPU_CTRL_Read.LCF << "; CTRL=" << hex << GPU_CTRL_Read.Value << "; BusyCycles=" << dec << BusyCycles;
//#endif
		// draw screen //

		// draw output to program window @ vblank! - if output window is available
		// this is actually probably wrong. Should actually draw screen after vblank is over
		if ( DisplayOutput_Window )
		{
			// if multi-threading, then only want to send the next frame draw if the previous one is done
			if ( ulNumberOfThreads )
			{
				WaitForSingleObject ( ghEvent_PS1GPU_Frame, INFINITE );
				//while ( MsgWaitForMultipleObjectsEx ( 1, &ghEvent_PS1GPU_Frame, INFINITE, QS_ALLINPUT, MWMO_ALERTABLE ) > WAIT_OBJECT_0 )
				//MsgWaitForMultipleObjects ( 1, &ghEvent_PS1GPU_Frame, FALSE, INFINITE, QS_ALLINPUT );
				//{
				//	WindowClass::DoEventsNoWait ();
				//}
			}
			
			Draw_Screen ();
			
			if ( DebugWindow_Enabled )
			{
				if ( !ulNumberOfThreads )
				{
					Draw_FrameBuffer ();
				}
			}
		}
		
#ifdef INLINE_DEBUG_RASTER_VBLANK
			debug << "; (after) LCF=" << GPU_CTRL_Read.LCF;
#endif

		//GetNextEvent_V ();
		
		
	}

#ifdef INLINE_DEBUG_RASTER_SCANLINE
	debug << "\r\n\r\n***SCANLINE*** " << hex << setw( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; (after) LCF=" << GPU_CTRL_Read.LCF << "; CTRL=" << hex << GPU_CTRL_Read.Value << "; BusyCycles=" << dec << BusyCycles << " Scanline_Number=" << Y_Pixel << " VBlank_Y=" << VBlank_Y << " CyclesPerScanline=" << dCyclesPerScanline;
#endif

	if ( ! bEnable_OpenCL )
	{
		// flush pending drawing commands
		Flush ();
	}
	
	
	// update timers //
#ifdef USE_SCANLINE_TIMER
	// ***todo*** actually, no need to update the timer here if it is in free-run mode
	Timers::_TIMERS->UpdateTimer_Scanline_Sync ( 0 );
	Timers::_TIMERS->UpdateTimer_Scanline_Sync ( 1 );
	//Timers::_TIMERS->UpdateTimer_Scanline ( 2 );
#ifdef PS2_COMPILE
	Timers::_TIMERS->UpdateTimer_Scanline_Sync ( 3 );
#endif
#else
	Timers::_TIMERS->UpdateTimer ( 0 );
	Timers::_TIMERS->UpdateTimer ( 1 );
	Timers::_TIMERS->UpdateTimer ( 2 );
#endif
	
	// get the cycle at which we reach next vblank or draw start
	//GetNextEvent ();
	Update_NextEvent ();
	
	
	// update timer events //
#ifdef USE_SCANLINE_TIMER
	// ***todo*** actually, no need to get the next timer event if it is in free-run mode
	Timers::_TIMERS->Get_NextEvent_Scanline_Sync ( 0 );
	Timers::_TIMERS->Get_NextEvent_Scanline_Sync ( 1 );
	//Timers::_TIMERS->Get_NextEvent_Scanline ( 2 );
#ifdef PS2_COMPILE
	Timers::_TIMERS->Get_NextEvent_Scanline_Sync ( 3 );
#endif
#else
	Timers::_TIMERS->Get_NextEvent ( 0, NextEvent_Cycle );
	Timers::_TIMERS->Get_NextEvent ( 1, NextEvent_Cycle );
	Timers::_TIMERS->Get_NextEvent ( 2, NextEvent_Cycle );
#endif
	
	// need to update pixel clock
	//UpdateRaster ();
	
#ifdef INLINE_DEBUG_RASTER_SCANLINE
	debug << " NextScanline=" << dec << llNextScanlineStart;
#endif
	
}


void GPU::SetNextEvent ( u64 CycleOffset )
{
	NextEvent_Cycle = CycleOffset + *_DebugCycleCount;
	
	Update_NextEventCycle ();
}

void GPU::SetNextEvent_Cycle ( u64 Cycle )
{
	NextEvent_Cycle = Cycle;
	
	Update_NextEventCycle ();
}

void GPU::Update_NextEventCycle ()
{
	//if ( NextEvent_Cycle > *_SystemCycleCount && ( NextEvent_Cycle < *_NextSystemEvent || *_NextSystemEvent <= *_SystemCycleCount ) ) *_NextSystemEvent = NextEvent_Cycle;
	if ( NextEvent_Cycle < *_NextSystemEvent )
	{
		*_NextSystemEvent = NextEvent_Cycle;
		*_NextEventIdx = NextEvent_Idx;
	}
}


void GPU::Update_NextEvent ()
{
	//u64 CycleOffset1;
	//double dCyclesToNext;

	lScanline = Y_Pixel;
	lNextScanline = lScanline + 2;
	if ( lNextScanline >= Raster_YMax )
	{
		// End of VBLANK //
		lNextScanline -= Raster_YMax;
	}
	
	dNextScanlineStart += dCyclesPerScanline;
	dHBlankStart += dCyclesPerScanline;
	//iGPU_NextEventCycle += iCyclesPerScanline;
	//dCyclesToNext = (double)(*_DebugCycleCount)
	//CycleOffset1 = (u64) dGPU_NextEventCycle;
	
	llScanlineStart = llNextScanlineStart;
	llNextScanlineStart = (u64) dNextScanlineStart;
	if ( ( dNextScanlineStart - ( (double) llNextScanlineStart ) ) > 0.0L ) llNextScanlineStart++;
	
	llHBlankStart = (u64) dHBlankStart;
	if ( ( dHBlankStart - ( (double) llHBlankStart ) ) > 0.0L ) llHBlankStart++;
	
	
	//SetNextEvent_Cycle ( iGPU_NextEventCycle );
	SetNextEvent_Cycle ( llNextScanlineStart );
	
#ifdef INLINE_DEBUG_EVENT
	debug << "\r\nGPU::Update_NextEvent CycleOffset=" << dec << dCyclesPerScanline;
#endif
}

void GPU::GetNextEvent_V ()
{
	u64 CycleOffset1;


#ifdef USE_DIVIDE_GCC
	// *** divide ***
	// *** testing *** get next vsync event
	CycleOffset1 = (u64) ceil ( GetCycles_ToNextVBlank () );
#else
	CycleOffset1 = CEILD ( GetCycles_ToNextVBlank () );
#endif

	
	NextEvent_Cycle_Vsync = *_DebugCycleCount + CycleOffset1;

#ifdef INLINE_DEBUG_EVENT
	debug << "\r\nGPU::GetNextEvent CycleOffset=" << dec << dCyclesToNext;
#endif

	//SetNextEvent_V ( CycleOffset1 );
}

void GPU::GetNextEvent ()
{
	//u64 CycleOffset1;	//, CycleOffset2;
	//double dCyclesToNext;
	
	//dCyclesToNext = GetCycles_ToNextScanline ();
	//dCyclesToNext = GetCycles_ToNextVBlank ();
	//dGPU_NextEventCycle = ((double) (*_DebugCycleCount)) + dCyclesToNext;
	
	// *** testing *** run gpu event per scanline
	//CycleOffset1 = (u64) ceil ( GetCycles_ToNextVBlank () );
	lScanline = Y_Pixel;
	lNextScanline = lScanline + 2;
	if ( lNextScanline >= Raster_YMax )
	{
		// End of VBLANK //
		lNextScanline -= Raster_YMax;
	}
	
	dScanlineStart = GetScanline_Start ();
	dNextScanlineStart = dScanlineStart + dCyclesPerScanline;
	dHBlankStart = dNextScanlineStart - dHBlankArea_Cycles;
	
	llNextScanlineStart = (u64) dNextScanlineStart;
	if ( ( dNextScanlineStart - ( (double) llNextScanlineStart ) ) > 0.0L ) llNextScanlineStart++;

	llHBlankStart = (u64) dHBlankStart;
	if ( ( dHBlankStart - ( (double) llHBlankStart ) ) > 0.0L ) llHBlankStart++;
	
	SetNextEvent_Cycle ( llNextScanlineStart );
	
	/*
	dNextScanlineStart = GetCycles_ToNextScanlineStart ();
	
	
	//CycleOffset1 = (u64) ceil ( GetCycles_ToNextScanlineStart () );
	CycleOffset1 = (u64) ceil ( dNextScanlineStart );
	
	// need to store cycle number of the next scanline start
	dNextScanlineStart += (double) ( *_DebugCycleCount );
	
	//CycleOffset1 = (u64) ceil ( dGPU_NextEventCycle );

#ifdef INLINE_DEBUG_EVENT
	debug << "\r\nGPU::GetNextEvent CycleOffset=" << dec << dCyclesToNext;
#endif




	//if ( CycleOffset1 < CycleOffset2 )
	//{
		// set the vblank as the next event
		SetNextEvent ( CycleOffset1 );
	//}
	//else
	//{
		// set drawing the screen as the next event
		//SetNextEvent ( CycleOffset2 );
	//}
	*/
}


u32 GPU::Read ( u32 Address )
{
	u32 Temp;

#ifdef INLINE_DEBUG_READ
	debug << "\r\n\r\nGPU::Read; " << hex << setw ( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address;
#endif
	
	// handle register specific issues before read
	
	// Read GPU register value
	switch ( Address )
	{
		case GPU_DATA:
#ifdef INLINE_DEBUG_READ
			debug << "; GPU_DATA = ";
#endif

			// incoming read of DATA register from bus
			
			Temp = _GPU->ProcessDataRegRead ();
						
#ifdef INLINE_DEBUG_READ
			debug << hex << Temp;
#endif

			return Temp;
			
			break;
			
		case GPU_CTRL:
		
			// determine if gpu is busy or not and set flags accordingly before returning them
			if ( *_DebugCycleCount >= _GPU->BusyUntil_Cycle )
			{
				// gpu is not busy
				_GPU->GPU_CTRL_Read.BUSY = 1;
				_GPU->GPU_CTRL_Read.COM = 1;
			}


			// *note* if gpu is not in interlaced mode, then LCF changes per scanline
			// ***TODO*** LCF is zero during vblank?
			/*
			if ( !_GPU->GPU_CTRL_Read.ISINTER )
			{
				// graphics mode is NOT interlaced //
				
				u64 Scanline_Number, Scanline_Offset;


#ifdef USE_DIVIDE_GCC
				// *** divide ***
				Scanline_Number = ((u64)( ( *_DebugCycleCount ) / _GPU->dCyclesPerScanline ) );
#else
				Scanline_Number = (u64) ( ( *_DebugCycleCount ) * _GPU->dScanlinesPerCycle );
#endif


				// *** testing *** get value of LCF for current scanline
				//_GPU->GPU_CTRL_Read.LCF = ((u32) ( ( *_DebugCycleCount ) / _GPU->dCyclesPerScanline ) ) & 1;
				_GPU->GPU_CTRL_Read.LCF = ( (u32) Scanline_Number ) & 1;
				
				// check if scanline is in vblank
				if ( !_GPU->GPU_CTRL_Read.VIDEO )
				{
					// NTSC //
					
#ifdef USE_DIVIDE_GCC
					// *** divide ***
					// modulo the number of scanlines to get scanline offset from start of the full frame
					Scanline_Offset = Scanline_Number % NTSC_ScanlinesPerFrame;
#else
					Scanline_Offset = RMOD ( Scanline_Number, NTSC_ScanlinesPerFrame, NTSC_FramesPerScanline );
#endif
					
					// check if it is in the even field or the odd field
					if ( Scanline_Offset < NTSC_ScanlinesPerField_Even )
					{
						// even field //
						
						// check that we are past vblank
						if ( Scanline_Offset >= NTSC_VBlank )
						{
							// vblank area //
							_GPU->GPU_CTRL_Read.LCF = 0;
						}
						
					}
					else
					{
						// odd field //
						
						// start from zero
						Scanline_Offset -= NTSC_ScanlinesPerField_Even;
						
						// check that we are past vblank
						if ( Scanline_Offset >= NTSC_VBlank )
						{
							// vblank area //
							_GPU->GPU_CTRL_Read.LCF = 0;
						}
					}
				}
				else
				{
					// PAL //
					
#ifdef USE_DIVIDE_GCC
					// *** divide ***
					// modulo the number of scanlines to get scanline offset from start of the full frame
					Scanline_Offset = Scanline_Number % PAL_Scanline PerFrame;
#else
					Scanline_Offset = RMOD ( Scanline_Number, PAL_ScanlinesPerFrame, PAL_FramesPerScanline );
#endif
					
					// check if it is in the even field or the odd field
					if ( Scanline_Offset < PAL_ScanlinesPerField_Even )
					{
						// even field //
						
						// check that we are past vblank
						if ( Scanline_Offset >= PAL_VBlank )
						{
							// vblank area //
							_GPU->GPU_CTRL_Read.LCF = 0;
						}
						
					}
					else
					{
						// odd field //
						
						// start from zero
						Scanline_Offset -= PAL_ScanlinesPerField_Even;
						
						// check that we are past vblank
						if ( Scanline_Offset >= PAL_VBlank )
						{
							// vblank area //
							_GPU->GPU_CTRL_Read.LCF = 0;
						}
					}
				}
			}
			*/
			
			// set data request bit
			switch ( _GPU->GPU_CTRL_Read.DMA )
			{
				case 0:
					// always zero //
					_GPU->GPU_CTRL_Read.DataRequest = 0;
					break;
					
				case 1:
					
					if ( _GPU->BufferSize >= 16 )
					{
						// buffer is full //
						_GPU->GPU_CTRL_Read.DataRequest = 0;
					}
					else
					{
						// buffer is NOT full //
						_GPU->GPU_CTRL_Read.DataRequest = 1;
					}
					
					break;
					
				case 2:
					// same as bit 28
					_GPU->GPU_CTRL_Read.DataRequest = _GPU->GPU_CTRL_Read.COM;
					break;
					
				case 3:
					// same as bit 27
					_GPU->GPU_CTRL_Read.DataRequest = _GPU->GPU_CTRL_Read.IMG;
					break;
			}
			
			if ( _GPU->BufferMode == MODE_IMAGEIN )
			{
				// gpu is receiving an image, so is not ready for commands and is busy
				_GPU->GPU_CTRL_Read.BUSY = 0;
			}
			

#ifdef INLINE_DEBUG_READ
			debug << "; GPU_CTRL = " << hex << _GPU->GPU_CTRL_Read.Value << " CycleCount=" << dec << *_DebugCycleCount << " BusyUntil=" << _GPU->BusyUntil_Cycle;
#endif

			// incoming read of CTRL register from bus
			return _GPU->GPU_CTRL_Read.Value;
			break;
			
			
		default:
#ifdef INLINE_DEBUG_READ
			debug << "; Invalid";
#endif
		
			// invalid GPU Register
			cout << "\nhps1x64 ALERT: Unknown GPU READ @ Cycle#" << dec << *_DebugCycleCount << " Address=" << hex << Address << "\n";
			break;
	}
	
	return 0;
}

void GPU::Write ( u32 Address, u32 Data, u32 Mask )
{
#ifdef INLINE_DEBUG_WRITE
	debug << "\r\n\r\nGPU::Write; " << hex << setw ( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << " Address = " << hex << Address << "; Data = " << Data;
#endif

	// *** testing *** check if mask is a word write
	if ( Mask != 0xffffffff )
	{
		cout << "\nhps1x64 ALERT: GPU::Write Mask=" << hex << Mask;
	}
	
	// Write GPU register value
	switch ( Address )
	{
		case GPU_DATA:
#ifdef INLINE_DEBUG_WRITE
			debug << "; GPU_DATA";
#endif

			// incoming write to DATA register from bus
			
			//_GPU->ProcessDataRegWrite ( Data );
			_GPU->ProcessDataRegWrite ( & Data, 1 );
			
			break;
			
		case GPU_CTRL:
#ifdef INLINE_DEBUG_WRITE
			debug << "; GPU_CTRL";
#endif

			// incoming write to CTRL register from bus
			
			// check if the GPU is busy

			// step 2: execute command written to CTRL register
			// the commands beyond 0x3f are mirrors of 0x0-0x3f
			switch ( ( Data >> 24 ) & 0x3f )
			{
				case CTRL_Write_ResetGPU:
#ifdef INLINE_DEBUG_WRITE
			debug << "; ResetGPU";
#endif
				
					// *note* this is supposed to reset the gpu
					_GPU->BufferSize = 0;
					//_GPU->X_Pixel = 0;
					//_GPU->Y_Pixel = 0;
				
					// set status to 0x14802000
					_GPU->GPU_CTRL_Read.Value = 0x14802000;
					
					// the resolution may have changed
					_GPU->UpdateRaster_VARS ();
					
					break;
					
				case CTRL_Write_ResetBuffer:
#ifdef INLINE_DEBUG_WRITE
			debug << "; ResetBuffer";
#endif
				
					_GPU->BufferSize = 0;
					break;
					
				case CTRL_Write_ResetIRQ:
#ifdef INLINE_DEBUG_WRITE
			debug << "; ResetIRQ";
#endif
				
					break;
					
				case CTRL_Write_DisplayEnable:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_DISPLAYENABLE
			debug << "\r\n; DisplayEnable";
			debug << "; Data=" << hex << Data;
#endif
				
					_GPU->GPU_CTRL_Read.DEN = Data & 1;
					break;
					
				case CTRL_Write_DMASetup:
#ifdef INLINE_DEBUG_WRITE
			debug << "; DMASetup";
#endif

					_GPU->GPU_CTRL_Read.DMA = Data & 3;
					
					break;
					
				case CTRL_Write_DisplayArea:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_DISPLAYAREA
			debug << "\r\n; DisplayArea";
#endif

					_GPU->ScreenArea_TopLeft = Data & 0x7ffff;
					
					//DrawArea_TopLeftX = Data & 0x3ff;
					//DrawArea_TopLeftY = ( Data >> 10 ) & 0x1ff;
					_GPU->ScreenArea_TopLeftX = Data & 0x3ff;
					_GPU->ScreenArea_TopLeftY = ( Data >> 10 ) & 0x1ff;
					
					
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_DISPLAYAREA
			//debug << dec << "DrawArea_TopLeftX=" << DrawArea_TopLeftX << " DrawArea_TopLeftY=" << DrawArea_TopLeftY;
			debug << dec << "ScreenArea_TopLeftX=" << _GPU->ScreenArea_TopLeftX << " ScreenArea_TopLeftY=" << _GPU->ScreenArea_TopLeftY;
#endif

					break;
					
				case CTRL_Write_HorizontalDisplayRange:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_DISPLAYRANGE
			debug << "\r\n; HorizontalDisplayRange";
#endif

					////////////////////////////////////////////////////////////////
					// horizontal display range is given as location of the bit!!
					// so to get byte location in memory you divide by 8
					_GPU->DisplayRange_Horizontal = Data & 0xffffff;
					_GPU->DisplayRange_X1 = _GPU->DisplayRange_Horizontal & 0xfff;
					_GPU->DisplayRange_X2 = _GPU->DisplayRange_Horizontal >> 12;
					
#if defined INLINE_DEBUG_DISPLAYRANGE
			debug << "; DisplayRange_Horizontal=" << hex << _GPU->DisplayRange_Horizontal << dec << "; X1=" << _GPU->DisplayRange_X1 << "; X2=" << _GPU->DisplayRange_X2 << "; (X2-X1)/8=" << ((_GPU->DisplayRange_X2-_GPU->DisplayRange_X1)>>3);
#endif
					break;
					
				case CTRL_Write_VerticalDisplayRange:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_DISPLAYRANGE
			debug << "\r\n; VerticalDisplayRange";
#endif

					_GPU->DisplayRange_Vertical = Data & 0x1fffff;
					_GPU->DisplayRange_Y1 = _GPU->DisplayRange_Vertical & 0x3ff;
					_GPU->DisplayRange_Y2 = _GPU->DisplayRange_Vertical >> 10;
					
#if defined INLINE_DEBUG_DISPLAYRANGE
			debug << "; DisplayRange_Vertical=" << hex << _GPU->DisplayRange_Vertical << dec << "; Y1=" << _GPU->DisplayRange_Y1 << "; Y2=" << _GPU->DisplayRange_Y2 << "; Y2-Y1=" << (_GPU->DisplayRange_Y2-_GPU->DisplayRange_Y1);
#endif
					break;
					
				case CTRL_Write_DisplayMode:
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_DISPLAYMODE
			debug << "\r\n; DisplayMode";
			debug << "; Data=" << hex << Data;
#endif
					_GPU->GPU_CTRL_Read.WIDTH = ( ( Data & 3 ) << 1 ) | ( ( Data >> 6 ) & 1 );
					_GPU->GPU_CTRL_Read.HEIGHT = ( Data >> 2 ) & 1;
					_GPU->GPU_CTRL_Read.VIDEO = ( Data >> 3 ) & 1;
					_GPU->GPU_CTRL_Read.ISRGB24 = ( Data >> 4 ) & 1;
					_GPU->GPU_CTRL_Read.ISINTER = ( Data >> 5 ) & 1;
					
					// the resolution may have changed
					_GPU->UpdateRaster_VARS ();
					
#if defined INLINE_DEBUG_WRITE || defined INLINE_DEBUG_DISPLAYMODE
	debug << "; WIDTH=" << _GPU->GPU_CTRL_Read.WIDTH << "; HEIGHT=" << _GPU->GPU_CTRL_Read.HEIGHT << "; VIDEO=" << _GPU->GPU_CTRL_Read.VIDEO << "; ISRGB24=" << _GPU->GPU_CTRL_Read.ISRGB24 << "; ISINTER=" << _GPU->GPU_CTRL_Read.ISINTER;
#endif
					break;
					
				// has main register followed by another 15 mirrors, so in total 16
				case CTRL_Write_GPUInfo:
				case CTRL_Write_GPUInfo+0x1:
				case CTRL_Write_GPUInfo+0x2:
				case CTRL_Write_GPUInfo+0x3:
				case CTRL_Write_GPUInfo+0x4:
				case CTRL_Write_GPUInfo+0x5:
				case CTRL_Write_GPUInfo+0x6:
				case CTRL_Write_GPUInfo+0x7:
				case CTRL_Write_GPUInfo+0x8:
				case CTRL_Write_GPUInfo+0x9:
				case CTRL_Write_GPUInfo+0xa:
				case CTRL_Write_GPUInfo+0xb:
				case CTRL_Write_GPUInfo+0xc:
				case CTRL_Write_GPUInfo+0xd:
				case CTRL_Write_GPUInfo+0xe:
				case CTRL_Write_GPUInfo+0xf:
#ifdef INLINE_DEBUG_WRITE
			debug << "; GPUInfo";
#endif
				
					switch ( Data & 7 )
					{
						// read texture window setting
						case 2:
			
							_GPU->GPU_DATA_Read = ( _GPU->TWY << 15 ) | ( _GPU->TWX << 10 ) | ( _GPU->TWH << 5 ) | ( _GPU->TWW );
							break;
							
						case 3:
#ifdef INLINE_DEBUG_WRITE
			debug << "; DrawAreaTopLeft";
#endif

							// return draw area top left
							_GPU->GPU_DATA_Read = _GPU->DrawArea_TopLeftX | ( _GPU->DrawArea_TopLeftY << 10 );
							break;
							
						case 4:
#ifdef INLINE_DEBUG_WRITE
			debug << "; DrawAreaBottomRight";
#endif

							// return draw area bottom right
							_GPU->GPU_DATA_Read = _GPU->DrawArea_BottomRightX | ( _GPU->DrawArea_BottomRightY << 10 );
							break;
							
						case 5:
#ifdef INLINE_DEBUG_WRITE
			debug << "; DrawOffset";
#endif

							// return draw offset
							_GPU->GPU_DATA_Read = _GPU->DrawArea_OffsetX | ( _GPU->DrawArea_OffsetY << 11 );
							break;
							
						
						case 7:
#ifdef INLINE_DEBUG_WRITE
			debug << "; GPUType";
#endif

							// return GPU type
							_GPU->GPU_DATA_Read = GPU_VERSION;
							break;
						
						// returns zero
						case 8:
							_GPU->GPU_DATA_Read = 0;
							break;
					}
					
					break;
					
				default:
#ifdef INLINE_DEBUG_WRITE
			debug << "; Unknown";
#endif

					// unknown GPU command
					cout << "\nhps1x64 Error: Unknown GPU command @ Cycle#" << dec << *_DebugCycleCount << " PC=" << hex << *_DebugPC << " Command=" << Data << "\n";
					break;
			}
			
			break;
			
			
		default:
#ifdef INLINE_DEBUG_WRITE
			debug << "; Invalid";
#endif
		
			// invalid GPU Register
			cout << "\nhps1x64 ALERT: Unknown GPU WRITE @ Cycle#" << dec << *_DebugCycleCount << " Address=" << hex << Address << " Data=" << Data << "\n";
			break;
	}
	
}






double GPU::GetCycles_SinceLastPixel ()
{
#ifdef USE_DIVIDE_GCC
	// *** divide ***
	// modulo cycle count with cycles in a pixel
	return fmod ( (double) (*_DebugCycleCount), dCyclesPerPixel );
#else
	return RMOD ( (double) (*_DebugCycleCount), dCyclesPerPixel, dPixelsPerCycle );
#endif
}

double GPU::GetCycles_SinceLastPixel ( double dAtCycle )
{
#ifdef USE_DIVIDE_GCC
	// *** divide ***
	return fmod ( dAtCycle, dCyclesPerPixel );
#else
	return RMOD ( dAtCycle, dCyclesPerPixel, dPixelsPerCycle );
#endif
}



double GPU::GetCycles_SinceLastHBlank ()
{
#ifdef USE_DIVIDE_GCC
	// *** divide ***
	// get offset from start of scanline
	return fmod ( ( (double) (*_DebugCycleCount) ) + dHBlankArea_Cycles, dCyclesPerScanline );
#else
	return RMOD ( ( (double) (*_DebugCycleCount) ) + dHBlankArea_Cycles, dCyclesPerScanline, dScanlinesPerCycle );
#endif
}

double GPU::GetCycles_SinceLastHBlank ( double dAtCycle )
{
#ifdef USE_DIVIDE_GCC
	// *** divide ***
	return fmod ( dAtCycle + dHBlankArea_Cycles, dCyclesPerScanline );
#else
	return RMOD ( dAtCycle + dHBlankArea_Cycles, dCyclesPerScanline, dScanlinesPerCycle );
#endif
}


double GPU::GetCycles_SinceLastVBlank ()
{
	double dOffsetFromVBlankStart;
	
#ifdef USE_DIVIDE_GCC
	// *** divide ***
	// modulo cycle count with cycles in a frame (2 fields)
	dOffsetFromVBlankStart = fmod ( ( (double) (*_DebugCycleCount) ) + dVBlank1Area_Cycles, dCyclesPerFrame );
#else
	dOffsetFromVBlankStart = RMOD ( ( (double) (*_DebugCycleCount) ) + dVBlank1Area_Cycles, dCyclesPerFrame, dFramesPerCycle );
#endif
	
	if ( dOffsetFromVBlankStart >= dCyclesPerField1 ) dOffsetFromVBlankStart -= dCyclesPerField1;
	
	return dOffsetFromVBlankStart;
}

double GPU::GetCycles_SinceLastVBlank ( double dAtCycle )
{
	double dOffsetFromVBlankStart;
	
#ifdef USE_DIVIDE_GCC
	// *** divide ***
	// modulo cycle count with cycles in a frame (2 fields)
	dOffsetFromVBlankStart = fmod ( dAtCycle + dVBlank1Area_Cycles, dCyclesPerFrame );
#else
	dOffsetFromVBlankStart = RMOD ( dAtCycle + dVBlank1Area_Cycles, dCyclesPerFrame, dFramesPerCycle );
#endif
	
	if ( dOffsetFromVBlankStart >= dCyclesPerField1 ) dOffsetFromVBlankStart -= dCyclesPerField1;
	
	return dOffsetFromVBlankStart;
}


double GPU::GetCycles_ToNextPixel ()
{
	double dOffsetFromPixelStart;
	
#ifdef USE_DIVIDE_GCC
	// *** divide ***
	// modulo cycle count with cycles in a pixel
	dOffsetFromPixelStart = fmod ( (double) (*_DebugCycleCount), dCyclesPerPixel );
#else
	dOffsetFromPixelStart = RMOD ( (double) (*_DebugCycleCount), dCyclesPerPixel, dPixelsPerCycle );
#endif
	
	return dCyclesPerPixel - dOffsetFromPixelStart;
}


double GPU::GetCycles_ToNextPixel ( double dAtCycle )
{
	double dOffsetFromPixelStart;
	
#ifdef USE_DIVIDE_GCC
	// *** divide ***
	// modulo cycle count with cycles in a pixel
	dOffsetFromPixelStart = fmod ( dAtCycle, dCyclesPerPixel );
#else
	dOffsetFromPixelStart = RMOD ( dAtCycle, dCyclesPerPixel, dPixelsPerCycle );
#endif
	
	return dCyclesPerPixel - dOffsetFromPixelStart;
}



double GPU::GetCycles_ToNextHBlank ()
{
	double dOffsetFromHBlankStart;
	
#ifdef USE_DIVIDE_GCC
	// *** divide ***
	// get offset from start of scanline
	//dOffsetFromScanlineStart = fmod ( (double) (*_DebugCycleCount), dCyclesPerScanline );
	dOffsetFromHBlankStart = fmod ( ( (double) (*_DebugCycleCount) ) + dHBlankArea_Cycles, dCyclesPerScanline );
#else
	dOffsetFromHBlankStart = RMOD ( ( (double) (*_DebugCycleCount) ) + dHBlankArea_Cycles, dCyclesPerScanline, dScanlinesPerCycle );
#endif
	
	// return the cycles to the next hblank
	return dCyclesPerScanline - dOffsetFromHBlankStart;
}

double GPU::GetCycles_ToNextHBlank ( double dAtCycle )
{
	double dOffsetFromHBlankStart;
	
#ifdef USE_DIVIDE_GCC
	// *** divide ***
	// get offset from start of scanline
	//dOffsetFromScanlineStart = fmod ( (double) (*_DebugCycleCount), dCyclesPerScanline );
	dOffsetFromHBlankStart = fmod ( dAtCycle + dHBlankArea_Cycles, dCyclesPerScanline );
#else
	dOffsetFromHBlankStart = RMOD ( dAtCycle + dHBlankArea_Cycles, dCyclesPerScanline, dScanlinesPerCycle );
#endif
	
	// return the cycles to the next hblank
	return dCyclesPerScanline - dOffsetFromHBlankStart;
}


double GPU::GetCycles_ToNextVBlank ()
{
	double dOffsetFromFrameStart, dOffsetFromVBlankStart;
	
#ifdef USE_DIVIDE_GCC
	// *** divide ***
	// modulo cycle count with cycles in a frame (2 fields)
	dOffsetFromFrameStart = fmod ( ( (double) (*_DebugCycleCount) ) + dVBlank1Area_Cycles, dCyclesPerFrame );
#else
	dOffsetFromFrameStart = RMOD ( ( (double) (*_DebugCycleCount) ) + dVBlank1Area_Cycles, dCyclesPerFrame, dFramesPerCycle );
#endif
	
	dOffsetFromVBlankStart = dCyclesPerField1 - dOffsetFromFrameStart;
	if ( dOffsetFromVBlankStart <= 0 ) dOffsetFromVBlankStart += dCyclesPerField0;
	
	return dOffsetFromVBlankStart;
}

double GPU::GetCycles_ToNextVBlank ( double dAtCycle )
{
	double dOffsetFromFrameStart, dOffsetFromVBlankStart;
	
#ifdef USE_DIVIDE_GCC
	// *** divide ***
	// modulo cycle count with cycles in a frame (2 fields)
	dOffsetFromFrameStart = fmod ( dAtCycle + dVBlank1Area_Cycles, dCyclesPerFrame );
#else
	dOffsetFromFrameStart = RMOD ( dAtCycle + dVBlank1Area_Cycles, dCyclesPerFrame, dFramesPerCycle );
#endif
	
	dOffsetFromVBlankStart = dCyclesPerField1 - dOffsetFromFrameStart;
	if ( dOffsetFromVBlankStart <= 0 ) dOffsetFromVBlankStart += dCyclesPerField0;
	
	return dOffsetFromVBlankStart;
}




// also need functions to see if currently in hblank or vblank
bool GPU::isHBlank ()
{
	double dOffsetFromHBlankStart;
	
#ifdef USE_DIVIDE_GCC
	// *** divide ***
	// get offset from start of scanline
	//dOffsetFromScanlineStart = fmod ( (double) (*_DebugCycleCount), dCyclesPerScanline );
	dOffsetFromHBlankStart = fmod ( ( (double) (*_DebugCycleCount) ) + dHBlankArea_Cycles, dCyclesPerScanline );
#else
	dOffsetFromHBlankStart = RMOD ( ( (double) (*_DebugCycleCount) ) + dHBlankArea_Cycles, dCyclesPerScanline, dScanlinesPerCycle );
#endif
	
	return ( dOffsetFromHBlankStart < dHBlankArea_Cycles );
}

bool GPU::isHBlank ( double dAtCycle )
{
	double dOffsetFromHBlankStart;
	
#ifdef USE_DIVIDE_GCC
	// *** divide ***
	// get offset from start of scanline
	//dOffsetFromScanlineStart = fmod ( (double) (*_DebugCycleCount), dCyclesPerScanline );
	dOffsetFromHBlankStart = fmod ( dAtCycle + dHBlankArea_Cycles, dCyclesPerScanline );
#else
	dOffsetFromHBlankStart = RMOD ( dAtCycle + dHBlankArea_Cycles, dCyclesPerScanline, dScanlinesPerCycle );
#endif
	
	return ( dOffsetFromHBlankStart < dHBlankArea_Cycles );
}


bool GPU::isVBlank ()
{
	double dOffsetFromVBlankStart;
	
#ifdef USE_DIVIDE_GCC
	// *** divide ***
	// modulo cycle count with cycles in a frame (2 fields)
	dOffsetFromVBlankStart = fmod ( ( (double) (*_DebugCycleCount) ) + dVBlank1Area_Cycles, dCyclesPerFrame );
#else
	dOffsetFromVBlankStart = RMOD ( ( (double) (*_DebugCycleCount) ) + dVBlank1Area_Cycles, dCyclesPerFrame, dFramesPerCycle );
#endif
	
	if ( dOffsetFromVBlankStart >= dCyclesPerField1 ) return ( ( dOffsetFromVBlankStart - dCyclesPerField1 ) < dVBlank0Area_Cycles );

	return ( dOffsetFromVBlankStart < dVBlank1Area_Cycles );
	
	//return ( ( Y_Pixel & ~1 ) == VBlank_Y );
}

bool GPU::isVBlank ( double dAtCycle )
{
	double dOffsetFromVBlankStart;
	
#ifdef USE_DIVIDE_GCC
	// *** divide ***
	// modulo cycle count with cycles in a frame (2 fields)
	dOffsetFromVBlankStart = fmod ( dAtCycle + dVBlank1Area_Cycles, dCyclesPerFrame );
#else
	dOffsetFromVBlankStart = RMOD ( dAtCycle + dVBlank1Area_Cycles, dCyclesPerFrame, dFramesPerCycle );
#endif
	
	if ( dOffsetFromVBlankStart >= dCyclesPerField1 ) return ( ( dOffsetFromVBlankStart - dCyclesPerField1 ) < dVBlank0Area_Cycles );

	return ( dOffsetFromVBlankStart < dVBlank1Area_Cycles );
}




u64 GPU::GetScanline_Count ()
{
	// *** divide ***
	return (u64) ( ( *_DebugCycleCount ) / dCyclesPerScanline );
}


u64 GPU::GetScanline_Number ()
{
	u64 Scanline_Number;
	
	// *** divide ***
	//return ( ( (u64) ( ( *_DebugCycleCount ) / dCyclesPerScanline ) ) % ( (u64) Raster_YMax ) );
	Scanline_Number = ( ( (u64) ( ( *_DebugCycleCount ) / dCyclesPerScanline ) ) % ( (u64) Raster_YMax ) );
	
	// check if we are in field 0 or 1
	if ( Scanline_Number < ScanlinesPerField0 )
	{
		// field 0 //
		return ( Scanline_Number << 1 );
	}
	else
	{
		// field 1 //
		return 1 + ( ( Scanline_Number - ScanlinesPerField0 ) << 1 );
	}
}


double GPU::GetScanline_Start ()
{
//#ifdef USE_DIVIDE_GCC
//	// *** divide ***
//	return ( ( (double) ( *_DebugCycleCount ) ) / dCyclesPerScanline );
//#else
	//return ( ( (double) ( *_DebugCycleCount ) ) * dScanlinesPerCycle );
	return ( ( (u64) ( ( (double) ( *_DebugCycleCount ) ) * dScanlinesPerCycle ) ) * dCyclesPerScanline );
//#endif
}



double GPU::GetCycles_ToNextScanlineStart ()
{
#ifdef INLINE_DEBUG_EVENT
	debug << "\r\nGPU::GetCycles_ToNextScanline";
#endif

	double dOffsetFromScanlineStart;
	
#ifdef USE_DIVIDE_GCC
	// *** divide ***
	// get offset from start of scanline
	dOffsetFromScanlineStart = fmod ( (double) (*_DebugCycleCount), dCyclesPerScanline );
#else
	dOffsetFromScanlineStart = RMOD ( (double) (*_DebugCycleCount), dCyclesPerScanline, dScanlinesPerCycle );
#endif
	
	// return the offset to the start of the next scanline
	return dCyclesPerScanline - dOffsetFromScanlineStart;
}

double GPU::GetCycles_ToNextScanlineStart ( double dAtCycle )
{
#ifdef INLINE_DEBUG_EVENT
	debug << "\r\nGPU::GetCycles_ToNextScanline";
#endif

	double dOffsetFromScanlineStart;
	
#ifdef USE_DIVIDE_GCC
	// *** divide ***
	// get offset from start of scanline
	dOffsetFromScanlineStart = fmod ( dAtCycle, dCyclesPerScanline );
#else
	dOffsetFromScanlineStart = RMOD ( dAtCycle, dCyclesPerScanline, dScanlinesPerCycle );
#endif
	
	// return the offset to the start of the next scanline
	return dCyclesPerScanline - dOffsetFromScanlineStart;
}



double GPU::GetCycles_ToNextFieldStart ()
{
#ifdef INLINE_DEBUG_DRAWSTART
	debug << "\r\nGPU::GetCycles_ToNextFieldStart";
#endif

	double dOffsetFromFrameStart, dOffsetFromFieldStart;
	
#ifdef USE_DIVIDE_GCC
	// *** divide ***
	// modulo cycle count with cycles in a frame (2 fields)
	dOffsetFromFrameStart = fmod ( (double) (*_DebugCycleCount), dCyclesPerFrame );
#else
	dOffsetFromFrameStart = RMOD ( (double) (*_DebugCycleCount), dCyclesPerFrame, dFramesPerCycle );
#endif
	
	dOffsetFromFieldStart = dCyclesPerField0 - dOffsetFromFrameStart;
	if ( dOffsetFromFieldStart <= 0 ) dOffsetFromFieldStart += dCyclesPerField1;
	
	return dOffsetFromFieldStart;
}

double GPU::GetCycles_ToNextFieldStart ( double dAtCycle )
{
#ifdef INLINE_DEBUG_DRAWSTART
	debug << "\r\nGPU::GetCycles_ToNextFieldStart";
#endif

	double dOffsetFromFrameStart, dOffsetFromFieldStart;
	
#ifdef USE_DIVIDE_GCC
	// *** divide ***
	// modulo cycle count with cycles in a frame (2 fields)
	dOffsetFromFrameStart = fmod ( dAtCycle, dCyclesPerFrame );
#else
	dOffsetFromFrameStart = RMOD ( dAtCycle, dCyclesPerFrame, dFramesPerCycle );
#endif
	
	dOffsetFromFieldStart = dCyclesPerField0 - dOffsetFromFrameStart;
	if ( dOffsetFromFieldStart <= 0 ) dOffsetFromFieldStart += dCyclesPerField1;
	
	return dOffsetFromFieldStart;
}


double GPU::GetCycles_SinceLastScanlineStart ()
{
#ifdef INLINE_DEBUG_EVENT
	debug << "\r\nGPU::GetCycles_ToNextScanline";
#endif

	double dOffsetFromScanlineStart;
	
#ifdef USE_DIVIDE_GCC
	// *** divide ***
	// get offset from start of scanline
	dOffsetFromScanlineStart = fmod ( (double) (*_DebugCycleCount), dCyclesPerScanline );
#else
	dOffsetFromScanlineStart = RMOD ( (double) (*_DebugCycleCount), dCyclesPerScanline, dScanlinesPerCycle );
#endif
	
	// return the offset to the start of the next scanline
	return dOffsetFromScanlineStart;
}

double GPU::GetCycles_SinceLastScanlineStart ( double dAtCycle )
{
#ifdef INLINE_DEBUG_EVENT
	debug << "\r\nGPU::GetCycles_ToNextScanline";
#endif

	double dOffsetFromScanlineStart;
	
#ifdef USE_DIVIDE_GCC
	// *** divide ***
	// get offset from start of scanline
	dOffsetFromScanlineStart = fmod ( dAtCycle, dCyclesPerScanline );
#else
	dOffsetFromScanlineStart = RMOD ( dAtCycle, dCyclesPerScanline, dScanlinesPerCycle );
#endif
	
	// return the offset to the start of the next scanline
	return dOffsetFromScanlineStart;
}



double GPU::GetCycles_SinceLastFieldStart ()
{
#ifdef INLINE_DEBUG_DRAWSTART
	debug << "\r\nGPU::GetCycles_ToNextFieldStart";
#endif

	double dOffsetFromFrameStart, dOffsetFromFieldStart;
	
#ifdef USE_DIVIDE_GCC
	// *** divide ***
	// modulo cycle count with cycles in a frame (2 fields)
	dOffsetFromFrameStart = fmod ( (double) (*_DebugCycleCount), dCyclesPerFrame );
#else
	dOffsetFromFrameStart = RMOD ( (double) (*_DebugCycleCount), dCyclesPerFrame, dFramesPerCycle );
#endif
	
	if ( dOffsetFromFrameStart >= dCyclesPerField0 ) dOffsetFromFrameStart -= dCyclesPerField0;
	
	return dOffsetFromFrameStart;
}

double GPU::GetCycles_SinceLastFieldStart ( double dAtCycle )
{
#ifdef INLINE_DEBUG_DRAWSTART
	debug << "\r\nGPU::GetCycles_ToNextFieldStart";
#endif

	double dOffsetFromFrameStart, dOffsetFromFieldStart;
	
#ifdef USE_DIVIDE_GCC
	// *** divide ***
	// modulo cycle count with cycles in a frame (2 fields)
	dOffsetFromFrameStart = fmod ( dAtCycle, dCyclesPerFrame );
#else
	dOffsetFromFrameStart = RMOD ( dAtCycle, dCyclesPerFrame, dFramesPerCycle );
#endif
	
	if ( dOffsetFromFrameStart >= dCyclesPerField0 ) dOffsetFromFrameStart -= dCyclesPerField0;
	
	return dOffsetFromFrameStart;
}







void GPU::UpdateRaster_VARS ( void )
{
#ifdef INLINE_DEBUG_VARS
	debug << "\r\n->UpdateRaster_VARS";
#endif

	u32 HBlank_PixelCount;
	bool SettingsChange;
	
	// assume settings will not change
	SettingsChange = false;
	

	// if the display settings are going to change, then need to mark cycle at which they changed
	if ( HBlank_X != HBlank_X_LUT [ GPU_CTRL_Read.WIDTH ] ||
		VBlank_Y != VBlank_Y_LUT [ GPU_CTRL_Read.VIDEO ] ||
		Raster_XMax != Raster_XMax_LUT [ GPU_CTRL_Read.VIDEO ] [ GPU_CTRL_Read.WIDTH ] ||
		Raster_YMax != Raster_YMax_LUT [ GPU_CTRL_Read.VIDEO ] )
	{
#ifdef INLINE_DEBUG_VARS
	debug << "\r\nChange; StartCycle=" << dec << *_DebugCycleCount;
#endif

		// ***TODO*** need to update timers before clearing the pixel counts //
		
		// update timers 0 and 1 using settings from before they change
#ifdef USE_SCANLINE_TIMER
		Timers::_TIMERS->UpdateTimer_Scanline ( 0 );
		Timers::_TIMERS->UpdateTimer_Scanline ( 1 );
#else
		Timers::_TIMERS->UpdateTimer ( 0 );
		Timers::_TIMERS->UpdateTimer ( 1 );
#endif
		
		// at end of routine, calibrate timers

		//RasterChange_StartCycle = *_DebugCycleCount;
		SettingsChange = true;
	}


	HBlank_X = HBlank_X_LUT [ GPU_CTRL_Read.WIDTH ];

#ifdef INLINE_DEBUG_VARS
	debug << "; HBlank_X = " << dec << HBlank_X;
#endif

	VBlank_Y = VBlank_Y_LUT [ GPU_CTRL_Read.VIDEO ];

#ifdef INLINE_DEBUG_VARS
	debug << "; VBlank_Y = " << VBlank_Y;
#endif

	Raster_XMax = Raster_XMax_LUT [ GPU_CTRL_Read.VIDEO ] [ GPU_CTRL_Read.WIDTH ];

#ifdef INLINE_DEBUG_VARS
	debug << "; Raster_XMax = " << Raster_XMax;
#endif

	Raster_YMax = Raster_YMax_LUT [ GPU_CTRL_Read.VIDEO ];

#ifdef INLINE_DEBUG_VARS
	debug << "; Raster_YMax = " << Raster_YMax;
#endif

	CyclesPerPixel_INC = CyclesPerPixel_INC_Lookup [ GPU_CTRL_Read.VIDEO ] [ GPU_CTRL_Read.WIDTH ];
	dCyclesPerPixel = CyclesPerPixel_Lookup [ GPU_CTRL_Read.VIDEO ] [ GPU_CTRL_Read.WIDTH ];
	dPixelsPerCycle = PixelsPerCycle_Lookup [ GPU_CTRL_Read.VIDEO ] [ GPU_CTRL_Read.WIDTH ];
	
	// check if ntsc or pal
	if ( GPU_CTRL_Read.VIDEO )
	{
		// is PAL //
		dCyclesPerScanline = PAL_CyclesPerScanline;
		dCyclesPerFrame = PAL_CyclesPerFrame;
		dCyclesPerField0 = PAL_CyclesPerField_Even;
		dCyclesPerField1 = PAL_CyclesPerField_Odd;
		
		dScanlinesPerCycle = PAL_ScanlinesPerCycle;
		dFramesPerCycle = PAL_FramesPerCycle;
		dFieldsPerCycle0 = PAL_FieldsPerCycle_Even;
		dFieldsPerCycle1 = PAL_FieldsPerCycle_Odd;

		
		dDisplayArea_Cycles = PAL_DisplayAreaCycles;
		dVBlank0Area_Cycles = PAL_CyclesPerVBlank_Even;
		dVBlank1Area_Cycles = PAL_CyclesPerVBlank_Odd;
		
		// also need the scanlines per field
		ScanlinesPerField0 = PAL_ScanlinesPerField_Even;
		ScanlinesPerField1 = PAL_ScanlinesPerField_Odd;
	}
	else
	{
		// is NTSC //
		dCyclesPerScanline = NTSC_CyclesPerScanline;
		dCyclesPerFrame = NTSC_CyclesPerFrame;
		dCyclesPerField0 = NTSC_CyclesPerField_Even;
		dCyclesPerField1 = NTSC_CyclesPerField_Odd;
		
		dScanlinesPerCycle = NTSC_ScanlinesPerCycle;
		dFramesPerCycle = NTSC_FramesPerCycle;
		dFieldsPerCycle0 = NTSC_FieldsPerCycle_Even;
		dFieldsPerCycle1 = NTSC_FieldsPerCycle_Odd;
		
		
		dDisplayArea_Cycles = NTSC_DisplayAreaCycles;
		dVBlank0Area_Cycles = NTSC_CyclesPerVBlank_Even;
		dVBlank1Area_Cycles = NTSC_CyclesPerVBlank_Odd;
		
		// also need the scanlines per field
		ScanlinesPerField0 = NTSC_ScanlinesPerField_Even;
		ScanlinesPerField1 = NTSC_ScanlinesPerField_Odd;
	}

	// get number of pixels in hblank area
	HBlank_PixelCount = Raster_XMax - HBlank_X;
	
	// multiply by cycles per pixel
	dHBlankArea_Cycles = ( (double) HBlank_PixelCount ) * dCyclesPerPixel;

	
#ifdef INLINE_DEBUG_VARS
	debug << "; CyclesPerPixel_INC = " << CyclesPerPixel_INC;
#endif

	// the Display_Width is the HBlank_X
	Display_Width = HBlank_X;
	
	// the Display_Height is the VBlank_Y if interlaced, otherwise it is VBlank_Y/2
	if ( GPU_CTRL_Read.ISINTER )
	{
		Display_Height = VBlank_Y;
	}
	else
	{
		Display_Height = ( VBlank_Y >> 1 );
	}
	

	// check if the settings changed
	if ( SettingsChange )
	{
		// settings changed //
		
		
		// set the current scanline number
		// this does not appear to be used anymore
		// before I got this wrong, but this is a better idea
		Y_Pixel = GetScanline_Number ();
		
		// this needs to be updated whenever the settings change
		Update_LCF ();
		
		// get the next event for gpu if settings change
		GetNextEvent ();
		GetNextEvent_V ();
		
#ifdef USE_SCANLINE_TIMER

#ifdef USE_TEMPLATES_PS1_TIMER
		// calibrate timers 0 and 1
		Timers::_TIMERS->CalibrateTimer_Scanline ( 0 );
		Timers::_TIMERS->CalibrateTimer_Scanline ( 1 );
#endif
		
		// if doing calibrate, then also must do update of next event
		Timers::_TIMERS->Get_NextEvent_Scanline ( 0 );
		Timers::_TIMERS->Get_NextEvent_Scanline ( 1 );
#else
		// calibrate timers 0 and 1
		Timers::_TIMERS->CalibrateTimer ( 0 );
		Timers::_TIMERS->CalibrateTimer ( 1 );
		
		// if doing calibrate, then also must do update of next event
		Timers::_TIMERS->Get_NextEvent ( 0, NextEvent_Cycle );
		Timers::_TIMERS->Get_NextEvent ( 1, NextEvent_Cycle );
#endif
	}

	// get the new cycle of the next event after updating the vars
	//GetNextEvent ();

#ifdef INLINE_DEBUG_VARS
	debug << "->UpdateRaster_VARS";
#endif
}


#ifdef ENABLE_SCANLINES_SIMULATION

void GPU::Draw_Screen ()
{
	static const int c_iVisibleArea_StartY [] = { c_iVisibleArea_StartY_Pixel_NTSC, c_iVisibleArea_StartY_Pixel_PAL };
	static const int c_iVisibleArea_EndY [] = { c_iVisibleArea_EndY_Pixel_NTSC, c_iVisibleArea_EndY_Pixel_PAL };
	
	// so the max viewable width for PAL is 3232/4-544/4 = 808-136 = 672
	// so the max viewable height for PAL is 292-34 = 258
	
	// actually, will initially start with a 1 pixel border based on screen width/height and then will shift if something is off screen

	// need to know visible range of screen for NTSC and for PAL (each should be different)
	// NTSC visible y range is usually from 16-256 (0x10-0x100) (height=240)
	// PAL visible y range is usually from 35-291 (0x23-0x123) (height=256)
	// NTSC visible x range is.. I don't know. start with from about gpu cycle#544 to about gpu cycle#3232 (must use gpu cycles since res changes)
	s32 VisibleArea_StartX, VisibleArea_EndX, VisibleArea_StartY, VisibleArea_EndY, VisibleArea_Width, VisibleArea_Height;
	
	// there the frame buffer pixel, and then there's the screen buffer pixel
	u32 Pixel16, Pixel32_0, Pixel32_1;
	u64 Pixel64;
	
	//u32 runx_1, runx_2;
	
	// this allows you to calculate horizontal pixels
	u32 GPU_CyclesPerPixel;
	
	
	Pixel_24bit_Format Pixel24;
	
	
	s32 FramePixel_X, FramePixel_Y;
	
	// need to know where to draw the actual image at
	s32 Draw_StartX, Draw_StartY, Draw_EndX, Draw_EndY, Draw_Width, Draw_Height;
	
	s32 Source_Height;
	
	//u32 XResolution, YResolution;
	
	u16* ptr_vram16;
	u32* ptr_pixelbuffer32;
	
	s32 TopBorder_Height, BottomBorder_Height, LeftBorder_Width, RightBorder_Width;
	s32 current_x, current_y, current_width, current_height, current_size, current_xmax, current_ymax;

	u32* inputdata_ptr;
	
	// ***TESTING*** //
	//bEnableScanline = false;
	
	
	// GPU cycles per pixel depends on width
	GPU_CyclesPerPixel = c_iGPUCyclesPerPixel [ GPU_CTRL_Read.WIDTH ];
	
	// get the pixel to start and stop drawing at
	Draw_StartX = DisplayRange_X1 / GPU_CyclesPerPixel;
	Draw_EndX = DisplayRange_X2 / GPU_CyclesPerPixel;
	Draw_StartY = DisplayRange_Y1;
	Draw_EndY = DisplayRange_Y2;
	
	Draw_Width = Draw_EndX - Draw_StartX;
	Draw_Height = Draw_EndY - Draw_StartY;
	
	// get the pixel to start and stop at for visible area
	VisibleArea_StartX = c_iVisibleArea_StartX_Cycle / GPU_CyclesPerPixel;
	VisibleArea_EndX = c_iVisibleArea_EndX_Cycle / GPU_CyclesPerPixel;
	
	// visible area start and end y depends on pal/ntsc
	VisibleArea_StartY = c_iVisibleArea_StartY [ GPU_CTRL_Read.VIDEO ];
	VisibleArea_EndY = c_iVisibleArea_EndY [ GPU_CTRL_Read.VIDEO ];
	
	VisibleArea_Width = VisibleArea_EndX - VisibleArea_StartX;
	VisibleArea_Height = VisibleArea_EndY - VisibleArea_StartY;
	
	
	Source_Height = Draw_Height;
	
	if ( GPU_CTRL_Read.ISINTER && GPU_CTRL_Read.HEIGHT )
	{
		// 480i mode //
		
		// if not simulating scanlines, then the draw height should double too
		if ( !bEnableScanline )
		{
			VisibleArea_EndY += Draw_Height;
			VisibleArea_Height += Draw_Height;
			
			Draw_EndY += Draw_Height;
			
			Draw_Height <<= 1;
		}
		
		Source_Height <<= 1;
	}


	// get pointer into inputdata
	inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
	
	inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
	inputdata_ptr [ 1 ] = DisplayRange_Horizontal;
	inputdata_ptr [ 2 ] = DisplayRange_Vertical;
	inputdata_ptr [ 3 ] = ScreenArea_TopLeft;
	inputdata_ptr [ 4 ] = bEnableScanline;
	inputdata_ptr [ 5 ] = Y_Pixel;
	//inputdata_ptr [ 7 ] = 1 << 24;
	inputdata_ptr [ 7 ] = 0xfe << 24;

	ulInputBuffer_WriteIndex++;

	
	if ( ulNumberOfThreads )
	{
		// for now, wait to finish
		//while ( ulInputBuffer_Count & c_ulInputBuffer_Size );
		
		/*
		// get pointer into inputdata
		inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
		
		inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
		inputdata_ptr [ 1 ] = DisplayRange_Horizontal;
		inputdata_ptr [ 2 ] = DisplayRange_Vertical;
		inputdata_ptr [ 3 ] = ScreenArea_TopLeft;
		inputdata_ptr [ 4 ] = bEnableScanline;
		inputdata_ptr [ 5 ] = Y_Pixel;
		inputdata_ptr [ 7 ] = 1 << 24;
		
		
		//debug << "\r\ndraw_screen";
		
		// send the command to the other thread
		ulInputBuffer_WriteIndex++;
		*/
		
		//debug << "\r\nulInputBuffer_Count=" << ulInputBuffer_Count;
		
		// for now, wait to finish
		//while ( ulInputBuffer_Count & c_ulInputBuffer_Size );
		
		return;
		
		//debug << "\r\nulInputBuffer_Count=" << ulInputBuffer_Count;
	}
	else
	{

		if ( bEnable_OpenCL )
		{

		// make sure all drawing commands have been submitted
		Flush ();

		// copy from gpu into vram
		//Copy_FrameData ();
		}
		else
		{

	
#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << "\r\nGPU::Draw_Screen; GPU_CyclesPerPixel=" << dec << GPU_CyclesPerPixel << " Draw_StartX=" << Draw_StartX << " Draw_EndX=" << Draw_EndX;
	debug << "\r\nDraw_StartY=" << Draw_StartY << " Draw_EndY=" << Draw_EndY << " VisibleArea_StartX=" << VisibleArea_StartX;
	debug << "\r\nVisibleArea_EndX=" << VisibleArea_EndX << " VisibleArea_StartY=" << VisibleArea_StartY << " VisibleArea_EndY=" << VisibleArea_EndY;
#endif

	// *** new stuff *** //

	//FramePixel = 0;
	ptr_pixelbuffer32 = PixelBuffer;
	//ptr_pixelbuffer64 = (u64*) PixelBuffer;
	
	
	if ( !GPU_CTRL_Read.DEN )
	{
		BottomBorder_Height = VisibleArea_EndY - Draw_EndY;
		LeftBorder_Width = Draw_StartX - VisibleArea_StartX;
		TopBorder_Height = Draw_StartY - VisibleArea_StartY;
		RightBorder_Width = VisibleArea_EndX - Draw_EndX;
		
		if ( BottomBorder_Height < 0 ) BottomBorder_Height = 0;
		if ( LeftBorder_Width < 0 ) LeftBorder_Width = 0;
		
		//cout << "\n(before)Left=" << dec << LeftBorder_Width << " Bottom=" << BottomBorder_Height << " Draw_Width=" << Draw_Width << " VisibleArea_Width=" << VisibleArea_Width;
		
		
		current_ymax = Draw_Height + BottomBorder_Height;
		current_xmax = Draw_Width + LeftBorder_Width;
		
		// make suree that ymax and xmax are not greater than the size of visible area
		if ( current_xmax > VisibleArea_Width )
		{
			// entire image is not on the screen, so take from left border and recalc xmax //

			LeftBorder_Width -= ( current_xmax - VisibleArea_Width );
			if ( LeftBorder_Width < 0 ) LeftBorder_Width = 0;
			current_xmax = Draw_Width + LeftBorder_Width;
			
			// make sure again we do not draw past the edge of screen
			if ( current_xmax > VisibleArea_Width ) current_xmax = VisibleArea_Width;
		}
		
		if ( current_ymax > VisibleArea_Height )
		{
			BottomBorder_Height -= ( current_ymax - VisibleArea_Height );
			if ( BottomBorder_Height < 0 ) BottomBorder_Height = 0;
			current_ymax = Draw_Height + BottomBorder_Height;
			
			// make sure again we do not draw past the edge of screen
			if ( current_ymax > VisibleArea_Height ) current_ymax = VisibleArea_Height;
		}
		
		//cout << "\n(after)Left=" << dec << LeftBorder_Width << " Bottom=" << BottomBorder_Height << " Draw_Width=" << Draw_Width << " VisibleArea_Width=" << VisibleArea_Width;
		//cout << "\n(after2)current_xmax=" << current_xmax << " current_ymax=" << current_ymax;
		
#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << "\r\nGPU::Draw_Screen; Drawing bottom border";
#endif

		// current_y should start at zero for even field and one for odd
		//current_y = BottomBorder_Height;
		//current_y = Y_Pixel & 1;
		current_y = 0;
		
		// put in bottom border //
		
		//current_size = BottomBorder_Height * VisibleArea_Width;
		//current_x = 0;
		
		// check if scanlines simulation is enabled
		if ( bEnableScanline )
		{
			// if this is an odd field, then start writing on the next line
			if ( Y_Pixel & 1 )
			{
				// odd field //
				
				ptr_pixelbuffer32 += VisibleArea_Width;
			}
		}

		while ( current_y < BottomBorder_Height )
		{
			current_x = 0;
			
			//while ( current_x < current_size )
			//{
			while ( current_x < VisibleArea_Width )
			{
				*ptr_pixelbuffer32++ = 0;
				current_x++;
			}
			
			current_y++;
			
			// check if scanline simulation is enabled
			if ( bEnableScanline )
			{
				ptr_pixelbuffer32 += VisibleArea_Width;
			}
		}

#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << "\r\nGPU::Draw_Screen; Putting in screen";
	debug << " current_ymax=" << dec << current_ymax;
	debug << " current_xmax=" << current_xmax;
	debug << " VisibleArea_Width=" << VisibleArea_Width;
	debug << " VisibleArea_Height=" << VisibleArea_Height;
	debug << " LeftBorder_Width=" << LeftBorder_Width;
#endif
		
		// put in screen
		
		//FramePixel_Y = ScreenArea_TopLeftY + Draw_Height - 1;
		FramePixel_Y = ScreenArea_TopLeftY + Source_Height - 1;
		FramePixel_X = ScreenArea_TopLeftX;
		
		// check if simulating scanlines
		if ( bEnableScanline )
		{
			// check if 480i
			if ( GPU_CTRL_Read.ISINTER && GPU_CTRL_Read.HEIGHT )
			{
				// 480i //
				
				// check if in odd field or even field
				if ( Y_Pixel & 1 )
				{
					// odd field //
					
					// if the height is even, then it is ok
					// if the height is odd, need to compensate
					if ( ! ( Source_Height & 1 ) )
					{
						FramePixel_Y--;
					}
				}
				else
				{
					// even field //
					
					// if the height is odd, then it is ok
					// if the height is even, need to compensate
					if ( Source_Height & 1 )
					{
						FramePixel_Y--;
					}
				}
				
			} // end if ( GPU_CTRL_Read.ISINTER && GPU_CTRL_Read.HEIGHT )
		}
		
#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << "\r\nGPU::Draw_Screen; drawing screen pixels";
	debug << " current_x=" << dec << current_x;
	debug << " FramePixel_X=" << FramePixel_X;
	debug << " FramePixel_Y=" << FramePixel_Y;
#endif
#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << "\r\ncheck: current_x=" << current_x;
	debug << " current_xmax=" << current_xmax;
	debug << " ptr_vram32=" << ( (u64) ptr_vram32 );
	debug << " VRAM=" << ( (u64) VRAM );
	debug << " ptr_pixelbuffer64=" << ( (u64) ptr_pixelbuffer64 );
	debug << " PixelBuffer=" << ( (u64) PixelBuffer );
#endif




	
		while ( current_y < current_ymax )
		{
#ifdef INLINE_DEBUG_DRAW_SCREEN_2
	debug << "\r\nGPU::Draw_Screen; drawing left border";
	debug << " current_y=" << dec << current_y;
#endif
			// put in the left border
			current_x = 0;

			while ( current_x < LeftBorder_Width )
			{
				*ptr_pixelbuffer32++ = 0;
				current_x++;
			}
			
#ifdef INLINE_DEBUG_DRAW_SCREEN_2
	debug << "\r\nGPU::Draw_Screen; drawing screen pixels";
	debug << " current_x=" << dec << current_x;
	debug << " FramePixel_X=" << FramePixel_X;
	debug << " FramePixel_Y=" << FramePixel_Y;
#endif

			// *** important note *** this wraps around the VRAM
			//ptr_vram = & (VRAM [ FramePixel_X + ( FramePixel_Y << 10 ) ]);
			ptr_vram16 = & (VRAM [ FramePixel_X + ( ( FramePixel_Y & FrameBuffer_YMask ) << 10 ) ]);
			//ptr_vram32 = (u32*) ptr_vram;
			
#ifdef INLINE_DEBUG_DRAW_SCREEN_2
	debug << "\r\nGPU::Draw_Screen; got vram ptr";
#endif

			// put in screeen pixels
			if ( !GPU_CTRL_Read.ISRGB24 )
			{
#ifdef INLINE_DEBUG_DRAW_SCREEN_2
	debug << "\r\nGPU::Draw_Screen; !ISRGB24";
#endif

				while ( current_x < current_xmax )
				{
//#ifdef INLINE_DEBUG_DRAW_SCREEN
//	debug << "\r\ndrawx1; current_x=" << current_x;
//#endif

					Pixel16 = *ptr_vram16++;
					//Pixel64 = *ptr_vram16++;
					
					// the previous pixel conversion is wrong
					Pixel32_0 = ( ( Pixel16 & 0x1f ) << 3 ) | ( ( Pixel16 & 0x3e0 ) << 6 ) | ( ( Pixel16 & 0x7c00 ) << 9 );
					//Pixel32_0 = ( ( Pixel16 & 0x1f ) << 3 ) | ( ( Pixel16 & 0x3e0 ) << 6 ) | ( ( Pixel16 & 0x7c00 ) << 9 ) |
					//			( ( Pixel16 & 0x1c ) >> 2 ) | ( ( Pixel16 & 0x380 ) << 1 ) | ( ( Pixel16 & 7000 ) << 4 );
					//Pixel32_0 = ( ( Pixel64 & 0x1f0000001fLL ) << 3 ) | ( ( Pixel64 & 0x3e0000003e0LL ) << 6 ) | ( ( Pixel64 & 0x7c0000007c00LL ) << 9 ) |
					//			( ( Pixel64 & 0x1c0000001cLL ) >> 2 ) | ( ( Pixel64 & 0x38000000380LL ) << 1 ) | ( ( Pixel64 & 0x700000007000LL ) << 4 );
					*ptr_pixelbuffer32++ = Pixel32_0;
					current_x++;
				}
			}
			else
			{
				while ( current_x < current_xmax )
				{
					Pixel24.Pixel0 = *ptr_vram16++;
					Pixel24.Pixel1 = *ptr_vram16++;
					Pixel24.Pixel2 = *ptr_vram16++;
					
					// draw first pixel
					Pixel32_0 = ( ((u32)Pixel24.Red0) ) | ( ((u32)Pixel24.Green0) << 8 ) | ( ((u32)Pixel24.Blue0) << 16 );
					
					// draw second pixel
					Pixel32_1 = ( ((u32)Pixel24.Red1) ) | ( ((u32)Pixel24.Green1) << 8 ) | ( ((u32)Pixel24.Blue1) << 16 );
					
					*ptr_pixelbuffer32++ = Pixel32_0;
					*ptr_pixelbuffer32++ = Pixel32_1;
					current_x += 2;
				}
			}
			
#ifdef INLINE_DEBUG_DRAW_SCREEN_2
	debug << "\r\nGPU::Draw_Screen; drawing right border";
	debug << " current_x=" << dec << current_x;
#endif

			// put in right border
			while ( current_x < VisibleArea_Width )
			{
				*ptr_pixelbuffer32++ = 0;
				current_x++;
			}
			
			
			current_y++;
			
			if ( bEnableScanline )
			{
				// check if this is 480i
				if ( GPU_CTRL_Read.ISINTER && GPU_CTRL_Read.HEIGHT )
				{
					// 480i mode //
					
					// jump two lines in source image
					FramePixel_Y -= 2;
				}
				else
				{
					// go to next line in frame buffer
					FramePixel_Y--;
				}
				
				// also go to next line in destination buffer
				ptr_pixelbuffer32 += VisibleArea_Width;
			}
			else
			{
				// go to next line in frame buffer
				FramePixel_Y--;
			}
			
			
		} // end while ( current_y < current_ymax )
		
#ifdef INLINE_DEBUG_DRAW_SCREEN_2
	debug << "\r\nGPU::Draw_Screen; Drawing top border";
#endif

		// put in top border //
		
		// get number of pixels to black out
		//current_size = ( VisibleArea_Height - current_y ) * VisibleArea_Width;
		//current_x = 0;

		while ( current_y < VisibleArea_Height )
		{
			current_x = 0;
			
			//while ( current_x < current_size )
			while ( current_x < VisibleArea_Width )
			{
				*ptr_pixelbuffer32++ = 0;
				current_x++;
			} // end while ( current_x < VisibleArea_Width )
				
			current_y++;
				
			// check if scanline simulation is enabled
			if ( bEnableScanline )
			{
				// also go to next line in destination buffer
				ptr_pixelbuffer32 += VisibleArea_Width;
			}
			
		} // end while ( current_y < current_ymax )
	}
	else
	{
		// display disabled //
		
		//current_size = VisibleArea_Height * VisibleArea_Width;
		//current_x = 0;

		current_y = 0;
		
		if ( bEnableScanline )
		{
			if ( Y_Pixel & 1 )
			{
				// odd field //
				
				ptr_pixelbuffer32 += VisibleArea_Width;
			}
		}
		
		while ( current_y < VisibleArea_Height )
		{
			current_x = 0;
			
			while ( current_x < VisibleArea_Width )
			{
				*ptr_pixelbuffer32++ = 0;
				current_x++;
			}
			
			current_y++;
			
			if ( bEnableScanline )
			{
				ptr_pixelbuffer32 += VisibleArea_Width;
			}
		}

	}
	
	} // end if ( bEnable_OpenCL )

	}	// end if ( ulNumberOfThreads )

	
#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << "\r\nGPU::Draw_Screen; DisplayOutput_Window->OpenGL_MakeCurrentWindow";
#endif
		
	// *** output of pixel buffer to screen *** //

	// make this the current window we are drawing to
	DisplayOutput_Window->OpenGL_MakeCurrentWindow ();

#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << "\r\nGPU::Draw_Screen; glPixelZoom";
#endif


	// check if simulating scanlines
	if ( bEnableScanline )
	{
		// the visible height is actually times 2 in the buffer for odd and even fields
		VisibleArea_Height <<= 1;
		
		// but, its actually times 2 and then minus one
		VisibleArea_Height--;
	}
	
	if ( bEnable_OpenCL )
	{
		glBlitFramebuffer( 0, 0, VisibleArea_Width, VisibleArea_Height, 0, 0, MainProgramWindow_Width, MainProgramWindow_Height, GL_COLOR_BUFFER_BIT, GL_NEAREST );
	}
	else
	{
		glPixelZoom ( (float)MainProgramWindow_Width / (float)VisibleArea_Width, (float)MainProgramWindow_Height / (float)VisibleArea_Height );
		glDrawPixels ( VisibleArea_Width, VisibleArea_Height, GL_RGBA, GL_UNSIGNED_BYTE, (const GLvoid*) PixelBuffer );
	}


#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << "\r\nGPU::Draw_Screen; DisplayOutput_Window->FlipScreen";
#endif
	
	// update screen
	DisplayOutput_Window->FlipScreen ();

#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << "\r\nGPU::Draw_Screen; DisplayOutput_Window->OpenGL_ReleaseWindow";
#endif
	
	// this is no longer the current window we are drawing to
	DisplayOutput_Window->OpenGL_ReleaseWindow ();
}

#else

void GPU::Draw_Screen ()
{
	static const int c_iVisibleArea_StartY [] = { c_iVisibleArea_StartY_Pixel_NTSC, c_iVisibleArea_StartY_Pixel_PAL };
	static const int c_iVisibleArea_EndY [] = { c_iVisibleArea_EndY_Pixel_NTSC, c_iVisibleArea_EndY_Pixel_PAL };
	
	// so the max viewable width for PAL is 3232/4-544/4 = 808-136 = 672
	// so the max viewable height for PAL is 292-34 = 258
	
	// actually, will initially start with a 1 pixel border based on screen width/height and then will shift if something is off screen

	// need to know visible range of screen for NTSC and for PAL (each should be different)
	// NTSC visible y range is usually from 16-256 (0x10-0x100) (height=240)
	// PAL visible y range is usually from 35-291 (0x23-0x123) (height=256)
	// NTSC visible x range is.. I don't know. start with from about gpu cycle#544 to about gpu cycle#3232 (must use gpu cycles since res changes)
	s32 VisibleArea_StartX, VisibleArea_EndX, VisibleArea_StartY, VisibleArea_EndY, VisibleArea_Width, VisibleArea_Height;
	
	// there the frame buffer pixel, and then there's the screen buffer pixel
	u32 Pixel16, Pixel32_0, Pixel32_1;
	u64 Pixel64;
	
	u32 runx_1, runx_2;
	
	// this allows you to calculate horizontal pixels
	u32 GPU_CyclesPerPixel;
	
	
	Pixel_24bit_Format Pixel24;
	
	
	s32 FramePixel_X, FramePixel_Y;
	
	// need to know where to draw the actual image at
	s32 Draw_StartX, Draw_StartY, Draw_EndX, Draw_EndY, Draw_Width, Draw_Height;
	
	//u32 XResolution, YResolution;
	
	u16* ptr_vram16;
	u32* ptr_pixelbuffer32;
	
	s32 TopBorder_Height, BottomBorder_Height, LeftBorder_Width, RightBorder_Width;
	s32 current_x, current_y, current_width, current_height, current_size, current_xmax, current_ymax;
	
	
	// GPU cycles per pixel depends on width
	GPU_CyclesPerPixel = c_iGPUCyclesPerPixel [ GPU_CTRL_Read.WIDTH ];
	
	// get the pixel to start and stop drawing at
	Draw_StartX = DisplayRange_X1 / GPU_CyclesPerPixel;
	Draw_EndX = DisplayRange_X2 / GPU_CyclesPerPixel;
	Draw_StartY = DisplayRange_Y1;
	Draw_EndY = DisplayRange_Y2;
	
	Draw_Width = Draw_EndX - Draw_StartX;
	Draw_Height = Draw_EndY - Draw_StartY;
	
	// get the pixel to start and stop at for visible area
	VisibleArea_StartX = c_iVisibleArea_StartX_Cycle / GPU_CyclesPerPixel;
	VisibleArea_EndX = c_iVisibleArea_EndX_Cycle / GPU_CyclesPerPixel;
	
	// visible area start and end y depends on pal/ntsc
	VisibleArea_StartY = c_iVisibleArea_StartY [ GPU_CTRL_Read.VIDEO ];
	VisibleArea_EndY = c_iVisibleArea_EndY [ GPU_CTRL_Read.VIDEO ];
	
	VisibleArea_Width = VisibleArea_EndX - VisibleArea_StartX;
	VisibleArea_Height = VisibleArea_EndY - VisibleArea_StartY;
	
	if ( GPU_CTRL_Read.ISINTER && GPU_CTRL_Read.HEIGHT )
	{
		// 480i mode //
		
		VisibleArea_EndY += Draw_Height;
		VisibleArea_Height += Draw_Height;
		
		Draw_EndY += Draw_Height;
		
		Draw_Height <<= 1;
	}
	
#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << "\r\nGPU::Draw_Screen; GPU_CyclesPerPixel=" << dec << GPU_CyclesPerPixel << " Draw_StartX=" << Draw_StartX << " Draw_EndX=" << Draw_EndX;
	debug << "\r\nDraw_StartY=" << Draw_StartY << " Draw_EndY=" << Draw_EndY << " VisibleArea_StartX=" << VisibleArea_StartX;
	debug << "\r\nVisibleArea_EndX=" << VisibleArea_EndX << " VisibleArea_StartY=" << VisibleArea_StartY << " VisibleArea_EndY=" << VisibleArea_EndY;
#endif

	// *** new stuff *** //

	//FramePixel = 0;
	ptr_pixelbuffer32 = PixelBuffer;
	//ptr_pixelbuffer64 = (u64*) PixelBuffer;
	
	
	if ( !GPU_CTRL_Read.DEN )
	{
		BottomBorder_Height = VisibleArea_EndY - Draw_EndY;
		LeftBorder_Width = Draw_StartX - VisibleArea_StartX;
		TopBorder_Height = Draw_StartY - VisibleArea_StartY;
		RightBorder_Width = VisibleArea_EndX - Draw_EndX;
		
		if ( BottomBorder_Height < 0 ) BottomBorder_Height = 0;
		if ( LeftBorder_Width < 0 ) LeftBorder_Width = 0;
		
		//cout << "\n(before)Left=" << dec << LeftBorder_Width << " Bottom=" << BottomBorder_Height << " Draw_Width=" << Draw_Width << " VisibleArea_Width=" << VisibleArea_Width;
		
		
		current_ymax = Draw_Height + BottomBorder_Height;
		current_xmax = Draw_Width + LeftBorder_Width;
		
		// make suree that ymax and xmax are not greater than the size of visible area
		if ( current_xmax > VisibleArea_Width )
		{
			// entire image is not on the screen, so take from left border and recalc xmax //

			LeftBorder_Width -= ( current_xmax - VisibleArea_Width );
			if ( LeftBorder_Width < 0 ) LeftBorder_Width = 0;
			current_xmax = Draw_Width + LeftBorder_Width;
			
			// make sure again we do not draw past the edge of screen
			if ( current_xmax > VisibleArea_Width ) current_xmax = VisibleArea_Width;
		}
		
		if ( current_ymax > VisibleArea_Height )
		{
			BottomBorder_Height -= ( current_ymax - VisibleArea_Height );
			if ( BottomBorder_Height < 0 ) BottomBorder_Height = 0;
			current_ymax = Draw_Height + BottomBorder_Height;
			
			// make sure again we do not draw past the edge of screen
			if ( current_ymax > VisibleArea_Height ) current_ymax = VisibleArea_Height;
		}
		
		//cout << "\n(after)Left=" << dec << LeftBorder_Width << " Bottom=" << BottomBorder_Height << " Draw_Width=" << Draw_Width << " VisibleArea_Width=" << VisibleArea_Width;
		//cout << "\n(after2)current_xmax=" << current_xmax << " current_ymax=" << current_ymax;
		
#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << "\r\nGPU::Draw_Screen; Drawing bottom border";
#endif


		current_y = BottomBorder_Height;
		
		// put in bottom border
		current_size = BottomBorder_Height * VisibleArea_Width;
		current_x = 0;

		while ( current_x < current_size )
		{
			*ptr_pixelbuffer32++ = 0;
			current_x++;
		}

#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << "\r\nGPU::Draw_Screen; Putting in screen";
	debug << " current_ymax=" << dec << current_ymax;
	debug << " current_xmax=" << current_xmax;
	debug << " VisibleArea_Width=" << VisibleArea_Width;
	debug << " VisibleArea_Height=" << VisibleArea_Height;
	debug << " LeftBorder_Width=" << LeftBorder_Width;
#endif
		
		// put in screen
		
		FramePixel_Y = ScreenArea_TopLeftY + Draw_Height - 1;
		FramePixel_X = ScreenArea_TopLeftX;
		
#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << "\r\nGPU::Draw_Screen; drawing screen pixels";
	debug << " current_x=" << dec << current_x;
	debug << " FramePixel_X=" << FramePixel_X;
	debug << " FramePixel_Y=" << FramePixel_Y;
#endif
#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << "\r\ncheck: current_x=" << current_x;
	debug << " current_xmax=" << current_xmax;
	debug << " ptr_vram32=" << ( (u64) ptr_vram32 );
	debug << " VRAM=" << ( (u64) VRAM );
	debug << " ptr_pixelbuffer64=" << ( (u64) ptr_pixelbuffer64 );
	debug << " PixelBuffer=" << ( (u64) PixelBuffer );
#endif


#ifdef ENABLE_SCANLINES_SIMULATION
	if ( Y_Pixel & 1 )
	{
		// odd scanline field //
		
		// check that current_y is odd
		if ( ! ( current_y & 1 ) )
		{
			// current_y is even //
			
			// move current_y down
			current_y++;
			
			// move destination buffer to next line
			ptr_pixelbuffer32 += VisibleArea_Width;
			
			// move source buffer to next line up
			FramePixel_Y--;
		}
	}
	else
	{
		// even scanline field //
		
		// check that current_y is even
		if ( current_y & 1 )
		{
			// current_y is odd //
			
			// move current_y down
			current_y++;
			
			// move destination buffer to next line
			ptr_pixelbuffer32 += VisibleArea_Width;
			
			// move source buffer to next line up
			FramePixel_Y--;
		}
	}
#endif


	
		while ( current_y < current_ymax )
		{
#ifdef INLINE_DEBUG_DRAW_SCREEN_2
	debug << "\r\nGPU::Draw_Screen; drawing left border";
	debug << " current_y=" << dec << current_y;
#endif
			// put in the left border
			current_x = 0;

			while ( current_x < LeftBorder_Width )
			{
				*ptr_pixelbuffer32++ = 0;
				current_x++;
			}
			
#ifdef INLINE_DEBUG_DRAW_SCREEN_2
	debug << "\r\nGPU::Draw_Screen; drawing screen pixels";
	debug << " current_x=" << dec << current_x;
	debug << " FramePixel_X=" << FramePixel_X;
	debug << " FramePixel_Y=" << FramePixel_Y;
#endif

			// *** important note *** this wraps around the VRAM
			//ptr_vram = & (VRAM [ FramePixel_X + ( FramePixel_Y << 10 ) ]);
			ptr_vram16 = & (VRAM [ FramePixel_X + ( ( FramePixel_Y & FrameBuffer_YMask ) << 10 ) ]);
			//ptr_vram32 = (u32*) ptr_vram;
			
#ifdef INLINE_DEBUG_DRAW_SCREEN_2
	debug << "\r\nGPU::Draw_Screen; got vram ptr";
#endif

			// put in screeen pixels
			if ( !GPU_CTRL_Read.ISRGB24 )
			{
#ifdef INLINE_DEBUG_DRAW_SCREEN_2
	debug << "\r\nGPU::Draw_Screen; !ISRGB24";
#endif

				while ( current_x < current_xmax )
				{
//#ifdef INLINE_DEBUG_DRAW_SCREEN
//	debug << "\r\ndrawx1; current_x=" << current_x;
//#endif

					Pixel16 = *ptr_vram16++;
					//Pixel64 = *ptr_vram16++;
					
					// the previous pixel conversion is wrong
					Pixel32_0 = ( ( Pixel16 & 0x1f ) << 3 ) | ( ( Pixel16 & 0x3e0 ) << 6 ) | ( ( Pixel16 & 0x7c00 ) << 9 );
					//Pixel32_0 = ( ( Pixel16 & 0x1f ) << 3 ) | ( ( Pixel16 & 0x3e0 ) << 6 ) | ( ( Pixel16 & 0x7c00 ) << 9 ) |
					//			( ( Pixel16 & 0x1c ) >> 2 ) | ( ( Pixel16 & 0x380 ) << 1 ) | ( ( Pixel16 & 7000 ) << 4 );
					//Pixel32_0 = ( ( Pixel64 & 0x1f0000001fLL ) << 3 ) | ( ( Pixel64 & 0x3e0000003e0LL ) << 6 ) | ( ( Pixel64 & 0x7c0000007c00LL ) << 9 ) |
					//			( ( Pixel64 & 0x1c0000001cLL ) >> 2 ) | ( ( Pixel64 & 0x38000000380LL ) << 1 ) | ( ( Pixel64 & 0x700000007000LL ) << 4 );
					*ptr_pixelbuffer32++ = Pixel32_0;
					current_x++;
				}
			}
			else
			{
				while ( current_x < current_xmax )
				{
					Pixel24.Pixel0 = *ptr_vram16++;
					Pixel24.Pixel1 = *ptr_vram16++;
					Pixel24.Pixel2 = *ptr_vram16++;
					
					// draw first pixel
					Pixel32_0 = ( ((u32)Pixel24.Red0) ) | ( ((u32)Pixel24.Green0) << 8 ) | ( ((u32)Pixel24.Blue0) << 16 );
					
					// draw second pixel
					Pixel32_1 = ( ((u32)Pixel24.Red1) ) | ( ((u32)Pixel24.Green1) << 8 ) | ( ((u32)Pixel24.Blue1) << 16 );
					
					*ptr_pixelbuffer32++ = Pixel32_0;
					*ptr_pixelbuffer32++ = Pixel32_1;
					current_x += 2;
				}
			}
			
#ifdef INLINE_DEBUG_DRAW_SCREEN_2
	debug << "\r\nGPU::Draw_Screen; drawing right border";
	debug << " current_x=" << dec << current_x;
#endif

			// put in right border
			while ( current_x < VisibleArea_Width )
			{
				*ptr_pixelbuffer32++ = 0;
				current_x++;
			}
			
			// go to next line in frame buffer
			FramePixel_Y--;
			
			current_y++;
			
#ifdef ENABLE_SCANLINES_SIMULATION
			// check if this is 480i
			if ( GPU_CTRL_Read.ISINTER && GPU_CTRL_Read.HEIGHT )
			{
				// 480i mode //
				
				// jump one more line in source image
				FramePixel_Y--;
			}
			
			// jump one more line in destination image
			current_y++;
			
			// also go to next line in destination buffer
			ptr_pixelbuffer32 += VisibleArea_Width;
#endif
		}
		
#ifdef INLINE_DEBUG_DRAW_SCREEN_2
	debug << "\r\nGPU::Draw_Screen; Drawing top border";
#endif

		// put in top border //
		
		// get number of pixels to black out
		//current_size = TopBorder_Height * VisibleArea_Width;
		current_size = ( VisibleArea_Height - current_y ) * VisibleArea_Width;
		current_x = 0;

		while ( current_x < current_size )
		{
			*ptr_pixelbuffer32++ = 0;
			current_x++;
		}
	}
	else
	{
		// display disabled //
		
		current_size = VisibleArea_Height * VisibleArea_Width;
		current_x = 0;

		while ( current_x < current_size )
		{
			*ptr_pixelbuffer32++ = 0;
			current_x++;
		}

	}

	
#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << "\r\nGPU::Draw_Screen; DisplayOutput_Window->OpenGL_MakeCurrentWindow";
#endif
		
	// *** output of pixel buffer to screen *** //

	// make this the current window we are drawing to
	DisplayOutput_Window->OpenGL_MakeCurrentWindow ();

#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << "\r\nGPU::Draw_Screen; glPixelZoom";
#endif
	
	//glPixelZoom ( (float)MainProgramWindow_Width / (float)DrawWidth, (float)MainProgramWindow_Height / (float)DrawHeight );
	//glDrawPixels ( DrawWidth, DrawHeight, GL_RGBA, GL_UNSIGNED_BYTE, (const GLvoid*) PixelBuffer );
	glPixelZoom ( (float)MainProgramWindow_Width / (float)VisibleArea_Width, (float)MainProgramWindow_Height / (float)VisibleArea_Height );
	
#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << "\r\nGPU::Draw_Screen; glDrawPixels";
#endif
	
	glDrawPixels ( VisibleArea_Width, VisibleArea_Height, GL_RGBA, GL_UNSIGNED_BYTE, (const GLvoid*) PixelBuffer );

#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << "\r\nGPU::Draw_Screen; DisplayOutput_Window->FlipScreen";
#endif
	
	// update screen
	DisplayOutput_Window->FlipScreen ();

#ifdef INLINE_DEBUG_DRAW_SCREEN
	debug << "\r\nGPU::Draw_Screen; DisplayOutput_Window->OpenGL_ReleaseWindow";
#endif
	
	// this is no longer the current window we are drawing to
	DisplayOutput_Window->OpenGL_ReleaseWindow ();
}

#endif


// copies gpu copy of vram to local copy of vram
void GPU::Copy_FrameData ()
{
	u32 *pBuf32;

	DisplayOutput_Window->OpenGL_MakeCurrentWindow ();

	glBindBuffer ( GL_COPY_WRITE_BUFFER, ssbo_outputdata );

	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo_pixelbuffer);

	glCopyBufferSubData ( GL_SHADER_STORAGE_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, 1024 * 512 * 4 );


	//pBuf32 = (u32*) glMapBuffer ( GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY );

	glFinish ();

	DisplayOutput_Window->OpenGL_ReleaseWindow ();

	//for ( int i = 0; i < 1024 * 512; i++ )
	//{
	//	VRAM [ i ] = (u16) pBuf32 [ i ];
	//}
	//for ( int i = 0; i < 1024 * 512; i++ )
	//{
	//	VRAM [ i ] = (u16) p_uHwOutputData32 [ i ];
	//}

	memcpy ( PixelBuffer, p_uHwOutputData32, 1024 * 512 * 4 );

	//glUnmapBuffer( GL_SHADER_STORAGE_BUFFER );



}

void GPU::Copy_VRAM_toGPU ()
{
	u32 *pBuf32;
	int iIdx;


	DisplayOutput_Window->OpenGL_MakeCurrentWindow ();

	// make sure gpu is completely finished with anything
	GPU::Finish ();

	// make sure opengl is all synced up
	glFinish();

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo);

	pBuf32 = (u32*) glMapBuffer ( GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY );


	
	//////////////////////////////////////////////////////
	// transfer pixel of image from VRAM
	for ( iIdx = 0; iIdx < 1024 * 512; iIdx++ )
	{
		//VRAM [ iIdx ] = (u16) pBuf32 [ iIdx ];
		pBuf32 [ iIdx ] = (u32) VRAM [ iIdx ];
	}

	

	glUnmapBuffer( GL_SHADER_STORAGE_BUFFER );


	DisplayOutput_Window->OpenGL_ReleaseWindow ();

}

void GPU::Copy_VRAM_toCPU ()
{
	u32 *pBuf32;
	int iIdx;


	DisplayOutput_Window->OpenGL_MakeCurrentWindow ();

	// make sure gpu is completely finished with anything
	GPU::Finish ();

	// make sure opengl is all synced up
	glFinish();

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo);

	pBuf32 = (u32*) glMapBuffer ( GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY );


	
	//////////////////////////////////////////////////////
	// transfer pixel of image from VRAM
	for ( iIdx = 0; iIdx < 1024 * 512; iIdx++ )
	{
		VRAM [ iIdx ] = (u16) pBuf32 [ iIdx ];
		//pBuf32 [ iIdx ] = (u32) VRAM [ iIdx ];
	}
	

	glUnmapBuffer( GL_SHADER_STORAGE_BUFFER );


	DisplayOutput_Window->OpenGL_ReleaseWindow ();

}



void GPU::Draw_FrameBuffer ()
{
	u32 Pixel, FramePixel;
	s32 Pixel_X, Pixel_Y;

	u32 *pBuf32;
	
	FramePixel = 0;


	if ( bEnable_OpenCL )
	{

		DisplayOutput_Window->OpenGL_MakeCurrentWindow ();


		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo);

		pBuf32 = (u32*) glMapBuffer ( GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY );

		for ( Pixel_Y = FrameBuffer_Height - 1; Pixel_Y >= 0; Pixel_Y-- )
		{
			for ( Pixel_X = 0; Pixel_X < FrameBuffer_Width; Pixel_X++ )
			{
				Pixel = pBuf32 [ Pixel_X + ( Pixel_Y << 10 ) ];
				PixelBuffer [ FramePixel++ ] = ( ( Pixel & 0x1f ) << ( 3 ) ) | ( ( (Pixel >> 5) & 0x1f ) << ( 3 + 8 ) ) | ( ( (Pixel >> 10) & 0x1f ) << ( 3 + 16 ) );
			}
		}

		glUnmapBuffer( GL_SHADER_STORAGE_BUFFER );


		DisplayOutput_Window->OpenGL_ReleaseWindow ();
	}
	else
	{

	
		/////////////////////////////////////////////////////////////////
		// Draw contents of frame buffer
		for ( Pixel_Y = FrameBuffer_Height - 1; Pixel_Y >= 0; Pixel_Y-- )
		{
			for ( Pixel_X = 0; Pixel_X < FrameBuffer_Width; Pixel_X++ )
			{
				Pixel = VRAM [ Pixel_X + ( Pixel_Y << 10 ) ];
				PixelBuffer [ FramePixel++ ] = ( ( Pixel & 0x1f ) << ( 3 ) ) | ( ( (Pixel >> 5) & 0x1f ) << ( 3 + 8 ) ) | ( ( (Pixel >> 10) & 0x1f ) << ( 3 + 16 ) );
			}
		}

	}
	
	// make this the current window we are drawing to
	FrameBuffer_DebugWindow->OpenGL_MakeCurrentWindow ();

	//glPixelZoom ( (float)MainProgramWindow_Width / (float)DrawWidth, (float)MainProgramWindow_Height / (float)DrawHeight );
	glDrawPixels ( FrameBuffer_Width, FrameBuffer_Height, GL_RGBA, GL_UNSIGNED_BYTE, (const GLvoid*) PixelBuffer );

	// update screen
	FrameBuffer_DebugWindow->FlipScreen ();
	
	// this is no longer the current window we are drawing to
	FrameBuffer_DebugWindow->OpenGL_ReleaseWindow ();
}



u64 GPU::DMA_ReadyForRead ( void )
{
	if ( *_DebugCycleCount >= _GPU->BusyUntil_Cycle )
	{
		// device is ready immediately
		return 1;
	}
	
	// device will be ready later at a specific cycle number
	return _GPU->BusyUntil_Cycle;
}

u64 GPU::DMA_ReadyForWrite ( void )
{
	if ( *_DebugCycleCount >= _GPU->BusyUntil_Cycle )
	{
		// device is ready immediately
		return 1;
	}
	
	// device will be ready later at a specific cycle number
	return _GPU->BusyUntil_Cycle;
}




void GPU::DMA_Read ( u32* Data, int ByteReadCount )
{
	u32 pix0, pix1;
#ifdef INLINE_DEBUG_DMA_READ
	debug << "\r\n\r\nDMA_Read " << hex << setw ( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Data = " << hex << Data [ 0 ];
#endif

	Data [ 0 ] = ProcessDataRegRead ();
}

u32 GPU::DMA_ReadBlock ( u32* pMemory, u32 Address, u32 BS )
{
	u32 *Data;
	
	Data = & ( pMemory [ Address >> 2 ] );
	
#ifdef INLINE_DEBUG_DMA_READ
	debug << "\r\n\r\nDMA_Read " << hex << setw ( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Data = " << hex << Data [ 0 ];
#endif

	for ( int i = 0; i < BS; i++ )
	{
		Data [ i ] = _GPU->ProcessDataRegRead ();
	}
	
	return BS;
}



// will use "55" to determine where you need termination code of 0x55555555
const u32 GPU_SizeOfCommands [ 256 ] = 
{
	/////////////////////////////////
	// GPU commands
	1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,	// 0x00-0x0f
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,	// 0x10-0x1f
	
	////////////////////////////////
	// Polygon commands
	4, 4, 4, 4, 7, 7, 7, 7, 5, 5, 5, 5, 9, 9, 9, 9,	// 0x20-0x2f
	6, 6, 6, 6, 9, 9, 9, 9, 8, 8, 8, 8, 12, 12, 12, 12,	// 0x30-0x3f
	
	///////////////////////////////////
	// Line commands
	3, 3, 3, 3, 3, 3, 3, 3, 55, 55, 55, 55, 55, 55, 55, 55,	// 0x40-0x4f
	4, 4, 4, 4, 4, 4, 4, 4, 66, 66, 66, 66, 66, 66, 66, 66,	// 0x50-0x5f
	
	/////////////////////////////////
	// Sprite commands
	3, 3, 3, 3, 4, 4, 4, 4, 2, 2, 2, 2, 0, 0, 0, 0,	// 0x60-0x6f
	2, 2, 2, 2, 3, 3, 3, 3, 2, 2, 2, 2, 3, 3, 3, 3,	// 0x70-0x7f
	
	//////////////////////////////////
	// Transfer commands
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,	// 0x80-0x8f
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,	// 0x90-0x9f
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,	// 0xa0-0xaf
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,	// 0xb0-0xbf
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,	// 0xc0-0xcf
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,	// 0xd0-0xdf
	
	///////////////////////////////////
	// Environment commands
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,	// 0xe0-0xef
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1	};	// 0xf0-0xff




void GPU::DMA_Write ( u32* Data, int ByteWriteCount )
{
#ifdef INLINE_DEBUG_DMA_WRITE
	debug << "\r\n\r\nDMA_Write " << hex << setw ( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount << "; Data = " << hex << Data [ 0 ];
#endif

	//ProcessDataRegWrite ( Data [ 0 ] );
	ProcessDataRegWrite ( Data, 1 );
}


//void GPU::DMA_WriteBlock ( u32* Data, u32 BS )
u32 GPU::DMA_WriteBlock ( u32* pMemory, u32 Address, u32 BS )
{
	u32 *Data;
	
	Data = & ( pMemory [ Address >> 2 ] );
	
#ifdef INLINE_DEBUG_DMA_WRITE
	debug << "\r\n\r\nDMA_WriteBlock " << hex << setw ( 8 ) << *_DebugPC << " BS=" << BS << " " << dec << *_DebugCycleCount << hex << "; Data= ";		//<< hex << Data [ 0 ];
	for ( int i = 0; i < BS; i++ ) debug << " " << (u32) Data [ i ];
	debug << "\r\n";
#endif



	//for ( int i = 0; i < BS; i++ )
	//{
	//	_GPU->ProcessDataRegWrite ( Data [ i ] );
	//}
	_GPU->ProcessDataRegWrite ( Data, BS );
	
	
	/*
#ifdef ENABLE_DRAW_OVERHEAD
	// this all actually happens AFTER the DMA transfer is done, so need to add in the time it takes to transfer the data
	// since the DMA only updates this AFTER the transfer, and this is DURING the transfer
	if ( _GPU->BusyCycles )
	{
		if ( _GPU->BusyUntil_Cycle <= ( R3000A::Cpu::_CPU->CycleCount + BS ) )
		{
			// so, add in the time it takes to transfer for DMA
			_GPU->BusyUntil_Cycle = ( R3000A::Cpu::_CPU->CycleCount + BS + 1 );
		}
	}
#endif
	*/

	return BS;
}


void GPU::ExecuteGPUBuffer ()
{
	static const u32 CyclesPerSpritePixel = 2;
	
	s32 tri_area;
	volatile u32 *inputdata_ptr;
	volatile u32 *inputdata_ptr2;
	u64 NumPixels;
	
	command_tge = Buffer [ 0 ].Command & 1;
	command_abe = ( Buffer [ 0 ].Command >> 1 ) & 1;
	
	// clear busy cycles
	BusyCycles = 0;
	
	
	

	////////////////////////////////////////
	// Check what the command is
	switch ( Buffer [ 0 ].Command /* & 0xfc */ )
	{
		////////////////////////////////////////
		// GPU Commands
		
		case 0x00:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME
			debug << "\r\nMust be ResetGPUArgBuffer";
#endif

			BufferSize = 0;
			break;
		
		case 0x01:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME
			debug << "\r\nClearCache";
#endif
			///////////////////////////////////////////
			// clear cache
			break;
			
		case 0x02:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME
			debug << "\r\nFrameBufferRectangle";
#endif
			///////////////////////////////////////////
			// frame buffer rectangle draw
			//GetBGR ( Buffer [ 0 ] );
			//GetXY ( Buffer [ 1 ] );
			//GetHW ( Buffer [ 2 ] );
			
			
			
#if defined INLINE_DEBUG_EXECUTE
			debug << dec << " x=" << x << " y=" << y << " h=" << h << " w=" << w << hex << " bgr=" << gbgr[0];
#endif

				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 9 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;

				
			NumPixels = Draw_FrameBufferRectangle_02_th ( (DATA_Write_Format*) inputdata_ptr, 0 );
			BusyCycles += ( NumPixels * dFrameBufferRectangle_02_CyclesPerPixel );

				ulInputBuffer_WriteIndex++;
			
#if defined INLINE_DEBUG_EXECUTE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;
			
			
		case 0x03:
		
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME
			debug << "\r\nUnknown Command " << hex << (u32) Buffer [ 0 ].Command;
#endif

			// *** Unknown *** //
			// command doesn't seem to have a noticeable effect
			//cout << "\nhps1x64 WARNING: Unknown GPU command: " << hex << (u32) Buffer [ 0 ].Command << "\n";
			
			break;
			
		
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
		case 0x0c:
		case 0x0d:
		case 0x0e:
		case 0x0f:
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME
			debug << "\r\nUnknown Command " << hex << (u32) Buffer [ 0 ].Command;
#endif

			// *** Unknown *** //
			
#ifdef VERBOSE_GPU
			cout << "\nhps1x64 WARNING: Unknown GPU command: " << hex << (u32) Buffer [ 0 ].Command << "\n";
#endif
			
			break;
			
			
		case 0x1f:
		
			// Interrupt Request //
			cout << "\nhps1x64 WARNING: GPU Interrupt Requested\n";
			
			break;
			
		//////////////////////////////////////////////////
		// Polygon commands
			
		case 0x20:
		case 0x21:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_MONO
			debug << "\r\nMonochromeTriangle Abe=0";
			debug << hex << *_DebugPC << " " << dec << *_DebugCycleCount;
#endif

			///////////////////////////////////////////////
			// monochrome 3-point polygon
			// *** TODO *** BGR should be passed as 24-bit color value, not 15-bit
			//GetBGR24 ( Buffer [ 0 ] );
			//GetXY0 ( Buffer [ 1 ] );
			//GetXY1 ( Buffer [ 2 ] );
			//GetXY2 ( Buffer [ 3 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_MONO
			debug << "\r\n" << dec << " x0=" << gx[0] << " y0=" << gy[0] << " x1=" << gx[1] << " y1=" << gy[1] << " x2=" << gx[2] << " y2=" << gy[2] << hex << " bgr=" << gbgr[0];
			debug << dec << "\r\nDrawArea: OffsetX=" << DrawArea_OffsetX << " OffsetY=" << DrawArea_OffsetY << " TopLeftX=" << DrawArea_TopLeftX << " TopLeftY=" << DrawArea_TopLeftY << " BottomRightX=" << DrawArea_BottomRightX << " BottomRightY=" << DrawArea_BottomRightY;
#endif

				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 11 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 14 ] = Buffer [ 3 ].Value;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;


				ulInputBuffer_WriteIndex++;
			
#ifdef USE_TEMPLATES_TRIANGLE_MONO
			Draw_MonoTriangle_20_t <0> ( 0, 1, 2 );
#else
			//Draw_MonoTriangle_20 ( 0, 1, 2 );
			Draw_MonoTriangle_20 ( (DATA_Write_Format*) inputdata_ptr, 0 );
#endif
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_MONO
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;

			
		case 0x22:
		case 0x23:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_MONO
			debug << "\r\nMonochromeTriangle Abe=1";
			debug << hex << *_DebugPC << " " << dec << *_DebugCycleCount;
#endif
		
			///////////////////////////////////////////////
			// monochrome 3-point polygon
			//GetBGR24 ( Buffer [ 0 ] );
			//GetXY0 ( Buffer [ 1 ] );
			//GetXY1 ( Buffer [ 2 ] );
			//GetXY2 ( Buffer [ 3 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_MONO
			debug << "\r\n" << dec << " x0=" << gx[0] << " y0=" << gy[0] << " x1=" << gx[1] << " y1=" << gy[1] << " x2=" << gx[2] << " y2=" << gy[2] << hex << " bgr=" << gbgr[0];
			debug << dec << "\r\nDrawArea: OffsetX=" << DrawArea_OffsetX << " OffsetY=" << DrawArea_OffsetY << " TopLeftX=" << DrawArea_TopLeftX << " TopLeftY=" << DrawArea_TopLeftY << " BottomRightX=" << DrawArea_BottomRightX << " BottomRightY=" << DrawArea_BottomRightY;
#endif

				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 11 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 14 ] = Buffer [ 3 ].Value;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;


				ulInputBuffer_WriteIndex++;
				
#ifdef USE_TEMPLATES_TRIANGLE_MONO
			Draw_MonoTriangle_20_t <1> ( 0, 1, 2 );
#else
			//Draw_MonoTriangle_20 ( 0, 1, 2 );
			Draw_MonoTriangle_20 ( (DATA_Write_Format*) inputdata_ptr, 0 );
#endif

#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_MONO
			debug << ";BusyCycles=" << BusyCycles;
#endif

			break;

			
		
		
			
		case 0x24:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_TEXTURE
			debug << "\r\nTexturedTriangle";
			debug << hex << *_DebugPC << " " << dec << *_DebugCycleCount;
#endif
			///////////////////////////////////////////////
			// textured 3-point polygon
			GetBGR24 ( Buffer [ 0 ] );
			GetXY0 ( Buffer [ 1 ] );
			GetUV0 ( Buffer [ 2 ] );
			GetCLUT ( Buffer [ 2 ] );
			GetXY1 ( Buffer [ 3 ] );
			GetUV1 ( Buffer [ 4 ] );
			GetTPAGE ( Buffer [ 4 ] );
			GetXY2 ( Buffer [ 5 ] );
			GetUV2 ( Buffer [ 6 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_TEXTURE
			debug << "\r\n" << dec << " x0=" << gx[0] << " y0=" << gy[0] << " x1=" << gx[1] << " y1=" << gy[1] << " x2=" << gx[2] << " y2=" << gy[2] << hex << " bgr=" << gbgr[0];
			debug << "\r\n" << dec << " u0=" << gu[0] << " v0=" << gv[0] << " u1=" << gu[1] << " v1=" << gv[1] << " u2=" << gu[2] << " v2=" << gv[2];
			debug << "\r\n" << dec << "clut_x=" << clut_x << " clut_y=" << clut_y << " tx=" << tpage_tx << " ty=" << tpage_ty << " tp=" << tpage_tp << " abr=" << tpage_abr;
			debug << dec << "\r\nDrawArea: OffsetX=" << DrawArea_OffsetX << " OffsetY=" << DrawArea_OffsetY << " TopLeftX=" << DrawArea_TopLeftX << " TopLeftY=" << DrawArea_TopLeftY << " BottomRightX=" << DrawArea_BottomRightX << " BottomRightY=" << DrawArea_BottomRightY;
#endif
			
				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				inputdata_ptr [ 4 ] = TextureWindow;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 9 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 11 ] = Buffer [ 3 ].Value;
				inputdata_ptr [ 12 ] = Buffer [ 4 ].Value;
				inputdata_ptr [ 14 ] = Buffer [ 5 ].Value;
				inputdata_ptr [ 15 ] = Buffer [ 6 ].Value;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;
				
				ulInputBuffer_WriteIndex++;

#ifdef USE_TEMPLATES_TRIANGLE_TEXTURE
			Draw_TextureTriangle_24_t <0,0> ( 0, 1, 2 );
#else
			//Draw_TextureTriangle_24 ( 0, 1, 2 );
			Draw_TextureTriangle_24 ( (DATA_Write_Format*) inputdata_ptr, 0 );
#endif
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_TEXTURE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;


		case 0x25:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_TEXTURE
			debug << "\r\nTexturedTriangle Abe=0 Tge=1";
			debug << hex << *_DebugPC << " " << dec << *_DebugCycleCount;
#endif
			///////////////////////////////////////////////
			// textured 3-point polygon
			GetBGR24 ( Buffer [ 0 ] );
			GetXY0 ( Buffer [ 1 ] );
			GetUV0 ( Buffer [ 2 ] );
			GetCLUT ( Buffer [ 2 ] );
			GetXY1 ( Buffer [ 3 ] );
			GetUV1 ( Buffer [ 4 ] );
			GetTPAGE ( Buffer [ 4 ] );
			GetXY2 ( Buffer [ 5 ] );
			GetUV2 ( Buffer [ 6 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_TEXTURE
			debug << "\r\n" << dec << " x0=" << gx[0] << " y0=" << gy[0] << " x1=" << gx[1] << " y1=" << gy[1] << " x2=" << gx[2] << " y2=" << gy[2] << hex << " bgr=" << gbgr[0];
			debug << "\r\n" << dec << " u0=" << gu[0] << " v0=" << gv[0] << " u1=" << gu[1] << " v1=" << gv[1] << " u2=" << gu[2] << " v2=" << gv[2];
			debug << "\r\n" << dec << "clut_x=" << clut_x << " clut_y=" << clut_y << " tx=" << tpage_tx << " ty=" << tpage_ty << " tp=" << tpage_tp << " abr=" << tpage_abr;
			debug << dec << "\r\nDrawArea: OffsetX=" << DrawArea_OffsetX << " OffsetY=" << DrawArea_OffsetY << " TopLeftX=" << DrawArea_TopLeftX << " TopLeftY=" << DrawArea_TopLeftY << " BottomRightX=" << DrawArea_BottomRightX << " BottomRightY=" << DrawArea_BottomRightY;
#endif
			
				
				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				inputdata_ptr [ 4 ] = TextureWindow;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 9 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 11 ] = Buffer [ 3 ].Value;
				inputdata_ptr [ 12 ] = Buffer [ 4 ].Value;
				inputdata_ptr [ 14 ] = Buffer [ 5 ].Value;
				inputdata_ptr [ 15 ] = Buffer [ 6 ].Value;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;
				
				ulInputBuffer_WriteIndex++;
			
#ifdef USE_TEMPLATES_TRIANGLE_TEXTURE
			Draw_TextureTriangle_24_t <0,1> ( 0, 1, 2 );
#else
			//Draw_TextureTriangle_24 ( 0, 1, 2 );
			Draw_TextureTriangle_24 ( (DATA_Write_Format*) inputdata_ptr, 0 );
#endif
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_TEXTURE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;
		
		case 0x26:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_TEXTURE
			debug << "\r\nTexturedTriangle Abe=1 Tge=0";
			debug << hex << *_DebugPC << " " << dec << *_DebugCycleCount;
#endif
			///////////////////////////////////////////////
			// textured 3-point polygon
			GetBGR24 ( Buffer [ 0 ] );
			GetXY0 ( Buffer [ 1 ] );
			GetUV0 ( Buffer [ 2 ] );
			GetCLUT ( Buffer [ 2 ] );
			GetXY1 ( Buffer [ 3 ] );
			GetUV1 ( Buffer [ 4 ] );
			GetTPAGE ( Buffer [ 4 ] );
			GetXY2 ( Buffer [ 5 ] );
			GetUV2 ( Buffer [ 6 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_TEXTURE
			debug << "\r\n" << dec << " x0=" << gx[0] << " y0=" << gy[0] << " x1=" << gx[1] << " y1=" << gy[1] << " x2=" << gx[2] << " y2=" << gy[2] << hex << " bgr=" << gbgr[0];
			debug << "\r\n" << dec << " u0=" << gu[0] << " v0=" << gv[0] << " u1=" << gu[1] << " v1=" << gv[1] << " u2=" << gu[2] << " v2=" << gv[2];
			debug << "\r\n" << dec << "clut_x=" << clut_x << " clut_y=" << clut_y << " tx=" << tpage_tx << " ty=" << tpage_ty << " tp=" << tpage_tp << " abr=" << tpage_abr;
			debug << dec << "\r\nDrawArea: OffsetX=" << DrawArea_OffsetX << " OffsetY=" << DrawArea_OffsetY << " TopLeftX=" << DrawArea_TopLeftX << " TopLeftY=" << DrawArea_TopLeftY << " BottomRightX=" << DrawArea_BottomRightX << " BottomRightY=" << DrawArea_BottomRightY;
#endif
			
				
				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				inputdata_ptr [ 4 ] = TextureWindow;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 9 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 11 ] = Buffer [ 3 ].Value;
				inputdata_ptr [ 12 ] = Buffer [ 4 ].Value;
				inputdata_ptr [ 14 ] = Buffer [ 5 ].Value;
				inputdata_ptr [ 15 ] = Buffer [ 6 ].Value;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;
				
				ulInputBuffer_WriteIndex++;

#ifdef USE_TEMPLATES_TRIANGLE_TEXTURE
			Draw_TextureTriangle_24_t <1,0> ( 0, 1, 2 );
#else
			//Draw_TextureTriangle_24 ( 0, 1, 2 );
			Draw_TextureTriangle_24 ( (DATA_Write_Format*) inputdata_ptr, 0 );
#endif
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_TEXTURE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;
		
		
		case 0x27:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_TEXTURE
			debug << "\r\nTexturedTriangle Abe=1 Tge=1";
			debug << hex << *_DebugPC << " " << dec << *_DebugCycleCount;
#endif
			///////////////////////////////////////////////
			// textured 3-point polygon
			GetBGR24 ( Buffer [ 0 ] );
			GetXY0 ( Buffer [ 1 ] );
			GetUV0 ( Buffer [ 2 ] );
			GetCLUT ( Buffer [ 2 ] );
			GetXY1 ( Buffer [ 3 ] );
			GetUV1 ( Buffer [ 4 ] );
			GetTPAGE ( Buffer [ 4 ] );
			GetXY2 ( Buffer [ 5 ] );
			GetUV2 ( Buffer [ 6 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_TEXTURE
			debug << "\r\n" << dec << " x0=" << gx[0] << " y0=" << gy[0] << " x1=" << gx[1] << " y1=" << gy[1] << " x2=" << gx[2] << " y2=" << gy[2] << hex << " bgr=" << gbgr[0];
			debug << "\r\n" << dec << " u0=" << gu[0] << " v0=" << gv[0] << " u1=" << gu[1] << " v1=" << gv[1] << " u2=" << gu[2] << " v2=" << gv[2];
			debug << "\r\n" << dec << "clut_x=" << clut_x << " clut_y=" << clut_y << " tx=" << tpage_tx << " ty=" << tpage_ty << " tp=" << tpage_tp << " abr=" << tpage_abr;
			debug << dec << "\r\nDrawArea: OffsetX=" << DrawArea_OffsetX << " OffsetY=" << DrawArea_OffsetY << " TopLeftX=" << DrawArea_TopLeftX << " TopLeftY=" << DrawArea_TopLeftY << " BottomRightX=" << DrawArea_BottomRightX << " BottomRightY=" << DrawArea_BottomRightY;
#endif
				
				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				inputdata_ptr [ 4 ] = TextureWindow;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 9 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 11 ] = Buffer [ 3 ].Value;
				inputdata_ptr [ 12 ] = Buffer [ 4 ].Value;
				inputdata_ptr [ 14 ] = Buffer [ 5 ].Value;
				inputdata_ptr [ 15 ] = Buffer [ 6 ].Value;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;
				
				ulInputBuffer_WriteIndex++;
			
#ifdef USE_TEMPLATES_TRIANGLE_TEXTURE
			Draw_TextureTriangle_24_t <1,1> ( 0, 1, 2 );
#else
			//Draw_TextureTriangle_24 ( 0, 1, 2 );
			Draw_TextureTriangle_24 ( (DATA_Write_Format*) inputdata_ptr, 0 );
#endif
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_TEXTURE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;
		
		
			
		case 0x28:
		case 0x29:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_MONO
			debug << "\r\nMonochromeRectangle Abe=0";
			debug << hex << *_DebugPC << " " << dec << *_DebugCycleCount;
#endif
			//////////////////////////////////////////////
			// monochrome 4-point polygon
			GetBGR24 ( Buffer [ 0 ] );
			GetXY0 ( Buffer [ 1 ] );
			GetXY1 ( Buffer [ 2 ] );
			GetXY2 ( Buffer [ 3 ] );
			GetXY3 ( Buffer [ 4 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_MONO
			debug << dec << " x0=" << gx[0] << " y0=" << gy[0] << " x1=" << gx[1] << " y1=" << gy[1] << " x2=" << gx[2] << " y2=" << gy[2] << " x3=" << gx[3] << " y3=" << gy[3] << " bgr=" << hex << gbgr[0];
			debug << dec << "\r\nDrawArea: OffsetX=" << DrawArea_OffsetX << " OffsetY=" << DrawArea_OffsetY << " TopLeftX=" << DrawArea_TopLeftX << " TopLeftY=" << DrawArea_TopLeftY << " BottomRightX=" << DrawArea_BottomRightX << " BottomRightY=" << DrawArea_BottomRightY;
#endif

				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				inputdata_ptr2 = & ( inputdata [ ( ( ulInputBuffer_WriteIndex + 1 ) & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 11 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 14 ] = Buffer [ 3 ].Value;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;
				
				ulInputBuffer_WriteIndex++;
			

				inputdata_ptr2 [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr2 [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr2 [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr2 [ 3 ] = DrawArea_Offset;

				inputdata_ptr2 [ 8 ] = Buffer [ 4 ].Value;
				inputdata_ptr2 [ 11 ] = Buffer [ 2 ].Value;
				inputdata_ptr2 [ 14 ] = Buffer [ 3 ].Value;
				inputdata_ptr2 [ 7 ] = Buffer [ 0 ].Value;
				
				ulInputBuffer_WriteIndex++;




#ifdef USE_TEMPLATES_RECTANGLE_MONO
			Draw_MonoRectangle_28_t <0> ();
#else
			//Draw_MonoRectangle_28 ();
			//Draw_MonoRectangle_28 ( inputdata_ptr, 0 );
			Draw_MonoTriangle_20 ( (DATA_Write_Format*) inputdata_ptr, 0 );
			Draw_MonoTriangle_20 ( (DATA_Write_Format*) inputdata_ptr2, 0 );
#endif
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_MONO
			debug << ";BusyCycles=" << BusyCycles << ";DrawArea_TopLeftX=" << DrawArea_TopLeftX << ";DrawArea_OffsetX=" << DrawArea_OffsetX << ";DrawArea_TopLeftY=" << DrawArea_TopLeftY << ";DrawArea_OffsetY=" << DrawArea_OffsetY;
#endif
			break;
			
			
		
		
		case 0x2a:
		case 0x2b:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_MONO
			debug << "\r\nMonochromeRectangle Abe=1";
			debug << hex << *_DebugPC << " " << dec << *_DebugCycleCount;
#endif
			//////////////////////////////////////////////
			// monochrome 4-point polygon
			GetBGR24 ( Buffer [ 0 ] );
			GetXY0 ( Buffer [ 1 ] );
			GetXY1 ( Buffer [ 2 ] );
			GetXY2 ( Buffer [ 3 ] );
			GetXY3 ( Buffer [ 4 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_MONO
			debug << dec << " x0=" << gx[0] << " y0=" << gy[0] << " x1=" << gx[1] << " y1=" << gy[1] << " x2=" << gx[2] << " y2=" << gy[2] << " x3=" << gx[3] << " y3=" << gy[3] << " bgr=" << hex << gbgr[0];
			debug << dec << "\r\nDrawArea: OffsetX=" << DrawArea_OffsetX << " OffsetY=" << DrawArea_OffsetY << " TopLeftX=" << DrawArea_TopLeftX << " TopLeftY=" << DrawArea_TopLeftY << " BottomRightX=" << DrawArea_BottomRightX << " BottomRightY=" << DrawArea_BottomRightY;
#endif
				
				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				inputdata_ptr2 = & ( inputdata [ ( ( ulInputBuffer_WriteIndex + 1 ) & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 11 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 14 ] = Buffer [ 3 ].Value;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;
				
				ulInputBuffer_WriteIndex++;
			

				inputdata_ptr2 [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr2 [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr2 [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr2 [ 3 ] = DrawArea_Offset;

				inputdata_ptr2 [ 8 ] = Buffer [ 4 ].Value;
				inputdata_ptr2 [ 11 ] = Buffer [ 2 ].Value;
				inputdata_ptr2 [ 14 ] = Buffer [ 3 ].Value;
				inputdata_ptr2 [ 7 ] = Buffer [ 0 ].Value;

				ulInputBuffer_WriteIndex++;
			


#ifdef USE_TEMPLATES_RECTANGLE_MONO
			Draw_MonoRectangle_28_t <1> ();
#else
			//Draw_MonoRectangle_28 ();
			//Draw_MonoRectangle_28 ( inputdata_ptr, 0 );
			Draw_MonoTriangle_20 ( (DATA_Write_Format*) inputdata_ptr, 0 );
			Draw_MonoTriangle_20 ( (DATA_Write_Format*) inputdata_ptr2, 0 );
#endif
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_MONO
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;
		
		
			
		case 0x2c:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_TEXTURE
			debug << "\r\nTexturedRectangle";
			debug << hex << *_DebugPC << " " << dec << *_DebugCycleCount;
#endif

			////////////////////////////////////////////
			// textured 4-point polygon
			GetBGR24 ( Buffer [ 0 ] );
			GetXY0 ( Buffer [ 1 ] );
			GetCLUT ( Buffer [ 2 ] );
			GetUV0 ( Buffer [ 2 ] );
			GetXY1 ( Buffer [ 3 ] );
			GetTPAGE ( Buffer [ 4 ] );
			GetUV1 ( Buffer [ 4 ] );
			GetXY2 ( Buffer [ 5 ] );
			GetUV2 ( Buffer [ 6 ] );
			GetXY3 ( Buffer [ 7 ] );
			GetUV3 ( Buffer [ 8 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_TEXTURE
			debug << dec << " x0=" << gx[0] << " y0=" << gy[0] << " x1=" << gx[1] << " y1=" << gy[1] << " x2=" << gx[2] << " y2=" << gy[2] << " x3=" << gx[3] << " y3=" << gy[3];
			debug << "\r\nu0=" << gu[0] << " v0=" << gv[0] << " u1=" << gu[1] << " v1=" << gv[1] << " u2=" << gu[2] << " v2=" << gv[2] << " u3=" << gu[3] << " v3=" << gv[3];
			debug << "\r\ntpage_tx=" << tpage_tx << " tpage_ty=" << tpage_ty << " tpage_abr=" << tpage_abr << " tpage_tp=" << tpage_tp << " clut_x=" << clut_x << " clut_y=" << clut_y << " bgr=" << hex << gbgr[0];
			debug << dec << "\r\nDrawArea: OffsetX=" << DrawArea_OffsetX << " OffsetY=" << DrawArea_OffsetY << " TopLeftX=" << DrawArea_TopLeftX << " TopLeftY=" << DrawArea_TopLeftY << " BottomRightX=" << DrawArea_BottomRightX << " BottomRightY=" << DrawArea_BottomRightY;
#endif
				
				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				inputdata_ptr2 = & ( inputdata [ ( ( ulInputBuffer_WriteIndex + 1 ) & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				inputdata_ptr [ 4 ] = TextureWindow;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 9 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 11 ] = Buffer [ 3 ].Value;
				inputdata_ptr [ 12 ] = Buffer [ 4 ].Value;
				inputdata_ptr [ 14 ] = Buffer [ 5 ].Value;
				inputdata_ptr [ 15 ] = Buffer [ 6 ].Value;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;
				
				ulInputBuffer_WriteIndex++;
			

				inputdata_ptr2 [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr2 [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr2 [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr2 [ 3 ] = DrawArea_Offset;
				inputdata_ptr2 [ 4 ] = TextureWindow;
				
				inputdata_ptr2 [ 8 ] = Buffer [ 7 ].Value;
				inputdata_ptr2 [ 9 ] = ( Buffer [ 2 ].Value & ~0xffff ) | ( Buffer [ 8 ].Value & 0xffff );
				inputdata_ptr2 [ 11 ] = Buffer [ 3 ].Value;
				inputdata_ptr2 [ 12 ] = Buffer [ 4 ].Value;
				inputdata_ptr2 [ 14 ] = Buffer [ 5 ].Value;
				inputdata_ptr2 [ 15 ] = Buffer [ 6 ].Value;
				inputdata_ptr2 [ 7 ] = Buffer [ 0 ].Value;
				
				ulInputBuffer_WriteIndex++;
			
#ifdef USE_TEMPLATES_RECTANGLE_TEXTURE
			Draw_TextureRectangle_2c_t <0,0> ();
#else
			//Draw_TextureRectangle_2c ();
			//Draw_TextureRectangle_2c ( inputdata_ptr, 0 );
			Draw_TextureTriangle_24 ( (DATA_Write_Format*) inputdata_ptr, 0 );
			Draw_TextureTriangle_24 ( (DATA_Write_Format*) inputdata_ptr2, 0 );
#endif
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_TEXTURE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;
			
		case 0x2d:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_TEXTURE
			debug << "\r\nTexturedRectangle Abe=0 Tge=1";
			debug << hex << *_DebugPC << " " << dec << *_DebugCycleCount;
#endif

			////////////////////////////////////////////
			// textured 4-point polygon
			GetBGR24 ( Buffer [ 0 ] );
			GetXY0 ( Buffer [ 1 ] );
			GetCLUT ( Buffer [ 2 ] );
			GetUV0 ( Buffer [ 2 ] );
			GetXY1 ( Buffer [ 3 ] );
			GetTPAGE ( Buffer [ 4 ] );
			GetUV1 ( Buffer [ 4 ] );
			GetXY2 ( Buffer [ 5 ] );
			GetUV2 ( Buffer [ 6 ] );
			GetXY3 ( Buffer [ 7 ] );
			GetUV3 ( Buffer [ 8 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_TEXTURE
			debug << dec << " x0=" << gx[0] << " y0=" << gy[0] << " x1=" << gx[1] << " y1=" << gy[1] << " x2=" << gx[2] << " y2=" << gy[2] << " x3=" << gx[3] << " y3=" << gy[3];
			debug << "\r\nu0=" << gu[0] << " v0=" << gv[0] << " u1=" << gu[1] << " v1=" << gv[1] << " u2=" << gu[2] << " v2=" << gv[2] << " u3=" << gu[3] << " v3=" << gv[3];
			debug << "\r\ntpage_tx=" << tpage_tx << " tpage_ty=" << tpage_ty << " tpage_abr=" << tpage_abr << " tpage_tp=" << tpage_tp << " clut_x=" << clut_x << " clut_y=" << clut_y << " bgr=" << hex << gbgr[0];
			debug << dec << "\r\nDrawArea: OffsetX=" << DrawArea_OffsetX << " OffsetY=" << DrawArea_OffsetY << " TopLeftX=" << DrawArea_TopLeftX << " TopLeftY=" << DrawArea_TopLeftY << " BottomRightX=" << DrawArea_BottomRightX << " BottomRightY=" << DrawArea_BottomRightY;
#endif

				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				inputdata_ptr2 = & ( inputdata [ ( ( ulInputBuffer_WriteIndex + 1 ) & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				inputdata_ptr [ 4 ] = TextureWindow;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 9 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 11 ] = Buffer [ 3 ].Value;
				inputdata_ptr [ 12 ] = Buffer [ 4 ].Value;
				inputdata_ptr [ 14 ] = Buffer [ 5 ].Value;
				inputdata_ptr [ 15 ] = Buffer [ 6 ].Value;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;
				
				ulInputBuffer_WriteIndex++;
			

				inputdata_ptr2 [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr2 [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr2 [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr2 [ 3 ] = DrawArea_Offset;
				inputdata_ptr2 [ 4 ] = TextureWindow;
				
				inputdata_ptr2 [ 8 ] = Buffer [ 7 ].Value;
				inputdata_ptr2 [ 9 ] = ( Buffer [ 2 ].Value & ~0xffff ) | ( Buffer [ 8 ].Value & 0xffff );
				inputdata_ptr2 [ 11 ] = Buffer [ 3 ].Value;
				inputdata_ptr2 [ 12 ] = Buffer [ 4 ].Value;
				inputdata_ptr2 [ 14 ] = Buffer [ 5 ].Value;
				inputdata_ptr2 [ 15 ] = Buffer [ 6 ].Value;
				inputdata_ptr2 [ 7 ] = Buffer [ 0 ].Value;

				ulInputBuffer_WriteIndex++;
			
#ifdef USE_TEMPLATES_RECTANGLE_TEXTURE
			Draw_TextureRectangle_2c_t <0,1> ();
#else
			//Draw_TextureRectangle_2c ();
			//Draw_TextureRectangle_2c ( inputdata_ptr, 0 );
			Draw_TextureTriangle_24 ( (DATA_Write_Format*) inputdata_ptr, 0 );
			Draw_TextureTriangle_24 ( (DATA_Write_Format*) inputdata_ptr2, 0 );
#endif
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_TEXTURE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;
		
		
		case 0x2e:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_TEXTURE
			debug << "\r\nTexturedRectangle Abe=1 Tge=0";
			debug << hex << *_DebugPC << " " << dec << *_DebugCycleCount;
#endif

			////////////////////////////////////////////
			// textured 4-point polygon
			GetBGR24 ( Buffer [ 0 ] );
			GetXY0 ( Buffer [ 1 ] );
			GetCLUT ( Buffer [ 2 ] );
			GetUV0 ( Buffer [ 2 ] );
			GetXY1 ( Buffer [ 3 ] );
			GetTPAGE ( Buffer [ 4 ] );
			GetUV1 ( Buffer [ 4 ] );
			GetXY2 ( Buffer [ 5 ] );
			GetUV2 ( Buffer [ 6 ] );
			GetXY3 ( Buffer [ 7 ] );
			GetUV3 ( Buffer [ 8 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_TEXTURE
			debug << dec << " x0=" << gx[0] << " y0=" << gy[0] << " x1=" << gx[1] << " y1=" << gy[1] << " x2=" << gx[2] << " y2=" << gy[2] << " x3=" << gx[3] << " y3=" << gy[3];
			debug << "\r\nu0=" << gu[0] << " v0=" << gv[0] << " u1=" << gu[1] << " v1=" << gv[1] << " u2=" << gu[2] << " v2=" << gv[2] << " u3=" << gu[3] << " v3=" << gv[3];
			debug << "\r\ntpage_tx=" << tpage_tx << " tpage_ty=" << tpage_ty << " tpage_abr=" << tpage_abr << " tpage_tp=" << tpage_tp << " clut_x=" << clut_x << " clut_y=" << clut_y << " bgr=" << hex << gbgr[0];
			debug << dec << "\r\nDrawArea: OffsetX=" << DrawArea_OffsetX << " OffsetY=" << DrawArea_OffsetY << " TopLeftX=" << DrawArea_TopLeftX << " TopLeftY=" << DrawArea_TopLeftY << " BottomRightX=" << DrawArea_BottomRightX << " BottomRightY=" << DrawArea_BottomRightY;
#endif

				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				inputdata_ptr2 = & ( inputdata [ ( ( ulInputBuffer_WriteIndex + 1 ) & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				inputdata_ptr [ 4 ] = TextureWindow;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 9 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 11 ] = Buffer [ 3 ].Value;
				inputdata_ptr [ 12 ] = Buffer [ 4 ].Value;
				inputdata_ptr [ 14 ] = Buffer [ 5 ].Value;
				inputdata_ptr [ 15 ] = Buffer [ 6 ].Value;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;
				
				ulInputBuffer_WriteIndex++;
			

				inputdata_ptr2 [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr2 [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr2 [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr2 [ 3 ] = DrawArea_Offset;
				inputdata_ptr2 [ 4 ] = TextureWindow;
				
				inputdata_ptr2 [ 8 ] = Buffer [ 7 ].Value;
				inputdata_ptr2 [ 9 ] = ( Buffer [ 2 ].Value & ~0xffff ) | ( Buffer [ 8 ].Value & 0xffff );
				inputdata_ptr2 [ 11 ] = Buffer [ 3 ].Value;
				inputdata_ptr2 [ 12 ] = Buffer [ 4 ].Value;
				inputdata_ptr2 [ 14 ] = Buffer [ 5 ].Value;
				inputdata_ptr2 [ 15 ] = Buffer [ 6 ].Value;
				inputdata_ptr2 [ 7 ] = Buffer [ 0 ].Value;
				
				ulInputBuffer_WriteIndex++;
			
#ifdef USE_TEMPLATES_RECTANGLE_TEXTURE
			Draw_TextureRectangle_2c_t <1,0> ();
#else
			//Draw_TextureRectangle_2c ();
			//Draw_TextureRectangle_2c ( inputdata_ptr, 0 );
			Draw_TextureTriangle_24 ( (DATA_Write_Format*) inputdata_ptr, 0 );
			Draw_TextureTriangle_24 ( (DATA_Write_Format*) inputdata_ptr2, 0 );
#endif
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_TEXTURE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;
		
		
		case 0x2f:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_TEXTURE
			debug << "\r\nTexturedRectangle Abe=1 Tge=1";
			debug << hex << *_DebugPC << " " << dec << *_DebugCycleCount;
#endif

			////////////////////////////////////////////
			// textured 4-point polygon
			GetBGR24 ( Buffer [ 0 ] );
			GetXY0 ( Buffer [ 1 ] );
			GetCLUT ( Buffer [ 2 ] );
			GetUV0 ( Buffer [ 2 ] );
			GetXY1 ( Buffer [ 3 ] );
			GetTPAGE ( Buffer [ 4 ] );
			GetUV1 ( Buffer [ 4 ] );
			GetXY2 ( Buffer [ 5 ] );
			GetUV2 ( Buffer [ 6 ] );
			GetXY3 ( Buffer [ 7 ] );
			GetUV3 ( Buffer [ 8 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_TEXTURE
			debug << dec << " x0=" << gx[0] << " y0=" << gy[0] << " x1=" << gx[1] << " y1=" << gy[1] << " x2=" << gx[2] << " y2=" << gy[2] << " x3=" << gx[3] << " y3=" << gy[3];
			debug << "\r\nu0=" << gu[0] << " v0=" << gv[0] << " u1=" << gu[1] << " v1=" << gv[1] << " u2=" << gu[2] << " v2=" << gv[2] << " u3=" << gu[3] << " v3=" << gv[3];
			debug << "\r\ntpage_tx=" << tpage_tx << " tpage_ty=" << tpage_ty << " tpage_abr=" << tpage_abr << " tpage_tp=" << tpage_tp << " clut_x=" << clut_x << " clut_y=" << clut_y << " bgr=" << hex << gbgr[0];
			debug << dec << "\r\nDrawArea: OffsetX=" << DrawArea_OffsetX << " OffsetY=" << DrawArea_OffsetY << " TopLeftX=" << DrawArea_TopLeftX << " TopLeftY=" << DrawArea_TopLeftY << " BottomRightX=" << DrawArea_BottomRightX << " BottomRightY=" << DrawArea_BottomRightY;
#endif

				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				inputdata_ptr2 = & ( inputdata [ ( ( ulInputBuffer_WriteIndex + 1 ) & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				inputdata_ptr [ 4 ] = TextureWindow;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 9 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 11 ] = Buffer [ 3 ].Value;
				inputdata_ptr [ 12 ] = Buffer [ 4 ].Value;
				inputdata_ptr [ 14 ] = Buffer [ 5 ].Value;
				inputdata_ptr [ 15 ] = Buffer [ 6 ].Value;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;
				
				ulInputBuffer_WriteIndex++;
			
			

				inputdata_ptr2 [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr2 [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr2 [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr2 [ 3 ] = DrawArea_Offset;
				inputdata_ptr2 [ 4 ] = TextureWindow;
				
				inputdata_ptr2 [ 8 ] = Buffer [ 7 ].Value;
				inputdata_ptr2 [ 9 ] = ( Buffer [ 2 ].Value & ~0xffff ) | ( Buffer [ 8 ].Value & 0xffff );
				inputdata_ptr2 [ 11 ] = Buffer [ 3 ].Value;
				inputdata_ptr2 [ 12 ] = Buffer [ 4 ].Value;
				inputdata_ptr2 [ 14 ] = Buffer [ 5 ].Value;
				inputdata_ptr2 [ 15 ] = Buffer [ 6 ].Value;
				inputdata_ptr2 [ 7 ] = Buffer [ 0 ].Value;
				
				ulInputBuffer_WriteIndex++;
			
#ifdef USE_TEMPLATES_RECTANGLE_TEXTURE
			Draw_TextureRectangle_2c_t <1,1> ();
#else
			//Draw_TextureRectangle_2c ();
			//Draw_TextureRectangle_2c ( inputdata_ptr, 0 );
			Draw_TextureTriangle_24 ( (DATA_Write_Format*) inputdata_ptr, 0 );
			Draw_TextureTriangle_24 ( (DATA_Write_Format*) inputdata_ptr2, 0 );
#endif
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_TEXTURE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;

		
		case 0x30:
		case 0x31:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_SHADED
			debug << "\r\nShadedTriangle Abe=0";
			debug << hex << *_DebugPC << " " << dec << *_DebugCycleCount;
#endif

			///////////////////////////////////////
			// gradated 3-point polygon
			//GetBGR0_24 ( Buffer [ 0 ] );
			GetBGR0_8 ( Buffer [ 0 ] );
			GetXY0 ( Buffer [ 1 ] );
			//GetBGR1_24 ( Buffer [ 2 ] );
			GetBGR1_8 ( Buffer [ 2 ] );
			GetXY1 ( Buffer [ 3 ] );
			//GetBGR2_24 ( Buffer [ 4 ] );
			GetBGR2_8 ( Buffer [ 4 ] );
			GetXY2 ( Buffer [ 5 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SHADED
			debug << dec << " x0=" << gx[0] << " y0=" << gy[0] << " bgr0=" << gbgr[0] << " x1=" << gx[1] << " y1=" << gy[1] << " bgr1=" << gbgr[1] << " x2=" << gx[2] << " y2=" << gy[2] << " bgr2=" << gbgr[2];
			debug << dec << "\r\nDrawArea: OffsetX=" << DrawArea_OffsetX << " OffsetY=" << DrawArea_OffsetY << " TopLeftX=" << DrawArea_TopLeftX << " TopLeftY=" << DrawArea_TopLeftY << " BottomRightX=" << DrawArea_BottomRightX << " BottomRightY=" << DrawArea_BottomRightY;
#endif

				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;

				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 10 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 11 ] = Buffer [ 3 ].Value;
				inputdata_ptr [ 13 ] = Buffer [ 4 ].Value;
				inputdata_ptr [ 14 ] = Buffer [ 5 ].Value;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;
				
				ulInputBuffer_WriteIndex++;



#ifdef USE_TEMPLATES_TRIANGLE_GRADIENT
			Draw_GradientTriangle_30_t <0> ( 0, 1, 2 );
#else
			//Draw_GradientTriangle_30 ( 0, 1, 2 );
			Draw_GradientTriangle_30 ( (DATA_Write_Format*) inputdata_ptr, 0 );
#endif
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SHADED
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;
			
		
		
		case 0x32:
		case 0x33:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_SHADED
			debug << "\r\nShadedTriangle Abe=1";
			debug << hex << *_DebugPC << " " << dec << *_DebugCycleCount;
#endif

			///////////////////////////////////////
			// gradated 3-point polygon
			//GetBGR0_24 ( Buffer [ 0 ] );
			GetBGR0_8 ( Buffer [ 0 ] );
			GetXY0 ( Buffer [ 1 ] );
			//GetBGR1_24 ( Buffer [ 2 ] );
			GetBGR1_8 ( Buffer [ 2 ] );
			GetXY1 ( Buffer [ 3 ] );
			//GetBGR2_24 ( Buffer [ 4 ] );
			GetBGR2_8 ( Buffer [ 4 ] );
			GetXY2 ( Buffer [ 5 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SHADED
			debug << dec << " x0=" << gx[0] << " y0=" << gy[0] << " bgr0=" << gbgr[0] << " x1=" << gx[1] << " y1=" << gy[1] << " bgr1=" << gbgr[1] << " x2=" << gx[2] << " y2=" << gy[2] << " bgr2=" << gbgr[2];
			debug << dec << "\r\nDrawArea: OffsetX=" << DrawArea_OffsetX << " OffsetY=" << DrawArea_OffsetY << " TopLeftX=" << DrawArea_TopLeftX << " TopLeftY=" << DrawArea_TopLeftY << " BottomRightX=" << DrawArea_BottomRightX << " BottomRightY=" << DrawArea_BottomRightY;
#endif


				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 10 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 11 ] = Buffer [ 3 ].Value;
				inputdata_ptr [ 13 ] = Buffer [ 4 ].Value;
				inputdata_ptr [ 14 ] = Buffer [ 5 ].Value;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;
				
				ulInputBuffer_WriteIndex++;


#ifdef USE_TEMPLATES_TRIANGLE_GRADIENT
			Draw_GradientTriangle_30_t <1> ( 0, 1, 2 );
#else
			//Draw_GradientTriangle_30 ( 0, 1, 2 );
			Draw_GradientTriangle_30 ( (DATA_Write_Format*) inputdata_ptr, 0 );
#endif

#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SHADED
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;
		
		
		
			
		case 0x34:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_TEXTURE
			debug << "\r\nShadedTexturedTriangle";
#endif
			////////////////////////////////////////
			// gradated textured 3-point polygon
			//GetBGR0_24 ( Buffer [ 0 ] );
			GetBGR0_8 ( Buffer [ 0 ] );
			GetXY0 ( Buffer [ 1 ] );
			GetCLUT ( Buffer [ 2 ] );
			GetUV0 ( Buffer [ 2 ] );
			//GetBGR1_24 ( Buffer [ 3 ] );
			GetBGR1_8 ( Buffer [ 3 ] );
			GetXY1 ( Buffer [ 4 ] );
			GetTPAGE ( Buffer [ 5 ] );
			GetUV1 ( Buffer [ 5 ] );
			//GetBGR2_24 ( Buffer [ 6 ] );
			GetBGR2_8 ( Buffer [ 6 ] );
			GetXY2 ( Buffer [ 7 ] );
			GetUV2 ( Buffer [ 8 ] );
			
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_TEXTURE
			debug << dec << " x0=" << gx[0] << " y0=" << gy[0] << " bgr0=" << gbgr[0] << " x1=" << gx[1] << " y1=" << gy[1] << " bgr1=" << gbgr[1] << " x2=" << gx[2] << " y2=" << gy[2] << " bgr2=" << gbgr[2];
			debug << "\r\nu0=" << gu[0] << " v0=" << gv[0] << " u1=" << gu[1] << " v1=" << gv[1] << " u2=" << gu[2] << " v2=" << gv[2];
			debug << "\r\ntpage_tx=" << tpage_tx << " tpage_ty=" << tpage_ty << " tpage_abr=" << tpage_abr << " tpage_tp=" << tpage_tp << " clut_x=" << clut_x << " clut_y=" << clut_y;
			debug << dec << "\r\nDrawArea: OffsetX=" << DrawArea_OffsetX << " OffsetY=" << DrawArea_OffsetY << " TopLeftX=" << DrawArea_TopLeftX << " TopLeftY=" << DrawArea_TopLeftY << " BottomRightX=" << DrawArea_BottomRightX << " BottomRightY=" << DrawArea_BottomRightY;
			debug << dec << " X=" << TextureWindow_X << " Y=" << TextureWindow_Y << " Height=" << TextureWindow_Height << " Width=" << TextureWindow_Width;
#endif

				
				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				inputdata_ptr [ 4 ] = TextureWindow;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 9 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 10 ] = Buffer [ 3 ].Value;
				inputdata_ptr [ 11 ] = Buffer [ 4 ].Value;
				inputdata_ptr [ 12 ] = Buffer [ 5 ].Value;
				inputdata_ptr [ 13 ] = Buffer [ 6 ].Value;
				inputdata_ptr [ 14 ] = Buffer [ 7 ].Value;
				inputdata_ptr [ 15 ] = Buffer [ 8 ].Value;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;
				
				ulInputBuffer_WriteIndex++;

#ifdef USE_TEMPLATES_TRIANGLE_TEXTUREGRADIENT
			Draw_TextureGradientTriangle_34_t <0,0> ( 0, 1, 2 );
#else
			//Draw_TextureGradientTriangle_34 ( 0, 1, 2 );
			Draw_TextureGradientTriangle_34 ( (DATA_Write_Format*) inputdata_ptr, 0 );
#endif
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_TEXTURE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;

			
		case 0x35:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_TEXTURE
			debug << "\r\nShadedTexturedTriangle Abe=0 Tge=1";
#endif
			////////////////////////////////////////
			// gradated textured 3-point polygon
			//GetBGR0_24 ( Buffer [ 0 ] );
			GetBGR0_8 ( Buffer [ 0 ] );
			GetXY0 ( Buffer [ 1 ] );
			GetCLUT ( Buffer [ 2 ] );
			GetUV0 ( Buffer [ 2 ] );
			//GetBGR1_24 ( Buffer [ 3 ] );
			GetBGR1_8 ( Buffer [ 3 ] );
			GetXY1 ( Buffer [ 4 ] );
			GetTPAGE ( Buffer [ 5 ] );
			GetUV1 ( Buffer [ 5 ] );
			//GetBGR2_24 ( Buffer [ 6 ] );
			GetBGR2_8 ( Buffer [ 6 ] );
			GetXY2 ( Buffer [ 7 ] );
			GetUV2 ( Buffer [ 8 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_TEXTURE
			debug << dec << " x0=" << gx[0] << " y0=" << gy[0] << " bgr0=" << gbgr[0] << " x1=" << gx[1] << " y1=" << gy[1] << " bgr1=" << gbgr[1] << " x2=" << gx[2] << " y2=" << gy[2] << " bgr2=" << gbgr[2];
			debug << "\r\nu0=" << gu[0] << " v0=" << gv[0] << " u1=" << gu[1] << " v1=" << gv[1] << " u2=" << gu[2] << " v2=" << gv[2];
			debug << "\r\ntpage_tx=" << tpage_tx << " tpage_ty=" << tpage_ty << " tpage_abr=" << tpage_abr << " tpage_tp=" << tpage_tp << " clut_x=" << clut_x << " clut_y=" << clut_y;
			debug << dec << "\r\nDrawArea: OffsetX=" << DrawArea_OffsetX << " OffsetY=" << DrawArea_OffsetY << " TopLeftX=" << DrawArea_TopLeftX << " TopLeftY=" << DrawArea_TopLeftY << " BottomRightX=" << DrawArea_BottomRightX << " BottomRightY=" << DrawArea_BottomRightY;
			debug << dec << " X=" << TextureWindow_X << " Y=" << TextureWindow_Y << " Height=" << TextureWindow_Height << " Width=" << TextureWindow_Width;
#endif
				
				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				inputdata_ptr [ 4 ] = TextureWindow;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 9 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 10 ] = Buffer [ 3 ].Value;
				inputdata_ptr [ 11 ] = Buffer [ 4 ].Value;
				inputdata_ptr [ 12 ] = Buffer [ 5 ].Value;
				inputdata_ptr [ 13 ] = Buffer [ 6 ].Value;
				inputdata_ptr [ 14 ] = Buffer [ 7 ].Value;
				inputdata_ptr [ 15 ] = Buffer [ 8 ].Value;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;
				
				ulInputBuffer_WriteIndex++;

#ifdef USE_TEMPLATES_TRIANGLE_TEXTUREGRADIENT
			Draw_TextureGradientTriangle_34_t <0,1> ( 0, 1, 2 );
#else
			//Draw_TextureGradientTriangle_34 ( 0, 1, 2 );
			Draw_TextureGradientTriangle_34 ( (DATA_Write_Format*) inputdata_ptr, 0 );
#endif
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_TEXTURE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;

		
		case 0x36:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_TEXTURE
			debug << "\r\nShadedTexturedTriangle Abe=1 Tge=0";
#endif
			////////////////////////////////////////
			// gradated textured 3-point polygon
			//GetBGR0_24 ( Buffer [ 0 ] );
			GetBGR0_8 ( Buffer [ 0 ] );
			GetXY0 ( Buffer [ 1 ] );
			GetCLUT ( Buffer [ 2 ] );
			GetUV0 ( Buffer [ 2 ] );
			//GetBGR1_24 ( Buffer [ 3 ] );
			GetBGR1_8 ( Buffer [ 3 ] );
			GetXY1 ( Buffer [ 4 ] );
			GetTPAGE ( Buffer [ 5 ] );
			GetUV1 ( Buffer [ 5 ] );
			//GetBGR2_24 ( Buffer [ 6 ] );
			GetBGR2_8 ( Buffer [ 6 ] );
			GetXY2 ( Buffer [ 7 ] );
			GetUV2 ( Buffer [ 8 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_TEXTURE
			debug << dec << " x0=" << gx[0] << " y0=" << gy[0] << " bgr0=" << gbgr[0] << " x1=" << gx[1] << " y1=" << gy[1] << " bgr1=" << gbgr[1] << " x2=" << gx[2] << " y2=" << gy[2] << " bgr2=" << gbgr[2];
			debug << "\r\nu0=" << gu[0] << " v0=" << gv[0] << " u1=" << gu[1] << " v1=" << gv[1] << " u2=" << gu[2] << " v2=" << gv[2];
			debug << "\r\ntpage_tx=" << tpage_tx << " tpage_ty=" << tpage_ty << " tpage_abr=" << tpage_abr << " tpage_tp=" << tpage_tp << " clut_x=" << clut_x << " clut_y=" << clut_y;
			debug << dec << "\r\nDrawArea: OffsetX=" << DrawArea_OffsetX << " OffsetY=" << DrawArea_OffsetY << " TopLeftX=" << DrawArea_TopLeftX << " TopLeftY=" << DrawArea_TopLeftY << " BottomRightX=" << DrawArea_BottomRightX << " BottomRightY=" << DrawArea_BottomRightY;
			debug << dec << " X=" << TextureWindow_X << " Y=" << TextureWindow_Y << " Height=" << TextureWindow_Height << " Width=" << TextureWindow_Width;
#endif
				
				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				inputdata_ptr [ 4 ] = TextureWindow;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 9 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 10 ] = Buffer [ 3 ].Value;
				inputdata_ptr [ 11 ] = Buffer [ 4 ].Value;
				inputdata_ptr [ 12 ] = Buffer [ 5 ].Value;
				inputdata_ptr [ 13 ] = Buffer [ 6 ].Value;
				inputdata_ptr [ 14 ] = Buffer [ 7 ].Value;
				inputdata_ptr [ 15 ] = Buffer [ 8 ].Value;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;
				
				ulInputBuffer_WriteIndex++;

#ifdef USE_TEMPLATES_TRIANGLE_TEXTUREGRADIENT
			Draw_TextureGradientTriangle_34_t <1,0> ( 0, 1, 2 );
#else
			//Draw_TextureGradientTriangle_34 ( 0, 1, 2 );
			Draw_TextureGradientTriangle_34 ( (DATA_Write_Format*) inputdata_ptr, 0 );
#endif
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_TEXTURE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;

		
		case 0x37:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_TEXTURE
			debug << "\r\nShadedTexturedTriangle Abe=1 Tge=1";
#endif
			////////////////////////////////////////
			// gradated textured 3-point polygon
			//GetBGR0_24 ( Buffer [ 0 ] );
			GetBGR0_8 ( Buffer [ 0 ] );
			GetXY0 ( Buffer [ 1 ] );
			GetCLUT ( Buffer [ 2 ] );
			GetUV0 ( Buffer [ 2 ] );
			//GetBGR1_24 ( Buffer [ 3 ] );
			GetBGR1_8 ( Buffer [ 3 ] );
			GetXY1 ( Buffer [ 4 ] );
			GetTPAGE ( Buffer [ 5 ] );
			GetUV1 ( Buffer [ 5 ] );
			//GetBGR2_24 ( Buffer [ 6 ] );
			GetBGR2_8 ( Buffer [ 6 ] );
			GetXY2 ( Buffer [ 7 ] );
			GetUV2 ( Buffer [ 8 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_TEXTURE
			debug << dec << " x0=" << gx[0] << " y0=" << gy[0] << " bgr0=" << gbgr[0] << " x1=" << gx[1] << " y1=" << gy[1] << " bgr1=" << gbgr[1] << " x2=" << gx[2] << " y2=" << gy[2] << " bgr2=" << gbgr[2];
			debug << "\r\nu0=" << gu[0] << " v0=" << gv[0] << " u1=" << gu[1] << " v1=" << gv[1] << " u2=" << gu[2] << " v2=" << gv[2];
			debug << "\r\ntpage_tx=" << tpage_tx << " tpage_ty=" << tpage_ty << " tpage_abr=" << tpage_abr << " tpage_tp=" << tpage_tp << " clut_x=" << clut_x << " clut_y=" << clut_y;
			debug << dec << "\r\nDrawArea: OffsetX=" << DrawArea_OffsetX << " OffsetY=" << DrawArea_OffsetY << " TopLeftX=" << DrawArea_TopLeftX << " TopLeftY=" << DrawArea_TopLeftY << " BottomRightX=" << DrawArea_BottomRightX << " BottomRightY=" << DrawArea_BottomRightY;
			debug << dec << " X=" << TextureWindow_X << " Y=" << TextureWindow_Y << " Height=" << TextureWindow_Height << " Width=" << TextureWindow_Width;
#endif
				
				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				inputdata_ptr [ 4 ] = TextureWindow;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 9 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 10 ] = Buffer [ 3 ].Value;
				inputdata_ptr [ 11 ] = Buffer [ 4 ].Value;
				inputdata_ptr [ 12 ] = Buffer [ 5 ].Value;
				inputdata_ptr [ 13 ] = Buffer [ 6 ].Value;
				inputdata_ptr [ 14 ] = Buffer [ 7 ].Value;
				inputdata_ptr [ 15 ] = Buffer [ 8 ].Value;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;
				
				ulInputBuffer_WriteIndex++;

#ifdef USE_TEMPLATES_TRIANGLE_TEXTUREGRADIENT
			Draw_TextureGradientTriangle_34_t <1,1> ( 0, 1, 2 );
#else
			//Draw_TextureGradientTriangle_34 ( 0, 1, 2 );
			Draw_TextureGradientTriangle_34 ( (DATA_Write_Format*) inputdata_ptr, 0 );
#endif
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_TEXTURE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;

		

		
		case 0x38:
		case 0x39:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_SHADED
			debug << "\r\nShadedRectangle Abe=0";
			debug << hex << *_DebugPC << " " << dec << *_DebugCycleCount;
#endif

			//////////////////////////////////////////
			// gradated 4-point polygon
			//GetBGR0_24 ( Buffer [ 0 ] );
			GetBGR0_8 ( Buffer [ 0 ] );
			GetXY0 ( Buffer [ 1 ] );
			//GetBGR1_24 ( Buffer [ 2 ] );
			GetBGR1_8 ( Buffer [ 2 ] );
			GetXY1 ( Buffer [ 3 ] );
			//GetBGR2_24 ( Buffer [ 4 ] );
			GetBGR2_8 ( Buffer [ 4 ] );
			GetXY2 ( Buffer [ 5 ] );
			//GetBGR3_24 ( Buffer [ 6 ] );
			GetBGR3_8 ( Buffer [ 6 ] );
			GetXY3 ( Buffer [ 7 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SHADED
			debug << dec << " x0=" << gx[0] << " y0=" << gy[0] << " bgr0=" << gbgr[0] << " x1=" << gx[1] << " y1=" << gy[1] << " bgr1=" << gbgr[1] << " x2=" << gx[2] << " y2=" << gy[2] << " bgr2=" << gbgr[2] << " x3=" << gx[3] << " y3=" << gy[3] << " bgr3=" << gbgr[3];
			debug << dec << "\r\nDrawArea: OffsetX=" << DrawArea_OffsetX << " OffsetY=" << DrawArea_OffsetY << " TopLeftX=" << DrawArea_TopLeftX << " TopLeftY=" << DrawArea_TopLeftY << " BottomRightX=" << DrawArea_BottomRightX << " BottomRightY=" << DrawArea_BottomRightY;
#endif

				
				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				inputdata_ptr2 = & ( inputdata [ ( ( ulInputBuffer_WriteIndex + 1 ) & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 10 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 11 ] = Buffer [ 3 ].Value;
				inputdata_ptr [ 13 ] = Buffer [ 4 ].Value;
				inputdata_ptr [ 14 ] = Buffer [ 5 ].Value;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;

				ulInputBuffer_WriteIndex++;
			

				inputdata_ptr2 [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr2 [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr2 [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr2 [ 3 ] = DrawArea_Offset;

				inputdata_ptr2 [ 8 ] = Buffer [ 7 ].Value;
				inputdata_ptr2 [ 10 ] = Buffer [ 2 ].Value;
				inputdata_ptr2 [ 11 ] = Buffer [ 3 ].Value;
				inputdata_ptr2 [ 13 ] = Buffer [ 4 ].Value;
				inputdata_ptr2 [ 14 ] = Buffer [ 5 ].Value;
				inputdata_ptr2 [ 7 ] = ( Buffer [ 0 ].Value & ~0xffffff ) | ( Buffer [ 6 ].Value & 0xffffff );

				ulInputBuffer_WriteIndex++;



#ifdef USE_TEMPLATES_RECTANGLE_GRADIENT
			Draw_GradientRectangle_38_t <0> ();
#else
			//Draw_GradientRectangle_38 ();
			//Draw_GradientRectangle_38 ( inputdata_ptr, 0 );
			Draw_GradientTriangle_30 ( (DATA_Write_Format*) inputdata_ptr, 0 );
			Draw_GradientTriangle_30 ( (DATA_Write_Format*) inputdata_ptr2, 0 );
#endif

			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SHADED
			debug << ";BusyCycles=" << BusyCycles;
#endif

			break;
			
		
		
		
		case 0x3a:
		case 0x3b:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_SHADED
			debug << "\r\nShadedRectangle Abe=1";
			debug << hex << *_DebugPC << " " << dec << *_DebugCycleCount;
#endif

			//////////////////////////////////////////
			// gradated 4-point polygon
			//GetBGR0_24 ( Buffer [ 0 ] );
			GetBGR0_8 ( Buffer [ 0 ] );
			GetXY0 ( Buffer [ 1 ] );
			//GetBGR1_24 ( Buffer [ 2 ] );
			GetBGR1_8 ( Buffer [ 2 ] );
			GetXY1 ( Buffer [ 3 ] );
			//GetBGR2_24 ( Buffer [ 4 ] );
			GetBGR2_8 ( Buffer [ 4 ] );
			GetXY2 ( Buffer [ 5 ] );
			//GetBGR3_24 ( Buffer [ 6 ] );
			GetBGR3_8 ( Buffer [ 6 ] );
			GetXY3 ( Buffer [ 7 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SHADED
			debug << dec << " x0=" << gx[0] << " y0=" << gy[0] << " bgr0=" << gbgr[0] << " x1=" << gx[1] << " y1=" << gy[1] << " bgr1=" << gbgr[1] << " x2=" << gx[2] << " y2=" << gy[2] << " bgr2=" << gbgr[2] << " x3=" << gx[3] << " y3=" << gy[3] << " bgr3=" << gbgr[3];
			debug << dec << "\r\nDrawArea: OffsetX=" << DrawArea_OffsetX << " OffsetY=" << DrawArea_OffsetY << " TopLeftX=" << DrawArea_TopLeftX << " TopLeftY=" << DrawArea_TopLeftY << " BottomRightX=" << DrawArea_BottomRightX << " BottomRightY=" << DrawArea_BottomRightY;
#endif

				
				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				inputdata_ptr2 = & ( inputdata [ ( ( ulInputBuffer_WriteIndex + 1 ) & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 10 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 11 ] = Buffer [ 3 ].Value;
				inputdata_ptr [ 13 ] = Buffer [ 4 ].Value;
				inputdata_ptr [ 14 ] = Buffer [ 5 ].Value;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;

				ulInputBuffer_WriteIndex++;
			

				inputdata_ptr2 [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr2 [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr2 [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr2 [ 3 ] = DrawArea_Offset;

				inputdata_ptr2 [ 8 ] = Buffer [ 7 ].Value;
				inputdata_ptr2 [ 10 ] = Buffer [ 2 ].Value;
				inputdata_ptr2 [ 11 ] = Buffer [ 3 ].Value;
				inputdata_ptr2 [ 13 ] = Buffer [ 4 ].Value;
				inputdata_ptr2 [ 14 ] = Buffer [ 5 ].Value;
				inputdata_ptr2 [ 7 ] = ( Buffer [ 0 ].Value & ~0xffffff ) | ( Buffer [ 6 ].Value & 0xffffff );

				ulInputBuffer_WriteIndex++;



#ifdef USE_TEMPLATES_RECTANGLE_GRADIENT
			Draw_GradientRectangle_38_t <0> ();
#else
			//Draw_GradientRectangle_38 ();
			//Draw_GradientRectangle_38 ( inputdata_ptr, 0 );
			Draw_GradientTriangle_30 ( (DATA_Write_Format*) inputdata_ptr, 0 );
			Draw_GradientTriangle_30 ( (DATA_Write_Format*) inputdata_ptr2, 0 );
#endif

#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SHADED
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;
		
		

		
		case 0x3c:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_TEXTURE
			debug << "\r\nShadedTexturedRectangle";
#endif
			/////////////////////////////////////////
			// gradated textured 4-point polygon
			//GetBGR0_24 ( Buffer [ 0 ] );
			GetBGR0_8 ( Buffer [ 0 ] );
			GetXY0 ( Buffer [ 1 ] );
			GetCLUT ( Buffer [ 2 ] );
			GetUV0 ( Buffer [ 2 ] );
			//GetBGR1_24 ( Buffer [ 3 ] );
			GetBGR1_8 ( Buffer [ 3 ] );
			GetXY1 ( Buffer [ 4 ] );
			GetTPAGE ( Buffer [ 5 ] );
			GetUV1 ( Buffer [ 5 ] );
			//GetBGR2_24 ( Buffer [ 6 ] );
			GetBGR2_8 ( Buffer [ 6 ] );
			GetXY2 ( Buffer [ 7 ] );
			GetUV2 ( Buffer [ 8 ] );
			//GetBGR3_24 ( Buffer [ 9 ] );
			GetBGR3_8 ( Buffer [ 9 ] );
			GetXY3 ( Buffer [ 10 ] );
			GetUV3 ( Buffer [ 11 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_TEXTURE
			debug << dec << " x0=" << gx[0] << " y0=" << gy[0] << " bgr0=" << gbgr[0] << " x1=" << gx[1] << " y1=" << gy[1] << " bgr1=" << gbgr[1] << " x2=" << gx[2] << " y2=" << gy[2] << " bgr2=" << gbgr[2] << " x3=" << gx[3] << " y3=" << gy[3] << " bgr3=" << gbgr[3];
			debug << "\r\nu0=" << gu[0] << " v0=" << gv[0] << " u1=" << gu[1] << " v1=" << gv[1] << " u2=" << gu[2] << " v2=" << gv[2] << " u3=" << gu[3] << " v3=" << gv[3];
			debug << "\r\ntpage_tx=" << tpage_tx << " tpage_ty=" << tpage_ty << " tpage_abr=" << tpage_abr << " tpage_tp=" << tpage_tp << " clut_x=" << clut_x << " clut_y=" << clut_y;
			debug << dec << "\r\nDrawArea: OffsetX=" << DrawArea_OffsetX << " OffsetY=" << DrawArea_OffsetY << " TopLeftX=" << DrawArea_TopLeftX << " TopLeftY=" << DrawArea_TopLeftY << " BottomRightX=" << DrawArea_BottomRightX << " BottomRightY=" << DrawArea_BottomRightY;
			debug << dec << " X=" << TextureWindow_X << " Y=" << TextureWindow_Y << " Height=" << TextureWindow_Height << " Width=" << TextureWindow_Width;
#endif

				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				inputdata_ptr2 = & ( inputdata [ ( ( ulInputBuffer_WriteIndex + 1 ) & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				inputdata_ptr [ 4 ] = TextureWindow;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 9 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 10 ] = Buffer [ 3 ].Value;
				inputdata_ptr [ 11 ] = Buffer [ 4 ].Value;
				inputdata_ptr [ 12 ] = Buffer [ 5 ].Value;
				inputdata_ptr [ 13 ] = Buffer [ 6 ].Value;
				inputdata_ptr [ 14 ] = Buffer [ 7 ].Value;
				inputdata_ptr [ 15 ] = Buffer [ 8 ].Value;	// & 0xffff ) | ( Buffer [ 11 ].Value << 16 );
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;
				
				ulInputBuffer_WriteIndex++;
			

				inputdata_ptr2 [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr2 [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr2 [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr2 [ 3 ] = DrawArea_Offset;
				inputdata_ptr2 [ 4 ] = TextureWindow;

				inputdata_ptr2 [ 8 ] = Buffer [ 10 ].Value;
				inputdata_ptr2 [ 9 ] = ( Buffer [ 2 ].Value & ~0xffff ) | ( Buffer [ 11 ].Value & 0xffff );
				inputdata_ptr2 [ 10 ] = Buffer [ 3 ].Value;
				inputdata_ptr2 [ 11 ] = Buffer [ 4 ].Value;
				inputdata_ptr2 [ 12 ] = Buffer [ 5 ].Value;
				inputdata_ptr2 [ 13 ] = Buffer [ 6 ].Value;
				inputdata_ptr2 [ 14 ] = Buffer [ 7 ].Value;
				inputdata_ptr2 [ 15 ] = Buffer [ 8 ].Value;	// & 0xffff ) | ( Buffer [ 11 ].Value << 16 );
				inputdata_ptr2 [ 7 ] = ( Buffer [ 0 ].Value & ~0xffffff ) | ( Buffer [ 9 ].Value & 0xffffff );

				ulInputBuffer_WriteIndex++;
			
#ifdef USE_TEMPLATES_RECTANGLE_TEXTUREGRADIENT
			Draw_TextureGradientRectangle_3c_t <0,0> ();
#else
			//Draw_TextureGradientRectangle_3c ();
			//Draw_TextureGradientRectangle_3c ( inputdata_ptr, 0 );
			Draw_TextureGradientTriangle_34 ( (DATA_Write_Format*) inputdata_ptr, 0 );
			Draw_TextureGradientTriangle_34 ( (DATA_Write_Format*) inputdata_ptr2, 0 );
#endif
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_TEXTURE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;
			
			
		case 0x3d:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_TEXTURE
			debug << "\r\nShadedTexturedRectangle Abe=0 Tge=1";
#endif
			/////////////////////////////////////////
			// gradated textured 4-point polygon
			//GetBGR0_24 ( Buffer [ 0 ] );
			GetBGR0_8 ( Buffer [ 0 ] );
			GetXY0 ( Buffer [ 1 ] );
			GetCLUT ( Buffer [ 2 ] );
			GetUV0 ( Buffer [ 2 ] );
			//GetBGR1_24 ( Buffer [ 3 ] );
			GetBGR1_8 ( Buffer [ 3 ] );
			GetXY1 ( Buffer [ 4 ] );
			GetTPAGE ( Buffer [ 5 ] );
			GetUV1 ( Buffer [ 5 ] );
			//GetBGR2_24 ( Buffer [ 6 ] );
			GetBGR2_8 ( Buffer [ 6 ] );
			GetXY2 ( Buffer [ 7 ] );
			GetUV2 ( Buffer [ 8 ] );
			//GetBGR3_24 ( Buffer [ 9 ] );
			GetBGR3_8 ( Buffer [ 9 ] );
			GetXY3 ( Buffer [ 10 ] );
			GetUV3 ( Buffer [ 11 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_TEXTURE
			debug << dec << " x0=" << gx[0] << " y0=" << gy[0] << " bgr0=" << gbgr[0] << " x1=" << gx[1] << " y1=" << gy[1] << " bgr1=" << gbgr[1] << " x2=" << gx[2] << " y2=" << gy[2] << " bgr2=" << gbgr[2] << " x3=" << gx[3] << " y3=" << gy[3] << " bgr3=" << gbgr[3];
			debug << "\r\nu0=" << gu[0] << " v0=" << gv[0] << " u1=" << gu[1] << " v1=" << gv[1] << " u2=" << gu[2] << " v2=" << gv[2] << " u3=" << gu[3] << " v3=" << gv[3];
			debug << "\r\ntpage_tx=" << tpage_tx << " tpage_ty=" << tpage_ty << " tpage_abr=" << tpage_abr << " tpage_tp=" << tpage_tp << " clut_x=" << clut_x << " clut_y=" << clut_y;
			debug << dec << "\r\nDrawArea: OffsetX=" << DrawArea_OffsetX << " OffsetY=" << DrawArea_OffsetY << " TopLeftX=" << DrawArea_TopLeftX << " TopLeftY=" << DrawArea_TopLeftY << " BottomRightX=" << DrawArea_BottomRightX << " BottomRightY=" << DrawArea_BottomRightY;
			debug << dec << " X=" << TextureWindow_X << " Y=" << TextureWindow_Y << " Height=" << TextureWindow_Height << " Width=" << TextureWindow_Width;
#endif
				
				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				inputdata_ptr2 = & ( inputdata [ ( ( ulInputBuffer_WriteIndex + 1 ) & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				inputdata_ptr [ 4 ] = TextureWindow;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 9 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 10 ] = Buffer [ 3 ].Value;
				inputdata_ptr [ 11 ] = Buffer [ 4 ].Value;
				inputdata_ptr [ 12 ] = Buffer [ 5 ].Value;
				inputdata_ptr [ 13 ] = Buffer [ 6 ].Value;
				inputdata_ptr [ 14 ] = Buffer [ 7 ].Value;
				inputdata_ptr [ 15 ] = Buffer [ 8 ].Value;	// & 0xffff ) | ( Buffer [ 11 ].Value << 16 );
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;
				
				
				ulInputBuffer_WriteIndex++;
			

				inputdata_ptr2 [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr2 [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr2 [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr2 [ 3 ] = DrawArea_Offset;
				inputdata_ptr2 [ 4 ] = TextureWindow;

				inputdata_ptr2 [ 8 ] = Buffer [ 10 ].Value;
				inputdata_ptr2 [ 9 ] = ( Buffer [ 2 ].Value & ~0xffff ) | ( Buffer [ 11 ].Value & 0xffff );
				inputdata_ptr2 [ 10 ] = Buffer [ 3 ].Value;
				inputdata_ptr2 [ 11 ] = Buffer [ 4 ].Value;
				inputdata_ptr2 [ 12 ] = Buffer [ 5 ].Value;
				inputdata_ptr2 [ 13 ] = Buffer [ 6 ].Value;
				inputdata_ptr2 [ 14 ] = Buffer [ 7 ].Value;
				inputdata_ptr2 [ 15 ] = Buffer [ 8 ].Value;	// & 0xffff ) | ( Buffer [ 11 ].Value << 16 );
				inputdata_ptr2 [ 7 ] = ( Buffer [ 0 ].Value & ~0xffffff ) | ( Buffer [ 9 ].Value & 0xffffff );
				
				ulInputBuffer_WriteIndex++;
			
#ifdef USE_TEMPLATES_RECTANGLE_TEXTUREGRADIENT
			Draw_TextureGradientRectangle_3c_t <0,1> ();
#else
			//Draw_TextureGradientRectangle_3c ();
			//Draw_TextureGradientRectangle_3c ( inputdata_ptr, 0 );
			Draw_TextureGradientTriangle_34 ( (DATA_Write_Format*) inputdata_ptr, 0 );
			Draw_TextureGradientTriangle_34 ( (DATA_Write_Format*) inputdata_ptr2, 0 );
#endif
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_TEXTURE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;
		
		
		case 0x3e:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_TEXTURE
			debug << "\r\nShadedTexturedRectangle Abe=1 Tge=0";
#endif
			/////////////////////////////////////////
			// gradated textured 4-point polygon
			//GetBGR0_24 ( Buffer [ 0 ] );
			GetBGR0_8 ( Buffer [ 0 ] );
			GetXY0 ( Buffer [ 1 ] );
			GetCLUT ( Buffer [ 2 ] );
			GetUV0 ( Buffer [ 2 ] );
			//GetBGR1_24 ( Buffer [ 3 ] );
			GetBGR1_8 ( Buffer [ 3 ] );
			GetXY1 ( Buffer [ 4 ] );
			GetTPAGE ( Buffer [ 5 ] );
			GetUV1 ( Buffer [ 5 ] );
			//GetBGR2_24 ( Buffer [ 6 ] );
			GetBGR2_8 ( Buffer [ 6 ] );
			GetXY2 ( Buffer [ 7 ] );
			GetUV2 ( Buffer [ 8 ] );
			//GetBGR3_24 ( Buffer [ 9 ] );
			GetBGR3_8 ( Buffer [ 9 ] );
			GetXY3 ( Buffer [ 10 ] );
			GetUV3 ( Buffer [ 11 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_TEXTURE
			debug << dec << " x0=" << gx[0] << " y0=" << gy[0] << " bgr0=" << gbgr[0] << " x1=" << gx[1] << " y1=" << gy[1] << " bgr1=" << gbgr[1] << " x2=" << gx[2] << " y2=" << gy[2] << " bgr2=" << gbgr[2] << " x3=" << gx[3] << " y3=" << gy[3] << " bgr3=" << gbgr[3];
			debug << "\r\nu0=" << gu[0] << " v0=" << gv[0] << " u1=" << gu[1] << " v1=" << gv[1] << " u2=" << gu[2] << " v2=" << gv[2] << " u3=" << gu[3] << " v3=" << gv[3];
			debug << "\r\ntpage_tx=" << tpage_tx << " tpage_ty=" << tpage_ty << " tpage_abr=" << tpage_abr << " tpage_tp=" << tpage_tp << " clut_x=" << clut_x << " clut_y=" << clut_y;
			debug << dec << "\r\nDrawArea: OffsetX=" << DrawArea_OffsetX << " OffsetY=" << DrawArea_OffsetY << " TopLeftX=" << DrawArea_TopLeftX << " TopLeftY=" << DrawArea_TopLeftY << " BottomRightX=" << DrawArea_BottomRightX << " BottomRightY=" << DrawArea_BottomRightY;
			debug << dec << " X=" << TextureWindow_X << " Y=" << TextureWindow_Y << " Height=" << TextureWindow_Height << " Width=" << TextureWindow_Width;
#endif

				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				inputdata_ptr2 = & ( inputdata [ ( ( ulInputBuffer_WriteIndex + 1 ) & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				inputdata_ptr [ 4 ] = TextureWindow;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 9 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 10 ] = Buffer [ 3 ].Value;
				inputdata_ptr [ 11 ] = Buffer [ 4 ].Value;
				inputdata_ptr [ 12 ] = Buffer [ 5 ].Value;
				inputdata_ptr [ 13 ] = Buffer [ 6 ].Value;
				inputdata_ptr [ 14 ] = Buffer [ 7 ].Value;
				inputdata_ptr [ 15 ] = Buffer [ 8 ].Value;	// & 0xffff ) | ( Buffer [ 11 ].Value << 16 );
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;
				
				ulInputBuffer_WriteIndex++;
			

				inputdata_ptr2 [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr2 [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr2 [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr2 [ 3 ] = DrawArea_Offset;
				inputdata_ptr2 [ 4 ] = TextureWindow;

				inputdata_ptr2 [ 8 ] = Buffer [ 10 ].Value;
				inputdata_ptr2 [ 9 ] = ( Buffer [ 2 ].Value & ~0xffff ) | ( Buffer [ 11 ].Value & 0xffff );
				inputdata_ptr2 [ 10 ] = Buffer [ 3 ].Value;
				inputdata_ptr2 [ 11 ] = Buffer [ 4 ].Value;
				inputdata_ptr2 [ 12 ] = Buffer [ 5 ].Value;
				inputdata_ptr2 [ 13 ] = Buffer [ 6 ].Value;
				inputdata_ptr2 [ 14 ] = Buffer [ 7 ].Value;
				inputdata_ptr2 [ 15 ] = Buffer [ 8 ].Value;	// & 0xffff ) | ( Buffer [ 11 ].Value << 16 );
				inputdata_ptr2 [ 7 ] = ( Buffer [ 0 ].Value & ~0xffffff ) | ( Buffer [ 9 ].Value & 0xffffff );
				
				ulInputBuffer_WriteIndex++;

#ifdef USE_TEMPLATES_RECTANGLE_TEXTUREGRADIENT
			Draw_TextureGradientRectangle_3c_t <1,0> ();
#else
			//Draw_TextureGradientRectangle_3c ();
			//Draw_TextureGradientRectangle_3c ( inputdata_ptr, 0 );
			Draw_TextureGradientTriangle_34 ( (DATA_Write_Format*) inputdata_ptr, 0 );
			Draw_TextureGradientTriangle_34 ( (DATA_Write_Format*) inputdata_ptr2, 0 );
#endif
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_TEXTURE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;
		
		
		case 0x3f:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_TEXTURE
			debug << "\r\nShadedTexturedRectangle Abe=1 Tge=1";
#endif
			/////////////////////////////////////////
			// gradated textured 4-point polygon
			//GetBGR0_24 ( Buffer [ 0 ] );
			GetBGR0_8 ( Buffer [ 0 ] );
			GetXY0 ( Buffer [ 1 ] );
			GetCLUT ( Buffer [ 2 ] );
			GetUV0 ( Buffer [ 2 ] );
			//GetBGR1_24 ( Buffer [ 3 ] );
			GetBGR1_8 ( Buffer [ 3 ] );
			GetXY1 ( Buffer [ 4 ] );
			GetTPAGE ( Buffer [ 5 ] );
			GetUV1 ( Buffer [ 5 ] );
			//GetBGR2_24 ( Buffer [ 6 ] );
			GetBGR2_8 ( Buffer [ 6 ] );
			GetXY2 ( Buffer [ 7 ] );
			GetUV2 ( Buffer [ 8 ] );
			//GetBGR3_24 ( Buffer [ 9 ] );
			GetBGR3_8 ( Buffer [ 9 ] );
			GetXY3 ( Buffer [ 10 ] );
			GetUV3 ( Buffer [ 11 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_TEXTURE
			debug << dec << " x0=" << gx[0] << " y0=" << gy[0] << " bgr0=" << gbgr[0] << " x1=" << gx[1] << " y1=" << gy[1] << " bgr1=" << gbgr[1] << " x2=" << gx[2] << " y2=" << gy[2] << " bgr2=" << gbgr[2] << " x3=" << gx[3] << " y3=" << gy[3] << " bgr3=" << gbgr[3];
			debug << "\r\nu0=" << gu[0] << " v0=" << gv[0] << " u1=" << gu[1] << " v1=" << gv[1] << " u2=" << gu[2] << " v2=" << gv[2] << " u3=" << gu[3] << " v3=" << gv[3];
			debug << "\r\ntpage_tx=" << tpage_tx << " tpage_ty=" << tpage_ty << " tpage_abr=" << tpage_abr << " tpage_tp=" << tpage_tp << " clut_x=" << clut_x << " clut_y=" << clut_y;
			debug << dec << "\r\nDrawArea: OffsetX=" << DrawArea_OffsetX << " OffsetY=" << DrawArea_OffsetY << " TopLeftX=" << DrawArea_TopLeftX << " TopLeftY=" << DrawArea_TopLeftY << " BottomRightX=" << DrawArea_BottomRightX << " BottomRightY=" << DrawArea_BottomRightY;
			debug << dec << " X=" << TextureWindow_X << " Y=" << TextureWindow_Y << " Height=" << TextureWindow_Height << " Width=" << TextureWindow_Width;
#endif
				
				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				inputdata_ptr2 = & ( inputdata [ ( ( ulInputBuffer_WriteIndex + 1 ) & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				inputdata_ptr [ 4 ] = TextureWindow;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 9 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 10 ] = Buffer [ 3 ].Value;
				inputdata_ptr [ 11 ] = Buffer [ 4 ].Value;
				inputdata_ptr [ 12 ] = Buffer [ 5 ].Value;
				inputdata_ptr [ 13 ] = Buffer [ 6 ].Value;
				inputdata_ptr [ 14 ] = Buffer [ 7 ].Value;
				inputdata_ptr [ 15 ] = Buffer [ 8 ].Value;	// & 0xffff ) | ( Buffer [ 11 ].Value << 16 );
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;
				
				
				ulInputBuffer_WriteIndex++;
			

				inputdata_ptr2 [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr2 [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr2 [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr2 [ 3 ] = DrawArea_Offset;
				inputdata_ptr2 [ 4 ] = TextureWindow;

				inputdata_ptr2 [ 8 ] = Buffer [ 10 ].Value;
				inputdata_ptr2 [ 9 ] = ( Buffer [ 2 ].Value & ~0xffff ) | ( Buffer [ 11 ].Value & 0xffff );
				inputdata_ptr2 [ 10 ] = Buffer [ 3 ].Value;
				inputdata_ptr2 [ 11 ] = Buffer [ 4 ].Value;
				inputdata_ptr2 [ 12 ] = Buffer [ 5 ].Value;
				inputdata_ptr2 [ 13 ] = Buffer [ 6 ].Value;
				inputdata_ptr2 [ 14 ] = Buffer [ 7 ].Value;
				inputdata_ptr2 [ 15 ] = Buffer [ 8 ].Value;	// & 0xffff ) | ( Buffer [ 11 ].Value << 16 );
				inputdata_ptr2 [ 7 ] = ( Buffer [ 0 ].Value & ~0xffffff ) | ( Buffer [ 9 ].Value & 0xffffff );
				
				ulInputBuffer_WriteIndex++;

#ifdef USE_TEMPLATES_RECTANGLE_TEXTUREGRADIENT
			Draw_TextureGradientRectangle_3c_t <1,1> ();
#else
			//Draw_TextureGradientRectangle_3c ();
			//Draw_TextureGradientRectangle_3c ( inputdata_ptr, 0 );
			Draw_TextureGradientTriangle_34 ( (DATA_Write_Format*) inputdata_ptr, 0 );
			Draw_TextureGradientTriangle_34 ( (DATA_Write_Format*) inputdata_ptr2, 0 );
#endif

#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_TEXTURE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;
		
		
		

		////////////////////////////////////////
		// Line commands
		
		case 0x40:
		case 0x41:
		case 0x44:
		case 0x45:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_LINE
			debug << "\r\nMonochromeLine";
#endif
			//////////////////////////////////////////
			// monochrome line
			GetBGR24 ( Buffer [ 0 ] );
			GetXY0 ( Buffer [ 1 ] );
			GetXY1 ( Buffer [ 2 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_LINE
			debug << dec << " x0=" << gx[0] << " y0=" << gy[0] << " x1=" << gx[1] << " y1=" << gy[1] << " bgr=" << hex << gbgr[0];
			debug << dec << "\r\nDrawArea: OffsetX=" << DrawArea_OffsetX << " OffsetY=" << DrawArea_OffsetY << " TopLeftX=" << DrawArea_TopLeftX << " TopLeftY=" << DrawArea_TopLeftY << " BottomRightX=" << DrawArea_BottomRightX << " BottomRightY=" << DrawArea_BottomRightY;
#endif

				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 10 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;
				
				ulInputBuffer_WriteIndex++;
			
#ifdef USE_TEMPLATES_LINE_MONO
			Select_MonoLine_t <0> ();
#else
	
#ifdef USE_TEMPLATES_PS1_LINE
			NumPixels = Select_Line_Renderer_t( (DATA_Write_Format*) inputdata_ptr, 0 );
#else
			//Draw_MonoLine_40 ();
			NumPixels = DrawLine_Mono_th( (DATA_Write_Format*) inputdata_ptr, 0 );
#endif

			BusyCycles += NumPixels;
#endif
			
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_LINE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;
			
		case 0x42:
		case 0x43:
		case 0x46:
		case 0x47:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_LINE
			debug << "\r\nMonochromeLine Abe=1";
#endif
			//////////////////////////////////////////
			// monochrome line
			GetBGR24 ( Buffer [ 0 ] );
			GetXY0 ( Buffer [ 1 ] );
			GetXY1 ( Buffer [ 2 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_LINE
			debug << dec << " x0=" << gx[0] << " y0=" << gy[0] << " x1=" << gx[1] << " y1=" << gy[1] << " bgr=" << hex << gbgr[0];
			debug << dec << "\r\nDrawArea: OffsetX=" << DrawArea_OffsetX << " OffsetY=" << DrawArea_OffsetY << " TopLeftX=" << DrawArea_TopLeftX << " TopLeftY=" << DrawArea_TopLeftY << " BottomRightX=" << DrawArea_BottomRightX << " BottomRightY=" << DrawArea_BottomRightY;
#endif

				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 10 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;
			
				ulInputBuffer_WriteIndex++;
			
#ifdef USE_TEMPLATES_LINE_MONO
			Select_MonoLine_t <1> ();
#else
	
#ifdef USE_TEMPLATES_PS1_LINE
			NumPixels = Select_Line_Renderer_t( (DATA_Write_Format*) inputdata_ptr, 0 );
#else
			//Draw_MonoLine_40 ();
			NumPixels = DrawLine_Mono_th( (DATA_Write_Format*) inputdata_ptr, 0 );
#endif

			BusyCycles += NumPixels;
#endif
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_LINE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;
		
		
			
		case 0x48:
		case 0x49:
		case 0x4c:
		case 0x4d:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_LINE
			debug << "\r\nMonochromePolyline";
#endif
			/////////////////////////////////////////////
			// monochrome polyline
			
			// get color
			GetBGR24 ( Buffer [ 0 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_LINE
			debug << " bgr=" << hex << gbgr[0];
#endif

			
			// draw until termination code is reached
			for ( int i = 1; ( Buffer [ i + 1 ].Value & 0xf000f000 ) != 0x50005000; i++ )
			{
				GetXY0 ( Buffer [ i ] );
				GetXY1 ( Buffer [ i + 1 ] );
				
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_LINE
			debug << dec << " x0=" << gx[0] << " y0=" << gy[0] << " x1=" << gx[1] << " y1=" << gy[1];
#endif

					// get pointer into inputdata
					inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
					
					inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
					inputdata_ptr [ 1 ] = DrawArea_TopLeft;
					inputdata_ptr [ 2 ] = DrawArea_BottomRight;
					inputdata_ptr [ 3 ] = DrawArea_Offset;
					
					inputdata_ptr [ 8 ] = Buffer [ i ].Value;
					inputdata_ptr [ 10 ] = Buffer [ i + 1 ].Value;
					inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;
					
				ulInputBuffer_WriteIndex++;
				
#ifdef USE_TEMPLATES_POLYLINE_MONO
				Select_MonoLine_t <0> ();
#else
	
#ifdef USE_TEMPLATES_PS1_LINE
				NumPixels = Select_Line_Renderer_t( (DATA_Write_Format*) inputdata_ptr, 0 );
#else
				//Draw_MonoLine_40 ();
				NumPixels = DrawLine_Mono_th( (DATA_Write_Format*) inputdata_ptr, 0 );
#endif

				BusyCycles += NumPixels;
#endif

			}

#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_LINE
			debug << dec << "\r\nDrawArea: OffsetX=" << DrawArea_OffsetX << " OffsetY=" << DrawArea_OffsetY << " TopLeftX=" << DrawArea_TopLeftX << " TopLeftY=" << DrawArea_TopLeftY << " BottomRightX=" << DrawArea_BottomRightX << " BottomRightY=" << DrawArea_BottomRightY;
#endif
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_LINE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;

			
		case 0x4a:
		case 0x4b:
		case 0x4e:
		case 0x4f:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_LINE
			debug << "\r\nMonochromePolyline Abe=1";
#endif
			/////////////////////////////////////////////
			// monochrome polyline
			
			// get color
			GetBGR24 ( Buffer [ 0 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_LINE
			debug << " bgr=" << hex << gbgr[0];
#endif

			
			// draw until termination code is reached
			for ( int i = 1; ( Buffer [ i + 1 ].Value & 0xf000f000 ) != 0x50005000; i++ )
			{
				GetXY0 ( Buffer [ i ] );
				GetXY1 ( Buffer [ i + 1 ] );
				
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_LINE
			debug << dec << " x0=" << gx[0] << " y0=" << gy[0] << " x1=" << gx[1] << " y1=" << gy[1];
#endif
				
					// get pointer into inputdata
					inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
					
					inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
					inputdata_ptr [ 1 ] = DrawArea_TopLeft;
					inputdata_ptr [ 2 ] = DrawArea_BottomRight;
					inputdata_ptr [ 3 ] = DrawArea_Offset;
					
					inputdata_ptr [ 8 ] = Buffer [ i ].Value;
					inputdata_ptr [ 10 ] = Buffer [ i + 1 ].Value;
					inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;
					
				ulInputBuffer_WriteIndex++;
				
#ifdef USE_TEMPLATES_POLYLINE_MONO
				Select_MonoLine_t <1> ();
#else
	
#ifdef USE_TEMPLATES_PS1_LINE
				NumPixels = Select_Line_Renderer_t( (DATA_Write_Format*) inputdata_ptr, 0 );
#else
				//Draw_MonoLine_40 ();
				NumPixels = DrawLine_Mono_th( (DATA_Write_Format*) inputdata_ptr, 0 );
#endif

				BusyCycles += NumPixels;
#endif
			}

#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_LINE
			debug << dec << "\r\nDrawArea: OffsetX=" << DrawArea_OffsetX << " OffsetY=" << DrawArea_OffsetY << " TopLeftX=" << DrawArea_TopLeftX << " TopLeftY=" << DrawArea_TopLeftY << " BottomRightX=" << DrawArea_BottomRightX << " BottomRightY=" << DrawArea_BottomRightY;
#endif
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_LINE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;
		
		
			
		case 0x50:
		case 0x51:
		case 0x54:
		case 0x55:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_LINE
			debug << "\r\nShadedLine";
#endif
			///////////////////////////////////////////////
			// gradated line
			// *** TODO ***
			//GetBGR0_24 ( Buffer [ 0 ] );
			GetBGR0_8 ( Buffer [ 0 ] );
			GetXY0 ( Buffer [ 1 ] );
			//GetBGR1_24 ( Buffer [ 2 ] );
			GetBGR1_8 ( Buffer [ 2 ] );
			GetXY1 ( Buffer [ 3 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_LINE
			debug << dec << " x0=" << gx[0] << " y0=" << gy[0] << " x1=" << gx[1] << " y1=" << gy[1] << " bgr0=" << hex << gbgr[0] << " bgr1=" << hex << gbgr[1];
			debug << dec << "\r\nDrawArea: OffsetX=" << DrawArea_OffsetX << " OffsetY=" << DrawArea_OffsetY << " TopLeftX=" << DrawArea_TopLeftX << " TopLeftY=" << DrawArea_TopLeftY << " BottomRightX=" << DrawArea_BottomRightX << " BottomRightY=" << DrawArea_BottomRightY;
#endif

				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 9 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 10 ] = Buffer [ 3 ].Value;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;

				ulInputBuffer_WriteIndex++;

#ifdef USE_TEMPLATES_LINE_SHADED
			Select_ShadedLine_t <0> ();
#else
	
#ifdef USE_TEMPLATES_PS1_LINE
			NumPixels = Select_Line_Renderer_t( (DATA_Write_Format*) inputdata_ptr, 0 );
#else
			//Draw_ShadedLine_50 ();
			NumPixels = DrawLine_Gradient_th( (DATA_Write_Format*) inputdata_ptr, 0 );
#endif

			BusyCycles += NumPixels;
#endif
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_LINE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;

			
		case 0x52:
		case 0x53:
		case 0x56:
		case 0x57:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_LINE
			debug << "\r\nShadedLine Abe=1";
#endif
			///////////////////////////////////////////////
			// gradated line
			// *** TODO ***
			//GetBGR0_24 ( Buffer [ 0 ] );
			GetBGR0_8 ( Buffer [ 0 ] );
			GetXY0 ( Buffer [ 1 ] );
			//GetBGR1_24 ( Buffer [ 2 ] );
			GetBGR1_8 ( Buffer [ 2 ] );
			GetXY1 ( Buffer [ 3 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_LINE
			debug << dec << " x0=" << gx[0] << " y0=" << gy[0] << " x1=" << gx[1] << " y1=" << gy[1] << " bgr0=" << hex << gbgr[0] << " bgr1=" << hex << gbgr[1];
			debug << dec << "\r\nDrawArea: OffsetX=" << DrawArea_OffsetX << " OffsetY=" << DrawArea_OffsetY << " TopLeftX=" << DrawArea_TopLeftX << " TopLeftY=" << DrawArea_TopLeftY << " BottomRightX=" << DrawArea_BottomRightX << " BottomRightY=" << DrawArea_BottomRightY;
#endif

				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 9 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 10 ] = Buffer [ 3 ].Value;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;
				
				ulInputBuffer_WriteIndex++;

#ifdef USE_TEMPLATES_LINE_SHADED
			Select_ShadedLine_t <1> ();
#else
	
#ifdef USE_TEMPLATES_PS1_LINE
			NumPixels = Select_Line_Renderer_t( (DATA_Write_Format*) inputdata_ptr, 0 );
#else
			//Draw_ShadedLine_50 ();
			NumPixels = DrawLine_Gradient_th( (DATA_Write_Format*) inputdata_ptr, 0 );
#endif

			BusyCycles += NumPixels;
#endif
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_LINE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;
		
			
		case 0x58:
		case 0x59:
		case 0x5c:
		case 0x5d:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_LINE
			debug << "\r\nShadedPolyline";
#endif
			/////////////////////////////////////////////////
			// gradated line polyline

			
			// draw until termination code is reached
			for ( int i = 0; ( Buffer [ i + 2 ].Value & 0xf000f000 ) != 0x50005000; i += 2 )
			{
				// get color
				//GetBGR0_24 ( Buffer [ i ] );
				GetBGR0_8 ( Buffer [ i ] );
				
				// get coord
				GetXY0 ( Buffer [ i + 1 ] );
				
				// get color
				//GetBGR1_24 ( Buffer [ i + 2 ] );
				GetBGR1_8 ( Buffer [ i + 2 ] );

				// get coord
				GetXY1 ( Buffer [ i + 3	] );
				
					// get pointer into inputdata
					inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
					
					inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
					inputdata_ptr [ 1 ] = DrawArea_TopLeft;
					inputdata_ptr [ 2 ] = DrawArea_BottomRight;
					inputdata_ptr [ 3 ] = DrawArea_Offset;
					
					inputdata_ptr [ 8 ] = Buffer [ i + 1 ].Value;
					inputdata_ptr [ 9 ] = Buffer [ i + 2 ].Value;
					inputdata_ptr [ 10 ] = Buffer [ i + 3 ].Value;
					inputdata_ptr [ 7 ] = ( 0x58 << 24 ) | ( Buffer [ i ].Value & 0xffffff );
					
					// use command 0x58 here
					//inputdata_ptr [ 7 ] = ( 0x58 << 24 ) | ( inputdata_ptr [ 7 ] & 0xffffff );
					
				ulInputBuffer_WriteIndex++;
				
#ifdef USE_TEMPLATES_POLYLINE_SHADED
				Select_ShadedLine_t <0> ();
#else

#ifdef USE_TEMPLATES_PS1_LINE
				NumPixels = Select_Line_Renderer_t( (DATA_Write_Format*) inputdata_ptr, 0 );
#else
				//Draw_ShadedLine_50 ();
				NumPixels = DrawLine_Gradient_th( (DATA_Write_Format*) inputdata_ptr, 0 );
#endif

				BusyCycles += NumPixels;
#endif
			}
			
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_LINE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;
			
		
		case 0x5a:
		case 0x5b:
		case 0x5e:
		case 0x5f:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_LINE
			debug << "\r\nShadedPolyline Abe=1";
#endif
			/////////////////////////////////////////////////
			// gradated line polyline

			
			// draw until termination code is reached
			for ( int i = 0; ( Buffer [ i + 2 ].Value & 0xf000f000 ) != 0x50005000; i += 2 )
			{
				// get color
				//GetBGR0_24 ( Buffer [ i ] );
				GetBGR0_8 ( Buffer [ i ] );
				
				// get coord
				GetXY0 ( Buffer [ i + 1 ] );
				
				// get color
				//GetBGR1_24 ( Buffer [ i + 2 ] );
				GetBGR1_8 ( Buffer [ i + 2 ] );

				// get coord
				GetXY1 ( Buffer [ i + 3 ] );
				
					// get pointer into inputdata
					inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
					
					inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
					inputdata_ptr [ 1 ] = DrawArea_TopLeft;
					inputdata_ptr [ 2 ] = DrawArea_BottomRight;
					inputdata_ptr [ 3 ] = DrawArea_Offset;
					
					inputdata_ptr [ 8 ] = Buffer [ i + 1 ].Value;
					inputdata_ptr [ 9 ] = Buffer [ i + 2 ].Value;
					inputdata_ptr [ 10 ] = Buffer [ i + 3 ].Value;
					inputdata_ptr [ 7 ] = ( 0x5a << 24 ) | ( Buffer [ i ].Value & 0xffffff );
					
					// use command 0x5a here
					//inputdata_ptr [ 7 ] = ( 0x5a << 24 ) | ( inputdata_ptr [ 7 ] & 0xffffff );
					
				ulInputBuffer_WriteIndex++;
				
#ifdef USE_TEMPLATES_POLYLINE_SHADED
				Select_ShadedLine_t <1> ();
#else

#ifdef USE_TEMPLATES_PS1_LINE
				NumPixels = Select_Line_Renderer_t( (DATA_Write_Format*) inputdata_ptr, 0 );
#else
				//Draw_ShadedLine_50 ();
				NumPixels = DrawLine_Gradient_th( (DATA_Write_Format*) inputdata_ptr, 0 );
#endif

				BusyCycles += NumPixels;
#endif
			}
			
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_LINE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;
		
			
		///////////////////////////////////////
		// Sprite commands
			
		case 0x60:
		case 0x61:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_SPRITE
			debug << "\r\nRectangle";
#endif
			////////////////////////////////////////////
			// rectangle
			GetBGR24 ( Buffer [ 0 ] );
			GetXY ( Buffer [ 1 ] );
			GetHW ( Buffer [ 2 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SPRITE
			debug << dec << " x=" << x << " y=" << y << " h=" << h << " w=" << w << hex << " bgr=" << gbgr[0];
#endif

				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 10 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;

				ulInputBuffer_WriteIndex++;

#ifdef USE_TEMPLATES_RECTANGLE
			Draw_Rectangle_60_t <0> ();
#else
	
#ifdef USE_TEMPLATES_PS1_RECTANGLE
			NumPixels = Select_Sprite_Renderer_t ( (DATA_Write_Format*) inputdata_ptr, 0 );
#else
			//Draw_Rectangle_60 ();
			NumPixels = Draw_Rectangle_60_th ( (DATA_Write_Format*) inputdata_ptr, 0 );
#endif

			BusyCycles += NumPixels;
#endif

			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SPRITE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;

			
		case 0x62:
		case 0x63:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_SPRITE
			debug << "\r\nRectangle Abe=1";
#endif
			////////////////////////////////////////////
			// rectangle
			GetBGR24 ( Buffer [ 0 ] );
			GetXY ( Buffer [ 1 ] );
			GetHW ( Buffer [ 2 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SPRITE
			debug << dec << " x=" << x << " y=" << y << " h=" << h << " w=" << w << hex << " bgr=" << gbgr[0];
#endif

				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 10 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;

				ulInputBuffer_WriteIndex++;


#ifdef USE_TEMPLATES_RECTANGLE
			Draw_Rectangle_60_t <1> ();
#else
	
#ifdef USE_TEMPLATES_PS1_RECTANGLE
			NumPixels = Select_Sprite_Renderer_t ( (DATA_Write_Format*) inputdata_ptr, 0 );
#else
			//Draw_Rectangle_60 ();
			NumPixels = Draw_Rectangle_60_th ( (DATA_Write_Format*) inputdata_ptr, 0 );
#endif

			BusyCycles += NumPixels;
#endif


#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SPRITE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;

			
		case 0x64:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_SPRITE
			debug << "\r\nSprite";
#endif
			////////////////////////////////////////////
			// Sprite
			GetBGR24 ( Buffer [ 0 ] );
			GetXY ( Buffer [ 1 ] );
			GetCLUT ( Buffer [ 2 ] );
			GetUV ( Buffer [ 2 ] );
			GetHW ( Buffer [ 3 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SPRITE
			debug << dec << " x=" << x << " y=" << y << " u=" << u << " v=" << v << " h=" << h << " w=" << w << " bgr=" << gbgr[0] << " clut_x=" << clut_x << " clut_y=" << clut_y;
			debug << "\r\nTP=" << GPU_CTRL_Read.TP << " ABR=" << GPU_CTRL_Read.ABR;
#endif


//0: GPU_CTRL_Read
//1: DrawArea_TopLeft
//2: DrawArea_BottomRight
//3: DrawArea_Offset
//4: TextureWindow
//5: ------------
//6: ------------
//7: GetBGR24 ( Buffer [ 0 ] );
//8: GetXY ( Buffer [ 1 ] );
//9: GetCLUT ( Buffer [ 2 ] );
//9: GetUV ( Buffer [ 2 ] );
//10: GetHW ( Buffer [ 3 ] );

				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				inputdata_ptr [ 4 ] = TextureWindow;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 9 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 10 ] = Buffer [ 3 ].Value;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;
				
				ulInputBuffer_WriteIndex++;
				
#ifdef USE_TEMPLATES_SPRITE
			Draw_Sprite_64_t <0,0> ();
#else
			//Draw_Sprite_64 ();
			Draw_Sprite_64 ( (DATA_Write_Format*) inputdata_ptr, 0 );
#endif

			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SPRITE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;
			
		case 0x65:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_SPRITE
			debug << "\r\nSprite Abe=0 Tge=1";
#endif
			////////////////////////////////////////////
			// Sprite
			GetBGR24 ( Buffer [ 0 ] );
			GetXY ( Buffer [ 1 ] );
			GetCLUT ( Buffer [ 2 ] );
			GetUV ( Buffer [ 2 ] );
			GetHW ( Buffer [ 3 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SPRITE
			debug << dec << " x=" << x << " y=" << y << " u=" << u << " v=" << v << " h=" << h << " w=" << w << " bgr=" << gbgr[0] << " clut_x=" << clut_x << " clut_y=" << clut_y;
			debug << "\r\nTP=" << GPU_CTRL_Read.TP << " ABR=" << GPU_CTRL_Read.ABR;
#endif

				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				inputdata_ptr [ 4 ] = TextureWindow;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 9 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 10 ] = Buffer [ 3 ].Value;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;

				ulInputBuffer_WriteIndex++;
			
#ifdef USE_TEMPLATES_SPRITE
			Draw_Sprite_64_t <0,1> ();
#else
			//Draw_Sprite_64 ();
			Draw_Sprite_64 ( (DATA_Write_Format*) inputdata_ptr, 0 );
#endif

#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SPRITE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;
			
		case 0x66:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_SPRITE
			debug << "\r\nSprite Abe=1 Tge=0";
#endif
			////////////////////////////////////////////
			// Sprite
			GetBGR24 ( Buffer [ 0 ] );
			GetXY ( Buffer [ 1 ] );
			GetCLUT ( Buffer [ 2 ] );
			GetUV ( Buffer [ 2 ] );
			GetHW ( Buffer [ 3 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SPRITE
			debug << dec << " x=" << x << " y=" << y << " u=" << u << " v=" << v << " h=" << h << " w=" << w << " bgr=" << gbgr[0] << " clut_x=" << clut_x << " clut_y=" << clut_y;
			debug << "\r\nTP=" << GPU_CTRL_Read.TP << " ABR=" << GPU_CTRL_Read.ABR;
#endif

				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				inputdata_ptr [ 4 ] = TextureWindow;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 9 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 10 ] = Buffer [ 3 ].Value;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;

				ulInputBuffer_WriteIndex++;
			
#ifdef USE_TEMPLATES_SPRITE
			Draw_Sprite_64_t <1,0> ();
#else
			//Draw_Sprite_64 ();
			Draw_Sprite_64 ( (DATA_Write_Format*) inputdata_ptr, 0 );
#endif

#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SPRITE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;


		case 0x67:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_SPRITE
			debug << "\r\nSprite Abe=1 Tge=1";
#endif
			////////////////////////////////////////////
			// Sprite
			GetBGR24 ( Buffer [ 0 ] );
			GetXY ( Buffer [ 1 ] );
			GetCLUT ( Buffer [ 2 ] );
			GetUV ( Buffer [ 2 ] );
			GetHW ( Buffer [ 3 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SPRITE
			debug << dec << " x=" << x << " y=" << y << " u=" << u << " v=" << v << " h=" << h << " w=" << w << " bgr=" << gbgr[0] << " clut_x=" << clut_x << " clut_y=" << clut_y;
			debug << "\r\nTP=" << GPU_CTRL_Read.TP << " ABR=" << GPU_CTRL_Read.ABR;
#endif

				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				inputdata_ptr [ 4 ] = TextureWindow;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 9 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 10 ] = Buffer [ 3 ].Value;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;

				ulInputBuffer_WriteIndex++;
			
#ifdef USE_TEMPLATES_SPRITE
			Draw_Sprite_64_t <1,1> ();
#else
			//Draw_Sprite_64 ();
			Draw_Sprite_64 ( (DATA_Write_Format*) inputdata_ptr, 0 );
#endif

#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SPRITE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;
			
			
			
		case 0x68:
		case 0x69:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_SPRITE
			debug << "\r\nPixel";
#endif
			///////////////////////////////////////////
			// dot
			GetBGR24 ( Buffer [ 0 ] );
			GetXY ( Buffer [ 1 ] );
			
				BusyCycles += 1;
				
				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;

				// a pixel is a 1x1 rectangle
				inputdata_ptr [ 10 ] = 0x00010001;
				
			if ( ulNumberOfThreads )
			{
				ulInputBuffer_WriteIndex++;
				
			}
			else
			{
			Draw_Pixel_68 ();
			}
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SPRITE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;
			
			
		case 0x6a:
		case 0x6b:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_SPRITE
			debug << "\r\nPixel Abe=1";
#endif
			///////////////////////////////////////////
			// dot
			GetBGR24 ( Buffer [ 0 ] );
			GetXY ( Buffer [ 1 ] );
			
				BusyCycles += 1;
				
				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;
				
				// a pixel is a 1x1 rectangle
				inputdata_ptr [ 10 ] = 0x00010001;

			if ( ulNumberOfThreads )
			{
				ulInputBuffer_WriteIndex++;
				
			}
			else
			{
			Draw_Pixel_68 ();
			}
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SPRITE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;
			
			
		case 0x6c:
		
			// note is copied from martin korth psx documentation
			// GP0(6Ch) - Textured Rectangle, 1x1 (nonsense), opaque, texture-blending

			cout << "\nhps1x64 WARNING: GPU Command 0x6c : 1x1 textured rectangle; opaque; texture-blending\n";
			break;
			
		case 0x6d:
		
			// note is copied from martin korth psx documentation
			// Textured Rectangle, 1x1 (nonsense), opaque, raw-texture
			
			cout << "\nhps1x64 WARNING: GPU Command 0x6d : 1x1 textured rectangle; opaque; raw-texture\n";
			break;
			
			
		case 0x6e:
		
			// note is copied from martin korth psx documentation
			// Textured Rectangle, 1x1 (nonsense), semi-transp, texture-blending
			
			cout << "\nhps1x64 WARNING: GPU Command 0x6e : 1x1 textured rectangle; semi-transparent; texture-blending\n";
			break;
			
			
		case 0x6f:
		
			// note is copied from martin korth psx documentation
			// Textured Rectangle, 1x1 (nonsense), semi-transp, raw-texture
			
			cout << "\nhps1x64 WARNING: GPU Command 0x6f : 1x1 textured rectangle; semi-transparent; raw-texture\n";
			break;
			
		case 0x70:
		case 0x71:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_SPRITE
			debug << "\r\nRectangle8";
#endif
			/////////////////////////////////////////
			// 8x8 rectangle
			GetBGR24 ( Buffer [ 0 ] );
			GetXY ( Buffer [ 1 ] );
			
				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 10 ] = 0x00080008;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;
				
				ulInputBuffer_WriteIndex++;
			
#ifdef USE_TEMPLATES_RECTANGLE8
			Draw_Rectangle8x8_70_t <0> ();
#else
	
#ifdef USE_TEMPLATES_PS1_RECTANGLE
			NumPixels = Select_Sprite_Renderer_t ( (DATA_Write_Format*) inputdata_ptr, 0 );
#else
			//Draw_Rectangle8x8_70 ();
			NumPixels = Draw_Rectangle_60_th ( (DATA_Write_Format*) inputdata_ptr, 0 );
#endif

			BusyCycles += NumPixels;
#endif
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SPRITE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;
			
			
		case 0x72:
		case 0x73:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_SPRITE
			debug << "\r\nRectangle8 Abe=1";
#endif
			/////////////////////////////////////////
			// 8x8 rectangle
			GetBGR24 ( Buffer [ 0 ] );
			GetXY ( Buffer [ 1 ] );
			
				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 10 ] = 0x00080008;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;
				
				ulInputBuffer_WriteIndex++;
			
#ifdef USE_TEMPLATES_RECTANGLE8
			Draw_Rectangle8x8_70_t <1> ();
#else
	
#ifdef USE_TEMPLATES_PS1_RECTANGLE
			NumPixels = Select_Sprite_Renderer_t ( (DATA_Write_Format*) inputdata_ptr, 0 );
#else
			//Draw_Rectangle8x8_70 ();
			NumPixels = Draw_Rectangle_60_th ( (DATA_Write_Format*) inputdata_ptr, 0 );
#endif

			BusyCycles += NumPixels;
#endif
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SPRITE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;
		
		
			
		case 0x74:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_SPRITE
			debug << "\r\nSprite8";
#endif

			///////////////////////////////////////////
			// 8x8 sprite
			GetBGR24 ( Buffer [ 0 ] );
			GetXY ( Buffer [ 1 ] );
			GetCLUT ( Buffer [ 2 ] );
			GetUV ( Buffer [ 2 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SPRITE
			debug << dec << " x=" << x << " y=" << y << " u=" << u << " v=" << v << " clut_x=" << clut_x << " clut_y=" << clut_y << hex << " bgr=" << gbgr[0];
			debug << "\r\nTP=" << GPU_CTRL_Read.TP << " ABR=" << GPU_CTRL_Read.ABR;
#endif

				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				inputdata_ptr [ 4 ] = TextureWindow;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 9 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 10 ] = 0x00080008;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;
				
				ulInputBuffer_WriteIndex++;
			
#ifdef USE_TEMPLATES_SPRITE8
			Draw_Sprite8x8_74_t <0,0> ();
#else
			//Draw_Sprite8x8_74 ();
			Draw_Sprite_64 ( (DATA_Write_Format*) inputdata_ptr, 0 );
#endif
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SPRITE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;



		case 0x75:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_SPRITE
			debug << "\r\nSprite8 Abe=0 Tge=1";
#endif

			///////////////////////////////////////////
			// 8x8 sprite
			GetBGR24 ( Buffer [ 0 ] );
			GetXY ( Buffer [ 1 ] );
			GetCLUT ( Buffer [ 2 ] );
			GetUV ( Buffer [ 2 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SPRITE
			debug << dec << " x=" << x << " y=" << y << " u=" << u << " v=" << v << " clut_x=" << clut_x << " clut_y=" << clut_y << hex << " bgr=" << gbgr[0];
			debug << "\r\nTP=" << GPU_CTRL_Read.TP << " ABR=" << GPU_CTRL_Read.ABR;
#endif

				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				inputdata_ptr [ 4 ] = TextureWindow;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 9 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 10 ] = 0x00080008;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;

				ulInputBuffer_WriteIndex++;
			
#ifdef USE_TEMPLATES_SPRITE8
			Draw_Sprite8x8_74_t <0,1> ();
#else
			//Draw_Sprite8x8_74 ();
			Draw_Sprite_64 ( (DATA_Write_Format*) inputdata_ptr, 0 );
#endif
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SPRITE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;
			
			
		case 0x76:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_SPRITE
			debug << "\r\nSprite8 Abe=1 Tge=0";
#endif

			///////////////////////////////////////////
			// 8x8 sprite
			GetBGR24 ( Buffer [ 0 ] );
			GetXY ( Buffer [ 1 ] );
			GetCLUT ( Buffer [ 2 ] );
			GetUV ( Buffer [ 2 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SPRITE
			debug << dec << " x=" << x << " y=" << y << " u=" << u << " v=" << v << " clut_x=" << clut_x << " clut_y=" << clut_y << hex << " bgr=" << gbgr[0];
			debug << "\r\nTP=" << GPU_CTRL_Read.TP << " ABR=" << GPU_CTRL_Read.ABR;
#endif

				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				inputdata_ptr [ 4 ] = TextureWindow;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 9 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 10 ] = 0x00080008;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;
				
				ulInputBuffer_WriteIndex++;
			
#ifdef USE_TEMPLATES_SPRITE8
			Draw_Sprite8x8_74_t <1,0> ();
#else
			//Draw_Sprite8x8_74 ();
			Draw_Sprite_64 ( (DATA_Write_Format*) inputdata_ptr, 0 );
#endif
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SPRITE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;



		case 0x77:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_SPRITE
			debug << "\r\nSprite8 Abe=1 Tge=1";
#endif

			///////////////////////////////////////////
			// 8x8 sprite
			GetBGR24 ( Buffer [ 0 ] );
			GetXY ( Buffer [ 1 ] );
			GetCLUT ( Buffer [ 2 ] );
			GetUV ( Buffer [ 2 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SPRITE
			debug << dec << " x=" << x << " y=" << y << " u=" << u << " v=" << v << " clut_x=" << clut_x << " clut_y=" << clut_y << hex << " bgr=" << gbgr[0];
			debug << "\r\nTP=" << GPU_CTRL_Read.TP << " ABR=" << GPU_CTRL_Read.ABR;
#endif

				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				inputdata_ptr [ 4 ] = TextureWindow;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 9 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 10 ] = 0x00080008;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;
				
				ulInputBuffer_WriteIndex++;

#ifdef USE_TEMPLATES_SPRITE8
			Draw_Sprite8x8_74_t <1,1> ();
#else
			//Draw_Sprite8x8_74 ();
			Draw_Sprite_64 ( (DATA_Write_Format*) inputdata_ptr, 0 );
#endif
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SPRITE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;
			
			
			
		case 0x78:
		case 0x79:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_SPRITE
			debug << "\r\nRectangle16";
#endif
			/////////////////////////////////////////
			// 16x16 rectangle
			GetBGR24 ( Buffer [ 0 ] );
			GetXY ( Buffer [ 1 ] );

				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 10 ] = 0x00100010;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;
				
				ulInputBuffer_WriteIndex++;
			
#ifdef USE_TEMPLATES_RECTANGLE16
			Draw_Rectangle16x16_78_t <0> ();
#else
	
#ifdef USE_TEMPLATES_PS1_RECTANGLE
			NumPixels = Select_Sprite_Renderer_t ( (DATA_Write_Format*) inputdata_ptr, 0 );
#else
			//Draw_Rectangle16x16_78 ();
			NumPixels = Draw_Rectangle_60_th ( (DATA_Write_Format*) inputdata_ptr, 0 );
#endif

			BusyCycles += NumPixels;
#endif
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SPRITE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;
			
			
		case 0x7a:
		case 0x7b:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_SPRITE
			debug << "\r\nRectangle16 Abe=1";
#endif
			/////////////////////////////////////////
			// 16x16 rectangle
			GetBGR24 ( Buffer [ 0 ] );
			GetXY ( Buffer [ 1 ] );

				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 10 ] = 0x00100010;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;
				
				ulInputBuffer_WriteIndex++;
			
#ifdef USE_TEMPLATES_RECTANGLE16
			Draw_Rectangle16x16_78_t <1> ();
#else
	
#ifdef USE_TEMPLATES_PS1_RECTANGLE
			NumPixels = Select_Sprite_Renderer_t ( (DATA_Write_Format*) inputdata_ptr, 0 );
#else
			//Draw_Rectangle16x16_78 ();
			NumPixels = Draw_Rectangle_60_th ( (DATA_Write_Format*) inputdata_ptr, 0 );
#endif

			BusyCycles += NumPixels;
#endif
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SPRITE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;
			
			
			
		case 0x7c:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_SPRITE
			debug << "\r\nSprite16";
#endif
			//////////////////////////////////////////
			// 16x16 sprite
			GetBGR24 ( Buffer [ 0 ] );
			GetXY ( Buffer [ 1 ] );
			GetCLUT ( Buffer [ 2 ] );
			GetUV ( Buffer [ 2 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SPRITE
			debug << dec << " x=" << x << " y=" << y << " u=" << u << " v=" << v << " clut_x=" << clut_x << " clut_y=" << clut_y << hex << " bgr=" << gbgr[0];
			debug << "\r\nTP=" << GPU_CTRL_Read.TP << " ABR=" << GPU_CTRL_Read.ABR << " CycleCount=" << dec << *_DebugCycleCount;
#endif

				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				inputdata_ptr [ 4 ] = TextureWindow;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 9 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 10 ] = 0x00100010;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;
				
				ulInputBuffer_WriteIndex++;

#ifdef USE_TEMPLATES_SPRITE16
			Draw_Sprite16x16_7c_t <0,0> ();
#else
			//Draw_Sprite16x16_7c ();
			Draw_Sprite_64 ( (DATA_Write_Format*) inputdata_ptr, 0 );
#endif
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SPRITE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;


		case 0x7d:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_SPRITE
			debug << "\r\nSprite16 Abe=0 Tge=1";
#endif
			//////////////////////////////////////////
			// 16x16 sprite
			GetBGR24 ( Buffer [ 0 ] );
			GetXY ( Buffer [ 1 ] );
			GetCLUT ( Buffer [ 2 ] );
			GetUV ( Buffer [ 2 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SPRITE
			debug << dec << " x=" << x << " y=" << y << " u=" << u << " v=" << v << " clut_x=" << clut_x << " clut_y=" << clut_y << hex << " bgr=" << gbgr[0];
			debug << "\r\nTP=" << GPU_CTRL_Read.TP << " ABR=" << GPU_CTRL_Read.ABR << " CycleCount=" << dec << *_DebugCycleCount;
#endif

				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				inputdata_ptr [ 4 ] = TextureWindow;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 9 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 10 ] = 0x00100010;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;
				
				ulInputBuffer_WriteIndex++;

#ifdef USE_TEMPLATES_SPRITE16
			Draw_Sprite16x16_7c_t <0,1> ();
#else
			//Draw_Sprite16x16_7c ();
			Draw_Sprite_64 ( (DATA_Write_Format*) inputdata_ptr, 0 );
#endif

#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SPRITE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;
			
			
		case 0x7e:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_SPRITE
			debug << "\r\nSprite16 Abe=1 Tge=0";
#endif
			//////////////////////////////////////////
			// 16x16 sprite
			GetBGR24 ( Buffer [ 0 ] );
			GetXY ( Buffer [ 1 ] );
			GetCLUT ( Buffer [ 2 ] );
			GetUV ( Buffer [ 2 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SPRITE
			debug << dec << " x=" << x << " y=" << y << " u=" << u << " v=" << v << " clut_x=" << clut_x << " clut_y=" << clut_y << hex << " bgr=" << gbgr[0];
			debug << "\r\nTP=" << GPU_CTRL_Read.TP << " ABR=" << GPU_CTRL_Read.ABR << " CycleCount=" << dec << *_DebugCycleCount;
#endif

				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				inputdata_ptr [ 4 ] = TextureWindow;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 9 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 10 ] = 0x00100010;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;
				
				ulInputBuffer_WriteIndex++;

#ifdef USE_TEMPLATES_SPRITE16
			Draw_Sprite16x16_7c_t <1,0> ();
#else
			//Draw_Sprite16x16_7c ();
			Draw_Sprite_64 ( (DATA_Write_Format*) inputdata_ptr, 0 );
#endif
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SPRITE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;



		case 0x7f:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_SPRITE
			debug << "\r\nSprite16 Abe=1 Tge=1";
#endif
			//////////////////////////////////////////
			// 16x16 sprite
			GetBGR24 ( Buffer [ 0 ] );
			GetXY ( Buffer [ 1 ] );
			GetCLUT ( Buffer [ 2 ] );
			GetUV ( Buffer [ 2 ] );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SPRITE
			debug << dec << " x=" << x << " y=" << y << " u=" << u << " v=" << v << " clut_x=" << clut_x << " clut_y=" << clut_y << hex << " bgr=" << gbgr[0];
			debug << "\r\nTP=" << GPU_CTRL_Read.TP << " ABR=" << GPU_CTRL_Read.ABR << " CycleCount=" << dec << *_DebugCycleCount;
#endif

				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				inputdata_ptr [ 1 ] = DrawArea_TopLeft;
				inputdata_ptr [ 2 ] = DrawArea_BottomRight;
				inputdata_ptr [ 3 ] = DrawArea_Offset;
				inputdata_ptr [ 4 ] = TextureWindow;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 9 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 10 ] = 0x00100010;
				inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;
				
				ulInputBuffer_WriteIndex++;
			
#ifdef USE_TEMPLATES_SPRITE16
			Draw_Sprite16x16_7c_t <1,1> ();
#else
			//Draw_Sprite16x16_7c ();
			Draw_Sprite_64 ( (DATA_Write_Format*) inputdata_ptr, 0 );
#endif

#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_SPRITE
			debug << ";BusyCycles=" << BusyCycles;
#endif
			break;
			
			
		////////////////////////////////////////
		// Transfer commands
			
		case 0x80:
		case 0x81:
		case 0x82:
		case 0x83:
		case 0x84:
		case 0x85:
		case 0x86:
		case 0x87:
		case 0x88:
		case 0x89:
		case 0x8a:
		case 0x8b:
		case 0x8c:
		case 0x8d:
		case 0x8e:
		case 0x8f:
		case 0x90:
		case 0x91:
		case 0x92:
		case 0x93:
		case 0x94:
		case 0x95:
		case 0x96:
		case 0x97:
		case 0x98:
		case 0x99:
		case 0x9a:
		case 0x9b:
		case 0x9c:
		case 0x9d:
		case 0x9e:
		case 0x9f:		
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_TRANSFER
			debug << "\r\nMoveImage";
			debug << " " << hex << setw ( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount;
#endif

			
			///////////////////////////////////////////
			// move image in frame buffer

			
			/*
			sX = Buffer [ 1 ].x;
			sY = Buffer [ 1 ].y;
			dX = Buffer [ 2 ].x;
			dY = Buffer [ 2 ].y;
			h = Buffer [ 3 ].h;
			w = Buffer [ 3 ].w;
			*/

			//if ( ulNumberOfThreads )
			//{
				
				// get pointer into inputdata
				inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
				
				inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
				
				inputdata_ptr [ 8 ] = Buffer [ 1 ].Value;
				inputdata_ptr [ 9 ] = Buffer [ 2 ].Value;
				inputdata_ptr [ 10 ] = Buffer [ 3 ].Value;
				//inputdata_ptr [ 7 ] = Buffer [ 0 ].Value;
				inputdata_ptr [ 7 ] = 0x80000000;
				
				// send the command to the other thread
				ulInputBuffer_WriteIndex++;
				
				//BusyCycles += ( ( ( h - 1 ) & 0x1ff ) + 1 ) * ( ( ( w - 1 ) & 0x3ff ) + 1 ) * dMoveImage_80_CyclesPerPixel;
				BusyCycles += ( ( ( Buffer [ 3 ].h - 1 ) & 0x1ff ) + 1 ) * ( ( ( Buffer [ 3 ].w - 1 ) & 0x3ff ) + 1 ) * dMoveImage_80_CyclesPerPixel;
			//}
			//else
			//{
				
				//Transfer_MoveImage_80 ();
				Transfer_MoveImage_80_th ( (DATA_Write_Format*) inputdata_ptr, 0 );
			//}
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_TRANSFER
	debug << "; COMMAND: IMAGE MOVE; h = " << dec << h << "; w = " << w << "; sX=" << sX << "; sY=" << sY << "; dX=" << dX << "; dY=" << dY;
#endif
			break;
			
		case 0xa0:
		case 0xa1:
		case 0xa2:
		case 0xa3:
		case 0xa4:
		case 0xa5:
		case 0xa6:
		case 0xa7:
		case 0xa8:
		case 0xa9:
		case 0xaa:
		case 0xab:
		case 0xac:
		case 0xad:
		case 0xae:
		case 0xaf:
		case 0xb0:
		case 0xb1:
		case 0xb2:
		case 0xb3:
		case 0xb4:
		case 0xb5:
		case 0xb6:
		case 0xb7:
		case 0xb8:
		case 0xb9:
		case 0xba:
		case 0xbb:
		case 0xbc:
		case 0xbd:
		case 0xbe:
		case 0xbf:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_TRANSFER
			debug << "\r\nImportImage";
			debug << " " << hex << setw ( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount;
#endif
			
			// don't know why it would need to wait here
			//if ( ulNumberOfThreads )
			//{
			//	// for now, wait to finish
			//	Finish ();
			//}
			

			//////////////////////////////////////////
			// send image to frame buffer
			dX = Buffer [ 1 ].x;
			dY = Buffer [ 1 ].y;
			h = Buffer [ 2 ].h;
			w = Buffer [ 2 ].w;
			iX = 0;
			iY = 0;
			
			// xpos & 0x3ff
			dX &= 0x3ff;
			
			// ypos & 0x1ff
			dY &= 0x1ff;
			
			// Xsiz=((Xsiz-1) AND 3FFh)+1
			w = ( ( w - 1 ) & 0x3ff ) + 1;
			
			// Ysiz=((Ysiz-1) AND 1FFh)+1
			h = ( ( h - 1 ) & 0x1ff ) + 1;
			
			BufferMode = MODE_IMAGEIN;
			
			iCurrentCount = 0;
			iTotalCount = ( ( w * h ) + 1 ) >> 1;
			
			// set busy cycles to 1 so that we get debug info
			BusyCycles = 1;

			// set the image id for bulk processing
			uImageId++;
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_TRANSFER
	debug << dec << " x=" << dX << " y=" << dY << " h=" << h << " w=" << w;
#endif
			break;
			
		case 0xc0:
		case 0xc1:
		case 0xc2:
		case 0xc3:
		case 0xc4:
		case 0xc5:
		case 0xc6:
		case 0xc7:
		case 0xc8:
		case 0xc9:
		case 0xca:
		case 0xcb:
		case 0xcc:
		case 0xcd:
		case 0xce:
		case 0xcf:
		case 0xd0:
		case 0xd1:
		case 0xd2:
		case 0xd3:
		case 0xd4:
		case 0xd5:
		case 0xd6:
		case 0xd7:
		case 0xd8:
		case 0xd9:
		case 0xda:
		case 0xdb:
		case 0xdc:
		case 0xdd:
		case 0xde:
		case 0xdf:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_TRANSFER
			debug << "\r\nExportImage";
			debug << " " << hex << setw ( 8 ) << *_DebugPC << " " << dec << *_DebugCycleCount;
#endif

			if ( ulNumberOfThreads | bEnable_OpenCL )
			{
				// for now, wait to finish
				//while ( ulInputBuffer_Count );
				Finish ();
			}


			/////////////////////////////////////////
			// copy image from frame buffer
			sX = Buffer [ 1 ].x;
			sY = Buffer [ 1 ].y;
			h = Buffer [ 2 ].h;
			w = Buffer [ 2 ].w;
			
			// xpos & 0x3ff
			sX &= 0x3ff;
			
			// ypos & 0x1ff
			sY &= 0x1ff;
			
			// Xsiz=((Xsiz-1) AND 3FFh)+1
			w = ( ( w - 1 ) & 0x3ff ) + 1;
			
			// Ysiz=((Ysiz-1) AND 1FFh)+1
			h = ( ( h - 1 ) & 0x1ff ) + 1;

			// if using gpu for drawing, then need to pull the image out into local vram first
			if ( bEnable_OpenCL )
			{
				PreTransferPixelPacketOut ();
			}
			
			// initialize the transfer (starting from upper left corner of image)
			iX = 0;
			iY = 0;

			BufferMode = MODE_IMAGEOUT;
			
			////////////////////////////////////////////////////////
			// mark GPU as being ready to send an image
			GPU_CTRL_Read.IMG = 1;
			
			// set busy cycles to 1 so that we get debug info
			BusyCycles = 1;
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_TRANSFER
	debug << dec << " x=" << sX << " y=" << sY << " h=" << h << " w=" << w;
#endif
			break;
			
		///////////////////////////////////////
		// Environment commands
		
		
		case 0xe0:
		
			// Unknown //
			
#ifdef VERBOSE_GPU
			cout << "\nhps1x64 WARNING: Unknown GPU command: " << hex << (u32) Buffer [ 0 ].Command << "\n";
#endif
			
			break;
			
			
		
		case 0xe1:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_DISPLAYMODE || defined INLINE_DEBUG_RUN_ENVIRON
			debug << "\r\nSetDrawMode";
#endif
			////////////////////////////////////////////
			// draw mode setting
			
			
			// sets GPU Status up to bit 10 or more depending on GPU version - I'll use 11 bits total
			// gpu version 2 only sets bits 0x0-0xa - I'll stick with this - 11 bits
			GPU_CTRL_Read.Value = ( GPU_CTRL_Read.Value & 0xfffff800 ) | ( Buffer [ 0 ].Value & 0x7ff );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_DISPLAYMODE || defined INLINE_DEBUG_RUN_ENVIRON
			debug << dec << " TX=" << GPU_CTRL_Read.TX << " TY=" << GPU_CTRL_Read.TY << " ABR=" << GPU_CTRL_Read.ABR << " TP=" << GPU_CTRL_Read.TP << " DTD=" << GPU_CTRL_Read.DTD << " DFE=" << GPU_CTRL_Read.DFE;
#endif

			break;
			
		case 0xe2:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_ENVIRON || defined INLINE_DEBUG_WINDOW
			debug << "\r\nSetTextureWindow";
#endif
			////////////////////////////////////////////
			// texture window setting
			TextureWindow = Buffer [ 0 ].Value;
			
			TWX = ( Buffer [ 0 ].Value >> 10 ) & 0x1f;
			TWY = ( Buffer [ 0 ].Value >> 15 ) & 0x1f;
			TWH = ( Buffer [ 0 ].Value >> 5 ) & 0x1f;
			TWW = Buffer [ 0 ].Value & 0x1f;
			
			// it's actually value*8 for x and y
			TextureWindow_X = ( TWX << 3 );
			TextureWindow_Y = ( TWY << 3 );
			
			// it's actually 256-(value*8) for width and height
			TextureWindow_Height = 256 - ( TWH << 3 );
			TextureWindow_Width = 256 - ( TWW << 3 );
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_ENVIRON || defined INLINE_DEBUG_WINDOW
			debug << dec << " X=" << TextureWindow_X << " Y=" << TextureWindow_Y << " Height=" << TextureWindow_Height << " Width=" << TextureWindow_Width;
#endif
			
			break;
			
		case 0xe3:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_DISPLAYAREA || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_ENVIRON
			debug << "\r\nSetDrawingAreaTopLeft";
#endif
			////////////////////////////////////////////
			// set drawing area top left
			
			DrawArea_TopLeft = Buffer [ 0 ].Value & 0xfffff;
			
			//DrawArea_TopLeftX = Buffer [ 0 ].Value & 0x3ff;
			//DrawArea_TopLeftY = ( Buffer [ 0 ].Value >> 10 ) & 0x3ff;
			iREG_DrawArea_TopLeftX = Buffer [ 0 ].Value & 0x3ff;
			iREG_DrawArea_TopLeftY = ( Buffer [ 0 ].Value >> 10 ) & 0x3ff;
			
			// *problem* this might cause problems if drawing area is outside the framebuffer
			DrawArea_TopLeftX = iREG_DrawArea_TopLeftX;
			DrawArea_TopLeftY = iREG_DrawArea_TopLeftY;
			
			// perform a boundary check on the x and y
			if ( DrawArea_TopLeftX >= FrameBuffer_Width ) DrawArea_TopLeftX = FrameBuffer_Width - 1;
			if ( DrawArea_TopLeftY >= FrameBuffer_Height ) DrawArea_TopLeftY = FrameBuffer_Height - 1;
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_DISPLAYAREA || defined INLINE_DEBUG_RUN_ENVIRON
			debug << dec << "; X = " << DrawArea_TopLeftX << "; Y = " << DrawArea_TopLeftY;
#endif

			break;
			
		case 0xe4:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_DISPLAYAREA || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_ENVIRON
			debug << "\r\nSetDrawingAreaBottomRight";
#endif
			/////////////////////////////////////////////
			// set drawing area bottom right
			
			DrawArea_BottomRight = Buffer [ 0 ].Value & 0xfffff;
			
			iREG_DrawArea_BottomRightX = Buffer [ 0 ].Value & 0x3ff;
			iREG_DrawArea_BottomRightY = ( Buffer [ 0 ].Value >> 10 ) & 0x3ff;
			
			// *problem* this might cause problems if drawing area is outside the framebuffer
			DrawArea_BottomRightX = iREG_DrawArea_BottomRightX;
			DrawArea_BottomRightY = iREG_DrawArea_BottomRightY;
			
			// perform a boundary check on the x and y
			if ( DrawArea_BottomRightX >= FrameBuffer_Width ) DrawArea_BottomRightX = FrameBuffer_Width - 1;
			if ( DrawArea_BottomRightY >= FrameBuffer_Height ) DrawArea_BottomRightY = FrameBuffer_Height - 1;
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_DISPLAYAREA || defined INLINE_DEBUG_RUN_ENVIRON
			debug << dec << "; X = " << DrawArea_BottomRightX << "; Y = " << DrawArea_BottomRightY;
#endif

			break;
			
		case 0xe5:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_DISPLAYOFFSET || defined INLINE_DEBUG_EXECUTE_NAME || defined INLINE_DEBUG_RUN_ENVIRON
			debug << "\r\nSetDrawingOffset";
#endif

			///////////////////////////////////////////////
			// drawing offset
			// *note* draw offset is signed and both x and y go from -1024 to +1023 (11 bits)
			s32 sTemp;
			
			DrawArea_Offset = Buffer [ 0 ].Value & 0x3fffff;
			
			// get x offset
			sTemp = Buffer [ 0 ].Value & 0x7ff;
			
			// sign extend
			sTemp = ( sTemp << 21 );
			sTemp = ( sTemp >> 21 );
			
			// store
			DrawArea_OffsetX = sTemp;
			
			// get y offset
			sTemp = ( Buffer [ 0 ].Value >> 11 ) & 0x7ff;
			
			// sign extend
			sTemp = ( sTemp << 21 );
			sTemp = ( sTemp >> 21 );
			
			// store
			DrawArea_OffsetY = sTemp;
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_DISPLAYOFFSET || defined INLINE_DEBUG_RUN_ENVIRON
			debug << dec << "; X = " << DrawArea_OffsetX << "; Y = " << DrawArea_OffsetY;
#endif

			break;
			
		case 0xe6:
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_ENVIRON || defined INLINE_DEBUG_MASK
			debug << "\r\nMaskSetting";
#endif
			////////////////////////////////////////////////
			// mask setting
			SetMaskBitWhenDrawing = Buffer [ 0 ].Value & 1;
			DoNotDrawToMaskAreas = ( Buffer [ 0 ].Value >> 1 ) & 1;
			
			// set gpu status flags
			GPU_CTRL_Read.MD = SetMaskBitWhenDrawing;
			GPU_CTRL_Read.ME = DoNotDrawToMaskAreas;
			
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_ENVIRON || defined INLINE_DEBUG_MASK
			debug << "; SetMaskBitWhenDrawing=" << SetMaskBitWhenDrawing << "; DoNotDrawToMaskAreas=" << DoNotDrawToMaskAreas;
#endif

			break;


		case 0xe7:
		case 0xe8:
		case 0xe9:
		case 0xea:
		case 0xeb:
		case 0xec:
		case 0xed:
		case 0xee:
		case 0xef:
		case 0xf0:
		case 0xf1:
		case 0xf2:
		case 0xf3:
		case 0xf4:
		case 0xf5:
		case 0xf6:
		case 0xf7:
		case 0xf8:
		case 0xf9:
		case 0xfa:
		case 0xfb:
		case 0xfc:
		case 0xfd:
		case 0xfe:
		case 0xff:
		
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_EXECUTE_NAME
			debug << "\r\nUnknown Command " << hex << (u32) Buffer [ 0 ].Command;
#endif

			// Unknown //
			
#ifdef VERBOSE_GPU
			cout << "\nhps1x64 WARNING: Unknown GPU command: " << hex << (u32) Buffer [ 0 ].Command << "\n";
#endif

			break;


			
		default:
			cout << "\nhps1x64 WARNING: GPU::ExecuteGPUBuffer: Unknown GPU Command @ Cycle#" << dec << *_DebugCycleCount << " PC=" << hex << *_DebugPC << " Command=" << (u32) Buffer [ 0 ].Command;
			break;
			
	}
	
	
	if ( !GPU_CTRL_Read.DFE )
	{
		// *** testing *** only drew half of scanlines so half cycles
		BusyCycles >>= 1;
	}
	
	
	////////////////////////////////////////////////////////////////////////////////////
	// check for how long GPU should be busy for after executing the last command
	if ( BusyCycles )
	{
		/////////////////////////////////////
		// mark GPU as busy
		GPU_CTRL_Read.BUSY = 0;
		
		////////////////////////////////////////////////
		// mark GPU as not ready to receive commands
		GPU_CTRL_Read.COM = 0;
		
		// update the cycle that device is busy until
		BusyUntil_Cycle = BusyCycles + *_DebugCycleCount;
		
		// update count of primitives drawn (will not count invisible primitives for now
		Primitive_Count++;
	}
	else
	{
		///////////////////////
		// GPU is not busy
		GPU_CTRL_Read.BUSY = 1;
		GPU_CTRL_Read.COM = 1;
	}

}


// returns count of pixels transferred in
u32 GPU::TransferPixelPacketIn ( u32* pData, s32 BS )
{
	u32 bgr2;
	u32 pix0, pix1;
	u32 DestPixel, PixelMask = 0, SetPixelMask = 0;
	u32 Data, Count = 0;
	u32 *inputdata_ptr;
	u32 *inputdata_ptr2;
	
#ifdef INLINE_DEBUG_PIX_WRITE
	debug << "; TRANSFER PIX IN; h = " << dec << h << "; w = " << w << "; iX = " << iX << "; iY = " << iY;
#endif

	if ( ulNumberOfThreads || bEnable_OpenCL )
	{
		// for now, wait to finish
		//while ( ulInputBuffer_Count & c_ulInputBuffer_Size );
		
		//BusyCycles += 16 * 16 * 1;
		
		// get pointer into inputdata
		inputdata_ptr = & ( inputdata [ ( ulInputBuffer_WriteIndex & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );
		
		inputdata_ptr [ 0 ] = GPU_CTRL_Read.Value;
		inputdata_ptr [ 1 ] = dX;
		inputdata_ptr [ 2 ] = dY;
		inputdata_ptr [ 3 ] = w;
		inputdata_ptr [ 4 ] = h;
		inputdata_ptr [ 5 ] = iX;
		inputdata_ptr [ 6 ] = iY;
		
		// get up to 8 pixels
		Count = ( BS > 8 ) ? 8 : BS;

		// put in command and count here
		inputdata_ptr [ 7 ] = ( 0xa0 << 24 ) | ( Count & 0xf ) | ( ( uImageId << 4 ) & 0x00fffff0 );

		iCurrentCount += Count;
		if ( iCurrentCount >= iTotalCount )
		{
			Count -= ( iCurrentCount - iTotalCount );
			iCurrentCount = iTotalCount;
		}
		
#ifdef ENABLE_HWPIXEL_INPUT
		inputdata_ptr [ 7 ] = ( 0xa0 << 24 );
		
		inputdata_ptr2 = & ( inputdata [ ( (ulInputBuffer_WriteIndex-1) & c_ulInputBuffer_Mask ) << c_ulInputBuffer_Shift ] );

		// check if we are creating a new command
		// check if previous command is part of this transfer
		if (
			( ulInputBuffer_TargetIndex > (ulInputBuffer_WriteIndex-1) )
			|| ( inputdata_ptr2 [ 8 ] != uImageId )
			|| ( inputdata_ptr2 [ 7 ] != ( 0xa0 << 24 ) )
		)
		{
			// new command //

			// set the id
			inputdata_ptr [ 8 ] = uImageId;

			// set the start index in pixel input buffer (start index from target index)
			inputdata_ptr [ 9 ] = ullPixelInBuffer_WriteIndex - ullPixelInBuffer_TargetIndex;

			// set the count of 32-bit words in block
			inputdata_ptr [ 10 ] = BS;

			// set total pixel count
			inputdata_ptr [ 11 ] = w * h;


			// new command filled in
			// send the command to the other thread
			ulInputBuffer_WriteIndex++;
		}
		else
		{
			// same command //

			// update the count of pixels in previous command
			inputdata_ptr2 [ 10 ] += BS;
		}

		// copy in the data into buffer //
		for ( int i = 0; i < BS; i++ )
		{
			ulPixelInBuffer32 [ ullPixelInBuffer_WriteIndex & c_ullPixelInBuffer_Mask ] = pData [ i ];
			ullPixelInBuffer_WriteIndex++;
		}

		// used BS as the count
		//Count = BS;

		// update ix,iy
		iX += BS << 1;
		while ( iX >= w )
		{
			iX -= w;
			iY += 1;

			if ( iY >= h )
			{
				/////////////////////////////////////
				// set buffer mode back to normal
				BufferMode = MODE_NORMAL;
				break;
			}
		}

		return BS;

#else
		
		for ( int i = 0; i < Count; i++ )
		{
			inputdata_ptr [ 8 + i ] = pData [ i ];
		}

		// send the command to the other thread
		ulInputBuffer_WriteIndex++;
#endif
		
		
		// get the count
		iX += ( Count << 1 );
		while ( iX >= w )
		{
			iX -= w;
			iY++;
			
			if ( iY >= h )
			{
				/////////////////////////////////////
				// set buffer mode back to normal
				BufferMode = MODE_NORMAL;
				
				////////////////////////////////////////
				// done
				return Count;
			}
		}
		
		// need to do this for each send
		return Count;
	}

	if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
	if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;
	
	while ( Count < BS )
	{
		Data = *pData++;
		Count++;
		
	//////////////////////////////////////////////////////
	// transfer pixel of image to VRAM
	pix0 = Data & 0xffff;
	pix1 = ( Data >> 16 );
	
	// transfer pix0
	//if ( ( dX + iX ) < FrameBuffer_Width && ( dY + iY ) < FrameBuffer_Height )
	//{
		bgr2 = pix0;
		
		// read pixel from frame buffer if we need to check mask bit
		DestPixel = VRAM [ ( (dX + iX) & 0x3ff ) + ( ( (dY + iY) & 0x1ff ) << 10 ) ];
		
		//VRAM [ (dX + iX) + ( (dY + iY) << 10 ) ] = pix0;
		
		// check if we should set mask bit when drawing
		//if ( GPU_CTRL_Read.MD ) bgr2 |= 0x8000;
		bgr2 |= SetPixelMask;

		// draw pixel if we can draw to mask pixels or mask bit not set
		if ( ! ( DestPixel & PixelMask ) ) VRAM [ ( (dX + iX) & 0x3ff ) + ( ( (dY + iY) & 0x1ff ) << 10 ) ] = bgr2;
		//VRAM [ ( (dX + iX) & 0x3ff ) + ( ( (dY + iY) & 0x1ff ) << 10 ) ] = bgr2;
	//}
	//else
	//{
		//cout << "\nGPU::TransferPixelPacketIn: Error: Transferring pix0 outside of VRAM bounds. x=" << dec << (dX+iX) << " y=" << (dY+iY) << " DrawArea_OffsetX=" << DrawArea_OffsetX << " DrawArea_OffsetY=" << DrawArea_OffsetY;
	//}

	
	// update x
	iX++;
	
	// if it is at width then go to next line
	if ( iX == w )
	{
		iX = 0;
		iY++;
		
		// if this was the last pixel, then we are done
		if ( iY == h )
		{
			/////////////////////////////////////
			// set buffer mode back to normal
			BufferMode = MODE_NORMAL;
			
			////////////////////////////////////////
			// done
			return Count;
		}
	}
	
	
	// transfer pix 1
	//if ( ( dX + iX ) < FrameBuffer_Width && ( dY + iY ) < FrameBuffer_Height )
	//{
		//VRAM [ (dX + iX) + ( (dY + iY) << 10 ) ] = pix1;
		bgr2 = pix1;
		
		// read pixel from frame buffer if we need to check mask bit
		DestPixel = VRAM [ ( (dX + iX) & 0x3ff ) + ( ( (dY + iY) & 0x1ff ) << 10 ) ];
		
		//VRAM [ (dX + iX) + ( (dY + iY) << 10 ) ] = pix0;
		
		// check if we should set mask bit when drawing
		//if ( GPU_CTRL_Read.MD ) bgr2 |= 0x8000;
		bgr2 |= SetPixelMask;

		// draw pixel if we can draw to mask pixels or mask bit not set
		if ( ! ( DestPixel & PixelMask ) ) VRAM [ ( (dX + iX) & 0x3ff ) + ( ( (dY + iY) & 0x1ff ) << 10 ) ] = bgr2;
		//VRAM [ ( (dX + iX) & 0x3ff ) + ( ( (dY + iY) & 0x1ff ) << 10 ) ] = bgr2;
	//}
	//else
	//{
		//cout << "\nGPU::TransferPixelPacketIn: Error: Transferring pix1 outside of VRAM bounds. x=" << dec << (dX+iX) << " y=" << (dY+iY) << " DrawArea_OffsetX=" << DrawArea_OffsetX << " DrawArea_OffsetY=" << DrawArea_OffsetY;
	//}

	
	// update x
	iX++;
	
	// if it is at width then go to next line
	if ( iX == w )
	{
		iX = 0;
		iY++;
		
		// if this was the last pixel, then we are done
		if ( iY == h )
		{
			/////////////////////////////////////
			// set buffer mode back to normal
			BufferMode = MODE_NORMAL;
			
			////////////////////////////////////////
			// done
			return Count;
		}
	}
	
	}
	
	return Count;
	
}


void GPU::PreTransferPixelPacketOut ()
{
	u32 *pBuf32;
	u32 ulOffset;

	//u32 pix0, pix1;
	//u32 SetPixelMask = 0;
	
	//if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x80008000;
	//cout << "\n***PreTransferPixelPacketOut***";
	//cout << " w=" << dec << w << " h=" << h << " sx=" << sX << " sY=" << sY;

	DisplayOutput_Window->OpenGL_MakeCurrentWindow ();


	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo);

	pBuf32 = (u32*) glMapBuffer ( GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY );


	
	//////////////////////////////////////////////////////
	// transfer pixel of image from VRAM
	for ( iY = 0; iY < h; iY++ )
	{
		for ( iX = 0; iX < w; iX++ )
		{
			//cout << "\niX=" << iX << " iY=" << iY;
			ulOffset = ( (sX + iX) & 0x3ff ) + ( ( (sY + iY) & 0x1ff ) << 10 );
			VRAM [ ulOffset ] = (u16) pBuf32 [ ulOffset ];
		}
	}
	

	glUnmapBuffer( GL_SHADER_STORAGE_BUFFER );


	DisplayOutput_Window->OpenGL_ReleaseWindow ();

}


u32 GPU::TransferPixelPacketOut ()
{
#ifdef INLINE_DEBUG_PIX_READ
	debug << "; TRANSFER PIX OUT; h = " << dec << h << "; w = " << w << "; iX = " << iX << "; iY = " << iY;
#endif

	u32 pix0, pix1;
	u32 SetPixelMask = 0;
	
	if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x80008000;
	
	//////////////////////////////////////////////////////
	// transfer pixel of image from VRAM
	
	// transfer pix0
	//if ( ( sX + iX ) < FrameBuffer_Width && ( sY + iY ) < FrameBuffer_Height )
	//{
		pix0 = VRAM [ ( (sX + iX) & 0x3ff ) + ( ( (sY + iY) & 0x1ff ) << 10 ) ];
	//}
	//else
	//{
		//cout << "\nGPU::TransferPixelPacketOut: Error: Transferring pix0 outside of VRAM bounds. x=" << dec << (sX+iX) << " y=" << (sY+iY) << " DrawArea_OffsetX=" << DrawArea_OffsetX << " DrawArea_OffsetY=" << DrawArea_OffsetY;
	//}
	
	// update x
	iX++;
	
	// if it is at width then go to next line
	if ( iX == w )
	{
		iX = 0;
		iY++;
		
		// if this was the last pixel, then we are done
		if ( iY == h )
		{
			/////////////////////////////////////
			// set buffer mode back to normal
			BufferMode = MODE_NORMAL;
			
			////////////////////////////////////////////////////////
			// mark GPU no longer ready to send an image
			GPU_CTRL_Read.IMG = 0;
			
			////////////////////////////////////////
			// done
			return pix0;
		}
	}
	
	
	// transfer pix 1
	//if ( ( sX + iX ) < FrameBuffer_Width && ( sY + iY ) < FrameBuffer_Height )
	//{
		pix1 = VRAM [ ( (sX + iX) & 0x3ff ) + ( ( (sY + iY) & 0x1ff ) << 10 ) ];
	//}
	//else
	//{
		//cout << "\nGPU::TransferPixelPacketOut: Error: Transferring pix1 outside of VRAM bounds. x=" << dec << (sX+iX) << " y=" << (sY+iY) << " DrawArea_OffsetX=" << DrawArea_OffsetX << " DrawArea_OffsetY=" << DrawArea_OffsetY;
	//}
	
	// update x
	iX++;
	
	// if it is at width then go to next line
	if ( iX == w )
	{
		iX = 0;
		iY++;
		
		// if this was the last pixel, then we are done
		if ( iY == h )
		{
			/////////////////////////////////////
			// set buffer mode back to normal
			BufferMode = MODE_NORMAL;
			
			////////////////////////////////////////////////////////
			// mark GPU no longer ready to send an image
			GPU_CTRL_Read.IMG = 0;
			
			////////////////////////////////////////
			// done
		}
	}
	
	return pix0 | ( pix1 << 16 );
	//return pix0 | ( pix1 << 16 ) | SetPixelMask;
}


//void GPU::ProcessDataRegWrite ( u32 Data )
void GPU::ProcessDataRegWrite ( u32* pData, s32 BS )
{
#ifdef INLINE_DEBUG_DMA_WRITE
	debug << "; DataRegWrite";
#endif

	u32 pix0, pix1;
	u32 Data, Count;

	while ( BS )
	{
		
	// make sure we are not sending or receiving images
	if ( BufferMode == MODE_NORMAL )
	{
#ifdef INLINE_DEBUG_DMA_WRITE
	debug << "; NORMAL; Data = " << hex << Data << ";(Before) BufferSize=" << dec << BufferSize;
#endif

		Data = *pData++;
		BS--;

		// add data into buffer
		if ( BufferSize < 16 )
		{
			Buffer [ BufferSize++ ].Value = Data;
			//BufferSize &= 0xf;
		}
		else
		{
			// extended past edge of gpu buffer //
			cout << "\nhps1x64 ERROR: GPU: Extended past end of buffer.\n";
		}

#ifdef INLINE_DEBUG_DMA_WRITE
	debug << ";(After) BufferSize=" << dec << BufferSize;
#endif
		// check if we have all the arguments that are needed to execute command
		if ( BufferSize == GPU_SizeOfCommands [ Buffer [ 0 ].Command ] )
		{
#ifdef INLINE_DEBUG_DMA_WRITE
	debug << "; EXEC BUFFER; Command = " << hex << ((u32)(Buffer [ 0 ].Command));
#endif
			ExecuteGPUBuffer ();
			BufferSize = 0;
		}
		
		// if drawing a poly line, then check for termination code
		// note: must add-in the "BufferSize > 1" part since it is possible command could be interpreted as termination code
		else if ( GPU_SizeOfCommands [ Buffer [ 0 ].Command ] == 55 && ( ( Buffer [ BufferSize - 1 ].Value & 0xf000f000 ) == 0x50005000 ) && ( BufferSize > 1 ) )
		{
			ExecuteGPUBuffer ();
			BufferSize = 0;
		}
		else if ( GPU_SizeOfCommands [ Buffer [ 0 ].Command ] == 66 && ( ( Buffer [ BufferSize - 1 ].Value & 0xf000f000 ) == 0x50005000 ) && ( BufferSize > 4 ) && !( ( BufferSize - 1 ) & 0x1 ) )
		{
			// shaded poly-line //
			ExecuteGPUBuffer ();
			BufferSize = 0;
		}
		
	}
	
	//////////////////////////////////////////////////////////////
	// Check if we are receiving an image
	else if ( BufferMode == MODE_IMAGEIN )
	{
#ifdef INLINE_DEBUG_DMA_WRITE
	debug << "; IMAGE IN";
#endif

		// receive a pixel from bus
		Count = TransferPixelPacketIn ( pData, BS );
		pData += Count;
		BS -= Count;
	}
	else
	{
#ifdef INLINE_DEBUG_DMA_WRITE
	debug << "; InvalidBufferMode; BufferMode=" << dec << BufferMode << " w=" << w << " h=" << h;
#endif

		BS = 0;
	}
	
	}
}


u32 GPU::ProcessDataRegRead ()
{
	u32 pix0, pix1;
#ifdef INLINE_DEBUG_DMA_READ
	debug << "; DataRegRead";
#endif

	if ( BufferMode == MODE_NORMAL )
	{
#ifdef INLINE_DEBUG_DMA_READ
		debug << "; READING RESULT=" << hex << GPU_DATA_Read;
#endif

		// if the GPU is not transferring an image from the GPU, then it must be reading a result from command sent to CTRL reg
		return GPU_DATA_Read;
	}
	
	//////////////////////////////////////////////////////////////////////
	// check if GPU is transferring an image from GPU
	else if ( BufferMode == MODE_IMAGEOUT )
	{
#ifdef INLINE_DEBUG_DMA_READ
	debug << "; IMAGE OUT";
#endif

		// send a pixel to bus
		return TransferPixelPacketOut ();
	}

	return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GPU::Draw_FrameBufferRectangle_02 ()
{
	// *** todo *** fix wrapping and sizing of frame buffer fill //
	
#ifdef _ENABLE_SSE2
	// with sse2, can send 8 pixels at a time
	static const int c_iVectorSize = 8;
#else
	// before sse2, was sending 4 pixels at a time
	static const int c_iVectorSize = 4;
#endif

	// ptr to vram
	//u16 *ptr;
	u64 *ptr;
	u64 bgr64;
	u32 width1, width2, height1, height2, xmax, ymax;
	
#ifdef _ENABLE_SSE2
	__m128i vbgr;
	vbgr = _mm_set1_epi16 ( bgr );
#endif
	
	/////////////////////////////////////
	// mark GPU as busy
	//GPU_CTRL_Read.BUSY = 0;
	
	////////////////////////////////////////////////
	// mark GPU as not ready to receive commands
	//GPU_CTRL_Read.COM = 0;
	
	// set bgr64
	bgr64 = gbgr [ 0 ];
	bgr64 |= ( bgr64 << 16 );
	bgr64 |= ( bgr64 << 32 );
	
	
	// Xpos=(Xpos AND 3F0h)
	x &= 0x3f0;
	
	// ypos & 0x1ff
	y &= 0x1ff;
	
	// Xsiz=((Xsiz AND 3FFh)+0Fh) AND (NOT 0Fh)
	w = ( ( w & 0x3ff ) + 0xf ) & ~0xf;
	
	// Ysiz=((Ysiz AND 1FFh))
	h &= 0x1ff;
	
	
	// *** NOTE: coordinates wrap *** //
	
	///////////////////////////////////////////////
	// set amount of time GPU will be busy for
	BusyCycles += (u32) ( ( (u64) h * (u64) w * dFrameBufferRectangle_02_CyclesPerPixel ) );
	
	// get width of segment 1 and segment 2
	xmax = x + w;
	width2 = 0;
	if ( xmax > FrameBuffer_Width ) width2 = xmax - FrameBuffer_Width;
	width1 = w - width2;
	
	// get height of segment 1 and segment 2
	ymax = y + h;
	height2 = 0;
	if ( ymax > FrameBuffer_Height ) height2 = ymax - FrameBuffer_Height;
	height1 = h - height2;
	
	// need to first make sure there is something to draw
	if ( h > 0 && w > 0 )
	{
	
	// draw segment 1 height
	for ( ; y < ymax; y++ )
	{
		// wraparound y
		iY = y & 0x1ff;
		
		//ptr = & (VRAM [ x + ( iY << 10 ) ]);
		ptr = (u64*) ( & (VRAM [ x + ( iY << 10 ) ]) );
		

		// draw segment 1 width
		//for ( iX = 0; iX < width1; iX += 4 )
		for ( iX = 0; iX < width1; iX += c_iVectorSize )
		{
#ifdef _ENABLE_SSE2
			_mm_store_si128 ( (__m128i*) ptr, vbgr );
			ptr += 2;
#else
			*ptr++ = bgr64;
#endif
		}

		
		//ptr = & (VRAM [ y << 10 ]);
		//ptr -= FrameBuffer_Width;
		//ptr = & (VRAM [ iY << 10 ]);
		ptr = (u64*) ( & (VRAM [ iY << 10 ]) );
		
		// draw segment 2 width
		//for ( ; iX < w; iX += 4 )
		while ( iX < w )
		{
#ifdef _ENABLE_SSE2
			_mm_store_si128 ( (__m128i*) ptr, vbgr );
			ptr += 2;
#else
			*ptr++ = bgr64;
#endif

			iX += c_iVectorSize;
		}
	} // end for ( ; y < ymax; y++ )
	
	} // end if ( h > 0 && w > 0 )
	
}


#ifndef EXCLUDE_RECTANGLE_NONTEMPLATE

void GPU::Draw_Rectangle_60 ()
{
#ifdef _ENABLE_SSE2_RECTANGLE_NONTEMPLATE
	// with sse2, can send 8 pixels at a time
	static const int c_iVectorSize = 8;
#else
	// before sse2, was sending 1 pixels at a time
	static const int c_iVectorSize = 1;
#endif

	//u32 Pixel;
	
	s32 StartX, EndX, StartY, EndY;
	u32 PixelsPerLine;
	u16 *ptr;
	
	// new local variables
	s32 x0, x1, y0, y1;
	u32 bgr, bgr_temp;
	s64 x_across;
	s32 Line;
	
#ifdef _ENABLE_SSE2_RECTANGLE_NONTEMPLATE
	__m128i DestPixel, PixelMask, SetPixelMask;
	__m128i vbgr, vbgr_temp, vStartX, vEndX, vx_across, vSeq, vVectorSize;
	
	vSeq = _mm_set_epi16 ( 7, 6, 5, 4, 3, 2, 1, 0 );
	vVectorSize = _mm_set1_epi16 ( c_iVectorSize );
	
	vbgr = _mm_set1_epi16 ( bgr );
	PixelMask = _mm_setzero_si128 ();
	SetPixelMask = _mm_setzero_si128 ();
	if ( GPU_CTRL_Read.ME ) PixelMask = _mm_set1_epi16 ( 0x8080 );
	if ( GPU_CTRL_Read.MD ) SetPixelMask = _mm_set1_epi16 ( 0x8000 );
#else
	u32 DestPixel, PixelMask = 0, SetPixelMask = 0;
	
	if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
	if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;
#endif


	

	// initialize number of pixels drawn
	NumberOfPixelsDrawn = 0;
	
	// check for some important conditions
	if ( DrawArea_BottomRightX < DrawArea_TopLeftX )
	{
		//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightX < DrawArea_TopLeftX.\n";
		return;
	}
	
	
	if ( DrawArea_BottomRightY < DrawArea_TopLeftY )
	{
		//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightY < DrawArea_TopLeftY.\n";
		return;
	}

	
	// get color(s)
	bgr = gbgr [ 0 ];
	
	// ?? convert to 16-bit ?? (or should leave 24-bit?)
	bgr = ( ( bgr & ( 0xf8 << 0 ) ) >> 3 ) | ( ( bgr & ( 0xf8 << 8 ) ) >> 6 ) | ( ( bgr & ( 0xf8 << 16 ) ) >> 9 );
	
	
	// get top left corner of sprite and bottom right corner of sprite
	x0 = x;
	y0 = y;
	x1 = x + w - 1;
	y1 = y + h - 1;
	
	//////////////////////////////////////////
	// get coordinates on screen
	x0 = DrawArea_OffsetX + x0;
	y0 = DrawArea_OffsetY + y0;
	x1 = DrawArea_OffsetX + x1;
	y1 = DrawArea_OffsetY + y1;
	
	// check if sprite is within draw area
	if ( x1 < ((s32)DrawArea_TopLeftX) || x0 > ((s32)DrawArea_BottomRightX) || y1 < ((s32)DrawArea_TopLeftY) || y0 > ((s32)DrawArea_BottomRightY) ) return;
	
	
	
	
	StartX = x0;
	EndX = x1;
	StartY = y0;
	EndY = y1;

	if ( StartY < ((s32)DrawArea_TopLeftY) )
	{
		StartY = DrawArea_TopLeftY;
	}
	
	if ( EndY > ((s32)DrawArea_BottomRightY) )
	{
		EndY = DrawArea_BottomRightY;
	}
	
	if ( StartX < ((s32)DrawArea_TopLeftX) )
	{
		StartX = DrawArea_TopLeftX;
	}
	
	if ( EndX > ((s32)DrawArea_BottomRightX) )
	{
		EndX = DrawArea_BottomRightX;
	}

	
	NumberOfPixelsDrawn = ( EndX - StartX + 1 ) * ( EndY - StartY + 1 );
	
#ifdef _ENABLE_SSE2_RECTANGLE_NONTEMPLATE
	vStartX = _mm_add_epi16 ( vSeq, _mm_set1_epi16 ( StartX ) );
	vEndX = _mm_set1_epi16 ( EndX + 1 );
#endif
	
	for ( Line = StartY; Line <= EndY; Line++ )
	{
		ptr = & ( VRAM [ StartX + ( Line << 10 ) ] );
		
#ifdef _ENABLE_SSE2_RECTANGLE_NONTEMPLATE
		vx_across = vStartX;
#endif

		// draw horizontal line
		//for ( x_across = StartX; x_across <= EndX; x_across++ )
		for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize )
		{
#ifdef _ENABLE_SSE2_RECTANGLE_NONTEMPLATE
			DestPixel = _mm_loadu_si128 ((__m128i const*) ptr);
			vbgr_temp = vbgr;
			if ( command_abe ) vbgr_temp = vSemiTransparency16( DestPixel, vbgr_temp, GPU_CTRL_Read.ABR );
			vbgr_temp = _mm_or_si128 ( vbgr_temp, SetPixelMask );
			_mm_maskmoveu_si128 ( vbgr_temp, _mm_andnot_si128 ( _mm_and_si128 ( _mm_srai_epi16 ( DestPixel, 15 ), PixelMask ), _mm_cmplt_epi16 ( vx_across, vEndX ) ), (char*) ptr );
			vx_across = _mm_add_epi16 ( vx_across, vVectorSize );
#else
			// read pixel from frame buffer if we need to check mask bit
			DestPixel = *ptr;
			
			bgr_temp = bgr;

			// semi-transparency
			if ( command_abe )
			{
				bgr_temp = SemiTransparency16 ( DestPixel, bgr_temp, GPU_CTRL_Read.ABR );
			}
			
			// check if we should set mask bit when drawing
			bgr_temp |= SetPixelMask;

			// draw pixel if we can draw to mask pixels or mask bit not set
			if ( ! ( DestPixel & PixelMask ) ) *ptr = bgr_temp;
#endif
			
			// update pointer for pixel out
			ptr += c_iVectorSize;
		}
	}
	
	// set the amount of time drawing used up
	BusyCycles = NumberOfPixelsDrawn * 1;
}

#endif


#ifndef EXCLUDE_RECTANGLE8_NONTEMPLATE

void GPU::Draw_Rectangle8x8_70 ()
{
	w = 8; h = 8;
	Draw_Rectangle_60 ();
}

#endif


#ifndef EXCLUDE_RECTANGLE16_NONTEMPLATE

void GPU::Draw_Rectangle16x16_78 ()
{
	w = 16; h = 16;
	Draw_Rectangle_60 ();
}

#endif


void GPU::Draw_Pixel_68 ()
{
	u32 bgr;
	s32 Absolute_DrawX, Absolute_DrawY;
	
	u16* ptr16;
	
	u32 DestPixel, PixelMask = 0;

	// check for some important conditions
	if ( DrawArea_BottomRightX < DrawArea_TopLeftX )
	{
		//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightX < DrawArea_TopLeftX.\n";
		return;
	}
	
	
	if ( DrawArea_BottomRightY < DrawArea_TopLeftY )
	{
		//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightY < DrawArea_TopLeftY.\n";
		return;
	}


	// get color(s)
	bgr = gbgr [ 0 ];
	
	// ?? convert to 16-bit ?? (or should leave 24-bit?)
	bgr = ( ( bgr & ( 0xf8 << 0 ) ) >> 3 ) | ( ( bgr & ( 0xf8 << 8 ) ) >> 6 ) | ( ( bgr & ( 0xf8 << 16 ) ) >> 9 );
	
	
	if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
	
	/////////////////////////////////////
	// mark GPU as busy
	//GPU_CTRL_Read.BUSY = 0;
	
	////////////////////////////////////////////////
	// mark GPU as not ready to receive commands
	//GPU_CTRL_Read.COM = 0;
	
	///////////////////////////////////////////////
	// set amount of time GPU will be busy for
	BusyCycles += 1;
	
	/////////////////////////////////////////
	// Draw the pixel
	Absolute_DrawX = DrawArea_OffsetX + x;
	Absolute_DrawY = DrawArea_OffsetY + y;

	// make sure we are putting pixel within draw area
	if ( Absolute_DrawX >= DrawArea_TopLeftX && Absolute_DrawY >= DrawArea_TopLeftY
	&& Absolute_DrawX <= DrawArea_BottomRightX && Absolute_DrawY <= DrawArea_BottomRightY )
	{
		ptr16 = & ( VRAM [ Absolute_DrawX + ( Absolute_DrawY << 10 ) ] );
		
		//bgr2 = bgr;
		//bgr = gbgr [ 0 ];
		
		// read pixel from frame buffer if we need to check mask bit
		//DestPixel = VRAM [ Absolute_DrawX + ( Absolute_DrawY << 10 ) ];
		DestPixel = *ptr16;
		
		// semi-transparency
		if ( command_abe )
		{
			bgr = SemiTransparency16 ( DestPixel, bgr, GPU_CTRL_Read.ABR );
		}
		
		// check if we should set mask bit when drawing
		if ( GPU_CTRL_Read.MD ) bgr |= 0x8000;

		// draw pixel if we can draw to mask pixels or mask bit not set
		//if ( ! ( DestPixel & PixelMask ) ) VRAM [ Absolute_DrawX + ( Absolute_DrawY << 10 ) ] = bgr;
		if ( ! ( DestPixel & PixelMask ) ) *ptr16 = bgr;
	}
}






void GPU::Transfer_MoveImage_80 ()
{
	u32 SrcPixel, DstPixel;
	u32 DestPixel, PixelMask = 0, SetPixelMask = 0;
	
	u32 SrcStartX, SrcStartY, DstStartX, DstStartY, Height, Width, SrcXRun, DstXRun, Width1, Width2, CurX, CurY;
	u16 *SrcPtr, *DstPtr, *SrcLinePtr, *DstLinePtr;
	
	///////////////////////////////////////////////
	// set amount of time GPU will be busy for
	BusyCycles += h * w * dMoveImage_80_CyclesPerPixel;	//CyclesPerPixelMove;

	// nocash psx specifications: transfer/move vram-to-vram use masking
	if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
	if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;
	
	// xpos & 0x3ff
	//sX &= 0x3ff;
	SrcStartX = sX & 0x3ff;
	//dX &= 0x3ff;
	DstStartX = dX & 0x3ff;
	
	// ypos & 0x1ff
	//sY &= 0x1ff;
	SrcStartY = sY & 0x1ff;
	//dY &= 0x1ff;
	DstStartY = dY & 0x1ff;
	
	// Xsiz=((Xsiz-1) AND 3FFh)+1
	Width = ( ( w - 1 ) & 0x3ff ) + 1;
	
	// Ysiz=((Ysiz-1) AND 1FFh)+1
	Height = ( ( h - 1 ) & 0x1ff ) + 1;
	
	// *** NOTE: coordinates wrap *** //
	
	SrcXRun = FrameBuffer_Width - SrcStartX;
	SrcXRun = ( Width <= SrcXRun ) ? Width : SrcXRun;
	
	DstXRun = FrameBuffer_Width - DstStartX;
	DstXRun = ( Width <= DstXRun ) ? Width : DstXRun;
	
	Width1 = ( SrcXRun < DstXRun ) ? SrcXRun : DstXRun;
	Width2 = ( SrcXRun > DstXRun ) ? SrcXRun : DstXRun;
	
	for ( CurY = 0; CurY < Height; CurY++ )
	{
		// start Src/Dst pointers for line
		SrcLinePtr = & ( VRAM [ ( ( SrcStartY + CurY ) & FrameBuffer_YMask ) << 10 ] );
		DstLinePtr = & ( VRAM [ ( ( DstStartY + CurY ) & FrameBuffer_YMask ) << 10 ] );
		
		SrcPtr = & ( SrcLinePtr [ ( SrcStartX ) & FrameBuffer_XMask ] );
		DstPtr = & ( DstLinePtr [ ( DstStartX ) & FrameBuffer_XMask ] );
		
		// should always transfer this first block, since the width is always at least 1
		for ( CurX = 0; CurX < Width1; CurX++ )
		{
			SrcPixel = *SrcPtr++;
			DstPixel = *DstPtr;
			
			//SrcPixel |= SetPixelMask;
			
			if ( ! ( DstPixel & PixelMask ) ) *DstPtr++ = ( SrcPixel | SetPixelMask );
		}
		
		if ( CurX < Width2 )
		{
		
		SrcPtr = & ( SrcLinePtr [ ( SrcStartX + CurX ) & FrameBuffer_XMask ] );
		DstPtr = & ( DstLinePtr [ ( DstStartX + CurX ) & FrameBuffer_XMask ] );

		for ( ; CurX < Width2; CurX++ )
		{
			SrcPixel = *SrcPtr++;
			DstPixel = *DstPtr;
			
			//SrcPixel |= SetPixelMask;
			
			if ( ! ( DstPixel & PixelMask ) ) *DstPtr++ = ( SrcPixel | SetPixelMask );
		}
		
		} // end if ( CurX < Width2 )
	
		if ( CurX < Width )
		{
		
		SrcPtr = & ( SrcLinePtr [ ( SrcStartX + CurX ) & FrameBuffer_XMask ] );
		DstPtr = & ( DstLinePtr [ ( DstStartX + CurX ) & FrameBuffer_XMask ] );
		
		for ( ; CurX < Width; CurX++ )
		{
			SrcPixel = *SrcPtr++;
			DstPixel = *DstPtr;
			
			//SrcPixel |= SetPixelMask;
			
			if ( ! ( DstPixel & PixelMask ) ) *DstPtr++ = ( SrcPixel | SetPixelMask );
		}
		
		} // end if ( CurX < Width )
	}
	
}




////////////////////////////////////////////////////////////////
// *** Triangle Drawing ***


#ifndef EXCLUDE_TRIANGLE_MONO_NONTEMPLATE

void GPU::DrawTriangle_Mono ( u32 Coord0, u32 Coord1, u32 Coord2 )
{
#ifdef _ENABLE_SSE2_TRIANGLE_MONO_NONTEMPLATE
	// with sse2, can send 8 pixels at a time
	static const int c_iVectorSize = 8;
#else
	// before sse2, was sending 1 pixels at a time
	static const int c_iVectorSize = 1;
#endif

	//u32 Pixel, TexelIndex, Y1_OnLeft;
	//u32 color_add;
	
	//u32 Y1_OnLeft;
	
	u16 *ptr;
	//u32 TexCoordX, TexCoordY;
	//u32 Shift1 = 0, Shift2 = 0, And1 = 0, And2 = 0;
	s64 Temp;
	s64 LeftMostX, RightMostX;
	
	s32 StartX, EndX, StartY, EndY;
	//s64 Error_Left;
	s64 r10, r20, r21;
	
	// new local variables
	s32 x0, x1, x2, y0, y1, y2;
	s64 dx_left, dx_right;
	s64 x_left, x_right, x_across;
	u32 bgr, bgr_temp;
	s32 Line;
	s64 t0, t1, denominator;
	
	//u32 X1Index = 0, X0Index = 1;
	
	//u32 Coord0 = 0, Coord1 = 1, Coord2 = 2;
	
#ifdef _ENABLE_SSE2_TRIANGLE_MONO_NONTEMPLATE
	__m128i DestPixel, PixelMask, SetPixelMask;
	__m128i vbgr, vbgr_temp, vStartX, vEndX, vx_across, vSeq, vVectorSize;
	
	vSeq = _mm_set_epi16 ( 7, 6, 5, 4, 3, 2, 1, 0 );
	vVectorSize = _mm_set1_epi16 ( c_iVectorSize );
	
	vbgr = _mm_set1_epi16 ( bgr );
	PixelMask = _mm_setzero_si128 ();
	SetPixelMask = _mm_setzero_si128 ();
	if ( GPU_CTRL_Read.ME ) PixelMask = _mm_set1_epi16 ( 0x8080 );
	if ( GPU_CTRL_Read.MD ) SetPixelMask = _mm_set1_epi16 ( 0x8000 );
#else
	u32 DestPixel, PixelMask = 0, SetPixelMask = 0;
	
	if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
	if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;
#endif
	
	
	// initialize number of pixels drawn
	NumberOfPixelsDrawn = 0;
	
	// check for some important conditions
	if ( DrawArea_BottomRightX < DrawArea_TopLeftX )
	{
		//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightX < DrawArea_TopLeftX.\n";
		return;
	}
	
	if ( DrawArea_BottomRightY < DrawArea_TopLeftY )
	{
		//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightY < DrawArea_TopLeftY.\n";
		return;
	}
	
	// get color(s)
	bgr = gbgr [ 0 ];
	
	// ?? convert to 16-bit ?? (or should leave 24-bit?)
	bgr = ( ( bgr & ( 0xf8 << 0 ) ) >> 3 ) | ( ( bgr & ( 0xf8 << 8 ) ) >> 6 ) | ( ( bgr & ( 0xf8 << 16 ) ) >> 9 );
	
	// get y-values
	//y0 = Buffer [ Coord0 ].y;
	//y1 = Buffer [ Coord1 ].y;
	//y2 = Buffer [ Coord2 ].y;
	
	///////////////////////////////////
	// put top coordinates in x0,y0
	//if ( y1 < y0 )
	if ( gy [ Coord1 ] < gy [ Coord0 ] )
	{
		//Swap ( y0, y1 );
		Swap ( Coord0, Coord1 );
	}
	
	//if ( y2 < y0 )
	if ( gy [ Coord2 ] < gy [ Coord0 ] )
	{
		//Swap ( y0, y2 );
		Swap ( Coord0, Coord2 );
	}
	
	///////////////////////////////////////
	// put middle coordinates in x1,y1
	//if ( y2 < y1 )
	if ( gy [ Coord2 ] < gy [ Coord1 ] )
	{
		//Swap ( y1, y2 );
		Swap ( Coord1, Coord2 );
	}
	
	// get x-values
	x0 = gx [ Coord0 ];
	x1 = gx [ Coord1 ];
	x2 = gx [ Coord2 ];
	
	// get y-values
	y0 = gy [ Coord0 ];
	y1 = gy [ Coord1 ];
	y2 = gy [ Coord2 ];
	
	//////////////////////////////////////////
	// get coordinates on screen
	x0 = DrawArea_OffsetX + x0;
	y0 = DrawArea_OffsetY + y0;
	x1 = DrawArea_OffsetX + x1;
	y1 = DrawArea_OffsetY + y1;
	x2 = DrawArea_OffsetX + x2;
	y2 = DrawArea_OffsetY + y2;
	
	
	
	
	//u32 NibblesPerPixel;
	
	//if ( tpage_tp == 0 ) NibblesPerPixel = 1; else if ( tpage_tp == 1 ) NibblesPerPixel = 2; else NibblesPerPixel = 4;

	//if ( tpage_tp == 0 )
	//{
	//	Shift1 = 2; Shift2 = 2; And1 = 3; And2 = 0xf;
	//}
	//else if ( tpage_tp == 1 )
	//{
	//	Shift1 = 1; Shift2 = 3; And1 = 1; And2 = 0xff;
	//}
	
	
	
	// get the left/right most x
	LeftMostX = ( ( x0 < x1 ) ? x0 : x1 );
	LeftMostX = ( ( x2 < LeftMostX ) ? x2 : LeftMostX );
	RightMostX = ( ( x0 > x1 ) ? x0 : x1 );
	RightMostX = ( ( x2 > RightMostX ) ? x2 : RightMostX );

	// check if sprite is within draw area
	if ( RightMostX <= ((s32)DrawArea_TopLeftX) || LeftMostX > ((s32)DrawArea_BottomRightX) || y2 <= ((s32)DrawArea_TopLeftY) || y0 > ((s32)DrawArea_BottomRightY) ) return;
	
	// skip drawing if distance between vertices is greater than max allowed by GPU
	if ( ( _Abs( x1 - x0 ) > c_MaxPolygonWidth ) || ( _Abs( x2 - x1 ) > c_MaxPolygonWidth ) || ( y1 - y0 > c_MaxPolygonHeight ) || ( y2 - y1 > c_MaxPolygonHeight ) )
	{
		// skip drawing polygon
		return;
	}
	
	
	
	/////////////////////////////////////////////////
	// draw top part of triangle
	
	// denominator is negative when x1 is on the left, positive when x1 is on the right
	t0 = y1 - y2;
	t1 = y0 - y2;
	denominator = ( ( x0 - x2 ) * t0 ) - ( ( x1 - x2 ) * t1 );

	
	// get reciprocals
	if ( y1 - y0 ) r10 = ( 1LL << 48 ) / ((s64)( y1 - y0 ));
	if ( y2 - y0 ) r20 = ( 1LL << 48 ) / ((s64)( y2 - y0 ));
	if ( y2 - y1 ) r21 = ( 1LL << 48 ) / ((s64)( y2 - y1 ));

	
	///////////////////////////////////////////
	// start at y0
	//Line = y0;
	
	
	//if ( denominator < 0 )
	//{
		// x1 is on the left and x0 is on the right //
		
		////////////////////////////////////
		// get slopes
		
		if ( y1 - y0 )
		{
			/////////////////////////////////////////////
			// init x on the left and right
			x_left = ( ((s64)x0) << 16 );
			x_right = x_left;
			
			if ( denominator < 0 )
			{
				//dx_left = (((s64)( x1 - x0 )) << 16 ) / ((s64)( y1 - y0 ));
				//dx_right = (((s64)( x2 - x0 )) << 16 ) / ((s64)( y2 - y0 ));
				dx_left = ( ((s64)( x1 - x0 )) * r10 ) >> 32;
				dx_right = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
			}
			else
			{
				dx_right = ( ((s64)( x1 - x0 )) * r10 ) >> 32;
				dx_left = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
			}
		}
		else
		{
			if ( denominator < 0 )
			{
				// change x_left and x_right where y1 is on left
				x_left = ( ((s64)x1) << 16 );
				x_right = ( ((s64)x0) << 16 );
			
				//dx_left = (((s64)( x2 - x1 )) << 16 ) / ((s64)( y2 - y1 ));
				//dx_right = (((s64)( x2 - x0 )) << 16 ) / ((s64)( y2 - y0 ));
				dx_left = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
				dx_right = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
			}
			else
			{
				x_right = ( ((s64)x1) << 16 );
				x_left = ( ((s64)x0) << 16 );
			
				dx_right = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
				dx_left = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
			}
		}
	//}

	/////////////////////////////////////////////////
	// swap left and right if they are reversed
	//if ( ( ( x_right + dx_right ) < ( x_left + dx_left ) ) || ( x_right < x_left ) )
	/*
	if ( denominator > 0 )
	{
		// x1,y1 is on the right //
		
		Swap ( x_left, x_right );
		Swap ( dx_left, dx_right );
	}
	*/
	
	////////////////
	// *** TODO *** at this point area of full triangle can be calculated and the rest of the drawing can be put on another thread *** //
	
	
	
	StartY = y0;
	EndY = y1;

	if ( StartY < ((s32)DrawArea_TopLeftY) )
	{
		
		if ( EndY < ((s32)DrawArea_TopLeftY) )
		{
			Temp = EndY - StartY;
			StartY = EndY;
		}
		else
		{
			Temp = DrawArea_TopLeftY - StartY;
			StartY = DrawArea_TopLeftY;
		}
		
		x_left += dx_left * Temp;
		x_right += dx_right * Temp;
	}
	
	if ( EndY > ((s32)DrawArea_BottomRightY) )
	{
		EndY = DrawArea_BottomRightY + 1;
	}


	if ( EndY > StartY )
	{
	
	//////////////////////////////////////////////
	// draw down to y1
	for ( Line = StartY; Line < EndY; Line++ )
	{
		
		// left point is included if points are equal
		StartX = ( (s64) ( x_left + 0xffffLL ) ) >> 16;
		EndX = ( x_right - 1 ) >> 16;
		
		if ( StartX <= ((s32)DrawArea_BottomRightX) && EndX >= ((s32)DrawArea_TopLeftX) && EndX >= StartX )
		{
			// get the difference between x_left and StartX
			//Temp = ( StartX << 16 ) - ( x_left >> 16 );
		
			if ( StartX < ((s32)DrawArea_TopLeftX) )
			{
				//Temp = DrawArea_TopLeftX - StartX;
				StartX = DrawArea_TopLeftX;
			}
			
			if ( EndX > ((s32)DrawArea_BottomRightX) )
			{
				//EndX = DrawArea_BottomRightX + 1;
				EndX = DrawArea_BottomRightX;
			}
			
			ptr = & ( VRAM [ StartX + ( Line << 10 ) ] );
			
			
			/////////////////////////////////////////////////////
			// update number of cycles used to draw polygon
			NumberOfPixelsDrawn += EndX - StartX + 1;
			
#ifdef _ENABLE_SSE2_TRIANGLE_MONO_NONTEMPLATE
	vx_across = _mm_add_epi16 ( vSeq, _mm_set1_epi16 ( StartX ) );
	vEndX = _mm_set1_epi16 ( EndX + 1 );
#endif

			// draw horizontal line
			// x_left and x_right need to be rounded off
			//for ( x_across = StartX; x_across < EndX; x_across++ )
			for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize )
			{
#ifdef _ENABLE_SSE2_TRIANGLE_MONO_NONTEMPLATE
				DestPixel = _mm_loadu_si128 ((__m128i const*) ptr);
				vbgr_temp = vbgr;
				if ( command_abe ) vbgr_temp = vSemiTransparency16( DestPixel, vbgr_temp, GPU_CTRL_Read.ABR );
				vbgr_temp = _mm_or_si128 ( vbgr_temp, SetPixelMask );
				_mm_maskmoveu_si128 ( vbgr_temp, _mm_andnot_si128 ( _mm_and_si128 ( _mm_srai_epi16 ( DestPixel, 15 ), PixelMask ), _mm_cmplt_epi16 ( vx_across, vEndX ) ), (char*) ptr );
				vx_across = _mm_add_epi16 ( vx_across, vVectorSize );
#else

				// read pixel from frame buffer if we need to check mask bit
				DestPixel = *ptr;
				
				bgr_temp = bgr;
	
				// semi-transparency
				if ( command_abe )
				{
					bgr_temp = SemiTransparency16 ( DestPixel, bgr_temp, GPU_CTRL_Read.ABR );
				}
				
				// check if we should set mask bit when drawing
				bgr_temp |= SetPixelMask;

				// draw pixel if we can draw to mask pixels or mask bit not set
				if ( ! ( DestPixel & PixelMask ) ) *ptr = bgr_temp;
#endif
				
				//ptr++;
				ptr += c_iVectorSize;
			}
			
		}
		
		//////////////////////////////////
		// draw next line
		//Line++;
		
		/////////////////////////////////////
		// update x on left and right
		x_left += dx_left;
		x_right += dx_right;
	}

	} // end if ( EndY > StartY )

	
	////////////////////////////////////////////////
	// draw bottom part of triangle

	/////////////////////////////////////////////
	// init x on the left and right
	
	//////////////////////////////////////////////////////
	// check if y1 is on the left or on the right
	//if ( Y1_OnLeft )
	if ( denominator < 0 )
	{
		// y1 is on the left //
		
		x_left = ( ((s64)x1) << 16 );
		
		//if ( y2 - y1 )
		//{
			//dx_left = (((s64)( x2 - x1 )) << 16 ) / ((s64)( y2 - y1 ));
			dx_left = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
		//}
	}
	else
	{
		// y1 is on the right //
		
		x_right = ( ((s64)x1) << 16 );
		
		//if ( y2 - y1 )
		//{
			//dx_right = (((s64)( x2 - x1 )) << 16 ) / ((s64)( y2 - y1 ));
			dx_right = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
		//}
	}
	
	// the line starts at y1 from here
	//Line = y1;

	StartY = y1;
	EndY = y2;

	if ( StartY < ((s32)DrawArea_TopLeftY) )
	{
		
		if ( EndY < ((s32)DrawArea_TopLeftY) )
		{
			Temp = EndY - StartY;
			StartY = EndY;
		}
		else
		{
			Temp = DrawArea_TopLeftY - StartY;
			StartY = DrawArea_TopLeftY;
		}
		
		x_left += dx_left * Temp;
		x_right += dx_right * Temp;
	}
	
	if ( EndY > ((s32)DrawArea_BottomRightY) )
	{
		EndY = DrawArea_BottomRightY + 1;
	}


	if ( EndY > StartY )
	{
	
	//////////////////////////////////////////////
	// draw down to y2
	//while ( Line < y2 )
	for ( Line = StartY; Line < EndY; Line++ )
	{
		//cy = Line;

		//StartX = _Round( x_left ) >> 32;
		//EndX = _Round( x_right ) >> 32;
		
		// left point is included if points are equal
		StartX = ( x_left + 0xffffLL ) >> 16;
		EndX = ( x_right - 1 ) >> 16;
		
		
		if ( StartX <= ((s32)DrawArea_BottomRightX) && EndX >= ((s32)DrawArea_TopLeftX) && EndX >= StartX )
		{
			if ( StartX < ((s32)DrawArea_TopLeftX) )
			{
				//Temp = DrawArea_TopLeftX - StartX;
				StartX = DrawArea_TopLeftX;
			}
			
			if ( EndX > ((s32)DrawArea_BottomRightX) )
			{
				//EndX = DrawArea_BottomRightX + 1;
				EndX = DrawArea_BottomRightX;
			}
			
			ptr = & ( VRAM [ StartX + ( Line << 10 ) ] );
			
			
			/////////////////////////////////////////////////////
			// update number of cycles used to draw polygon
			NumberOfPixelsDrawn += EndX - StartX + 1;
			
#ifdef _ENABLE_SSE2_TRIANGLE_MONO_NONTEMPLATE
	vx_across = _mm_add_epi16 ( vSeq, _mm_set1_epi16 ( StartX ) );
	vEndX = _mm_set1_epi16 ( EndX + 1 );
#endif

			// draw horizontal line
			// x_left and x_right need to be rounded off
			//for ( x_across = StartX; x_across < EndX; x_across++ )
			for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize )
			{
#ifdef _ENABLE_SSE2_TRIANGLE_MONO_NONTEMPLATE
				DestPixel = _mm_loadu_si128 ((__m128i const*) ptr);
				vbgr_temp = vbgr;
				if ( command_abe ) vbgr_temp = vSemiTransparency16( DestPixel, vbgr_temp, GPU_CTRL_Read.ABR );
				vbgr_temp = _mm_or_si128 ( vbgr_temp, SetPixelMask );
				_mm_maskmoveu_si128 ( vbgr_temp, _mm_andnot_si128 ( _mm_and_si128 ( _mm_srai_epi16 ( DestPixel, 15 ), PixelMask ), _mm_cmplt_epi16 ( vx_across, vEndX ) ), (char*) ptr );
				vx_across = _mm_add_epi16 ( vx_across, vVectorSize );
#else
				// read pixel from frame buffer if we need to check mask bit
				DestPixel = *ptr;
				
				bgr_temp = bgr;
	
				// semi-transparency
				if ( command_abe )
				{
					bgr_temp = SemiTransparency16 ( DestPixel, bgr_temp, GPU_CTRL_Read.ABR );
				}
				
				// check if we should set mask bit when drawing
				bgr_temp |= SetPixelMask;

				// draw pixel if we can draw to mask pixels or mask bit not set
				if ( ! ( DestPixel & PixelMask ) ) *ptr = bgr_temp;
#endif
				
				//ptr++;
				ptr += c_iVectorSize;
			}
			
		}
		
		/////////////////////////////////////
		// update x on left and right
		x_left += dx_left;
		x_right += dx_right;
	}
	
	} // end if ( EndY > StartY )

}

#endif



#ifndef EXCLUDE_TRIANGLE_GRADIENT_NONTEMPLATE

void GPU::DrawTriangle_Gradient ( u32 Coord0, u32 Coord1, u32 Coord2 )
{
#ifdef _ENABLE_SSE2_TRIANGLE_GRADIENT_NONTEMPLATE
	// with sse2, can send 8 pixels at a time
	static const int c_iVectorSize = 8;
#else
	// before sse2, was sending 1 pixels at a time
	static const int c_iVectorSize = 1;
#endif

	//u32 Pixel, TexelIndex,
	
	//u32 Y1_OnLeft;
	
	//u32 color_add;
	
	u16 *ptr;
	
	s64 Temp;
	s64 LeftMostX, RightMostX;
	
	s32 StartX, EndX, StartY, EndY;

	//s64 Error_Left;
	s64 r10, r20, r21;
	
	s64* DitherArray;
	s64* DitherLine;
	s64 DitherValue;

	// new local variables
	s32 x0, x1, x2, y0, y1, y2;
	s64 dx_left, dx_right;
	s64 x_left, x_right, x_across;
	u32 bgr, bgr_temp;
	s32 Line;
	s64 t0, t1, denominator;

	// more local variables for gradient triangle
	s64 dR_left, dG_left, dB_left, dR_across, dG_across, dB_across, iR, iG, iB, R_left, G_left, B_left;
	s32 r0, r1, r2, g0, g1, g2, b0, b1, b2;
	
#ifdef _ENABLE_SSE2_TRIANGLE_GRADIENT_NONTEMPLATE
	s16* vDitherArray_add;
	s16* vDitherArray_sub;
	s16* vDitherLine_add;
	s16* vDitherLine_sub;
	__m128i viR1, viG1, viB1, viR2, viG2, viB2, vRed, vGreen, vBlue, vdR_across, vdG_across, vdB_across, vDitherValue_add, vDitherValue_sub, vTemp;
	
	__m128i DestPixel, PixelMask, SetPixelMask;
	__m128i vbgr, vbgr_temp, vStartX, vEndX, vx_across, vSeq, vVectorSize;
	__m128i vSeq32_1, vSeq32_2;
	
	vSeq32_1 = _mm_set_epi32 ( 3, 2, 1, 0 );
	vSeq32_2 = _mm_set_epi32 ( 7, 6, 5, 4 );

	__m128i vSeq32_dr1, vSeq32_dr2, vSeq32_dg1, vSeq32_dg2, vSeq32_db1, vSeq32_db2;
	
	vSeq = _mm_set_epi16 ( 7, 6, 5, 4, 3, 2, 1, 0 );
	vVectorSize = _mm_set1_epi16 ( c_iVectorSize );
	
	vbgr = _mm_set1_epi16 ( bgr );
	PixelMask = _mm_setzero_si128 ();
	SetPixelMask = _mm_setzero_si128 ();
	if ( GPU_CTRL_Read.ME ) PixelMask = _mm_set1_epi16 ( 0x8080 );
	if ( GPU_CTRL_Read.MD ) SetPixelMask = _mm_set1_epi16 ( 0x8000 );
#else
	
	s64 Red, Green, Blue;
	u32 DestPixel, PixelMask = 0, SetPixelMask = 0;
	
	if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
	if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;
#endif
	
	// initialize number of pixels drawn
	NumberOfPixelsDrawn = 0;
	
	// check for some important conditions
	if ( DrawArea_BottomRightX < DrawArea_TopLeftX )
	{
		//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightX < DrawArea_TopLeftX.\n";
		return;
	}
	
	if ( DrawArea_BottomRightY < DrawArea_TopLeftY )
	{
		//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightY < DrawArea_TopLeftY.\n";
		return;
	}
	
	///////////////////////////////////////////////////
	// Initialize dithering
	
#ifdef _ENABLE_SSE2_TRIANGLE_GRADIENT_NONTEMPLATE
	vDitherArray_add = (s16*) c_iDitherZero;
	vDitherArray_sub = (s16*) c_iDitherZero;
#else
	DitherArray = (s64*) c_iDitherZero;
#endif
	
	if ( GPU_CTRL_Read.DTD )
	{
#ifdef _ENABLE_SSE2_TRIANGLE_GRADIENT_NONTEMPLATE
		vDitherArray_add = c_viDitherValues16_add;
		vDitherArray_sub = c_viDitherValues16_sub;
#else
		//DitherArray = c_iDitherValues;
		DitherArray = (s64*) c_iDitherValues24;
#endif
	}



	// get color(s)
	//bgr = Buffer [ 0 ].Value & 0xffffff;
	
	// ?? convert to 16-bit ?? (or should leave 24-bit?)
	//bgr = ( ((u32)Buffer [ 0 ].Red) >> 3 ) | ( ( ((u32)Buffer [ 0 ].Green) >> 3 ) << 5 ) | ( ( ((u32)Buffer [ 0 ].Blue) >> 3 ) << 10 )
	
	// get y-values
	//y0 = Buffer [ Coord0 ].y;
	//y1 = Buffer [ Coord1 ].y;
	//y2 = Buffer [ Coord2 ].y;
	
	///////////////////////////////////
	// put top coordinates in x0,y0
	//if ( y1 < y0 )
	if ( gy [ Coord1 ] < gy [ Coord0 ] )
	{
		//Swap ( y0, y1 );
		Swap ( Coord0, Coord1 );
	}
	
	//if ( y2 < y0 )
	if ( gy [ Coord2 ] < gy [ Coord0 ] )
	{
		//Swap ( y0, y2 );
		Swap ( Coord0, Coord2 );
	}
	
	///////////////////////////////////////
	// put middle coordinates in x1,y1
	//if ( y2 < y1 )
	if ( gy [ Coord2 ] < gy [ Coord1 ] )
	{
		//Swap ( y1, y2 );
		Swap ( Coord1, Coord2 );
	}
	
	// get x-values
	x0 = gx [ Coord0 ];
	x1 = gx [ Coord1 ];
	x2 = gx [ Coord2 ];
	
	// get y-values
	y0 = gy [ Coord0 ];
	y1 = gy [ Coord1 ];
	y2 = gy [ Coord2 ];

	// get rgb-values
	r0 = gr [ Coord0 ];
	r1 = gr [ Coord1 ];
	r2 = gr [ Coord2 ];
	g0 = gg [ Coord0 ];
	g1 = gg [ Coord1 ];
	g2 = gg [ Coord2 ];
	b0 = gb [ Coord0 ];
	b1 = gb [ Coord1 ];
	b2 = gb [ Coord2 ];

	////////////////////////////////
	// y1 starts on the left
	//Y1_OnLeft = 1;


	
	//////////////////////////////////////////
	// get coordinates on screen
	x0 = DrawArea_OffsetX + x0;
	y0 = DrawArea_OffsetY + y0;
	x1 = DrawArea_OffsetX + x1;
	y1 = DrawArea_OffsetY + y1;
	x2 = DrawArea_OffsetX + x2;
	y2 = DrawArea_OffsetY + y2;
	
	
	
	
	
	// get the left/right most x
	LeftMostX = ( ( x0 < x1 ) ? x0 : x1 );
	LeftMostX = ( ( x2 < LeftMostX ) ? x2 : LeftMostX );
	RightMostX = ( ( x0 > x1 ) ? x0 : x1 );
	RightMostX = ( ( x2 > RightMostX ) ? x2 : RightMostX );

	// check if sprite is within draw area
	if ( RightMostX <= ((s32)DrawArea_TopLeftX) || LeftMostX > ((s32)DrawArea_BottomRightX) || y2 <= ((s32)DrawArea_TopLeftY) || y0 > ((s32)DrawArea_BottomRightY) ) return;
	
	// skip drawing if distance between vertices is greater than max allowed by GPU
	if ( ( _Abs( x1 - x0 ) > c_MaxPolygonWidth ) || ( _Abs( x2 - x1 ) > c_MaxPolygonWidth ) || ( y1 - y0 > c_MaxPolygonHeight ) || ( y2 - y1 > c_MaxPolygonHeight ) )
	{
		// skip drawing polygon
		return;
	}
	
	
	////////////////////////////////////////////////
	// get length of longest scanline
	
	// calculate across
	// denominator is negative when x1 is on the left, positive when x1 is on the right
	t0 = y1 - y2;
	t1 = y0 - y2;
	denominator = ( ( x0 - x2 ) * t0 ) - ( ( x1 - x2 ) * t1 );
	if ( denominator )
	{
		//dR_across = ( ( (s64) ( ( ( r0 - r2 ) * t0 ) - ( ( r1 - r2 ) * t1 ) ) ) << 24 ) / denominator;
		//dG_across = ( ( (s64) ( ( ( g0 - g2 ) * t0 ) - ( ( g1 - g2 ) * t1 ) ) ) << 24 ) / denominator;
		//dB_across = ( ( (s64) ( ( ( b0 - b2 ) * t0 ) - ( ( b1 - b2 ) * t1 ) ) ) << 24 ) / denominator;
		
		denominator = ( 1ll << 48 ) / denominator;
		dR_across = ( ( (s64) ( ( ( r0 - r2 ) * t0 ) - ( ( r1 - r2 ) * t1 ) ) ) * denominator ) >> 24;
		dG_across = ( ( (s64) ( ( ( g0 - g2 ) * t0 ) - ( ( g1 - g2 ) * t1 ) ) ) * denominator ) >> 24;
		dB_across = ( ( (s64) ( ( ( b0 - b2 ) * t0 ) - ( ( b1 - b2 ) * t1 ) ) ) * denominator ) >> 24;
		
#ifdef _ENABLE_SSE2_TRIANGLE_GRADIENT_NONTEMPLATE
		vdR_across = _mm_set1_epi32 ( dR_across * 8 );
		vdG_across = _mm_set1_epi32 ( dG_across * 8 );
		vdB_across = _mm_set1_epi32 ( dB_across * 8 );
#endif
	}
	
	//debug << dec << "\r\nx0=" << x0 << " y0=" << y0 << " x1=" << x1 << " y1=" << y1 << " x2=" << x2 << " y2=" << y2;
	//debug << "\r\nfixed: denominator=" << denominator;
	
	/////////////////////////////////////////////////
	// draw top part of triangle
	

	///////////////////////////////////////////
	// start at y0
	//Line = y0;
	
	/////////////////////////////////////////////
	// init x on the left and right
	
	
	// get reciprocals
	if ( y1 - y0 ) r10 = ( 1LL << 48 ) / ((s64)( y1 - y0 ));
	if ( y2 - y0 ) r20 = ( 1LL << 48 ) / ((s64)( y2 - y0 ));
	if ( y2 - y1 ) r21 = ( 1LL << 48 ) / ((s64)( y2 - y1 ));

	////////////////////////////////////
	// get slopes
	
	if ( y1 - y0 )
	{
		x_left = ( ((s64)x0) << 16 );
		x_right = x_left;
		
		//R_left = ( ((s64)r0) << 32 );
		//G_left = ( ((s64)g0) << 32 );
		//B_left = ( ((s64)b0) << 32 );
		R_left = ( ((s64)r0) << 24 );
		G_left = ( ((s64)g0) << 24 );
		B_left = ( ((s64)b0) << 24 );
		//R_right = R_left;
		//G_right = G_left;
		//B_right = B_left;
		
		if ( denominator < 0 )
		{
			//dx_left = (((s64)( x1 - x0 )) << 16 ) / ((s64)( y1 - y0 ));
			//dx_right = (((s64)( x2 - x0 )) << 16 ) / ((s64)( y2 - y0 ));
			dx_left = ( ((s64)( x1 - x0 )) * r10 ) >> 32;
			dx_right = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
			
			//dR_left = (((s64)( r1 - r0 )) << 24 ) / ((s64)( y1 - y0 ));
			//dG_left = (((s64)( g1 - g0 )) << 24 ) / ((s64)( y1 - y0 ));
			//dB_left = (((s64)( b1 - b0 )) << 24 ) / ((s64)( y1 - y0 ));
			dR_left = ( ((s64)( r1 - r0 )) * r10 ) >> 24;
			dG_left = ( ((s64)( g1 - g0 )) * r10 ) >> 24;
			dB_left = ( ((s64)( b1 - b0 )) * r10 ) >> 24;
		}
		else
		{
			dx_right = ( ((s64)( x1 - x0 )) * r10 ) >> 32;
			dx_left = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
			
			//dR_right = (((s64)( r2 - r0 )) << 24 ) / ((s64)( y2 - y0 ));
			//dG_right = (((s64)( g2 - g0 )) << 24 ) / ((s64)( y2 - y0 ));
			//dB_right = (((s64)( b2 - b0 )) << 24 ) / ((s64)( y2 - y0 ));
			dR_left = ( ((s64)( r2 - r0 )) * r20 ) >> 24;
			dG_left = ( ((s64)( g2 - g0 )) * r20 ) >> 24;
			dB_left = ( ((s64)( b2 - b0 )) * r20 ) >> 24;
		}
		
	}
	else
	{
		if ( denominator < 0 )
		{
			// change x_left and x_right where y1 is on left
			x_left = ( ((s64)x1) << 16 );
			x_right = ( ((s64)x0) << 16 );
			
			//dx_left = (((s64)( x2 - x1 )) << 16 ) / ((s64)( y2 - y1 ));
			//dx_right = (((s64)( x2 - x0 )) << 16 ) / ((s64)( y2 - y0 ));
			dx_left = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
			dx_right = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
			
			R_left = ( ((s64)r1) << 24 );
			G_left = ( ((s64)g1) << 24 );
			B_left = ( ((s64)b1) << 24 );

			//dR_left = (((s64)( r2 - r1 )) << 24 ) / ((s64)( y2 - y1 ));
			//dG_left = (((s64)( g2 - g1 )) << 24 ) / ((s64)( y2 - y1 ));
			//dB_left = (((s64)( b2 - b1 )) << 24 ) / ((s64)( y2 - y1 ));
			dR_left = ( ((s64)( r2 - r1 )) * r21 ) >> 24;
			dG_left = ( ((s64)( g2 - g1 )) * r21 ) >> 24;
			dB_left = ( ((s64)( b2 - b1 )) * r21 ) >> 24;
		}
		else
		{
			x_right = ( ((s64)x1) << 16 );
			x_left = ( ((s64)x0) << 16 );
			
			dx_right = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
			dx_left = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
			
			R_left = ( ((s64)r0) << 24 );
			G_left = ( ((s64)g0) << 24 );
			B_left = ( ((s64)b0) << 24 );
			
			//dR_right = (((s64)( r2 - r0 )) << 24 ) / ((s64)( y2 - y0 ));
			//dG_right = (((s64)( g2 - g0 )) << 24 ) / ((s64)( y2 - y0 ));
			//dB_right = (((s64)( b2 - b0 )) << 24 ) / ((s64)( y2 - y0 ));
			dR_left = ( ((s64)( r2 - r0 )) * r20 ) >> 24;
			dG_left = ( ((s64)( g2 - g0 )) * r20 ) >> 24;
			dB_left = ( ((s64)( b2 - b0 )) * r20 ) >> 24;
		}
	}

	/////////////////////////////////////////////////
	// swap left and right if they are reversed
	//if ( ( ( x_right + dx_right ) < ( x_left + dx_left ) ) || ( x_right < x_left ) )
	/*
	if ( denominator > 0 )
	{
		// x1,y1 is on the right //
		
		//Y1_OnLeft = 0;
		
		Swap ( x_left, x_right );
		Swap ( dx_left, dx_right );

		Swap ( dR_left, dR_right );
		Swap ( dG_left, dG_right );
		Swap ( dB_left, dB_right );

		Swap ( R_left, R_right );
		Swap ( G_left, G_right );
		Swap ( B_left, B_right );
	}
	*/
	
	
	// r,g,b values are not specified with a fractional part, so there must be an initial fractional part
	R_left |= ( 1 << 23 );
	G_left |= ( 1 << 23 );
	B_left |= ( 1 << 23 );
	
	
	StartY = y0;
	EndY = y1;

	if ( StartY < ((s32)DrawArea_TopLeftY) )
	{
		
		if ( EndY < ((s32)DrawArea_TopLeftY) )
		{
			Temp = EndY - StartY;
			StartY = EndY;
		}
		else
		{
			Temp = DrawArea_TopLeftY - StartY;
			StartY = DrawArea_TopLeftY;
		}
		
		x_left += dx_left * Temp;
		x_right += dx_right * Temp;
		
		R_left += dR_left * Temp;
		G_left += dG_left * Temp;
		B_left += dB_left * Temp;
		
		//R_right += dR_right * Temp;
		//G_right += dG_right * Temp;
		//B_right += dB_right * Temp;
	}
	
	if ( EndY > ((s32)DrawArea_BottomRightY) )
	{
		EndY = DrawArea_BottomRightY + 1;
	}

#ifdef _ENABLE_SSE2_TRIANGLE_GRADIENT_NONTEMPLATE
	// setup the values to add going across
	vSeq32_dr1 = _custom_mul_32 ( _mm_set1_epi32 ( dR_across ), vSeq32_1 );
	vSeq32_dr2 = _custom_mul_32 ( _mm_set1_epi32 ( dR_across ), vSeq32_2 );
	vSeq32_dg1 = _custom_mul_32 ( _mm_set1_epi32 ( dG_across ), vSeq32_1 );
	vSeq32_dg2 = _custom_mul_32 ( _mm_set1_epi32 ( dG_across ), vSeq32_2 );
	vSeq32_db1 = _custom_mul_32 ( _mm_set1_epi32 ( dB_across ), vSeq32_1 );
	vSeq32_db2 = _custom_mul_32 ( _mm_set1_epi32 ( dB_across ), vSeq32_2 );
#endif

	if ( EndY > StartY )
	{
	
	//////////////////////////////////////////////
	// draw down to y1
	for ( Line = StartY; Line < EndY; Line++ )
	{
		//StartX = _Round( x_left ) >> 32;
		//EndX = _Round( x_right ) >> 32;
		
		// left point is included if points are equal
		StartX = ( x_left + 0xffffLL ) >> 16;
		EndX = ( x_right - 1 ) >> 16;
		
		
		if ( StartX <= ((s32)DrawArea_BottomRightX) && EndX >= ((s32)DrawArea_TopLeftX) && EndX >= StartX )
		{
			
			iR = R_left;
			iG = G_left;
			iB = B_left;
			
			// get the difference between x_left and StartX
			Temp = ( StartX << 16 ) - x_left;
			
			if ( StartX < ((s32)DrawArea_TopLeftX) )
			{
				Temp += ( DrawArea_TopLeftX - StartX ) << 16;
				StartX = DrawArea_TopLeftX;
				
				//iR += dR_across * Temp;
				//iG += dG_across * Temp;
				//iB += dB_across * Temp;
			}
			
			iR += ( dR_across >> 12 ) * ( Temp >> 4 );
			iG += ( dG_across >> 12 ) * ( Temp >> 4 );
			iB += ( dB_across >> 12 ) * ( Temp >> 4 );
			
			if ( EndX > ((s32)DrawArea_BottomRightX) )
			{
				//EndX = DrawArea_BottomRightX + 1;
				EndX = DrawArea_BottomRightX;
			}
			
			ptr = & ( VRAM [ StartX + ( Line << 10 ) ] );
			DitherLine = & ( DitherArray [ ( Line & 0x3 ) << 2 ] );
			
			
			/////////////////////////////////////////////////////
			// update number of cycles used to draw polygon
			NumberOfPixelsDrawn += EndX - StartX + 1;
			
#ifdef _ENABLE_SSE2_TRIANGLE_GRADIENT_NONTEMPLATE
	vx_across = _mm_add_epi16 ( vSeq, _mm_set1_epi16 ( StartX ) );
	vEndX = _mm_set1_epi16 ( EndX + 1 );
	
	//viR1 = _mm_add_epi32 ( _mm_set1_epi32 ( iR ), _custom_mul_32 ( _mm_set1_epi32 ( dR_across ), vSeq32_1 ) );
	//viR2 = _mm_add_epi32 ( _mm_set1_epi32 ( iR ), _custom_mul_32 ( _mm_set1_epi32 ( dR_across ), vSeq32_2 ) );
	//viG1 = _mm_add_epi32 ( _mm_set1_epi32 ( iG ), _custom_mul_32 ( _mm_set1_epi32 ( dG_across ), vSeq32_1 ) );
	//viG2 = _mm_add_epi32 ( _mm_set1_epi32 ( iG ), _custom_mul_32 ( _mm_set1_epi32 ( dG_across ), vSeq32_2 ) );
	//viB1 = _mm_add_epi32 ( _mm_set1_epi32 ( iB ), _custom_mul_32 ( _mm_set1_epi32 ( dB_across ), vSeq32_1 ) );
	//viB2 = _mm_add_epi32 ( _mm_set1_epi32 ( iB ), _custom_mul_32 ( _mm_set1_epi32 ( dB_across ), vSeq32_2 ) );
	
	viR1 = _mm_add_epi32 ( _mm_set1_epi32 ( iR ), vSeq32_dr1 );
	viR2 = _mm_add_epi32 ( _mm_set1_epi32 ( iR ), vSeq32_dr2 );
	viG1 = _mm_add_epi32 ( _mm_set1_epi32 ( iG ), vSeq32_dg1 );
	viG2 = _mm_add_epi32 ( _mm_set1_epi32 ( iG ), vSeq32_dg2 );
	viB1 = _mm_add_epi32 ( _mm_set1_epi32 ( iB ), vSeq32_db1 );
	viB2 = _mm_add_epi32 ( _mm_set1_epi32 ( iB ), vSeq32_db2 );
	
	vDitherValue_add = _mm_loadu_si128 ((__m128i const*) ( &(vDitherArray_add [ ( StartX & 0x3 ) + ( ( Line & 0x3 ) << 4 ) ]) ));
	vDitherValue_sub = _mm_loadu_si128 ((__m128i const*) ( &(vDitherArray_sub [ ( StartX & 0x3 ) + ( ( Line & 0x3 ) << 4 ) ]) ));
#endif

//#define TEST10

//#ifdef TEST10
//	debug << hex << "\r\nr0=" << r0 << " r1=" << r1 << " r2=" << r2 << " g0=" << g0 << " g1=" << g1 << " g2=" << g2;
//	debug << " dR_left=" << dR_left << " dR_right=" << dR_right << " dG_left=" << dG_left << " dG_right=" << dG_right << " dB_left=" << dB_left << " dB_right=" << dB_right;
//	debug << hex << "\r\niR=" << iR << " iG=" << iG << " iB=" << iB << " dR_across=" << dR_across << " dG_across=" << dG_across << " dB_across=" << dB_across;
//	debug << " R_left=" << R_left << " R_right=" << R_right << " G_left=" << G_left << " G_right=" << G_right << " B_left=" << B_left << " B_right=" << B_right;
//#endif

			// draw horizontal line
			// x_left and x_right need to be rounded off
			//for ( x_across = StartX; x_across < EndX; x_across++ )
			for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize )
			{
#ifdef _ENABLE_SSE2_TRIANGLE_GRADIENT_NONTEMPLATE
					vRed = _mm_packs_epi32 ( _mm_srai_epi32 ( viR1, 16 ), _mm_srai_epi32 ( viR2, 16 ) );
					vRed = _mm_adds_epu16 ( vRed, vDitherValue_add );
					vRed = _mm_subs_epu16 ( vRed, vDitherValue_sub );
					
					vGreen = _mm_packs_epi32 ( _mm_srai_epi32 ( viG1, 16 ), _mm_srai_epi32 ( viG2, 16 ) );
					vGreen = _mm_adds_epu16 ( vGreen, vDitherValue_add );
					vGreen = _mm_subs_epu16 ( vGreen, vDitherValue_sub );
					
					vBlue = _mm_packs_epi32 ( _mm_srai_epi32 ( viB1, 16 ), _mm_srai_epi32 ( viB2, 16 ) );
					vBlue = _mm_adds_epu16 ( vBlue, vDitherValue_add );
					vBlue = _mm_subs_epu16 ( vBlue, vDitherValue_sub );
					
					vRed = _mm_srli_epi16 ( vRed, 11 );
					vGreen = _mm_srli_epi16 ( vGreen, 11 );
					vBlue = _mm_srli_epi16 ( vBlue, 11 );
					
					//vRed = _mm_slli_epi16 ( vRed, 0 );
					vGreen = _mm_slli_epi16 ( vGreen, 5 );
					vBlue = _mm_slli_epi16 ( vBlue, 10 );
					
					vbgr_temp = _mm_or_si128 ( _mm_or_si128 ( vRed, vGreen ), vBlue );
					
					DestPixel = _mm_loadu_si128 ((__m128i const*) ptr);
					//vbgr_temp = vbgr;
					if ( command_abe ) vbgr_temp = vSemiTransparency16( DestPixel, vbgr_temp, GPU_CTRL_Read.ABR );
					vbgr_temp = _mm_or_si128 ( vbgr_temp, SetPixelMask );
					_mm_maskmoveu_si128 ( vbgr_temp, _mm_andnot_si128 ( _mm_and_si128 ( _mm_srai_epi16 ( DestPixel, 15 ), PixelMask ), _mm_cmplt_epi16 ( vx_across, vEndX ) ), (char*) ptr );
					
					
					vx_across = _mm_add_epi16 ( vx_across, vVectorSize );
					
					viR1 = _mm_add_epi32 ( viR1, vdR_across );
					viR2 = _mm_add_epi32 ( viR2, vdR_across );
					viG1 = _mm_add_epi32 ( viG1, vdG_across );
					viG2 = _mm_add_epi32 ( viG2, vdG_across );
					viB1 = _mm_add_epi32 ( viB1, vdB_across );
					viB2 = _mm_add_epi32 ( viB2, vdB_across );
#else
					//bgr = ( _Round( iR ) >> 32 ) | ( ( _Round( iG ) >> 32 ) << 8 ) | ( ( _Round( iB ) >> 32 ) << 16 );
					//bgr = ( _Round( iR ) >> 35 ) | ( ( _Round( iG ) >> 35 ) << 5 ) | ( ( _Round( iB ) >> 35 ) << 10 );
					DitherValue = DitherLine [ x_across & 0x3 ];
					
					// perform dither
					Red = iR + DitherValue;
					Green = iG + DitherValue;
					Blue = iB + DitherValue;
					
					//Red = Clamp5 ( ( iR + DitherValue ) >> 27 );
					//Green = Clamp5 ( ( iG + DitherValue ) >> 27 );
					//Blue = Clamp5 ( ( iB + DitherValue ) >> 27 );
					
					// perform shift
					Red >>= 27;
					Green >>= 27;
					Blue >>= 27;
					
					// if dithering, perform signed clamp to 5 bits
					Red = AddSignedClamp<s64,5> ( Red );
					Green = AddSignedClamp<s64,5> ( Green );
					Blue = AddSignedClamp<s64,5> ( Blue );
					
					bgr = ( Blue << 10 ) | ( Green << 5 ) | Red;
					
					// shade pixel color
				
					// read pixel from frame buffer if we need to check mask bit
					DestPixel = *ptr;
					
					// *** testing ***
					//debug << "\r\nDestPixel=" << hex << DestPixel;
					
					bgr_temp = bgr;
		
					// *** testing ***
					//debug << " (before)bgr_temp=" << hex << bgr_temp;
					
					// semi-transparency
					if ( command_abe )
					{
						bgr_temp = SemiTransparency16 ( DestPixel, bgr_temp, GPU_CTRL_Read.ABR );
					}
					
					// check if we should set mask bit when drawing
					bgr_temp |= SetPixelMask;

					// *** testing ***
					//debug << " (before)bgr_temp=" << hex << bgr_temp;
					//debug << " SetPixelMask=" << SetPixelMask << " PixelMask=" << PixelMask << " DestPixel=" << DestPixel;
					
					// draw pixel if we can draw to mask pixels or mask bit not set
					if ( ! ( DestPixel & PixelMask ) ) *ptr = bgr_temp;
						
					iR += dR_across;
					iG += dG_across;
					iB += dB_across;
#endif
				
				//ptr++;
				ptr += c_iVectorSize;
			}
			
		}
		
		//////////////////////////////////
		// draw next line
		//Line++;
		
		/////////////////////////////////////
		// update x on left and right
		x_left += dx_left;
		x_right += dx_right;
		
		R_left += dR_left;
		G_left += dG_left;
		B_left += dB_left;
		//R_right += dR_right;
		//G_right += dG_right;
		//B_right += dB_right;
	}

	} // end if ( EndY > StartY )
	
	
	////////////////////////////////////////////////
	// draw bottom part of triangle

	/////////////////////////////////////////////
	// init x on the left and right
	
	//////////////////////////////////////////////////////
	// check if y1 is on the left or on the right
	//if ( Y1_OnLeft )
	if ( denominator < 0 )
	{
		x_left = ( ((s64)x1) << 16 );

		R_left = ( ((s64)r1) << 24 );
		G_left = ( ((s64)g1) << 24 );
		B_left = ( ((s64)b1) << 24 );
		
		// r,g,b values are not specified with a fractional part, so there must be an initial fractional part
		R_left |= ( 1 << 23 );
		G_left |= ( 1 << 23 );
		B_left |= ( 1 << 23 );
		
		//if ( y2 - y1 )
		//{
			//dx_left = (((s64)( x2 - x1 )) << 16 ) / ((s64)( y2 - y1 ));
			dx_left = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
			
			//dR_left = (((s64)( r2 - r1 )) << 24 ) / ((s64)( y2 - y1 ));
			//dG_left = (((s64)( g2 - g1 )) << 24 ) / ((s64)( y2 - y1 ));
			//dB_left = (((s64)( b2 - b1 )) << 24 ) / ((s64)( y2 - y1 ));
			dR_left = ( ((s64)( r2 - r1 )) * r21 ) >> 24;
			dG_left = ( ((s64)( g2 - g1 )) * r21 ) >> 24;
			dB_left = ( ((s64)( b2 - b1 )) * r21 ) >> 24;
		//}
	}
	else
	{
		x_right = ( ((s64)x1) << 16 );

		//R_right = ( ((s64)r1) << 24 );
		//G_right = ( ((s64)g1) << 24 );
		//B_right = ( ((s64)b1) << 24 );
		
		//if ( y2 - y1 )
		//{
			//dx_right = (((s64)( x2 - x1 )) << 16 ) / ((s64)( y2 - y1 ));
			dx_right = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
			
			//dR_right = ( ((s64)( r2 - r1 )) * r21 ) >> 24;
			//dG_right = ( ((s64)( g2 - g1 )) * r21 ) >> 24;
			//dB_right = ( ((s64)( b2 - b1 )) * r21 ) >> 24;
		//}
	}
	
	// *** testing ***
	//debug << "\r\nfixed: dR_across=" << dR_across << " dG_across=" << dG_across << " dB_across=" << dB_across;
	
	// the line starts at y1 from here
	//Line = y1;

	StartY = y1;
	EndY = y2;

	if ( StartY < ((s32)DrawArea_TopLeftY) )
	{
		
		if ( EndY < ((s32)DrawArea_TopLeftY) )
		{
			Temp = EndY - StartY;
			StartY = EndY;
		}
		else
		{
			Temp = DrawArea_TopLeftY - StartY;
			StartY = DrawArea_TopLeftY;
		}
		
		x_left += dx_left * Temp;
		x_right += dx_right * Temp;
		
		R_left += dR_left * Temp;
		G_left += dG_left * Temp;
		B_left += dB_left * Temp;
		
		//R_right += dR_right * Temp;
		//G_right += dG_right * Temp;
		//B_right += dB_right * Temp;
	}
	
	if ( EndY > ((s32)DrawArea_BottomRightY) )
	{
		EndY = DrawArea_BottomRightY + 1;
	}


	if ( EndY > StartY )
	{
	
	//////////////////////////////////////////////
	// draw down to y2
	for ( Line = StartY; Line < EndY; Line++ )
	{
		//StartX = _Round( x_left ) >> 32;
		//EndX = _Round( x_right ) >> 32;
		
		// left point is included if points are equal
		StartX = ( x_left + 0xffffLL ) >> 16;
		EndX = ( x_right - 1 ) >> 16;
		
		
		if ( StartX <= ((s32)DrawArea_BottomRightX) && EndX >= ((s32)DrawArea_TopLeftX) && EndX >= StartX )
		{
			iR = R_left;
			iG = G_left;
			iB = B_left;
			
			// get the difference between x_left and StartX
			Temp = ( StartX << 16 ) - x_left;
			
			if ( StartX < ((s32)DrawArea_TopLeftX) )
			{
				Temp += ( DrawArea_TopLeftX - StartX ) << 16;
				StartX = DrawArea_TopLeftX;
				
				//iR += dR_across * Temp;
				//iG += dG_across * Temp;
				//iB += dB_across * Temp;
			}
			
			iR += ( dR_across >> 12 ) * ( Temp >> 4 );
			iG += ( dG_across >> 12 ) * ( Temp >> 4 );
			iB += ( dB_across >> 12 ) * ( Temp >> 4 );
			
			if ( EndX > ((s32)DrawArea_BottomRightX) )
			{
				//EndX = DrawArea_BottomRightX + 1;
				EndX = DrawArea_BottomRightX;
			}
			
			ptr = & ( VRAM [ StartX + ( Line << 10 ) ] );
			DitherLine = & ( DitherArray [ ( Line & 0x3 ) << 2 ] );
			
			
			/////////////////////////////////////////////////////
			// update number of cycles used to draw polygon
			NumberOfPixelsDrawn += EndX - StartX + 1;
			
#ifdef _ENABLE_SSE2_TRIANGLE_GRADIENT_NONTEMPLATE
	vx_across = _mm_add_epi16 ( vSeq, _mm_set1_epi16 ( StartX ) );
	vEndX = _mm_set1_epi16 ( EndX + 1 );
	
	//viR1 = _mm_add_epi32 ( _mm_set1_epi32 ( iR ), _custom_mul_32 ( _mm_set1_epi32 ( dR_across ), vSeq32_1 ) );
	//viR2 = _mm_add_epi32 ( _mm_set1_epi32 ( iR ), _custom_mul_32 ( _mm_set1_epi32 ( dR_across ), vSeq32_2 ) );
	//viG1 = _mm_add_epi32 ( _mm_set1_epi32 ( iG ), _custom_mul_32 ( _mm_set1_epi32 ( dG_across ), vSeq32_1 ) );
	//viG2 = _mm_add_epi32 ( _mm_set1_epi32 ( iG ), _custom_mul_32 ( _mm_set1_epi32 ( dG_across ), vSeq32_2 ) );
	//viB1 = _mm_add_epi32 ( _mm_set1_epi32 ( iB ), _custom_mul_32 ( _mm_set1_epi32 ( dB_across ), vSeq32_1 ) );
	//viB2 = _mm_add_epi32 ( _mm_set1_epi32 ( iB ), _custom_mul_32 ( _mm_set1_epi32 ( dB_across ), vSeq32_2 ) );
	
	viR1 = _mm_add_epi32 ( _mm_set1_epi32 ( iR ), vSeq32_dr1 );
	viR2 = _mm_add_epi32 ( _mm_set1_epi32 ( iR ), vSeq32_dr2 );
	viG1 = _mm_add_epi32 ( _mm_set1_epi32 ( iG ), vSeq32_dg1 );
	viG2 = _mm_add_epi32 ( _mm_set1_epi32 ( iG ), vSeq32_dg2 );
	viB1 = _mm_add_epi32 ( _mm_set1_epi32 ( iB ), vSeq32_db1 );
	viB2 = _mm_add_epi32 ( _mm_set1_epi32 ( iB ), vSeq32_db2 );
	
	vDitherValue_add = _mm_loadu_si128 ((__m128i const*) ( &(vDitherArray_add [ ( StartX & 0x3 ) + ( ( Line & 0x3 ) << 4 ) ]) ));
	vDitherValue_sub = _mm_loadu_si128 ((__m128i const*) ( &(vDitherArray_sub [ ( StartX & 0x3 ) + ( ( Line & 0x3 ) << 4 ) ]) ));
#endif

			// draw horizontal line
			// x_left and x_right need to be rounded off
			//for ( x_across = StartX; x_across < EndX; x_across++ )
			for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize )
			{
#ifdef _ENABLE_SSE2_TRIANGLE_GRADIENT_NONTEMPLATE
					vRed = _mm_packs_epi32 ( _mm_srai_epi32 ( viR1, 16 ), _mm_srai_epi32 ( viR2, 16 ) );
					vRed = _mm_adds_epu16 ( vRed, vDitherValue_add );
					vRed = _mm_subs_epu16 ( vRed, vDitherValue_sub );
					
					vGreen = _mm_packs_epi32 ( _mm_srai_epi32 ( viG1, 16 ), _mm_srai_epi32 ( viG2, 16 ) );
					vGreen = _mm_adds_epu16 ( vGreen, vDitherValue_add );
					vGreen = _mm_subs_epu16 ( vGreen, vDitherValue_sub );
					
					vBlue = _mm_packs_epi32 ( _mm_srai_epi32 ( viB1, 16 ), _mm_srai_epi32 ( viB2, 16 ) );
					vBlue = _mm_adds_epu16 ( vBlue, vDitherValue_add );
					vBlue = _mm_subs_epu16 ( vBlue, vDitherValue_sub );
					
					vRed = _mm_srli_epi16 ( vRed, 11 );
					vGreen = _mm_srli_epi16 ( vGreen, 11 );
					vBlue = _mm_srli_epi16 ( vBlue, 11 );
					
					//vRed = _mm_slli_epi16 ( vRed, 0 );
					vGreen = _mm_slli_epi16 ( vGreen, 5 );
					vBlue = _mm_slli_epi16 ( vBlue, 10 );
					
					vbgr_temp = _mm_or_si128 ( _mm_or_si128 ( vRed, vGreen ), vBlue );
					
					DestPixel = _mm_loadu_si128 ((__m128i const*) ptr);
					//vbgr_temp = vbgr;
					if ( command_abe ) vbgr_temp = vSemiTransparency16( DestPixel, vbgr_temp, GPU_CTRL_Read.ABR );
					vbgr_temp = _mm_or_si128 ( vbgr_temp, SetPixelMask );
					_mm_maskmoveu_si128 ( vbgr_temp, _mm_andnot_si128 ( _mm_and_si128 ( _mm_srai_epi16 ( DestPixel, 15 ), PixelMask ), _mm_cmplt_epi16 ( vx_across, vEndX ) ), (char*) ptr );
					
					
					vx_across = _mm_add_epi16 ( vx_across, vVectorSize );
					
					viR1 = _mm_add_epi32 ( viR1, vdR_across );
					viR2 = _mm_add_epi32 ( viR2, vdR_across );
					viG1 = _mm_add_epi32 ( viG1, vdG_across );
					viG2 = _mm_add_epi32 ( viG2, vdG_across );
					viB1 = _mm_add_epi32 ( viB1, vdB_across );
					viB2 = _mm_add_epi32 ( viB2, vdB_across );
#else
					//bgr = ( _Round( iR ) >> 32 ) | ( ( _Round( iG ) >> 32 ) << 8 ) | ( ( _Round( iB ) >> 32 ) << 16 );
					//bgr = ( _Round( iR ) >> 35 ) | ( ( _Round( iG ) >> 35 ) << 5 ) | ( ( _Round( iB ) >> 35 ) << 10 );
					DitherValue = DitherLine [ x_across & 0x3 ];
					//bgr = ( ( iR + DitherValue ) >> 35 ) | ( ( ( iG + DitherValue ) >> 35 ) << 5 ) | ( ( ( iB + DitherValue ) >> 35 ) << 10 );

					// perform dither
					Red = iR + DitherValue;
					Green = iG + DitherValue;
					Blue = iB + DitherValue;
					
					//Red = Clamp5 ( ( iR + DitherValue ) >> 27 );
					//Green = Clamp5 ( ( iG + DitherValue ) >> 27 );
					//Blue = Clamp5 ( ( iB + DitherValue ) >> 27 );
					
					// perform shift
					Red >>= 27;
					Green >>= 27;
					Blue >>= 27;
					
					// if dithering, perform signed clamp to 5 bits
					Red = AddSignedClamp<s64,5> ( Red );
					Green = AddSignedClamp<s64,5> ( Green );
					Blue = AddSignedClamp<s64,5> ( Blue );
					
					bgr = ( Blue << 10 ) | ( Green << 5 ) | Red;
					
					// shade pixel color
				
					// read pixel from frame buffer if we need to check mask bit
					DestPixel = *ptr;
					
					bgr_temp = bgr;
		
					// semi-transparency
					if ( command_abe )
					{
						bgr_temp = SemiTransparency16 ( DestPixel, bgr_temp, GPU_CTRL_Read.ABR );
					}
					
					// check if we should set mask bit when drawing
					bgr_temp |= SetPixelMask;

					// draw pixel if we can draw to mask pixels or mask bit not set
					if ( ! ( DestPixel & PixelMask ) ) *ptr = bgr_temp;

					
				iR += dR_across;
				iG += dG_across;
				iB += dB_across;
#endif
				
				//ptr++;
				ptr += c_iVectorSize;
			}
			
		}
		
		/////////////////////////////////////
		// update x on left and right
		x_left += dx_left;
		x_right += dx_right;
		
		R_left += dR_left;
		G_left += dG_left;
		B_left += dB_left;
		//R_right += dR_right;
		//G_right += dG_right;
		//B_right += dB_right;
	}
	
	} // end if ( EndY > StartY )
		
}

#endif


#ifndef EXCLUDE_TRIANGLE_TEXTURE_NONTEMPLATE

// draw texture mapped triangle
void GPU::DrawTriangle_Texture ( u32 Coord0, u32 Coord1, u32 Coord2 )
{
#ifdef _ENABLE_SSE2_TRIANGLE_TEXTURE_NONTEMPLATE
	// with sse2, can send 8 pixels at a time
	static const int c_iVectorSize = 8;
#else
	// before sse2, was sending 1 pixels at a time
	static const int c_iVectorSize = 1;
#endif

	u32 clut_xoffset;

	u32 Pixel, TexelIndex, Y1_OnLeft;
	
	u32 color_add;
	
	u16 *ptr_texture, *ptr_clut;
	u16 *ptr;
	
	s32 StartX, EndX, StartY, EndY;
	s64 Temp, TMin, TMax;
	s64 LeftMostX, RightMostX;
	s64 r10, r20, r21;
	
	u32 uTemp32, uIndex32;

	// new local variables
	s32 x0, x1, x2, y0, y1, y2;
	s64 dx_left, dx_right;
	s64 x_left, x_right, x_across;
	u32 bgr, bgr_temp;
	s32 Line;
	s64 t0, t1, denominator;

	// new local variables for texture mapping
	s64 dU_left, dV_left, dU_across, dV_across, U_left, V_left, iU, iV;
	s32 u0, v0, u1, v1, u2, v2;
	//u32 clut_x, clut_y, tpage_tx, tpage_ty, tpage_abr, tpage_tp;
	//u32 ClutOffset;

#ifdef _ENABLE_SSE2_TRIANGLE_TEXTURE_NONTEMPLATE
	__m128i DestPixel, PixelMask, SetPixelMask;
	__m128i vbgr, vbgr_temp, vStartX, vEndX, vx_across, vSeq, vVectorSize;
	__m128i vbgr_temp_transparent, vbgr_select;
	
	__m128i vSeq32_1, vSeq32_2;
	__m128i vSeq32_u1, vSeq32_u2, vSeq32_v1, vSeq32_v2;
	
	__m128i TexCoordX, TexCoordY, Mask, tMask;	//, vTexCoordX2, vTexCoordY1, vTexCoordY2;
	u32 And2 = 0;
	__m128i And1, Shift1, Shift2;
	__m128i vIndex1, vIndex2;
	__m128i viU, viU1, viU2, viV, viV1, viV2;
	__m128i vdV_across, vdU_across;
	
	//u32 TWYTWH, Not_TWH;
	__m128i TWXTWW, Not_TWW, TWYTWH, Not_TWH;
	__m128i color_add_r, color_add_g, color_add_b;
	__m128i vRound24;
	
	vRound24 = _mm_set1_epi32 ( 0x00800000 );
	
	color_add_r = _mm_set1_epi16 ( bgr & 0xff );
	color_add_g = _mm_set1_epi16 ( ( bgr >> 8 ) & 0xff );
	color_add_b = _mm_set1_epi16 ( ( bgr >> 16 ) & 0xff );
	
	Mask = _mm_set1_epi16 ( 0xff );
	tMask = _mm_set1_epi16 ( 0x8000 );
	
	vSeq32_1 = _mm_set_epi32 ( 3, 2, 1, 0 );
	vSeq32_2 = _mm_set_epi32 ( 7, 6, 5, 4 );
	
	TWYTWH = _mm_set1_epi16 ( ( ( TWY & TWH ) << 3 ) );
	TWXTWW = _mm_set1_epi16 ( ( ( TWX & TWW ) << 3 ) );
	
	Not_TWH = _mm_set1_epi16 ( ~( TWH << 3 ) );
	Not_TWW = _mm_set1_epi16 ( ~( TWW << 3 ) );
	
	And1 = _mm_setzero_si128 ();
	Shift1 = _mm_setzero_si128 ();
	
	//TextureOffset = _mm_set1_epi32 ( ( tpage_tx << 6 ) + ( ( tpage_ty << 8 ) << 10 ) );
	
	vSeq = _mm_set_epi16 ( 7, 6, 5, 4, 3, 2, 1, 0 );
	vVectorSize = _mm_set1_epi16 ( c_iVectorSize );
	
	vbgr = _mm_set1_epi16 ( bgr );
	PixelMask = _mm_setzero_si128 ();
	SetPixelMask = _mm_setzero_si128 ();
	if ( GPU_CTRL_Read.ME ) PixelMask = _mm_set1_epi16 ( 0x8080 );
	if ( GPU_CTRL_Read.MD ) SetPixelMask = _mm_set1_epi16 ( 0x8000 );
	
	u16 TexCoordXList [ 8 ] __attribute__ ((aligned (32)));
	u16 TexCoordYList [ 8 ] __attribute__ ((aligned (32)));
	u16 TempList [ 8 ] __attribute__ ((aligned (32)));
#else

	u32 DestPixel, PixelMask = 0, SetPixelMask = 0;
	u32 TexCoordX, TexCoordY;
	u32 Shift1 = 0, Shift2 = 0, And1 = 0, And2 = 0;
	

	//s64 Error_Left;
	
	s64 TexOffset_X, TexOffset_Y;
	
	u32 TWYTWH, TWXTWW, Not_TWH, Not_TWW;
	
	TWYTWH = ( ( TWY & TWH ) << 3 );
	TWXTWW = ( ( TWX & TWW ) << 3 );
	
	Not_TWH = ~( TWH << 3 );
	Not_TWW = ~( TWW << 3 );
	
	if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
	if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;
#endif
	
	// initialize number of pixels drawn
	NumberOfPixelsDrawn = 0;


	
	// check for some important conditions
	if ( DrawArea_BottomRightX < DrawArea_TopLeftX )
	{
		//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightX < DrawArea_TopLeftX.\n";
		return;
	}
	
	if ( DrawArea_BottomRightY < DrawArea_TopLeftY )
	{
		//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightY < DrawArea_TopLeftY.\n";
		return;
	}
	
	
	////////////////////////////////
	// y1 starts on the left
	//Y1_OnLeft = 1;
	

	// get color(s)
	bgr = gbgr [ 0 ];
	
	// ?? convert to 16-bit ?? (or should leave 24-bit?)
	//bgr = ( ((u32)Buffer [ 0 ].Red) >> 3 ) | ( ( ((u32)Buffer [ 0 ].Green) >> 3 ) << 5 ) | ( ( ((u32)Buffer [ 0 ].Blue) >> 3 ) << 10 );
	
	// get y-values
	//y0 = Buffer [ Coord0 ].y;
	//y1 = Buffer [ Coord1 ].y;
	//y2 = Buffer [ Coord2 ].y;
	
	///////////////////////////////////
	// put top coordinates in x0,y0
	//if ( y1 < y0 )
	if ( gy [ Coord1 ] < gy [ Coord0 ] )
	{
		//Swap ( y0, y1 );
		Swap ( Coord0, Coord1 );
	}
	
	//if ( y2 < y0 )
	if ( gy [ Coord2 ] < gy [ Coord0 ] )
	{
		//Swap ( y0, y2 );
		Swap ( Coord0, Coord2 );
	}
	
	///////////////////////////////////////
	// put middle coordinates in x1,y1
	//if ( y2 < y1 )
	if ( gy [ Coord2 ] < gy [ Coord1 ] )
	{
		//Swap ( y1, y2 );
		Swap ( Coord1, Coord2 );
	}
	
	// get x-values
	x0 = gx [ Coord0 ];
	x1 = gx [ Coord1 ];
	x2 = gx [ Coord2 ];

	// get y-values
	y0 = gy [ Coord0 ];
	y1 = gy [ Coord1 ];
	y2 = gy [ Coord2 ];
	
	// get texture coords
	u0 = gu [ Coord0 ];
	u1 = gu [ Coord1 ];
	u2 = gu [ Coord2 ];
	v0 = gv [ Coord0 ];
	v1 = gv [ Coord1 ];
	v2 = gv [ Coord2 ];
	
	//////////////////////////////////////////
	// get coordinates on screen
	x0 = DrawArea_OffsetX + x0;
	y0 = DrawArea_OffsetY + y0;
	x1 = DrawArea_OffsetX + x1;
	y1 = DrawArea_OffsetY + y1;
	x2 = DrawArea_OffsetX + x2;
	y2 = DrawArea_OffsetY + y2;
	
	// set value for color addition
	color_add = bgr;

	
	clut_xoffset = clut_x << 4;
	ptr_clut = & ( VRAM [ clut_y << 10 ] );
	ptr_texture = & ( VRAM [ ( tpage_tx << 6 ) + ( ( tpage_ty << 8 ) << 10 ) ] );
	

	//////////////////////////////////////////////////////
	// Get offset into color lookup table
	//u32 ClutOffset = ( clut_x << 4 ) + ( clut_y << 10 );
	
	/////////////////////////////////////////////////////////
	// Get offset into texture page
	//u32 TextureOffset = ( tpage_tx << 6 ) + ( ( tpage_ty << 8 ) << 10 );
	
	
	if ( tpage_tp == 0 )
	{
		And2 = 0xf;
		
#ifdef _ENABLE_SSE2_TRIANGLE_TEXTURE_NONTEMPLATE
		Shift1 = _mm_set_epi32 ( 0, 0, 0, 2 );
		Shift2 = _mm_set_epi32 ( 0, 0, 0, 2 );
		And1 = _mm_set1_epi16 ( 3 );
#else
		Shift1 = 2; Shift2 = 2;
		And1 = 3; And2 = 0xf;
#endif
	}
	else if ( tpage_tp == 1 )
	{
		And2 = 0xff;
		
#ifdef _ENABLE_SSE2_TRIANGLE_TEXTURE_NONTEMPLATE
		Shift1 = _mm_set_epi32 ( 0, 0, 0, 1 );
		Shift2 = _mm_set_epi32 ( 0, 0, 0, 3 );
		And1 = _mm_set1_epi16 ( 1 );
#else
		Shift1 = 1; Shift2 = 3;
		And1 = 1; And2 = 0xff;
#endif
	}
	
	
	
	// get the left/right most x
	LeftMostX = ( ( x0 < x1 ) ? x0 : x1 );
	LeftMostX = ( ( x2 < LeftMostX ) ? x2 : LeftMostX );
	RightMostX = ( ( x0 > x1 ) ? x0 : x1 );
	RightMostX = ( ( x2 > RightMostX ) ? x2 : RightMostX );

	// check if sprite is within draw area
	if ( RightMostX <= ((s32)DrawArea_TopLeftX) || LeftMostX > ((s32)DrawArea_BottomRightX) || y2 <= ((s32)DrawArea_TopLeftY) || y0 > ((s32)DrawArea_BottomRightY) ) return;
	
	// skip drawing if distance between vertices is greater than max allowed by GPU
	if ( ( _Abs( x1 - x0 ) > c_MaxPolygonWidth ) || ( _Abs( x2 - x1 ) > c_MaxPolygonWidth ) || ( y1 - y0 > c_MaxPolygonHeight ) || ( y2 - y1 > c_MaxPolygonHeight ) )
	{
		// skip drawing polygon
		return;
	}
	

	// calculate across
	t0 = y1 - y2;
	t1 = y0 - y2;
	denominator = ( ( x0 - x2 ) * t0 ) - ( ( x1 - x2 ) * t1 );
	if ( denominator )
	{
		denominator = ( 1LL << 48 ) / denominator;
		
		//dU_across = ( ( (s64) ( ( ( u0 - u2 ) * t0 ) - ( ( u1 - u2 ) * t1 ) ) ) << 24 ) / denominator;
		//dV_across = ( ( (s64) ( ( ( v0 - v2 ) * t0 ) - ( ( v1 - v2 ) * t1 ) ) ) << 24 ) / denominator;
		dU_across = ( ( (s64) ( ( ( u0 - u2 ) * t0 ) - ( ( u1 - u2 ) * t1 ) ) ) * denominator ) >> 24;
		dV_across = ( ( (s64) ( ( ( v0 - v2 ) * t0 ) - ( ( v1 - v2 ) * t1 ) ) ) * denominator ) >> 24;
		
#ifdef _ENABLE_SSE2_TRIANGLE_TEXTURE_NONTEMPLATE
		vdU_across = _mm_set1_epi32 ( dU_across * 8 );
		vdV_across = _mm_set1_epi32 ( dV_across * 8 );
#endif
	}

	
	// get reciprocals
	if ( y1 - y0 ) r10 = ( 1LL << 48 ) / ((s64)( y1 - y0 ));
	if ( y2 - y0 ) r20 = ( 1LL << 48 ) / ((s64)( y2 - y0 ));
	if ( y2 - y1 ) r21 = ( 1LL << 48 ) / ((s64)( y2 - y1 ));
	
	/////////////////////////////////////////////////
	// draw top part of triangle
	

	///////////////////////////////////////////
	// start at y0
	//Line = y0;
	
	/////////////////////////////////////////////
	// init x on the left and right
		
	////////////////////////////////////
	// get slopes
	
	if ( y1 - y0 )
	{
		x_left = ( ((s64)x0) << 16 );
		x_right = x_left;
		
		U_left = ( ((s64)u0) << 24 );
		V_left = ( ((s64)v0) << 24 );
		//U_right = U_left;
		//V_right = V_left;
		
		if ( denominator < 0 )
		{
			//dx_left = (((s64)( x1 - x0 )) << 16 ) / ((s64)( y1 - y0 ));
			//dx_right = (((s64)( x2 - x0 )) << 16 ) / ((s64)( y2 - y0 ));
			dx_left = ( ((s64)( x1 - x0 )) * r10 ) >> 32;
			dx_right = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
			
			//dU_left = (((s64)( u1 - u0 )) << 24 ) / ((s64)( y1 - y0 ));
			//dV_left = (((s64)( v1 - v0 )) << 24 ) / ((s64)( y1 - y0 ));
			dU_left = ( ((s64)( u1 - u0 )) * r10 ) >> 24;
			dV_left = ( ((s64)( v1 - v0 )) * r10 ) >> 24;
		}
		else
		{
			dx_right = ( ((s64)( x1 - x0 )) * r10 ) >> 32;
			dx_left = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
			
			//dU_right = (((s64)( u2 - u0 )) << 24 ) / ((s64)( y2 - y0 ));
			//dV_right = (((s64)( v2 - v0 )) << 24 ) / ((s64)( y2 - y0 ));
			dU_left = ( ((s64)( u2 - u0 )) * r20 ) >> 24;
			dV_left = ( ((s64)( v2 - v0 )) * r20 ) >> 24;
		}
	}
	else
	{
		if ( denominator < 0 )
		{
			// change x_left and x_right where y1 is on left
			x_left = ( ((s64)x1) << 16 );
			x_right = ( ((s64)x0) << 16 );
			
			//dx_left = (((s64)( x2 - x1 )) << 16 ) / ((s64)( y2 - y1 ));
			//dx_right = (((s64)( x2 - x0 )) << 16 ) / ((s64)( y2 - y0 ));
			dx_left = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
			dx_right = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
			
			// change U_left and V_right where y1 is on left
			U_left = ( ((s64)u1) << 24 );
			V_left = ( ((s64)v1) << 24 );
		
			//dU_left = (((s64)( u2 - u1 )) << 24 ) / ((s64)( y2 - y1 ));
			//dV_left = (((s64)( v2 - v1 )) << 24 ) / ((s64)( y2 - y1 ));
			dU_left = ( ((s64)( u2 - u1 )) * r21 ) >> 24;
			dV_left = ( ((s64)( v2 - v1 )) * r21 ) >> 24;
		}
		else
		{
			x_right = ( ((s64)x1) << 16 );
			x_left = ( ((s64)x0) << 16 );
			
			dx_right = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
			dx_left = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
			
			U_left = ( ((s64)u0) << 24 );
			V_left = ( ((s64)v0) << 24 );
		
			//dU_right = (((s64)( u2 - u0 )) << 24 ) / ((s64)( y2 - y0 ));
			//dV_right = (((s64)( v2 - v0 )) << 24 ) / ((s64)( y2 - y0 ));
			dU_left = ( ((s64)( u2 - u0 )) * r20 ) >> 24;
			dV_left = ( ((s64)( v2 - v0 )) * r20 ) >> 24;
		}
	}

	

	/////////////////////////////////////////////////
	// swap left and right if they are reversed
	//if ( ( ( x_right + dx_right ) < ( x_left + dx_left ) ) || ( x_right < x_left ) )
	/*
	if ( denominator > 0 )
	{
		// x1,y1 is on the right //
		
		Y1_OnLeft = 0;
		
		Swap ( x_left, x_right );
		Swap ( dx_left, dx_right );

		Swap ( dU_left, dU_right );
		Swap ( dV_left, dV_right );

		Swap ( U_left, U_right );
		Swap ( V_left, V_right );
	}
	*/
	
	////////////////
	// *** TODO *** at this point area of full triangle can be calculated and the rest of the drawing can be put on another thread *** //
	
	// u,v values are not specified with a fractional part, so there must be an initial fractional part
	U_left |= ( 1 << 23 );
	V_left |= ( 1 << 23 );
	
	
	StartY = y0;
	EndY = y1;

	if ( StartY < ((s32)DrawArea_TopLeftY) )
	{
		
		if ( EndY < ((s32)DrawArea_TopLeftY) )
		{
			Temp = EndY - StartY;
			StartY = EndY;
		}
		else
		{
			Temp = DrawArea_TopLeftY - StartY;
			StartY = DrawArea_TopLeftY;
		}
		
		x_left += dx_left * Temp;
		x_right += dx_right * Temp;
		
		U_left += dU_left * Temp;
		V_left += dV_left * Temp;
		
		//U_right += dU_right * Temp;
		//V_right += dV_right * Temp;
	}
	
	if ( EndY > ((s32)DrawArea_BottomRightY) )
	{
		EndY = DrawArea_BottomRightY + 1;
	}

#ifdef _ENABLE_SSE2_TRIANGLE_TEXTURE_NONTEMPLATE
	vSeq32_u1 = _custom_mul_32 ( _mm_set1_epi32 ( dU_across ), vSeq32_1 );
	vSeq32_u2 = _custom_mul_32 ( _mm_set1_epi32 ( dU_across ), vSeq32_2 );
	vSeq32_v1 = _custom_mul_32 ( _mm_set1_epi32 ( dV_across ), vSeq32_1 );
	vSeq32_v2 = _custom_mul_32 ( _mm_set1_epi32 ( dV_across ), vSeq32_2 );
#endif

	if ( EndY > StartY )
	{
	
	//////////////////////////////////////////////
	// draw down to y1
	//while ( Line < y1 )
	for ( Line = StartY; Line < EndY; Line++ )
	{
		//StartX = _Round( x_left ) >> 32;
		//EndX = _Round( x_right ) >> 32;
		
		// left point is included if points are equal
		StartX = ( x_left + 0xffffLL ) >> 16;
		EndX = ( x_right - 1 ) >> 16;
		
		
		if ( StartX <= ((s32)DrawArea_BottomRightX) && EndX >= ((s32)DrawArea_TopLeftX) && EndX >= StartX )
		{
			iU = U_left;
			iV = V_left;
			
			
			// get the difference between x_left and StartX
			Temp = ( StartX << 16 ) - x_left;
			
			if ( StartX < ((s32)DrawArea_TopLeftX) )
			{
				Temp += ( DrawArea_TopLeftX - StartX ) << 16;
				StartX = DrawArea_TopLeftX;
				
				//iU += dU_across * Temp;
				//iV += dV_across * Temp;
			}
			
			iU += ( dU_across >> 12 ) * ( Temp >> 4 );
			iV += ( dV_across >> 12 ) * ( Temp >> 4 );
			
			if ( EndX > ((s32)DrawArea_BottomRightX) )
			{
				//EndX = DrawArea_BottomRightX + 1;
				EndX = DrawArea_BottomRightX;
			}
			
			ptr = & ( VRAM [ StartX + ( Line << 10 ) ] );
			
			
			/////////////////////////////////////////////////////
			// update number of cycles used to draw polygon
			NumberOfPixelsDrawn += EndX - StartX + 1;


#ifdef _ENABLE_SSE2_TRIANGLE_TEXTURE_NONTEMPLATE
	vx_across = _mm_add_epi16 ( vSeq, _mm_set1_epi16 ( StartX ) );
	vEndX = _mm_set1_epi16 ( EndX + 1 );
	
	//viU1 = _mm_add_epi32 ( _mm_set1_epi32 ( iU ), _custom_mul_32 ( _mm_set1_epi32 ( dU_across ), vSeq1_32 ) );
	//viU2 = _mm_add_epi32 ( _mm_set1_epi32 ( iU ), _custom_mul_32 ( _mm_set1_epi32 ( dU_across ), vSeq2_32 ) );
	//viV1 = _mm_add_epi32 ( _mm_set1_epi32 ( iV ), _custom_mul_32 ( _mm_set1_epi32 ( dV_across ), vSeq1_32 ) );
	//viV2 = _mm_add_epi32 ( _mm_set1_epi32 ( iV ), _custom_mul_32 ( _mm_set1_epi32 ( dV_across ), vSeq2_32 ) );

	viU1 = _mm_add_epi32 ( _mm_set1_epi32 ( iU ), vSeq32_u1 );
	viU2 = _mm_add_epi32 ( _mm_set1_epi32 ( iU ), vSeq32_u2 );
	viV1 = _mm_add_epi32 ( _mm_set1_epi32 ( iV ), vSeq32_v1 );
	viV2 = _mm_add_epi32 ( _mm_set1_epi32 ( iV ), vSeq32_v2 );
	
#endif


			// draw horizontal line
			// x_left and x_right need to be rounded off
			for ( x_across = StartX; x_across <= EndX; x_across++ )
			{
#ifdef _ENABLE_SSE2_TRIANGLE_TEXTURE_NONTEMPLATE
				viV =  _mm_packs_epi32 ( _mm_srli_epi32 ( _mm_add_epi32 ( viV1, vRound24 ), 24 ), _mm_srli_epi32 ( _mm_add_epi32 ( viV2, vRound24 ), 24 ) );
				viU =  _mm_packs_epi32 ( _mm_srli_epi32 ( _mm_add_epi32 ( viU1, vRound24 ), 24 ), _mm_srli_epi32 ( _mm_add_epi32 ( viU2, vRound24 ), 24 ) );
				
				TexCoordY = _mm_and_si128 ( _mm_or_si128 ( _mm_and_si128 ( viV, Not_TWH ), TWYTWH ), Mask );
				TexCoordX = _mm_and_si128 ( _mm_or_si128 ( _mm_and_si128 ( viU, Not_TWW ), TWXTWW ), Mask );
				//vIndex1 = _mm_srl_epi16 ( TexCoordX, Shift1 );
				if ( And2 )
				{
					vIndex2 = _mm_sll_epi16 ( _mm_and_si128 ( TexCoordX, And1 ), Shift2 );
					
					vIndex1 = _mm_srl_epi16 ( TexCoordX, Shift1 );
					
					_mm_store_si128 ( (__m128i*) TexCoordXList, vIndex1 );
					_mm_store_si128 ( (__m128i*) TempList, vIndex2 );
					_mm_store_si128 ( (__m128i*) TexCoordYList, TexCoordY );
					
					// get number of pixels remaining to draw
					uTemp32 = EndX - x_across;
					
					// only get full 8 pixels if there are 8 or more left to draw, otherwize get remaining to draw
					uTemp32 = ( ( uTemp32 > 7 ) ? 7 : uTemp32 );
					
					for ( uIndex32 = 0; uIndex32 <= uTemp32; uIndex32++ )
					{
						bgr = ptr_texture [ TexCoordXList [ uIndex32 ] + ( ( (u32) TexCoordYList [ uIndex32 ] ) << 10 ) ];
						bgr = ptr_clut [ ( clut_xoffset + ( ( bgr >> TempList [ uIndex32 ] ) & And2 ) ) & FrameBuffer_XMask ];
						TexCoordXList [ uIndex32 ] = bgr;
					}
					
					vbgr = _mm_load_si128 ( (const __m128i*) TexCoordXList );
					
					
					/*
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 0 ) + ( _mm_extract_epi16 ( TexCoordY, 0 ) << 10 ) ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 0 ) ) & And2 ) ) & FrameBuffer_XMask ], 0 );
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 1 ) + ( _mm_extract_epi16 ( TexCoordY, 1 ) << 10 ) ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 1 ) ) & And2 ) ) & FrameBuffer_XMask ], 1 );
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 2 ) + ( _mm_extract_epi16 ( TexCoordY, 2 ) << 10 ) ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 2 ) ) & And2 ) ) & FrameBuffer_XMask ], 2 );
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 3 ) + ( _mm_extract_epi16 ( TexCoordY, 3 ) << 10 ) ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 3 ) ) & And2 ) ) & FrameBuffer_XMask ], 3 );
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 4 ) + ( _mm_extract_epi16 ( TexCoordY, 4 ) << 10 ) ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 4 ) ) & And2 ) ) & FrameBuffer_XMask ], 4 );
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 5 ) + ( _mm_extract_epi16 ( TexCoordY, 5 ) << 10 ) ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 5 ) ) & And2 ) ) & FrameBuffer_XMask ], 5 );
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 6 ) + ( _mm_extract_epi16 ( TexCoordY, 6 ) << 10 ) ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 6 ) ) & And2 ) ) & FrameBuffer_XMask ], 6 );
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 7 ) + ( _mm_extract_epi16 ( TexCoordY, 7 ) << 10 ) ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 7 ) ) & And2 ) ) & FrameBuffer_XMask ], 7 );
					*/
				}
				else
				{
					
					_mm_store_si128 ( (__m128i*) TexCoordXList, TexCoordX );
					_mm_store_si128 ( (__m128i*) TexCoordYList, TexCoordY );
					
					// get number of pixels remaining to draw
					uTemp32 = EndX - x_across;
					
					// only get full 8 pixels if there are 8 or more left to draw, otherwize get remaining to draw
					uTemp32 = ( ( uTemp32 > 7 ) ? 7 : uTemp32 );
					
					for ( uIndex32 = 0; uIndex32 <= uTemp32; uIndex32++ )
					{
						TexCoordXList [ uIndex32 ] = ptr_texture [ TexCoordXList [ uIndex32 ] + ( ( (u32) TexCoordYList [ uIndex32 ] ) << 10 ) ];
					}
					
					vbgr = _mm_load_si128 ( (const __m128i*) TexCoordXList );
					
					
					/*
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 0 ) + ( _mm_extract_epi16 ( TexCoordY, 0 ) << 10 ) ], 0 );
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 1 ) + ( _mm_extract_epi16 ( TexCoordY, 1 ) << 10 ) ], 1 );
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 2 ) + ( _mm_extract_epi16 ( TexCoordY, 2 ) << 10 ) ], 2 );
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 3 ) + ( _mm_extract_epi16 ( TexCoordY, 3 ) << 10 ) ], 3 );
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 4 ) + ( _mm_extract_epi16 ( TexCoordY, 4 ) << 10 ) ], 4 );
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 5 ) + ( _mm_extract_epi16 ( TexCoordY, 5 ) << 10 ) ], 5 );
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 6 ) + ( _mm_extract_epi16 ( TexCoordY, 6 ) << 10 ) ], 6 );
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 7 ) + ( _mm_extract_epi16 ( TexCoordY, 7 ) << 10 ) ], 7 );
					*/
				}
#else
					//TexCoordY = (u8) ( ( ( _Round24 ( iV ) >> 24 ) & Not_TWH ) | TWYTWH );
					//TexCoordX = (u8) ( ( ( _Round24 ( iU ) >> 24 ) & Not_TWW ) | TWXTWW );
					TexCoordY = (u8) ( ( ( iV >> 24 ) & Not_TWH ) | TWYTWH );
					TexCoordX = (u8) ( ( ( iU >> 24 ) & Not_TWW ) | TWXTWW );
					
					// *** testing ***
					//debug << dec << "\r\nTexCoordX=" << TexCoordX << " TexCoordY=" << TexCoordY;
					
					//bgr = VRAM [ TextureOffset + ( TexCoordX >> Shift1 ) + ( TexCoordY << 10 ) ];
					bgr = ptr_texture [ ( TexCoordX >> Shift1 ) + ( TexCoordY << 10 ) ];
					
					if ( Shift1 )
					{
						TexelIndex = ( bgr >> ( ( TexCoordX & And1 ) << Shift2 ) ) & And2;
						//bgr = VRAM [ ( ( ( clut_x << 4 ) + TexelIndex ) & FrameBuffer_XMask ) + ( clut_y << 10 ) ];
						bgr = ptr_clut [ ( clut_xoffset + TexelIndex ) & FrameBuffer_XMask ];
					}
#endif
					
					// *** testing ***
					//debug << "; TexelIndex=" << TexelIndex << hex << "; bgr=" << bgr;
					
#ifdef _ENABLE_SSE2_TRIANGLE_TEXTURE_NONTEMPLATE
				DestPixel = _mm_loadu_si128 ((__m128i const*) ptr);
				vbgr_temp = vbgr;
				if ( !command_tge ) vbgr_temp = vColorMultiply1624 ( vbgr_temp, color_add_r, color_add_g, color_add_b );
				if ( command_abe )
				{
					vbgr_temp_transparent = vSemiTransparency16( DestPixel, vbgr_temp, GPU_CTRL_Read.ABR );
					vbgr_select = _mm_srai_epi16 ( vbgr, 15 );
					vbgr_temp = _mm_or_si128 ( _mm_andnot_si128( vbgr_select, vbgr_temp ), _mm_and_si128 ( vbgr_select, vbgr_temp_transparent ) );
				}
				vbgr_temp = _mm_or_si128 ( _mm_or_si128 ( vbgr_temp, SetPixelMask ), _mm_and_si128 ( vbgr, tMask ) );
				_mm_maskmoveu_si128 ( vbgr_temp, _mm_andnot_si128 ( _mm_and_si128 ( _mm_srai_epi16 ( DestPixel, 15 ), PixelMask ), _mm_andnot_si128 ( _mm_cmpeq_epi16 ( vbgr, _mm_setzero_si128 () ), _mm_cmplt_epi16 ( vx_across, vEndX ) ) ), (char*) ptr );
				vx_across = _mm_add_epi16 ( vx_across, vVectorSize );
				//viU = _mm_add_epi16 ( viU, vVectorSize );
				viU1 = _mm_add_epi16 ( viU1, vdU_across );
				viU2 = _mm_add_epi16 ( viU2, vdU_across );
				viV1 = _mm_add_epi16 ( viV1, vdV_across );
				viV2 = _mm_add_epi16 ( viV2, vdV_across );
#else
					if ( bgr )
					{
						// shade pixel color
					
						// read pixel from frame buffer if we need to check mask bit
						//DestPixel = VRAM [ cx + ( cy << 10 ) ];
						DestPixel = *ptr;
						
						bgr_temp = bgr;
			
						if ( !command_tge )
						{
							// brightness calculation
							//bgr_temp = Color24To16 ( ColorMultiply24 ( Color16To24 ( bgr_temp ), color_add ) );
							bgr_temp = ColorMultiply1624 ( bgr_temp, color_add );
						}
						
						// semi-transparency
						if ( command_abe && ( bgr & 0x8000 ) )
						{
							bgr_temp = SemiTransparency16 ( DestPixel, bgr_temp, tpage_abr );
						}
						
						// check if we should set mask bit when drawing
						//if ( GPU_CTRL_Read.MD ) bgr_temp |= 0x8000;
						bgr_temp |= SetPixelMask | ( bgr & 0x8000 );

						// draw pixel if we can draw to mask pixels or mask bit not set
						//if ( ! ( DestPixel & PixelMask ) ) VRAM [ cx + ( cy << 10 ) ] = bgr_temp;
						if ( ! ( DestPixel & PixelMask ) ) *ptr = bgr_temp;
					}
					
					/////////////////////////////////////////////////////
					// update number of cycles used to draw polygon
					//NumberOfPixelsDrawn++;
				//}
				
				/////////////////////////////////////////////////////////
				// interpolate texture coords
				iU += dU_across;
				iV += dV_across;
#endif
				
				//ptr++;
				ptr += c_iVectorSize;
			}
			
		}
		
		/////////////////////////////////////
		// update x on left and right
		x_left += dx_left;
		x_right += dx_right;
		
		U_left += dU_left;
		V_left += dV_left;
		//U_right += dU_right;
		//V_right += dV_right;
	}

	} // end if ( EndY > StartY )
	
	////////////////////////////////////////////////
	// draw bottom part of triangle

	/////////////////////////////////////////////
	// init x on the left and right
	
	//////////////////////////////////////////////////////
	// check if y1 is on the left or on the right
	//if ( Y1_OnLeft )
	if ( denominator < 0 )
	{
		x_left = ( ((s64)x1) << 16 );

		U_left = ( ((s64)u1) << 24 );
		V_left = ( ((s64)v1) << 24 );

		// u,v values are not specified with a fractional part, so there must be an initial fractional part
		U_left |= ( 1 << 23 );
		V_left |= ( 1 << 23 );
		
		//if ( y2 - y1 )
		//{
			//dx_left = (((s64)( x2 - x1 )) << 16 ) / ((s64)( y2 - y1 ));
			dx_left = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
			
			//dU_left = (((s64)( u2 - u1 )) << 24 ) / ((s64)( y2 - y1 ));
			//dV_left = (((s64)( v2 - v1 )) << 24 ) / ((s64)( y2 - y1 ));
			dU_left = ( ((s64)( u2 - u1 )) * r21 ) >> 24;
			dV_left = ( ((s64)( v2 - v1 )) * r21 ) >> 24;
		//}
	}
	else
	{
		x_right = ( ((s64)x1) << 16 );

		//U_right = ( ((s64)u1) << 24 );
		//V_right = ( ((s64)v1) << 24 );

		//if ( y2 - y1 )
		//{
			//dx_right = (((s64)( x2 - x1 )) << 16 ) / ((s64)( y2 - y1 ));
			dx_right = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
			
			//dU_right = (((s64)( u2 - u1 )) << 24 ) / ((s64)( y2 - y1 ));
			//dV_right = (((s64)( v2 - v1 )) << 24 ) / ((s64)( y2 - y1 ));
			//dU_right = ( ((s64)( u2 - u1 )) * r21 ) >> 24;
			//dV_right = ( ((s64)( v2 - v1 )) * r21 ) >> 24;
		//}
	}
	
	// the line starts at y1 from here
	//Line = y1;

	StartY = y1;
	EndY = y2;

	if ( StartY < ((s32)DrawArea_TopLeftY) )
	{
		
		if ( EndY < ((s32)DrawArea_TopLeftY) )
		{
			Temp = EndY - StartY;
			StartY = EndY;
		}
		else
		{
			Temp = DrawArea_TopLeftY - StartY;
			StartY = DrawArea_TopLeftY;
		}
		
		x_left += dx_left * Temp;
		x_right += dx_right * Temp;
		
		U_left += dU_left * Temp;
		V_left += dV_left * Temp;
		
		//U_right += dU_right * Temp;
		//V_right += dV_right * Temp;
	}
	
	if ( EndY > ((s32)DrawArea_BottomRightY) )
	{
		EndY = DrawArea_BottomRightY + 1;
	}


	if ( EndY > StartY )
	{
	
	//////////////////////////////////////////////
	// draw down to y2
	//while ( Line < y2 )
	for ( Line = StartY; Line < EndY; Line++ )
	{
		//StartX = _Round( x_left ) >> 32;
		//EndX = _Round( x_right ) >> 32;
		
		// left point is included if points are equal
		StartX = ( (s64) ( x_left + 0xffffLL ) ) >> 16;
		EndX = ( x_right - 1 ) >> 16;
		
		
		if ( StartX <= ((s32)DrawArea_BottomRightX) && EndX >= ((s32)DrawArea_TopLeftX) && EndX >= StartX )
		{
			iU = U_left;
			iV = V_left;
			
			
			// get the difference between x_left and StartX
			Temp = ( StartX << 16 ) - x_left;
			
			if ( StartX < ((s32)DrawArea_TopLeftX) )
			{
				Temp += ( DrawArea_TopLeftX - StartX ) << 16;
				StartX = DrawArea_TopLeftX;
				
				//iU += dU_across * Temp;
				//iV += dV_across * Temp;
			}
			
			iU += ( dU_across >> 12 ) * ( Temp >> 4 );
			iV += ( dV_across >> 12 ) * ( Temp >> 4 );
			
			if ( EndX > ((s32)DrawArea_BottomRightX) )
			{
				//EndX = DrawArea_BottomRightX + 1;
				EndX = DrawArea_BottomRightX;
			}
			
			ptr = & ( VRAM [ StartX + ( Line << 10 ) ] );
			
			
			/////////////////////////////////////////////////////
			// update number of cycles used to draw polygon
			NumberOfPixelsDrawn += EndX - StartX + 1;
			
#ifdef _ENABLE_SSE2_TRIANGLE_TEXTURE_NONTEMPLATE
	vx_across = _mm_add_epi16 ( vSeq, _mm_set1_epi16 ( StartX ) );
	vEndX = _mm_set1_epi16 ( EndX + 1 );
	
	//viU1 = _mm_add_epi32 ( _mm_set1_epi32 ( iU ), _custom_mul_32 ( _mm_set1_epi32 ( dU_across ), vSeq1_32 ) );
	//viU2 = _mm_add_epi32 ( _mm_set1_epi32 ( iU ), _custom_mul_32 ( _mm_set1_epi32 ( dU_across ), vSeq2_32 ) );
	//viV1 = _mm_add_epi32 ( _mm_set1_epi32 ( iV ), _custom_mul_32 ( _mm_set1_epi32 ( dV_across ), vSeq1_32 ) );
	//viV2 = _mm_add_epi32 ( _mm_set1_epi32 ( iV ), _custom_mul_32 ( _mm_set1_epi32 ( dV_across ), vSeq2_32 ) );
	
	viU1 = _mm_add_epi32 ( _mm_set1_epi32 ( iU ), vSeq32_u1 );
	viU2 = _mm_add_epi32 ( _mm_set1_epi32 ( iU ), vSeq32_u2 );
	viV1 = _mm_add_epi32 ( _mm_set1_epi32 ( iV ), vSeq32_v1 );
	viV2 = _mm_add_epi32 ( _mm_set1_epi32 ( iV ), vSeq32_v2 );
#endif

			// draw horizontal line
			// x_left and x_right need to be rounded off
			//for ( x_across = StartX; x_across < EndX; x_across++ )
			for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize )
			{
#ifdef _ENABLE_SSE2_TRIANGLE_TEXTURE_NONTEMPLATE
				viV =  _mm_packs_epi32 ( _mm_srli_epi32 ( _mm_add_epi32 ( viV1, vRound24 ), 24 ), _mm_srli_epi32 ( _mm_add_epi32 ( viV2, vRound24 ), 24 ) );
				viU =  _mm_packs_epi32 ( _mm_srli_epi32 ( _mm_add_epi32 ( viU1, vRound24 ), 24 ), _mm_srli_epi32 ( _mm_add_epi32 ( viU2, vRound24 ), 24 ) );
				
				TexCoordY = _mm_and_si128 ( _mm_or_si128 ( _mm_and_si128 ( viV, Not_TWH ), TWYTWH ), Mask );
				TexCoordX = _mm_and_si128 ( _mm_or_si128 ( _mm_and_si128 ( viU, Not_TWW ), TWXTWW ), Mask );
				//vIndex1 = _mm_srl_epi16 ( TexCoordX, Shift1 );
				if ( And2 )
				{
					vIndex2 = _mm_sll_epi16 ( _mm_and_si128 ( TexCoordX, And1 ), Shift2 );
					
					vIndex1 = _mm_srl_epi16 ( TexCoordX, Shift1 );
					
					_mm_store_si128 ( (__m128i*) TexCoordXList, vIndex1 );
					_mm_store_si128 ( (__m128i*) TempList, vIndex2 );
					_mm_store_si128 ( (__m128i*) TexCoordYList, TexCoordY );
					
					// get number of pixels remaining to draw
					uTemp32 = EndX - x_across;
					
					// only get full 8 pixels if there are 8 or more left to draw, otherwize get remaining to draw
					uTemp32 = ( ( uTemp32 > 7 ) ? 7 : uTemp32 );
					
					for ( uIndex32 = 0; uIndex32 <= uTemp32; uIndex32++ )
					{
						bgr = ptr_texture [ TexCoordXList [ uIndex32 ] + ( ( (u32) TexCoordYList [ uIndex32 ] ) << 10 ) ];
						bgr = ptr_clut [ ( clut_xoffset + ( ( bgr >> TempList [ uIndex32 ] ) & And2 ) ) & FrameBuffer_XMask ];
						TexCoordXList [ uIndex32 ] = bgr;
					}
					
					vbgr = _mm_load_si128 ( (const __m128i*) TexCoordXList );
					
					
					/*
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 0 ) + ( _mm_extract_epi16 ( TexCoordY, 0 ) << 10 ) ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 0 ) ) & And2 ) ) & FrameBuffer_XMask ], 0 );
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 1 ) + ( _mm_extract_epi16 ( TexCoordY, 1 ) << 10 ) ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 1 ) ) & And2 ) ) & FrameBuffer_XMask ], 1 );
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 2 ) + ( _mm_extract_epi16 ( TexCoordY, 2 ) << 10 ) ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 2 ) ) & And2 ) ) & FrameBuffer_XMask ], 2 );
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 3 ) + ( _mm_extract_epi16 ( TexCoordY, 3 ) << 10 ) ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 3 ) ) & And2 ) ) & FrameBuffer_XMask ], 3 );
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 4 ) + ( _mm_extract_epi16 ( TexCoordY, 4 ) << 10 ) ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 4 ) ) & And2 ) ) & FrameBuffer_XMask ], 4 );
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 5 ) + ( _mm_extract_epi16 ( TexCoordY, 5 ) << 10 ) ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 5 ) ) & And2 ) ) & FrameBuffer_XMask ], 5 );
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 6 ) + ( _mm_extract_epi16 ( TexCoordY, 6 ) << 10 ) ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 6 ) ) & And2 ) ) & FrameBuffer_XMask ], 6 );
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 7 ) + ( _mm_extract_epi16 ( TexCoordY, 7 ) << 10 ) ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 7 ) ) & And2 ) ) & FrameBuffer_XMask ], 7 );
					*/
				}
				else
				{
					_mm_store_si128 ( (__m128i*) TexCoordXList, TexCoordX );
					_mm_store_si128 ( (__m128i*) TexCoordYList, TexCoordY );
					
					// get number of pixels remaining to draw
					uTemp32 = EndX - x_across;
					
					// only get full 8 pixels if there are 8 or more left to draw, otherwize get remaining to draw
					uTemp32 = ( ( uTemp32 > 7 ) ? 7 : uTemp32 );
					
					for ( uIndex32 = 0; uIndex32 <= uTemp32; uIndex32++ )
					{
						TexCoordXList [ uIndex32 ] = ptr_texture [ TexCoordXList [ uIndex32 ] + ( ( (u32) TexCoordYList [ uIndex32 ] ) << 10 ) ];
					}
					
					vbgr = _mm_load_si128 ( (const __m128i*) TexCoordXList );
				
					/*
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 0 ) + ( _mm_extract_epi16 ( TexCoordY, 0 ) << 10 ) ], 0 );
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 1 ) + ( _mm_extract_epi16 ( TexCoordY, 1 ) << 10 ) ], 1 );
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 2 ) + ( _mm_extract_epi16 ( TexCoordY, 2 ) << 10 ) ], 2 );
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 3 ) + ( _mm_extract_epi16 ( TexCoordY, 3 ) << 10 ) ], 3 );
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 4 ) + ( _mm_extract_epi16 ( TexCoordY, 4 ) << 10 ) ], 4 );
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 5 ) + ( _mm_extract_epi16 ( TexCoordY, 5 ) << 10 ) ], 5 );
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 6 ) + ( _mm_extract_epi16 ( TexCoordY, 6 ) << 10 ) ], 6 );
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 7 ) + ( _mm_extract_epi16 ( TexCoordY, 7 ) << 10 ) ], 7 );
					*/
				}
#else
					//TexCoordY = (u8) ( ( ( _Round24 ( iV ) >> 24 ) & Not_TWH ) | TWYTWH );
					//TexCoordX = (u8) ( ( ( _Round24 ( iU ) >> 24 ) & Not_TWW ) | TWXTWW );
					TexCoordY = (u8) ( ( ( iV >> 24 ) & Not_TWH ) | TWYTWH );
					TexCoordX = (u8) ( ( ( iU >> 24 ) & Not_TWW ) | TWXTWW );
					
					// *** testing ***
					//debug << dec << "\r\nTexCoordX=" << TexCoordX << " TexCoordY=" << TexCoordY;
					
					//bgr = VRAM [ TextureOffset + ( TexCoordX >> Shift1 ) + ( TexCoordY << 10 ) ];
					bgr = ptr_texture [ ( TexCoordX >> Shift1 ) + ( TexCoordY << 10 ) ];
					
					
					if ( Shift1 )
					{
						TexelIndex = ( bgr >> ( ( TexCoordX & And1 ) << Shift2 ) ) & And2;
						//bgr = VRAM [ ( ( ( clut_x << 4 ) + TexelIndex ) & FrameBuffer_XMask ) + ( clut_y << 10 ) ];
						bgr = ptr_clut [ ( clut_xoffset + TexelIndex ) & FrameBuffer_XMask ];
					}
#endif
					
					// *** testing ***
					//debug << "; TexelIndex=" << TexelIndex << hex << "; bgr=" << bgr;
					
#ifdef _ENABLE_SSE2_TRIANGLE_TEXTURE_NONTEMPLATE
				DestPixel = _mm_loadu_si128 ((__m128i const*) ptr);
				vbgr_temp = vbgr;
				if ( !command_tge ) vbgr_temp = vColorMultiply1624 ( vbgr_temp, color_add_r, color_add_g, color_add_b );
				if ( command_abe )
				{
					vbgr_temp_transparent = vSemiTransparency16( DestPixel, vbgr_temp, GPU_CTRL_Read.ABR );
					vbgr_select = _mm_srai_epi16 ( vbgr, 15 );
					vbgr_temp = _mm_or_si128 ( _mm_andnot_si128( vbgr_select, vbgr_temp ), _mm_and_si128 ( vbgr_select, vbgr_temp_transparent ) );
				}
				vbgr_temp = _mm_or_si128 ( _mm_or_si128 ( vbgr_temp, SetPixelMask ), _mm_and_si128 ( vbgr, tMask ) );
				_mm_maskmoveu_si128 ( vbgr_temp, _mm_andnot_si128 ( _mm_and_si128 ( _mm_srai_epi16 ( DestPixel, 15 ), PixelMask ), _mm_andnot_si128 ( _mm_cmpeq_epi16 ( vbgr, _mm_setzero_si128 () ), _mm_cmplt_epi16 ( vx_across, vEndX ) ) ), (char*) ptr );
				vx_across = _mm_add_epi16 ( vx_across, vVectorSize );
				//viU = _mm_add_epi16 ( viU, vVectorSize );
				viU1 = _mm_add_epi16 ( viU1, vdU_across );
				viU2 = _mm_add_epi16 ( viU2, vdU_across );
				viV1 = _mm_add_epi16 ( viV1, vdV_across );
				viV2 = _mm_add_epi16 ( viV2, vdV_across );
#else
					if ( bgr )
					{
						// shade pixel color
					
						// read pixel from frame buffer if we need to check mask bit
						//DestPixel = VRAM [ cx + ( cy << 10 ) ];
						DestPixel = *ptr;
						
						bgr_temp = bgr;
			
						if ( !command_tge )
						{
							// brightness calculation
							//bgr_temp = Color24To16 ( ColorMultiply24 ( Color16To24 ( bgr_temp ), color_add ) );
							bgr_temp = ColorMultiply1624 ( bgr_temp, color_add );
						}
						
						// semi-transparency
						if ( command_abe && ( bgr & 0x8000 ) )
						{
							bgr_temp = SemiTransparency16 ( DestPixel, bgr_temp, tpage_abr );
						}
						
						// check if we should set mask bit when drawing
						//if ( GPU_CTRL_Read.MD ) bgr_temp |= 0x8000;
						bgr_temp |= SetPixelMask | ( bgr & 0x8000 );

						// draw pixel if we can draw to mask pixels or mask bit not set
						//if ( ! ( DestPixel & PixelMask ) ) VRAM [ cx + ( cy << 10 ) ] = bgr_temp;
						if ( ! ( DestPixel & PixelMask ) ) *ptr = bgr_temp;
					}
					
				/////////////////////////////////////////////////////////
				// interpolate texture coords
				iU += dU_across;
				iV += dV_across;
#endif
				
				//ptr++;
				ptr += c_iVectorSize;
			}
			
		}
		
		/////////////////////////////////////
		// update x on left and right
		x_left += dx_left;
		x_right += dx_right;
		
		U_left += dU_left;
		V_left += dV_left;
		//U_right += dU_right;
		//V_right += dV_right;
	}
	
	} // end if ( EndY > StartY )

}

#endif


#ifndef EXCLUDE_TRIANGLE_TEXTUREGRADIENT_NONTEMPLATE

void GPU::DrawTriangle_TextureGradient ( u32 Coord0, u32 Coord1, u32 Coord2 )
{
#ifdef _ENABLE_SSE2_TRIANGLE_TEXTUREGRADIENT_NONTEMPLATE
	// with sse2, can send 8 pixels at a time
	static const int c_iVectorSize = 8;
#else
	// before sse2, was sending 1 pixels at a time
	static const int c_iVectorSize = 1;
#endif

	u32 clut_xoffset;

	u32 Pixel, TexelIndex;
	//u32 Y1_OnLeft;
	
	u32 color_add;
	
	u16 *ptr_texture, *ptr_clut;
	u16 *ptr;
	
	s64 Temp, TMin, TMax;
	s64 LeftMostX, RightMostX;
	s64 r10, r20, r21;
	
	s32 StartX, EndX, StartY, EndY;
	
	s32* DitherArray;
	s32* DitherLine;
	s32 DitherValue;
	
	u32 uTemp32, uIndex32;

	// new local variables
	s32 x0, x1, x2, y0, y1, y2;
	s64 dx_left, dx_right;
	s64 x_left, x_right, x_across;
	u32 bgr, bgr_temp;
	s32 Line;
	s64 t0, t1, denominator;

	// more local variables for gradient triangle
	s64 dR_left, dG_left, dB_left, dR_across, dG_across, dB_across, iR, iG, iB, R_left, G_left, B_left;
	s32 r0, r1, r2, g0, g1, g2, b0, b1, b2;
	
	// new local variables for texture mapping
	s64 dU_left, dV_left, dU_across, dV_across, U_left, V_left, iU, iV;
	s32 u0, v0, u1, v1, u2, v2;
	//u32 clut_x, clut_y, tpage_tx, tpage_ty, tpage_abr, tpage_tp;
	//u32 ClutOffset;

#ifdef _ENABLE_SSE2_TRIANGLE_TEXTUREGRADIENT_NONTEMPLATE
	s16* vDitherArray_add;
	s16* vDitherArray_sub;
	s16* vDitherLine_add;
	s16* vDitherLine_sub;
	__m128i viR1, viG1, viB1, viR2, viG2, viB2, vRed, vGreen, vBlue, vdR_across, vdG_across, vdB_across, vDitherValue_add, vDitherValue_sub, vTemp;
	
	__m128i DestPixel, PixelMask, SetPixelMask;
	__m128i vbgr, vbgr_temp, vStartX, vEndX, vx_across, vSeq, vVectorSize;
	__m128i vbgr_temp_transparent, vbgr_select;
	
	__m128i vSeq32_1, vSeq32_2;
	__m128i vSeq32_u1, vSeq32_u2, vSeq32_v1, vSeq32_v2, vSeq32_r1, vSeq32_r2, vSeq32_g1, vSeq32_g2, vSeq32_b1, vSeq32_b2;
	
	__m128i TexCoordX, TexCoordY, Mask, tMask;	//, vTexCoordX2, vTexCoordY1, vTexCoordY2;
	u32 And2 = 0;
	__m128i And1, Shift1, Shift2;
	__m128i vIndex1, vIndex2;
	__m128i viU, viU1, viU2, viV, viV1, viV2;
	__m128i vdV_across, vdU_across;
	
	//u32 TWYTWH, Not_TWH;
	__m128i TWXTWW, Not_TWW, TWYTWH, Not_TWH;
	//__m128i color_add_r, color_add_g, color_add_b;
	__m128i vRound24;
	
	vRound24 = _mm_set1_epi32 ( 0x00800000 );
	
	//color_add_r = _mm_set1_epi16 ( bgr & 0xff );
	//color_add_g = _mm_set1_epi16 ( ( bgr >> 8 ) & 0xff );
	//color_add_b = _mm_set1_epi16 ( ( bgr >> 16 ) & 0xff );
	
	Mask = _mm_set1_epi16 ( 0xff );
	tMask = _mm_set1_epi16 ( 0x8000 );
	
	vSeq32_1 = _mm_set_epi32 ( 3, 2, 1, 0 );
	vSeq32_2 = _mm_set_epi32 ( 7, 6, 5, 4 );
	
	TWYTWH = _mm_set1_epi16 ( ( ( TWY & TWH ) << 3 ) );
	TWXTWW = _mm_set1_epi16 ( ( ( TWX & TWW ) << 3 ) );
	
	Not_TWH = _mm_set1_epi16 ( ~( TWH << 3 ) );
	Not_TWW = _mm_set1_epi16 ( ~( TWW << 3 ) );
	
	And1 = _mm_setzero_si128 ();
	Shift1 = _mm_setzero_si128 ();
	
	//TextureOffset = _mm_set1_epi32 ( ( tpage_tx << 6 ) + ( ( tpage_ty << 8 ) << 10 ) );
	
	vSeq = _mm_set_epi16 ( 7, 6, 5, 4, 3, 2, 1, 0 );
	vVectorSize = _mm_set1_epi16 ( c_iVectorSize );
	
	//vbgr = _mm_set1_epi16 ( bgr );
	PixelMask = _mm_setzero_si128 ();
	SetPixelMask = _mm_setzero_si128 ();
	if ( GPU_CTRL_Read.ME ) PixelMask = _mm_set1_epi16 ( 0x8080 );
	if ( GPU_CTRL_Read.MD ) SetPixelMask = _mm_set1_epi16 ( 0x8000 );
	
	u16 TexCoordXList [ 8 ] __attribute__ ((aligned (32)));
	u16 TempList [ 8 ] __attribute__ ((aligned (32)));
	u16 TexCoordYList [ 8 ] __attribute__ ((aligned (32)));
#else

	u32 DestPixel, PixelMask = 0, SetPixelMask = 0;
	u32 TexCoordX, TexCoordY;
	u32 Shift1 = 0, Shift2 = 0, And1 = 0, And2 = 0;

	//s64 Error_Left;
	//s64 TexOffset_X, TexOffset_Y;
	
	
	s16 Red, Green, Blue;
	
	u32 TWYTWH, TWXTWW, Not_TWH, Not_TWW;
	
	TWYTWH = ( ( TWY & TWH ) << 3 );
	TWXTWW = ( ( TWX & TWW ) << 3 );
	
	Not_TWH = ~( TWH << 3 );
	Not_TWW = ~( TWW << 3 );
	
	if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
	if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;
#endif

	
	// initialize number of pixels drawn
	NumberOfPixelsDrawn = 0;

	
	// check for some important conditions
	if ( DrawArea_BottomRightX < DrawArea_TopLeftX )
	{
		//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightX < DrawArea_TopLeftX.\n";
		return;
	}
	
	if ( DrawArea_BottomRightY < DrawArea_TopLeftY )
	{
		//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightY < DrawArea_TopLeftY.\n";
		return;
	}
	
	///////////////////////////////////////////////////
	// Initialize dithering
	
#ifdef _ENABLE_SSE2_TRIANGLE_TEXTUREGRADIENT_NONTEMPLATE
	vDitherArray_add = (s16*) c_iDitherZero;
	vDitherArray_sub = (s16*) c_iDitherZero;
#else
	DitherArray = (s32*) c_iDitherZero;
#endif
	
	if ( GPU_CTRL_Read.DTD )
	{
#ifdef _ENABLE_SSE2_TRIANGLE_TEXTUREGRADIENT_NONTEMPLATE
		vDitherArray_add = c_viDitherValues16_add;
		vDitherArray_sub = c_viDitherValues16_sub;
#else
		//DitherArray = c_iDitherValues;
		DitherArray = (s32*) c_iDitherValues4;
#endif
	}



	// get y-values
	//y0 = Buffer [ Coord0 ].y;
	//y1 = Buffer [ Coord1 ].y;
	//y2 = Buffer [ Coord2 ].y;
	
	///////////////////////////////////
	// put top coordinates in x0,y0
	//if ( y1 < y0 )
	if ( gy [ Coord1 ] < gy [ Coord0 ] )
	{
		//Swap ( y0, y1 );
		Swap ( Coord0, Coord1 );
	}
	
	//if ( y2 < y0 )
	if ( gy [ Coord2 ] < gy [ Coord0 ] )
	{
		//Swap ( y0, y2 );
		Swap ( Coord0, Coord2 );
	}
	
	///////////////////////////////////////
	// put middle coordinates in x1,y1
	//if ( y2 < y1 )
	if ( gy [ Coord2 ] < gy [ Coord1 ] )
	{
		//Swap ( y1, y2 );
		Swap ( Coord1, Coord2 );
	}
	
	// get x-values
	x0 = gx [ Coord0 ];
	x1 = gx [ Coord1 ];
	x2 = gx [ Coord2 ];

	// get y-values
	y0 = gy [ Coord0 ];
	y1 = gy [ Coord1 ];
	y2 = gy [ Coord2 ];
	
	// get rgb-values
	r0 = gr [ Coord0 ];
	r1 = gr [ Coord1 ];
	r2 = gr [ Coord2 ];
	g0 = gg [ Coord0 ];
	g1 = gg [ Coord1 ];
	g2 = gg [ Coord2 ];
	b0 = gb [ Coord0 ];
	b1 = gb [ Coord1 ];
	b2 = gb [ Coord2 ];
	
	// get texture coords
	u0 = gu [ Coord0 ];
	u1 = gu [ Coord1 ];
	u2 = gu [ Coord2 ];
	v0 = gv [ Coord0 ];
	v1 = gv [ Coord1 ];
	v2 = gv [ Coord2 ];


	////////////////////////////////
	// y1 starts on the left
	//Y1_OnLeft = 1;


	
	//////////////////////////////////////////
	// get coordinates on screen
	x0 = DrawArea_OffsetX + x0;
	y0 = DrawArea_OffsetY + y0;
	x1 = DrawArea_OffsetX + x1;
	y1 = DrawArea_OffsetY + y1;
	x2 = DrawArea_OffsetX + x2;
	y2 = DrawArea_OffsetY + y2;
	
	
	clut_xoffset = clut_x << 4;
	ptr_clut = & ( VRAM [ clut_y << 10 ] );
	ptr_texture = & ( VRAM [ ( tpage_tx << 6 ) + ( ( tpage_ty << 8 ) << 10 ) ] );
	
	
	//////////////////////////////////////////////////////
	// Get offset into color lookup table
	//u32 ClutOffset = ( clut_x << 4 ) + ( clut_y << 10 );
	
	/////////////////////////////////////////////////////////
	// Get offset into texture page
	//u32 TextureOffset = ( tpage_tx << 6 ) + ( ( tpage_ty << 8 ) << 10 );
	
	
	//u32 NibblesPerPixel;
	
	//if ( tpage_tp == 0 ) NibblesPerPixel = 1; else if ( tpage_tp == 1 ) NibblesPerPixel = 2; else NibblesPerPixel = 4;

	if ( tpage_tp == 0 )
	{
		And2 = 0xf;
		
#ifdef _ENABLE_SSE2_TRIANGLE_TEXTUREGRADIENT_NONTEMPLATE
		Shift1 = _mm_set_epi32 ( 0, 0, 0, 2 );
		Shift2 = _mm_set_epi32 ( 0, 0, 0, 2 );
		And1 = _mm_set1_epi16 ( 3 );
#else
		Shift1 = 2; Shift2 = 2;
		And1 = 3; And2 = 0xf;
#endif
	}
	else if ( tpage_tp == 1 )
	{
		And2 = 0xff;
		
#ifdef _ENABLE_SSE2_TRIANGLE_TEXTUREGRADIENT_NONTEMPLATE
		Shift1 = _mm_set_epi32 ( 0, 0, 0, 1 );
		Shift2 = _mm_set_epi32 ( 0, 0, 0, 3 );
		And1 = _mm_set1_epi16 ( 1 );
#else
		Shift1 = 1; Shift2 = 3;
		And1 = 1; And2 = 0xff;
#endif
	}
	
	
	
	// get the left/right most x
	LeftMostX = ( ( x0 < x1 ) ? x0 : x1 );
	LeftMostX = ( ( x2 < LeftMostX ) ? x2 : LeftMostX );
	RightMostX = ( ( x0 > x1 ) ? x0 : x1 );
	RightMostX = ( ( x2 > RightMostX ) ? x2 : RightMostX );

	// check if sprite is within draw area
	if ( RightMostX <= ((s32)DrawArea_TopLeftX) || LeftMostX > ((s32)DrawArea_BottomRightX) || y2 <= ((s32)DrawArea_TopLeftY) || y0 > ((s32)DrawArea_BottomRightY) ) return;
	
	// skip drawing if distance between vertices is greater than max allowed by GPU
	if ( ( _Abs( x1 - x0 ) > c_MaxPolygonWidth ) || ( _Abs( x2 - x1 ) > c_MaxPolygonWidth ) || ( y1 - y0 > c_MaxPolygonHeight ) || ( y2 - y1 > c_MaxPolygonHeight ) )
	{
		// skip drawing polygon
		return;
	}
	
	
	// calculate across
	t0 = y1 - y2;
	t1 = y0 - y2;
	denominator = ( ( x0 - x2 ) * t0 ) - ( ( x1 - x2 ) * t1 );
	if ( denominator )
	{
		//dR_across = ( ( (s64) ( ( ( r0 - r2 ) * t0 ) - ( ( r1 - r2 ) * t1 ) ) ) << 24 ) / denominator;
		//dG_across = ( ( (s64) ( ( ( g0 - g2 ) * t0 ) - ( ( g1 - g2 ) * t1 ) ) ) << 24 ) / denominator;
		//dB_across = ( ( (s64) ( ( ( b0 - b2 ) * t0 ) - ( ( b1 - b2 ) * t1 ) ) ) << 24 ) / denominator;
		//dU_across = ( ( (s64) ( ( ( u0 - u2 ) * t0 ) - ( ( u1 - u2 ) * t1 ) ) ) << 24 ) / denominator;
		//dV_across = ( ( (s64) ( ( ( v0 - v2 ) * t0 ) - ( ( v1 - v2 ) * t1 ) ) ) << 24 ) / denominator;
		
		denominator = ( 1LL << 48 ) / denominator;
		dR_across = ( ( (s64) ( ( ( r0 - r2 ) * t0 ) - ( ( r1 - r2 ) * t1 ) ) ) * denominator ) >> 24;
		dG_across = ( ( (s64) ( ( ( g0 - g2 ) * t0 ) - ( ( g1 - g2 ) * t1 ) ) ) * denominator ) >> 24;
		dB_across = ( ( (s64) ( ( ( b0 - b2 ) * t0 ) - ( ( b1 - b2 ) * t1 ) ) ) * denominator ) >> 24;
		dU_across = ( ( (s64) ( ( ( u0 - u2 ) * t0 ) - ( ( u1 - u2 ) * t1 ) ) ) * denominator ) >> 24;
		dV_across = ( ( (s64) ( ( ( v0 - v2 ) * t0 ) - ( ( v1 - v2 ) * t1 ) ) ) * denominator ) >> 24;
		
#ifdef _ENABLE_SSE2_TRIANGLE_TEXTUREGRADIENT_NONTEMPLATE
		vdU_across = _mm_set1_epi32 ( dU_across * 8 );
		vdV_across = _mm_set1_epi32 ( dV_across * 8 );
		vdR_across = _mm_set1_epi32 ( dR_across * 8 );
		vdG_across = _mm_set1_epi32 ( dG_across * 8 );
		vdB_across = _mm_set1_epi32 ( dB_across * 8 );
#endif
	}

	
	// get reciprocals
	if ( y1 - y0 ) r10 = ( 1LL << 48 ) / ((s64)( y1 - y0 ));
	if ( y2 - y0 ) r20 = ( 1LL << 48 ) / ((s64)( y2 - y0 ));
	if ( y2 - y1 ) r21 = ( 1LL << 48 ) / ((s64)( y2 - y1 ));
	
	
	/////////////////////////////////////////////////
	// draw top part of triangle
	

	///////////////////////////////////////////
	// start at y0
	//Line = y0;
	
	/////////////////////////////////////////////
	// init x on the left and right

	////////////////////////////////////
	// get slopes
	
	if ( y1 - y0 )
	{
		x_left = ( ((s64)x0) << 16 );
		x_right = x_left;
		
		U_left = ( ((s64)u0) << 24 );
		V_left = ( ((s64)v0) << 24 );
		//U_right = U_left;
		//V_right = V_left;
			
		R_left = ( ((s64)r0) << 24 );
		G_left = ( ((s64)g0) << 24 );
		B_left = ( ((s64)b0) << 24 );
		//R_right = R_left;
		//G_right = G_left;
		//B_right = B_left;
		
		if ( denominator < 0 )
		{
			//dx_left = (((s64)( x1 - x0 )) << 16 ) / ((s64)( y1 - y0 ));
			//dx_right = (((s64)( x2 - x0 )) << 16 ) / ((s64)( y2 - y0 ));
			dx_left = ( ((s64)( x1 - x0 )) * r10 ) >> 32;
			dx_right = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
			
			//dU_left = (((s64)( u1 - u0 )) << 24 ) / ((s64)( y1 - y0 ));
			//dV_left = (((s64)( v1 - v0 )) << 24 ) / ((s64)( y1 - y0 ));
			dU_left = ( ((s64)( u1 - u0 )) * r10 ) >> 24;
			dV_left = ( ((s64)( v1 - v0 )) * r10 ) >> 24;

			//dR_left = (((s64)( r1 - r0 )) << 24 ) / ((s64)( y1 - y0 ));
			//dG_left = (((s64)( g1 - g0 )) << 24 ) / ((s64)( y1 - y0 ));
			//dB_left = (((s64)( b1 - b0 )) << 24 ) / ((s64)( y1 - y0 ));
			dR_left = ( ((s64)( r1 - r0 )) * r10 ) >> 24;
			dG_left = ( ((s64)( g1 - g0 )) * r10 ) >> 24;
			dB_left = ( ((s64)( b1 - b0 )) * r10 ) >> 24;
		}
		else
		{
			//dx_left = (((s64)( x1 - x0 )) << 16 ) / ((s64)( y1 - y0 ));
			//dx_right = (((s64)( x2 - x0 )) << 16 ) / ((s64)( y2 - y0 ));
			dx_right = ( ((s64)( x1 - x0 )) * r10 ) >> 32;
			dx_left = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
			
			//dU_right = (((s64)( u2 - u0 )) << 24 ) / ((s64)( y2 - y0 ));
			//dV_right = (((s64)( v2 - v0 )) << 24 ) / ((s64)( y2 - y0 ));
			dU_left = ( ((s64)( u2 - u0 )) * r20 ) >> 24;
			dV_left = ( ((s64)( v2 - v0 )) * r20 ) >> 24;
			
			//dR_right = (((s64)( r2 - r0 )) << 24 ) / ((s64)( y2 - y0 ));
			//dG_right = (((s64)( g2 - g0 )) << 24 ) / ((s64)( y2 - y0 ));
			//dB_right = (((s64)( b2 - b0 )) << 24 ) / ((s64)( y2 - y0 ));
			dR_left = ( ((s64)( r2 - r0 )) * r20 ) >> 24;
			dG_left = ( ((s64)( g2 - g0 )) * r20 ) >> 24;
			dB_left = ( ((s64)( b2 - b0 )) * r20 ) >> 24;
		}
	}
	else
	{
		
		if ( denominator < 0 )
		{
			// change x_left and x_right where y1 is on left
			x_left = ( ((s64)x1) << 16 );
			x_right = ( ((s64)x0) << 16 );
			
			//dx_left = (((s64)( x2 - x1 )) << 16 ) / ((s64)( y2 - y1 ));
			//dx_right = (((s64)( x2 - x0 )) << 16 ) / ((s64)( y2 - y0 ));
			dx_left = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
			dx_right = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
			
			// change U_left and V_right where y1 is on left
			U_left = ( ((s64)u1) << 24 );
			V_left = ( ((s64)v1) << 24 );
			
			R_left = ( ((s64)r1) << 24 );
			G_left = ( ((s64)g1) << 24 );
			B_left = ( ((s64)b1) << 24 );
		
			//dU_left = (((s64)( u2 - u1 )) << 24 ) / ((s64)( y2 - y1 ));
			//dV_left = (((s64)( v2 - v1 )) << 24 ) / ((s64)( y2 - y1 ));
			dU_left = ( ((s64)( u2 - u1 )) * r21 ) >> 24;
			dV_left = ( ((s64)( v2 - v1 )) * r21 ) >> 24;
			
			//dR_left = (((s64)( r2 - r1 )) << 24 ) / ((s64)( y2 - y1 ));
			//dG_left = (((s64)( g2 - g1 )) << 24 ) / ((s64)( y2 - y1 ));
			//dB_left = (((s64)( b2 - b1 )) << 24 ) / ((s64)( y2 - y1 ));
			dR_left = ( ((s64)( r2 - r1 )) * r21 ) >> 24;
			dG_left = ( ((s64)( g2 - g1 )) * r21 ) >> 24;
			dB_left = ( ((s64)( b2 - b1 )) * r21 ) >> 24;
		}
		else
		{
			// change x_left and x_right where y1 is on left
			x_right = ( ((s64)x1) << 16 );
			x_left = ( ((s64)x0) << 16 );
			
			//dx_left = (((s64)( x2 - x1 )) << 16 ) / ((s64)( y2 - y1 ));
			//dx_right = (((s64)( x2 - x0 )) << 16 ) / ((s64)( y2 - y0 ));
			dx_right = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
			dx_left = ( ((s64)( x2 - x0 )) * r20 ) >> 32;
			
			U_left = ( ((s64)u0) << 24 );
			V_left = ( ((s64)v0) << 24 );
			
			R_left = ( ((s64)r0) << 24 );
			G_left = ( ((s64)g0) << 24 );
			B_left = ( ((s64)b0) << 24 );
			
			//dU_right = (((s64)( u2 - u0 )) << 24 ) / ((s64)( y2 - y0 ));
			//dV_right = (((s64)( v2 - v0 )) << 24 ) / ((s64)( y2 - y0 ));
			dU_left = ( ((s64)( u2 - u0 )) * r20 ) >> 24;
			dV_left = ( ((s64)( v2 - v0 )) * r20 ) >> 24;
			
			//dR_right = (((s64)( r2 - r0 )) << 24 ) / ((s64)( y2 - y0 ));
			//dG_right = (((s64)( g2 - g0 )) << 24 ) / ((s64)( y2 - y0 ));
			//dB_right = (((s64)( b2 - b0 )) << 24 ) / ((s64)( y2 - y0 ));
			dR_left = ( ((s64)( r2 - r0 )) * r20 ) >> 24;
			dG_left = ( ((s64)( g2 - g0 )) * r20 ) >> 24;
			dB_left = ( ((s64)( b2 - b0 )) * r20 ) >> 24;
		}
	}

	
	
	/////////////////////////////////////////////////
	// swap left and right if they are reversed
	//if ( ( ( x_right + dx_right ) < ( x_left + dx_left ) ) || ( x_right < x_left ) )
	/*
	if ( denominator > 0 )
	{
		// x1,y1 is on the right //
		
		//Y1_OnLeft = 0;
		
		Swap ( x_left, x_right );
		Swap ( dx_left, dx_right );

		Swap ( dU_left, dU_right );
		Swap ( dV_left, dV_right );

		Swap ( U_left, U_right );
		Swap ( V_left, V_right );
		
		Swap ( dR_left, dR_right );
		Swap ( dG_left, dG_right );
		Swap ( dB_left, dB_right );

		Swap ( R_left, R_right );
		Swap ( G_left, G_right );
		Swap ( B_left, B_right );
	}
	*/

	
	// r,g,b,u,v values are not specified with a fractional part, so there must be an initial fractional part
	R_left |= ( 1 << 23 );
	G_left |= ( 1 << 23 );
	B_left |= ( 1 << 23 );
	U_left |= ( 1 << 23 );
	V_left |= ( 1 << 23 );
	
	
	StartY = y0;
	EndY = y1;

	if ( StartY < ((s32)DrawArea_TopLeftY) )
	{
		
		if ( EndY < ((s32)DrawArea_TopLeftY) )
		{
			Temp = EndY - StartY;
			StartY = EndY;
		}
		else
		{
			Temp = DrawArea_TopLeftY - StartY;
			StartY = DrawArea_TopLeftY;
		}
		
		x_left += dx_left * Temp;
		x_right += dx_right * Temp;
		
		U_left += dU_left * Temp;
		V_left += dV_left * Temp;
		R_left += dR_left * Temp;
		G_left += dG_left * Temp;
		B_left += dB_left * Temp;
		
		//U_right += dU_right * Temp;
		//V_right += dV_right * Temp;
		//R_right += dR_right * Temp;
		//G_right += dG_right * Temp;
		//B_right += dB_right * Temp;
	}
	
	if ( EndY > ((s32)DrawArea_BottomRightY) )
	{
		EndY = DrawArea_BottomRightY + 1;
	}

#ifdef _ENABLE_SSE2_TRIANGLE_TEXTUREGRADIENT_NONTEMPLATE
	vSeq32_u1 = _custom_mul_32 ( _mm_set1_epi32 ( dU_across ), vSeq32_1 );
	vSeq32_u2 = _custom_mul_32 ( _mm_set1_epi32 ( dU_across ), vSeq32_2 );
	vSeq32_v1 = _custom_mul_32 ( _mm_set1_epi32 ( dV_across ), vSeq32_1 );
	vSeq32_v2 = _custom_mul_32 ( _mm_set1_epi32 ( dV_across ), vSeq32_2 );
	vSeq32_r1 = _custom_mul_32 ( _mm_set1_epi32 ( dR_across ), vSeq32_1 );
	vSeq32_r2 = _custom_mul_32 ( _mm_set1_epi32 ( dR_across ), vSeq32_2 );
	vSeq32_g1 = _custom_mul_32 ( _mm_set1_epi32 ( dG_across ), vSeq32_1 );
	vSeq32_g2 = _custom_mul_32 ( _mm_set1_epi32 ( dG_across ), vSeq32_2 );
	vSeq32_b1 = _custom_mul_32 ( _mm_set1_epi32 ( dB_across ), vSeq32_1 );
	vSeq32_b2 = _custom_mul_32 ( _mm_set1_epi32 ( dB_across ), vSeq32_2 );
#endif
	
	if ( EndY > StartY )
	{
	
	//////////////////////////////////////////////
	// draw down to y1
	for ( Line = StartY; Line < EndY; Line++ )
	{
		//StartX = _Round( x_left ) >> 32;
		//EndX = _Round( x_right ) >> 32;
		
		// left point is included if points are equal
		StartX = ( x_left + 0xffffLL ) >> 16;
		EndX = ( x_right - 1 ) >> 16;
		
		
		if ( StartX <= ((s32)DrawArea_BottomRightX) && EndX >= ((s32)DrawArea_TopLeftX) && EndX >= StartX )
		{
			iU = U_left;
			iV = V_left;
			
			
			
			iR = R_left;
			iG = G_left;
			iB = B_left;

			// get the difference between x_left and StartX
			Temp = ( StartX << 16 ) - x_left;
			
			if ( StartX < ((s32)DrawArea_TopLeftX) )
			{
				Temp += ( DrawArea_TopLeftX - StartX ) << 16;
				StartX = DrawArea_TopLeftX;
				
				//iU += dU_across * Temp;
				//iV += dV_across * Temp;
				
				//iR += dR_across * Temp;
				//iG += dG_across * Temp;
				//iB += dB_across * Temp;
			}

			iU += ( dU_across >> 12 ) * ( Temp >> 4 );
			iV += ( dV_across >> 12 ) * ( Temp >> 4 );
			
			iR += ( dR_across >> 12 ) * ( Temp >> 4 );
			iG += ( dG_across >> 12 ) * ( Temp >> 4 );
			iB += ( dB_across >> 12 ) * ( Temp >> 4 );
			
			if ( EndX > ((s32)DrawArea_BottomRightX) )
			{
				//EndX = DrawArea_BottomRightX + 1;
				EndX = DrawArea_BottomRightX;
			}
			
			ptr = & ( VRAM [ StartX + ( Line << 10 ) ] );
			DitherLine = & ( DitherArray [ ( Line & 0x3 ) << 2 ] );

			
			/////////////////////////////////////////////////////
			// update number of cycles used to draw polygon
			NumberOfPixelsDrawn += EndX - StartX + 1;

#ifdef _ENABLE_SSE2_TRIANGLE_TEXTUREGRADIENT_NONTEMPLATE
	vx_across = _mm_add_epi16 ( vSeq, _mm_set1_epi16 ( StartX ) );
	vEndX = _mm_set1_epi16 ( EndX + 1 );
	
	//viU1 = _mm_add_epi32 ( _mm_set1_epi32 ( iU ), _custom_mul_32 ( _mm_set1_epi32 ( dU_across ), vSeq1_32 ) );
	//viU2 = _mm_add_epi32 ( _mm_set1_epi32 ( iU ), _custom_mul_32 ( _mm_set1_epi32 ( dU_across ), vSeq2_32 ) );
	//viV1 = _mm_add_epi32 ( _mm_set1_epi32 ( iV ), _custom_mul_32 ( _mm_set1_epi32 ( dV_across ), vSeq1_32 ) );
	//viV2 = _mm_add_epi32 ( _mm_set1_epi32 ( iV ), _custom_mul_32 ( _mm_set1_epi32 ( dV_across ), vSeq2_32 ) );
	//viR1 = _mm_add_epi32 ( _mm_set1_epi32 ( iR ), _custom_mul_32 ( _mm_set1_epi32 ( dR_across ), vSeq1_32 ) );
	//viR2 = _mm_add_epi32 ( _mm_set1_epi32 ( iR ), _custom_mul_32 ( _mm_set1_epi32 ( dR_across ), vSeq2_32 ) );
	//viG1 = _mm_add_epi32 ( _mm_set1_epi32 ( iG ), _custom_mul_32 ( _mm_set1_epi32 ( dG_across ), vSeq1_32 ) );
	//viG2 = _mm_add_epi32 ( _mm_set1_epi32 ( iG ), _custom_mul_32 ( _mm_set1_epi32 ( dG_across ), vSeq2_32 ) );
	//viB1 = _mm_add_epi32 ( _mm_set1_epi32 ( iB ), _custom_mul_32 ( _mm_set1_epi32 ( dB_across ), vSeq1_32 ) );
	//viB2 = _mm_add_epi32 ( _mm_set1_epi32 ( iB ), _custom_mul_32 ( _mm_set1_epi32 ( dB_across ), vSeq2_32 ) );

	viU1 = _mm_add_epi32 ( _mm_set1_epi32 ( iU ), vSeq32_u1 );
	viU2 = _mm_add_epi32 ( _mm_set1_epi32 ( iU ), vSeq32_u2 );
	viV1 = _mm_add_epi32 ( _mm_set1_epi32 ( iV ), vSeq32_v1 );
	viV2 = _mm_add_epi32 ( _mm_set1_epi32 ( iV ), vSeq32_v2 );
	viR1 = _mm_add_epi32 ( _mm_set1_epi32 ( iR ), vSeq32_r1 );
	viR2 = _mm_add_epi32 ( _mm_set1_epi32 ( iR ), vSeq32_r2 );
	viG1 = _mm_add_epi32 ( _mm_set1_epi32 ( iG ), vSeq32_g1 );
	viG2 = _mm_add_epi32 ( _mm_set1_epi32 ( iG ), vSeq32_g2 );
	viB1 = _mm_add_epi32 ( _mm_set1_epi32 ( iB ), vSeq32_b1 );
	viB2 = _mm_add_epi32 ( _mm_set1_epi32 ( iB ), vSeq32_b2 );
	
	vDitherValue_add = _mm_loadu_si128 ((__m128i const*) ( &(vDitherArray_add [ ( StartX & 0x3 ) + ( ( Line & 0x3 ) << 4 ) ]) ));
	vDitherValue_sub = _mm_loadu_si128 ((__m128i const*) ( &(vDitherArray_sub [ ( StartX & 0x3 ) + ( ( Line & 0x3 ) << 4 ) ]) ));
#endif

			// draw horizontal line
			// x_left and x_right need to be rounded off
			for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize )
			{
#ifdef _ENABLE_SSE2_TRIANGLE_TEXTUREGRADIENT_NONTEMPLATE
				viV =  _mm_packs_epi32 ( _mm_srli_epi32 ( _mm_add_epi32 ( viV1, vRound24 ), 24 ), _mm_srli_epi32 ( _mm_add_epi32 ( viV2, vRound24 ), 24 ) );
				viU =  _mm_packs_epi32 ( _mm_srli_epi32 ( _mm_add_epi32 ( viU1, vRound24 ), 24 ), _mm_srli_epi32 ( _mm_add_epi32 ( viU2, vRound24 ), 24 ) );
				
				TexCoordY = _mm_and_si128 ( _mm_or_si128 ( _mm_and_si128 ( viV, Not_TWH ), TWYTWH ), Mask );
				TexCoordX = _mm_and_si128 ( _mm_or_si128 ( _mm_and_si128 ( viU, Not_TWW ), TWXTWW ), Mask );
				//vIndex1 = _mm_srl_epi16 ( TexCoordX, Shift1 );
				if ( And2 )
				{
					vIndex2 = _mm_sll_epi16 ( _mm_and_si128 ( TexCoordX, And1 ), Shift2 );
					
					vIndex1 = _mm_srl_epi16 ( TexCoordX, Shift1 );
					
					_mm_store_si128 ( (__m128i*) TexCoordXList, vIndex1 );
					_mm_store_si128 ( (__m128i*) TempList, vIndex2 );
					_mm_store_si128 ( (__m128i*) TexCoordYList, TexCoordY );
					
					// get number of pixels remaining to draw
					uTemp32 = EndX - x_across;
					
					// only get full 8 pixels if there are 8 or more left to draw, otherwize get remaining to draw
					uTemp32 = ( ( uTemp32 > 7 ) ? 7 : uTemp32 );
					
					for ( uIndex32 = 0; uIndex32 <= uTemp32; uIndex32++ )
					{
						bgr = ptr_texture [ TexCoordXList [ uIndex32 ] + ( ( (u32) TexCoordYList [ uIndex32 ] ) << 10 ) ];
						bgr = ptr_clut [ ( clut_xoffset + ( ( bgr >> TempList [ uIndex32 ] ) & And2 ) ) & FrameBuffer_XMask ];
						TexCoordXList [ uIndex32 ] = bgr;
					}
					
					vbgr = _mm_load_si128 ( (const __m128i*) TexCoordXList );
					
					
					/*
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 0 ) + ( _mm_extract_epi16 ( TexCoordY, 0 ) << 10 ) ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 0 ) ) & And2 ) ) & FrameBuffer_XMask ], 0 );
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 1 ) + ( _mm_extract_epi16 ( TexCoordY, 1 ) << 10 ) ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 1 ) ) & And2 ) ) & FrameBuffer_XMask ], 1 );
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 2 ) + ( _mm_extract_epi16 ( TexCoordY, 2 ) << 10 ) ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 2 ) ) & And2 ) ) & FrameBuffer_XMask ], 2 );
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 3 ) + ( _mm_extract_epi16 ( TexCoordY, 3 ) << 10 ) ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 3 ) ) & And2 ) ) & FrameBuffer_XMask ], 3 );
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 4 ) + ( _mm_extract_epi16 ( TexCoordY, 4 ) << 10 ) ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 4 ) ) & And2 ) ) & FrameBuffer_XMask ], 4 );
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 5 ) + ( _mm_extract_epi16 ( TexCoordY, 5 ) << 10 ) ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 5 ) ) & And2 ) ) & FrameBuffer_XMask ], 5 );
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 6 ) + ( _mm_extract_epi16 ( TexCoordY, 6 ) << 10 ) ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 6 ) ) & And2 ) ) & FrameBuffer_XMask ], 6 );
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 7 ) + ( _mm_extract_epi16 ( TexCoordY, 7 ) << 10 ) ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 7 ) ) & And2 ) ) & FrameBuffer_XMask ], 7 );
					*/
				}
				else
				{
					_mm_store_si128 ( (__m128i*) TexCoordXList, TexCoordX );
					_mm_store_si128 ( (__m128i*) TexCoordYList, TexCoordY );
					
					// get number of pixels remaining to draw
					uTemp32 = EndX - x_across;
					
					// only get full 8 pixels if there are 8 or more left to draw, otherwize get remaining to draw
					uTemp32 = ( ( uTemp32 > 7 ) ? 7 : uTemp32 );
					
					for ( uIndex32 = 0; uIndex32 <= uTemp32; uIndex32++ )
					{
						TexCoordXList [ uIndex32 ] = ptr_texture [ TexCoordXList [ uIndex32 ] + ( ( (u32) TexCoordYList [ uIndex32 ] ) << 10 ) ];
					}
					
					vbgr = _mm_load_si128 ( (const __m128i*) TexCoordXList );
				
					/*
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 0 ) + ( _mm_extract_epi16 ( TexCoordY, 0 ) << 10 ) ], 0 );
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 1 ) + ( _mm_extract_epi16 ( TexCoordY, 1 ) << 10 ) ], 1 );
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 2 ) + ( _mm_extract_epi16 ( TexCoordY, 2 ) << 10 ) ], 2 );
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 3 ) + ( _mm_extract_epi16 ( TexCoordY, 3 ) << 10 ) ], 3 );
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 4 ) + ( _mm_extract_epi16 ( TexCoordY, 4 ) << 10 ) ], 4 );
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 5 ) + ( _mm_extract_epi16 ( TexCoordY, 5 ) << 10 ) ], 5 );
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 6 ) + ( _mm_extract_epi16 ( TexCoordY, 6 ) << 10 ) ], 6 );
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 7 ) + ( _mm_extract_epi16 ( TexCoordY, 7 ) << 10 ) ], 7 );
					*/
				}
#else
				
				//iX = x_across;
				//cx = iX;
			
				// make sure we are putting pixel within draw area
				//if ( x_across >= ((s32)DrawArea_TopLeftX) && x_across <= ((s32)DrawArea_BottomRightX) )
				//{
					//color_add = ( _Round( iR ) >> 35 ) | ( ( _Round( iG ) >> 35 ) << 5 ) | ( ( _Round( iB ) >> 35 ) << 10 );
					//color_add = ( _Round( iR ) >> 32 ) | ( ( _Round( iG ) >> 32 ) << 8 ) | ( ( _Round( iB ) >> 32 ) << 16 );
					//DitherValue = DitherLine [ x_across & 0x3 ] << 4;
					DitherValue = DitherLine [ x_across & 0x3 ];

					//TexCoordY = (u8) ( ( ( _Round24 ( iV ) >> 24 ) & Not_TWH ) | TWYTWH );
					//TexCoordX = (u8) ( ( ( _Round24 ( iU ) >> 24 ) & Not_TWW ) | TWXTWW );
					TexCoordY = (u8) ( ( ( iV >> 24 ) & Not_TWH ) | TWYTWH );
					TexCoordX = (u8) ( ( ( iU >> 24 ) & Not_TWW ) | TWXTWW );
					
					//bgr = VRAM [ TextureOffset + ( TexCoordX >> Shift1 ) + ( TexCoordY << 10 ) ];
					bgr = ptr_texture [ ( TexCoordX >> Shift1 ) + ( TexCoordY << 10 ) ];
					
					if ( Shift1 )
					{
						TexelIndex = ( bgr >> ( ( TexCoordX & And1 ) << Shift2 ) ) & And2;
						//bgr = VRAM [ ( ( ( clut_x << 4 ) + TexelIndex ) & FrameBuffer_XMask ) + ( clut_y << 10 ) ];
						bgr = ptr_clut [ ( clut_xoffset + TexelIndex ) & FrameBuffer_XMask ];
					}
#endif

					
#ifdef _ENABLE_SSE2_TRIANGLE_TEXTUREGRADIENT_NONTEMPLATE
					DestPixel = _mm_loadu_si128 ((__m128i const*) ptr);
					vbgr_temp = vbgr;
					if ( !command_tge )
					{
						vRed = _mm_packs_epi32 ( _mm_srai_epi32 ( viR1, 16 ), _mm_srai_epi32 ( viR2, 16 ) );
						vRed = _mm_adds_epu16 ( vRed, vDitherValue_add );
						vRed = _mm_subs_epu16 ( vRed, vDitherValue_sub );
						
						vGreen = _mm_packs_epi32 ( _mm_srai_epi32 ( viG1, 16 ), _mm_srai_epi32 ( viG2, 16 ) );
						vGreen = _mm_adds_epu16 ( vGreen, vDitherValue_add );
						vGreen = _mm_subs_epu16 ( vGreen, vDitherValue_sub );
						
						vBlue = _mm_packs_epi32 ( _mm_srai_epi32 ( viB1, 16 ), _mm_srai_epi32 ( viB2, 16 ) );
						vBlue = _mm_adds_epu16 ( vBlue, vDitherValue_add );
						vBlue = _mm_subs_epu16 ( vBlue, vDitherValue_sub );
						
						vRed = _mm_srli_epi16 ( vRed, 8 );
						vGreen = _mm_srli_epi16 ( vGreen, 8 );
						vBlue = _mm_srli_epi16 ( vBlue, 8 );
						vbgr_temp = vColorMultiply1624 ( vbgr_temp, vRed, vGreen, vBlue );
					}
					if ( command_abe )
					{
						vbgr_temp_transparent = vSemiTransparency16( DestPixel, vbgr_temp, tpage_abr );
						vbgr_select = _mm_srai_epi16 ( vbgr, 15 );
						vbgr_temp = _mm_or_si128 ( _mm_andnot_si128( vbgr_select, vbgr_temp ), _mm_and_si128 ( vbgr_select, vbgr_temp_transparent ) );
					}
					vbgr_temp = _mm_or_si128 ( _mm_or_si128 ( vbgr_temp, SetPixelMask ), _mm_and_si128 ( vbgr, tMask ) );
					_mm_maskmoveu_si128 ( vbgr_temp, _mm_andnot_si128 ( _mm_and_si128 ( _mm_srai_epi16 ( DestPixel, 15 ), PixelMask ), _mm_andnot_si128 ( _mm_cmpeq_epi16 ( vbgr, _mm_setzero_si128 () ), _mm_cmplt_epi16 ( vx_across, vEndX ) ) ), (char*) ptr );
					
					vx_across = _mm_add_epi16 ( vx_across, vVectorSize );

					viU1 = _mm_add_epi32 ( viU1, vdU_across );
					viU2 = _mm_add_epi32 ( viU2, vdU_across );
					viV1 = _mm_add_epi32 ( viV1, vdV_across );
					viV2 = _mm_add_epi32 ( viV2, vdV_across );
					
					viR1 = _mm_add_epi32 ( viR1, vdR_across );
					viR2 = _mm_add_epi32 ( viR2, vdR_across );
					viG1 = _mm_add_epi32 ( viG1, vdG_across );
					viG2 = _mm_add_epi32 ( viG2, vdG_across );
					viB1 = _mm_add_epi32 ( viB1, vdB_across );
					viB2 = _mm_add_epi32 ( viB2, vdB_across );
#else
					if ( bgr )
					{
						// shade pixel color
					
						// read pixel from frame buffer if we need to check mask bit
						//DestPixel = VRAM [ cx + ( cy << 10 ) ];
						DestPixel = *ptr;
						
						bgr_temp = bgr;
			
						if ( !command_tge )
						{
#ifdef NEW_PIXEL_SHADING
							Red = ( ( ( iR >> 24 ) | ( iB >> 8 ) ) & 0xff00ff ) * ( ( bgr_temp & 0x1f ) | ( bgr_temp << 5 ) & 0x1f0000 );
							Green = ( iG >> 24 ) * ( ( bgr_temp >> 5 ) & 0x1f );
							
							// Compose
							Red = ( ( Red >> 5 ) & 0xff ) | ( ( Red >> 29 ) & 0xff00 ) | ( ( Green << 11 ) & 0xff0000 );
							
							// add
							AddSignedClampC8 ( Red, DitherValue_Add );
							
							// sub
							SubSignedClampC8 ( Red, DitherValue_Sub );
							
							bgr_temp = ( ( Red >> 3 ) & 0x1f ) | ( ( Red >> 6 ) & 0x3e0 ) | ( ( Red >> 9 ) & 0x7c00 );
#else
							// shade pixel
							Red = ( ( (s16) ( iR >> 24 ) ) * ( (s16) ( bgr_temp & 0x1f ) ) );
							Green = ( ( (s16) ( iG >> 24 ) ) * ( (s16) ( ( bgr_temp >> 5 ) & 0x1f ) ) );
							Blue = ( ( (s16) ( iB >> 24 ) ) * ( (s16) ( ( bgr_temp >> 10 ) & 0x1f ) ) );
						
							// apply dithering if it is enabled
							// dithering must be applied after the color multiply
							Red = Red + DitherValue;
							Green = Green + DitherValue;
							Blue = Blue + DitherValue;
							
							// clamp
							//Red = Clamp5 ( Red >> 7 );
							//Green = Clamp5 ( Green >> 7 );
							//Blue = Clamp5 ( Blue >> 7 );
							Red = SignedClamp<s16,5> ( Red >> 7 );
							Green = SignedClamp<s16,5> ( Green >> 7 );
							Blue = SignedClamp<s16,5> ( Blue >> 7 );
							
							// combine
							bgr_temp = ( Blue << 10 ) | ( Green << 5 ) | ( Red );
#endif
						}
						
						
						// semi-transparency
						if ( command_abe && ( bgr & 0x8000 ) )
						{
							bgr_temp = SemiTransparency16 ( DestPixel, bgr_temp, tpage_abr );
						}
						
						// check if we should set mask bit when drawing
						//if ( GPU_CTRL_Read.MD ) bgr_temp |= 0x8000;
						bgr_temp |= SetPixelMask | ( bgr & 0x8000 );

						// draw pixel if we can draw to mask pixels or mask bit not set
						//if ( ! ( DestPixel & PixelMask ) ) VRAM [ cx + ( cy << 10 ) ] = bgr_temp;
						if ( ! ( DestPixel & PixelMask ) ) *ptr = bgr_temp;
					}
					
					/////////////////////////////////////////////////////
					// update number of cycles used to draw polygon
					//NumberOfPixelsDrawn++;
				//}
				
				/////////////////////////////////////////////////////////
				// interpolate texture coords
				iU += dU_across;
				iV += dV_across;
				
				iR += dR_across;
				iG += dG_across;
				iB += dB_across;
#endif
				
				ptr += c_iVectorSize;
			}
			
		}
		
		//////////////////////////////////
		// draw next line
		//Line++;
		
		/////////////////////////////////////
		// update x on left and right
		x_left += dx_left;
		x_right += dx_right;
		
		U_left += dU_left;
		V_left += dV_left;
		//U_right += dU_right;
		//V_right += dV_right;
		
		R_left += dR_left;
		G_left += dG_left;
		B_left += dB_left;
		//R_right += dR_right;
		//G_right += dG_right;
		//B_right += dB_right;
	}
	
	} // end if ( EndY > StartY )

	
	////////////////////////////////////////////////
	// draw bottom part of triangle

	/////////////////////////////////////////////
	// init x on the left and right
	
	//////////////////////////////////////////////////////
	// check if y1 is on the left or on the right
	//if ( Y1_OnLeft )
	if ( denominator < 0 )
	{
		x_left = ( ((s64)x1) << 16 );

		U_left = ( ((s64)u1) << 24 );
		V_left = ( ((s64)v1) << 24 );

		R_left = ( ((s64)r1) << 24 );
		G_left = ( ((s64)g1) << 24 );
		B_left = ( ((s64)b1) << 24 );
		
		// r,g,b,u,v values are not specified with a fractional part, so there must be an initial fractional part
		R_left |= ( 1 << 23 );
		G_left |= ( 1 << 23 );
		B_left |= ( 1 << 23 );
		U_left |= ( 1 << 23 );
		V_left |= ( 1 << 23 );
		
		//if ( y2 - y1 )
		//{
			//dx_left = (((s64)( x2 - x1 )) << 16 ) / ((s64)( y2 - y1 ));
			dx_left = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
			
			//dU_left = (((s64)( u2 - u1 )) << 24 ) / ((s64)( y2 - y1 ));
			//dV_left = (((s64)( v2 - v1 )) << 24 ) / ((s64)( y2 - y1 ));
			dU_left = ( ((s64)( u2 - u1 )) * r21 ) >> 24;
			dV_left = ( ((s64)( v2 - v1 )) * r21 ) >> 24;
			
			//dR_left = (((s64)( r2 - r1 )) << 24 ) / ((s64)( y2 - y1 ));
			//dG_left = (((s64)( g2 - g1 )) << 24 ) / ((s64)( y2 - y1 ));
			//dB_left = (((s64)( b2 - b1 )) << 24 ) / ((s64)( y2 - y1 ));
			dR_left = ( ((s64)( r2 - r1 )) * r21 ) >> 24;
			dG_left = ( ((s64)( g2 - g1 )) * r21 ) >> 24;
			dB_left = ( ((s64)( b2 - b1 )) * r21 ) >> 24;
		//}
	}
	else
	{
		x_right = ( ((s64)x1) << 16 );

		//U_right = ( ((s64)u1) << 24 );
		//V_right = ( ((s64)v1) << 24 );

		//R_right = ( ((s64)r1) << 24 );
		//G_right = ( ((s64)g1) << 24 );
		//B_right = ( ((s64)b1) << 24 );
		
		//if ( y2 - y1 )
		//{
			//dx_right = (((s64)( x2 - x1 )) << 16 ) / ((s64)( y2 - y1 ));
			dx_right = ( ((s64)( x2 - x1 )) * r21 ) >> 32;
			
			//dU_right = ( ((s64)( u2 - u1 )) * r21 ) >> 24;
			//dV_right = ( ((s64)( v2 - v1 )) * r21 ) >> 24;
			
			//dR_right = ( ((s64)( r2 - r1 )) * r21 ) >> 24;
			//dG_right = ( ((s64)( g2 - g1 )) * r21 ) >> 24;
			//dB_right = ( ((s64)( b2 - b1 )) * r21 ) >> 24;
		//}
	}
	
	// the line starts at y1 from here
	//Line = y1;

	StartY = y1;
	EndY = y2;

	if ( StartY < ((s32)DrawArea_TopLeftY) )
	{
		
		if ( EndY < ((s32)DrawArea_TopLeftY) )
		{
			Temp = EndY - StartY;
			StartY = EndY;
		}
		else
		{
			Temp = DrawArea_TopLeftY - StartY;
			StartY = DrawArea_TopLeftY;
		}
		
		x_left += dx_left * Temp;
		x_right += dx_right * Temp;
		
		U_left += dU_left * Temp;
		V_left += dV_left * Temp;
		R_left += dR_left * Temp;
		G_left += dG_left * Temp;
		B_left += dB_left * Temp;
		
		//U_right += dU_right * Temp;
		//V_right += dV_right * Temp;
		//R_right += dR_right * Temp;
		//G_right += dG_right * Temp;
		//B_right += dB_right * Temp;
	}
	
	if ( EndY > ((s32)DrawArea_BottomRightY) )
	{
		EndY = DrawArea_BottomRightY + 1;
	}


	if ( EndY > StartY )
	{
	
	//////////////////////////////////////////////
	// draw down to y2
	for ( Line = StartY; Line < EndY; Line++ )
	{
		//StartX = _Round( x_left ) >> 32;
		//EndX = _Round( x_right ) >> 32;
		
		// left point is included if points are equal
		StartX = ( x_left + 0xffffLL ) >> 16;
		EndX = ( x_right - 1 ) >> 16;
		
		
		if ( StartX <= ((s32)DrawArea_BottomRightX) && EndX >= ((s32)DrawArea_TopLeftX) && EndX >= StartX )
		{
			iU = U_left;
			iV = V_left;
			

			iR = R_left;
			iG = G_left;
			iB = B_left;

			// get the difference between x_left and StartX
			Temp = ( StartX << 16 ) - x_left;
			
			if ( StartX < ((s32)DrawArea_TopLeftX) )
			{
				Temp += ( DrawArea_TopLeftX - StartX ) << 16;
				StartX = DrawArea_TopLeftX;
				
				//iU += dU_across * Temp;
				//iV += dV_across * Temp;
				
				//iR += dR_across * Temp;
				//iG += dG_across * Temp;
				//iB += dB_across * Temp;
			}
			
			iU += ( dU_across >> 12 ) * ( Temp >> 4 );
			iV += ( dV_across >> 12 ) * ( Temp >> 4 );
			
			iR += ( dR_across >> 12 ) * ( Temp >> 4 );
			iG += ( dG_across >> 12 ) * ( Temp >> 4 );
			iB += ( dB_across >> 12 ) * ( Temp >> 4 );
			
			if ( EndX > ((s32)DrawArea_BottomRightX) )
			{
				//EndX = DrawArea_BottomRightX + 1;
				EndX = DrawArea_BottomRightX;
			}
			
			ptr = & ( VRAM [ StartX + ( Line << 10 ) ] );
			DitherLine = & ( DitherArray [ ( Line & 0x3 ) << 2 ] );
			
			
			/////////////////////////////////////////////////////
			// update number of cycles used to draw polygon
			NumberOfPixelsDrawn += EndX - StartX + 1;
			
#ifdef _ENABLE_SSE2_TRIANGLE_TEXTUREGRADIENT_NONTEMPLATE
	vx_across = _mm_add_epi16 ( vSeq, _mm_set1_epi16 ( StartX ) );
	vEndX = _mm_set1_epi16 ( EndX + 1 );
	
	//viU1 = _mm_add_epi32 ( _mm_set1_epi32 ( iU ), _custom_mul_32 ( _mm_set1_epi32 ( dU_across ), vSeq1_32 ) );
	//viU2 = _mm_add_epi32 ( _mm_set1_epi32 ( iU ), _custom_mul_32 ( _mm_set1_epi32 ( dU_across ), vSeq2_32 ) );
	//viV1 = _mm_add_epi32 ( _mm_set1_epi32 ( iV ), _custom_mul_32 ( _mm_set1_epi32 ( dV_across ), vSeq1_32 ) );
	//viV2 = _mm_add_epi32 ( _mm_set1_epi32 ( iV ), _custom_mul_32 ( _mm_set1_epi32 ( dV_across ), vSeq2_32 ) );
	//viR1 = _mm_add_epi32 ( _mm_set1_epi32 ( iR ), _custom_mul_32 ( _mm_set1_epi32 ( dR_across ), vSeq1_32 ) );
	//viR2 = _mm_add_epi32 ( _mm_set1_epi32 ( iR ), _custom_mul_32 ( _mm_set1_epi32 ( dR_across ), vSeq2_32 ) );
	//viG1 = _mm_add_epi32 ( _mm_set1_epi32 ( iG ), _custom_mul_32 ( _mm_set1_epi32 ( dG_across ), vSeq1_32 ) );
	//viG2 = _mm_add_epi32 ( _mm_set1_epi32 ( iG ), _custom_mul_32 ( _mm_set1_epi32 ( dG_across ), vSeq2_32 ) );
	//viB1 = _mm_add_epi32 ( _mm_set1_epi32 ( iB ), _custom_mul_32 ( _mm_set1_epi32 ( dB_across ), vSeq1_32 ) );
	//viB2 = _mm_add_epi32 ( _mm_set1_epi32 ( iB ), _custom_mul_32 ( _mm_set1_epi32 ( dB_across ), vSeq2_32 ) );
	
	viU1 = _mm_add_epi32 ( _mm_set1_epi32 ( iU ), vSeq32_u1 );
	viU2 = _mm_add_epi32 ( _mm_set1_epi32 ( iU ), vSeq32_u2 );
	viV1 = _mm_add_epi32 ( _mm_set1_epi32 ( iV ), vSeq32_v1 );
	viV2 = _mm_add_epi32 ( _mm_set1_epi32 ( iV ), vSeq32_v2 );
	viR1 = _mm_add_epi32 ( _mm_set1_epi32 ( iR ), vSeq32_r1 );
	viR2 = _mm_add_epi32 ( _mm_set1_epi32 ( iR ), vSeq32_r2 );
	viG1 = _mm_add_epi32 ( _mm_set1_epi32 ( iG ), vSeq32_g1 );
	viG2 = _mm_add_epi32 ( _mm_set1_epi32 ( iG ), vSeq32_g2 );
	viB1 = _mm_add_epi32 ( _mm_set1_epi32 ( iB ), vSeq32_b1 );
	viB2 = _mm_add_epi32 ( _mm_set1_epi32 ( iB ), vSeq32_b2 );
	
	vDitherValue_add = _mm_loadu_si128 ((__m128i const*) ( &(vDitherArray_add [ ( StartX & 0x3 ) + ( ( Line & 0x3 ) << 4 ) ]) ));
	vDitherValue_sub = _mm_loadu_si128 ((__m128i const*) ( &(vDitherArray_sub [ ( StartX & 0x3 ) + ( ( Line & 0x3 ) << 4 ) ]) ));
#endif
			
			// draw horizontal line
			// x_left and x_right need to be rounded off
			for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize )
			{
#ifdef _ENABLE_SSE2_TRIANGLE_TEXTUREGRADIENT_NONTEMPLATE
				viV =  _mm_packs_epi32 ( _mm_srli_epi32 ( _mm_add_epi32 ( viV1, vRound24 ), 24 ), _mm_srli_epi32 ( _mm_add_epi32 ( viV2, vRound24 ), 24 ) );
				viU =  _mm_packs_epi32 ( _mm_srli_epi32 ( _mm_add_epi32 ( viU1, vRound24 ), 24 ), _mm_srli_epi32 ( _mm_add_epi32 ( viU2, vRound24 ), 24 ) );
				
				TexCoordY = _mm_and_si128 ( _mm_or_si128 ( _mm_and_si128 ( viV, Not_TWH ), TWYTWH ), Mask );
				TexCoordX = _mm_and_si128 ( _mm_or_si128 ( _mm_and_si128 ( viU, Not_TWW ), TWXTWW ), Mask );
				//vIndex1 = _mm_srl_epi16 ( TexCoordX, Shift1 );
				if ( And2 )
				{
					vIndex2 = _mm_sll_epi16 ( _mm_and_si128 ( TexCoordX, And1 ), Shift2 );
					
					vIndex1 = _mm_srl_epi16 ( TexCoordX, Shift1 );
					
					_mm_store_si128 ( (__m128i*) TexCoordXList, vIndex1 );
					_mm_store_si128 ( (__m128i*) TempList, vIndex2 );
					_mm_store_si128 ( (__m128i*) TexCoordYList, TexCoordY );
					
					// get number of pixels remaining to draw
					uTemp32 = EndX - x_across;
					
					// only get full 8 pixels if there are 8 or more left to draw, otherwize get remaining to draw
					uTemp32 = ( ( uTemp32 > 7 ) ? 7 : uTemp32 );
					
					for ( uIndex32 = 0; uIndex32 <= uTemp32; uIndex32++ )
					{
						bgr = ptr_texture [ TexCoordXList [ uIndex32 ] + ( ( (u32) TexCoordYList [ uIndex32 ] ) << 10 ) ];
						bgr = ptr_clut [ ( clut_xoffset + ( ( bgr >> TempList [ uIndex32 ] ) & And2 ) ) & FrameBuffer_XMask ];
						TexCoordXList [ uIndex32 ] = bgr;
					}
					
					vbgr = _mm_load_si128 ( (const __m128i*) TexCoordXList );
					
					
					/*
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 0 ) + ( _mm_extract_epi16 ( TexCoordY, 0 ) << 10 ) ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 0 ) ) & And2 ) ) & FrameBuffer_XMask ], 0 );
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 1 ) + ( _mm_extract_epi16 ( TexCoordY, 1 ) << 10 ) ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 1 ) ) & And2 ) ) & FrameBuffer_XMask ], 1 );
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 2 ) + ( _mm_extract_epi16 ( TexCoordY, 2 ) << 10 ) ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 2 ) ) & And2 ) ) & FrameBuffer_XMask ], 2 );
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 3 ) + ( _mm_extract_epi16 ( TexCoordY, 3 ) << 10 ) ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 3 ) ) & And2 ) ) & FrameBuffer_XMask ], 3 );
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 4 ) + ( _mm_extract_epi16 ( TexCoordY, 4 ) << 10 ) ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 4 ) ) & And2 ) ) & FrameBuffer_XMask ], 4 );
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 5 ) + ( _mm_extract_epi16 ( TexCoordY, 5 ) << 10 ) ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 5 ) ) & And2 ) ) & FrameBuffer_XMask ], 5 );
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 6 ) + ( _mm_extract_epi16 ( TexCoordY, 6 ) << 10 ) ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 6 ) ) & And2 ) ) & FrameBuffer_XMask ], 6 );
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 7 ) + ( _mm_extract_epi16 ( TexCoordY, 7 ) << 10 ) ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 7 ) ) & And2 ) ) & FrameBuffer_XMask ], 7 );
					*/
				}
				else
				{
					_mm_store_si128 ( (__m128i*) TexCoordXList, TexCoordX );
					_mm_store_si128 ( (__m128i*) TexCoordYList, TexCoordY );
					
					// get number of pixels remaining to draw
					uTemp32 = EndX - x_across;
					
					// only get full 8 pixels if there are 8 or more left to draw, otherwize get remaining to draw
					uTemp32 = ( ( uTemp32 > 7 ) ? 7 : uTemp32 );
					
					for ( uIndex32 = 0; uIndex32 <= uTemp32; uIndex32++ )
					{
						TexCoordXList [ uIndex32 ] = ptr_texture [ TexCoordXList [ uIndex32 ] + ( ( (u32) TexCoordYList [ uIndex32 ] ) << 10 ) ];
					}
					
					vbgr = _mm_load_si128 ( (const __m128i*) TexCoordXList );
				
					/*
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 0 ) + ( _mm_extract_epi16 ( TexCoordY, 0 ) << 10 ) ], 0 );
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 1 ) + ( _mm_extract_epi16 ( TexCoordY, 1 ) << 10 ) ], 1 );
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 2 ) + ( _mm_extract_epi16 ( TexCoordY, 2 ) << 10 ) ], 2 );
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 3 ) + ( _mm_extract_epi16 ( TexCoordY, 3 ) << 10 ) ], 3 );
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 4 ) + ( _mm_extract_epi16 ( TexCoordY, 4 ) << 10 ) ], 4 );
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 5 ) + ( _mm_extract_epi16 ( TexCoordY, 5 ) << 10 ) ], 5 );
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 6 ) + ( _mm_extract_epi16 ( TexCoordY, 6 ) << 10 ) ], 6 );
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 7 ) + ( _mm_extract_epi16 ( TexCoordY, 7 ) << 10 ) ], 7 );
					*/
				}
#else
				
				//iX = x_across;
				//cx = iX;
			
				// make sure we are putting pixel within draw area
				//if ( x_across >= ((s32)DrawArea_TopLeftX) && x_across <= ((s32)DrawArea_BottomRightX) )
				//{
					//color_add = ( _Round( iR ) >> 35 ) | ( ( _Round( iG ) >> 35 ) << 5 ) | ( ( _Round( iB ) >> 35 ) << 10 );
					//color_add = ( _Round( iR ) >> 32 ) | ( ( _Round( iG ) >> 32 ) << 8 ) | ( ( _Round( iB ) >> 32 ) << 16 );
					DitherValue = DitherLine [ x_across & 0x3 ];

					//TexCoordY = (u8) ( ( ( _Round24 ( iV ) >> 24 ) & Not_TWH ) | TWYTWH );
					//TexCoordX = (u8) ( ( ( _Round24 ( iU ) >> 24 ) & Not_TWW ) | TWXTWW );
					TexCoordY = (u8) ( ( ( iV >> 24 ) & Not_TWH ) | TWYTWH );
					TexCoordX = (u8) ( ( ( iU >> 24 ) & Not_TWW ) | TWXTWW );
					
					//bgr = VRAM [ TextureOffset + ( TexCoordX >> Shift1 ) + ( TexCoordY << 10 ) ];
					bgr = ptr_texture [ ( TexCoordX >> Shift1 ) + ( TexCoordY << 10 ) ];
					
					if ( Shift1 )
					{
						TexelIndex = ( bgr >> ( ( TexCoordX & And1 ) << Shift2 ) ) & And2;
						//bgr = VRAM [ ( ( ( clut_x << 4 ) + TexelIndex ) & FrameBuffer_XMask ) + ( clut_y << 10 ) ];
						bgr = ptr_clut [ ( clut_xoffset + TexelIndex ) & FrameBuffer_XMask ];
					}
#endif


#ifdef _ENABLE_SSE2_TRIANGLE_TEXTUREGRADIENT_NONTEMPLATE
					DestPixel = _mm_loadu_si128 ((__m128i const*) ptr);
					vbgr_temp = vbgr;
					if ( !command_tge )
					{
						vRed = _mm_packs_epi32 ( _mm_srai_epi32 ( viR1, 16 ), _mm_srai_epi32 ( viR2, 16 ) );
						vRed = _mm_adds_epu16 ( vRed, vDitherValue_add );
						vRed = _mm_subs_epu16 ( vRed, vDitherValue_sub );
						
						vGreen = _mm_packs_epi32 ( _mm_srai_epi32 ( viG1, 16 ), _mm_srai_epi32 ( viG2, 16 ) );
						vGreen = _mm_adds_epu16 ( vGreen, vDitherValue_add );
						vGreen = _mm_subs_epu16 ( vGreen, vDitherValue_sub );
						
						vBlue = _mm_packs_epi32 ( _mm_srai_epi32 ( viB1, 16 ), _mm_srai_epi32 ( viB2, 16 ) );
						vBlue = _mm_adds_epu16 ( vBlue, vDitherValue_add );
						vBlue = _mm_subs_epu16 ( vBlue, vDitherValue_sub );
						
						vRed = _mm_srli_epi16 ( vRed, 8 );
						vGreen = _mm_srli_epi16 ( vGreen, 8 );
						vBlue = _mm_srli_epi16 ( vBlue, 8 );
						vbgr_temp = vColorMultiply1624 ( vbgr_temp, vRed, vGreen, vBlue );
					}
					if ( command_abe )
					{
						vbgr_temp_transparent = vSemiTransparency16( DestPixel, vbgr_temp, tpage_abr );
						vbgr_select = _mm_srai_epi16 ( vbgr, 15 );
						vbgr_temp = _mm_or_si128 ( _mm_andnot_si128( vbgr_select, vbgr_temp ), _mm_and_si128 ( vbgr_select, vbgr_temp_transparent ) );
					}
					vbgr_temp = _mm_or_si128 ( _mm_or_si128 ( vbgr_temp, SetPixelMask ), _mm_and_si128 ( vbgr, tMask ) );
					_mm_maskmoveu_si128 ( vbgr_temp, _mm_andnot_si128 ( _mm_and_si128 ( _mm_srai_epi16 ( DestPixel, 15 ), PixelMask ), _mm_andnot_si128 ( _mm_cmpeq_epi16 ( vbgr, _mm_setzero_si128 () ), _mm_cmplt_epi16 ( vx_across, vEndX ) ) ), (char*) ptr );
					
					vx_across = _mm_add_epi16 ( vx_across, vVectorSize );

					viU1 = _mm_add_epi32 ( viU1, vdU_across );
					viU2 = _mm_add_epi32 ( viU2, vdU_across );
					viV1 = _mm_add_epi32 ( viV1, vdV_across );
					viV2 = _mm_add_epi32 ( viV2, vdV_across );
					
					viR1 = _mm_add_epi32 ( viR1, vdR_across );
					viR2 = _mm_add_epi32 ( viR2, vdR_across );
					viG1 = _mm_add_epi32 ( viG1, vdG_across );
					viG2 = _mm_add_epi32 ( viG2, vdG_across );
					viB1 = _mm_add_epi32 ( viB1, vdB_across );
					viB2 = _mm_add_epi32 ( viB2, vdB_across );
#else

					if ( bgr )
					{
						// shade pixel color
					
						// read pixel from frame buffer if we need to check mask bit
						DestPixel = *ptr;
						
						bgr_temp = bgr;
			
						if ( !command_tge )
						{
							// shade pixel
							Red = ( ( (s16) ( iR >> 24 ) ) * ( (s16) ( bgr_temp & 0x1f ) ) );
							Green = ( ( (s16) ( iG >> 24 ) ) * ( (s16) ( ( bgr_temp >> 5 ) & 0x1f ) ) );
							Blue = ( ( (s16) ( iB >> 24 ) ) * ( (s16) ( ( bgr_temp >> 10 ) & 0x1f ) ) );
						
							// apply dithering if it is enabled
							// dithering must be applied after the color multiply
							Red = Red + DitherValue;
							Green = Green + DitherValue;
							Blue = Blue + DitherValue;
							
							// clamp
							//Red = Clamp5 ( Red >> 7 );
							//Green = Clamp5 ( Green >> 7 );
							//Blue = Clamp5 ( Blue >> 7 );
							Red = SignedClamp<s16,5> ( Red >> 7 );
							Green = SignedClamp<s16,5> ( Green >> 7 );
							Blue = SignedClamp<s16,5> ( Blue >> 7 );
							
							// combine
							bgr_temp = ( Blue << 10 ) | ( Green << 5 ) | ( Red );
						}

						// semi-transparency
						if ( command_abe && ( bgr & 0x8000 ) )
						{
							bgr_temp = SemiTransparency16 ( DestPixel, bgr_temp, tpage_abr );
						}
						
						// check if we should set mask bit when drawing
						//if ( GPU_CTRL_Read.MD ) bgr_temp |= 0x8000;
						bgr_temp |= SetPixelMask | ( bgr & 0x8000 );

						// draw pixel if we can draw to mask pixels or mask bit not set
						//if ( ! ( DestPixel & PixelMask ) ) VRAM [ cx + ( cy << 10 ) ] = bgr_temp;
						if ( ! ( DestPixel & PixelMask ) ) *ptr = bgr_temp;
					}
					
					/////////////////////////////////////////////////////
					// update number of cycles used to draw polygon
					//NumberOfPixelsDrawn++;
				//}
				
				/////////////////////////////////////////////////////////
				// interpolate texture coords
				iU += dU_across;
				iV += dV_across;
				
				iR += dR_across;
				iG += dG_across;
				iB += dB_across;
#endif
				
				ptr += c_iVectorSize;
			}
			
		}
		
		//////////////////////////////////
		// draw next line
		//Line++;
		
		/////////////////////////////////////
		// update x on left and right
		x_left += dx_left;
		x_right += dx_right;
		
		U_left += dU_left;
		V_left += dV_left;
		//U_right += dU_right;
		//V_right += dV_right;
		
		R_left += dR_left;
		G_left += dG_left;
		B_left += dB_left;
		//R_right += dR_right;
		//G_right += dG_right;
		//B_right += dB_right;
	}
	
	} // end if ( EndY > StartY )
		
}

#endif




///////////////////////////////////////////////////////////////////////////
// *** Sprite Drawing ***


#ifndef EXCLUDE_SPRITE_NONTEMPLATE

void GPU::DrawSprite ()
{
#ifdef _ENABLE_SSE2_SPRITE_NONTEMPLATE
	// with sse2, can send 8 pixels at a time
	static const int c_iVectorSize = 8;
#else
	// before sse2, was sending 1 pixels at a time
	static const int c_iVectorSize = 1;
#endif

	// notes: looks like sprite size is same as specified by w/h

	//u32 Pixel,
	
	u32 TexelIndex;
	
	u32 color_add;
	
	u16 *ptr_texture, *ptr_clut;
	u32 clut_xoffset, clut_yoffset;
	
	u16 *ptr;
	s32 StartX, EndX, StartY, EndY;
	
	u32 tge;
	
	u32 Temp, Index;
	
	// new local variables
	s32 x0, x1, y0, y1;
	s32 u0, v0;
	u32 bgr, bgr_temp;
	s64 iU, iV;
	s64 x_across;
	s32 Line;
	
#ifdef _ENABLE_SSE2_SPRITE_NONTEMPLATE
	__m128i DestPixel, PixelMask, SetPixelMask;
	__m128i vbgr, vbgr_temp, vStartX, vEndX, vx_across, vSeq, vVectorSize;
	__m128i vbgr_temp_transparent, vbgr_select;
	
	__m128i vSeq1_32, vSeq2_32;
	
	u32 TexCoordY;
	__m128i TexCoordX, Mask, tMask;	//, vTexCoordX2, vTexCoordY1, vTexCoordY2;
	u32 And2 = 0;
	__m128i And1, Shift1, Shift2;
	__m128i TextureOffset, vIndex1, vIndex2;
	__m128i viU, viV;
	__m128i vu;
	
	u32 TWYTWH, Not_TWH;
	__m128i TWXTWW, Not_TWW;
	__m128i color_add_r, color_add_g, color_add_b;
	
	color_add_r = _mm_set1_epi16 ( bgr & 0xff );
	color_add_g = _mm_set1_epi16 ( ( bgr >> 8 ) & 0xff );
	color_add_b = _mm_set1_epi16 ( ( bgr >> 16 ) & 0xff );
	
	Mask = _mm_set1_epi16 ( 0xff );
	tMask = _mm_set1_epi16 ( 0x8000 );
	
	vSeq1_32 = _mm_set_epi32 ( 3, 2, 1, 0 );
	vSeq2_32 = _mm_set_epi32 ( 7, 6, 5, 4 );
	
	TWYTWH = ( ( TWY & TWH ) << 3 );
	TWXTWW = _mm_set1_epi16 ( ( ( TWX & TWW ) << 3 ) );
	
	Not_TWH = ~( TWH << 3 );
	Not_TWW = _mm_set1_epi16 ( ~( TWW << 3 ) );
	
	And1 = _mm_setzero_si128 ();
	Shift1 = _mm_setzero_si128 ();
	
	TextureOffset = _mm_set1_epi32 ( ( tpage_tx << 6 ) + ( ( tpage_ty << 8 ) << 10 ) );
	
	vSeq = _mm_set_epi16 ( 7, 6, 5, 4, 3, 2, 1, 0 );
	vVectorSize = _mm_set1_epi16 ( c_iVectorSize );
	
	vbgr = _mm_set1_epi16 ( bgr );
	PixelMask = _mm_setzero_si128 ();
	SetPixelMask = _mm_setzero_si128 ();
	if ( GPU_CTRL_Read.ME ) PixelMask = _mm_set1_epi16 ( 0x8080 );
	if ( GPU_CTRL_Read.MD ) SetPixelMask = _mm_set1_epi16 ( 0x8000 );
	
	u16 TexCoordXList [ 8 ] __attribute__ ((aligned (32)));
	u16 TempList [ 8 ] __attribute__ ((aligned (32)));
#else
	u32 DestPixel, PixelMask = 0, SetPixelMask = 0;
	u32 TexCoordX, TexCoordY;
	u32 Shift1 = 0, Shift2 = 0, And1 = 0, And2 = 0;
	u32 TextureOffset;

	u32 TWYTWH, TWXTWW, Not_TWH, Not_TWW;
	
	TWYTWH = ( ( TWY & TWH ) << 3 );
	TWXTWW = ( ( TWX & TWW ) << 3 );
	
	Not_TWH = ~( TWH << 3 );
	Not_TWW = ~( TWW << 3 );

	
	if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
	if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;
	
	/////////////////////////////////////////////////////////
	// Get offset into texture page
	TextureOffset = ( tpage_tx << 6 ) + ( ( tpage_ty << 8 ) << 10 );
#endif
	
	u32 PixelsPerLine;
	
	// get the color
	bgr = gbgr [ 0 ];

	tge = command_tge;
	if ( ( bgr & 0x00ffffff ) == 0x00808080 ) tge = 1;
	
	//////////////////////////////////////////////////////
	// Get offset into color lookup table
	//u32 ClutOffset = ( clut_x << 4 ) + ( clut_y << 10 );
	
	clut_xoffset = clut_x << 4;
	ptr_clut = & ( VRAM [ clut_y << 10 ] );
	ptr_texture = & ( VRAM [ ( tpage_tx << 6 ) + ( ( tpage_ty << 8 ) << 10 ) ] );
	
	if ( tpage_tp == 0 )
	{
		And2 = 0xf;
		
#ifdef _ENABLE_SSE2_SPRITE_NONTEMPLATE
		Shift1 = _mm_set_epi32 ( 0, 0, 0, 2 );
		Shift2 = _mm_set_epi32 ( 0, 0, 0, 2 );
		And1 = _mm_set1_epi16 ( 3 );
#else
		Shift1 = 2; Shift2 = 2;
		And1 = 3; And2 = 0xf;
#endif
	}
	else if ( tpage_tp == 1 )
	{
		And2 = 0xff;
		
#ifdef _ENABLE_SSE2_SPRITE_NONTEMPLATE
		Shift1 = _mm_set_epi32 ( 0, 0, 0, 1 );
		Shift2 = _mm_set_epi32 ( 0, 0, 0, 3 );
		And1 = _mm_set1_epi16 ( 1 );
#else
		Shift1 = 1; Shift2 = 3;
		And1 = 1; And2 = 0xff;
#endif
	}
	
	
	color_add = bgr;

	// initialize number of pixels drawn
	NumberOfPixelsDrawn = 0;
	
	// check for some important conditions
	if ( DrawArea_BottomRightX < DrawArea_TopLeftX )
	{
		//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightX < DrawArea_TopLeftX.\n";
		return;
	}
	
	if ( DrawArea_BottomRightY < DrawArea_TopLeftY )
	{
		//cout << "\nhps1x64 ALERT: GPU: DrawArea_BottomRightY < DrawArea_TopLeftY.\n";
		return;
	}
	
	// get top left corner of sprite and bottom right corner of sprite
	x0 = x;
	y0 = y;
	x1 = x + w - 1;
	y1 = y + h - 1;
	
	// get texture coords
	u0 = u;
	v0 = v;
	
	//////////////////////////////////////////
	// get coordinates on screen
	x0 = DrawArea_OffsetX + x0;
	y0 = DrawArea_OffsetY + y0;
	x1 = DrawArea_OffsetX + x1;
	y1 = DrawArea_OffsetY + y1;
	
	// check if sprite is within draw area
	if ( x1 < ((s32)DrawArea_TopLeftX) || x0 > ((s32)DrawArea_BottomRightX) || y1 < ((s32)DrawArea_TopLeftY) || y0 > ((s32)DrawArea_BottomRightY) ) return;
	
	

	
	StartX = x0;
	EndX = x1;
	StartY = y0;
	EndY = y1;

	if ( StartY < ((s32)DrawArea_TopLeftY) )
	{
		v0 += ( DrawArea_TopLeftY - StartY );
		StartY = DrawArea_TopLeftY;
	}
	
	if ( EndY > ((s32)DrawArea_BottomRightY) )
	{
		EndY = DrawArea_BottomRightY;
	}
	
	if ( StartX < ((s32)DrawArea_TopLeftX) )
	{
		u0 += ( DrawArea_TopLeftX - StartX );
		StartX = DrawArea_TopLeftX;
	}
	
	if ( EndX > ((s32)DrawArea_BottomRightX) )
	{
		EndX = DrawArea_BottomRightX;
	}

	
	iV = v0;
	
	NumberOfPixelsDrawn = ( EndX - StartX + 1 ) * ( EndY - StartY + 1 );
		
#ifdef _ENABLE_SSE2_SPRITE_NONTEMPLATE
	viV = _mm_set1_epi32 ( iV );
	vEndX = _mm_set1_epi16 ( EndX + 1 );
	
	vStartX = _mm_add_epi16 ( _mm_set1_epi16 ( StartX ), vSeq );
	vu = _mm_add_epi16 ( _mm_set1_epi16 ( u ), vSeq );
#endif


//#define DEBUG_DRAWSPRITE
#ifdef DEBUG_DRAWSPRITE
	debug << "\r\nTWX=" << TWX << " TWY=" << TWY << " TWW=" << TWW << " TWH=" << TWH << " TextureWindow_X=" << TextureWindow_X << " TextureWindow_Y=" << TextureWindow_Y << " TextureWindow_Width=" << TextureWindow_Width << " TextureWindow_Height=" << TextureWindow_Height;
#endif

	for ( Line = StartY; Line <= EndY; Line++ )
	{
#ifdef _ENABLE_SSE2_SPRITE_NONTEMPLATE
		//viU = _mm_add_epi16 ( _mm_set1_epi16 ( u ), vSeq );
		viU = vu;
		
		//viU2 = _mm_add_epi32 ( _mm_set1_epi32 ( u ), vSeq2_32 );
		//TexCoordY = _mm_sll_epi32 ( _mm_set1_epi32 ( (u8) ( ( iV & ~( TWH << 3 ) ) | ( ( TWY & TWH ) << 3 ) ) ), 10 );
#else
			// need to start texture coord from left again
			iU = u0;
			//TexCoordY = (u8) ( ( iV & ~( TWH << 3 ) ) | ( ( TWY & TWH ) << 3 ) );
#endif

			TexCoordY = (u8) ( ( iV & Not_TWH ) | ( TWYTWH ) );
			TexCoordY <<= 10;

			ptr = & ( VRAM [ StartX + ( Line << 10 ) ] );
			
#ifdef _ENABLE_SSE2_SPRITE_NONTEMPLATE
		//vx_across = _mm_add_epi16 ( _mm_set1_epi16 ( StartX ), vSeq );
		vx_across = vStartX;
#endif

			// draw horizontal line
			//for ( x_across = StartX; x_across <= EndX; x_across++ )
			for ( x_across = StartX; x_across <= EndX; x_across += c_iVectorSize )
			{
#ifdef _ENABLE_SSE2_SPRITE_NONTEMPLATE
				TexCoordX = _mm_and_si128 ( _mm_or_si128 ( _mm_and_si128 ( viU, Not_TWW ), TWXTWW ), Mask );
				
				if ( And2 )
				{
					vIndex1 = _mm_srl_epi16 ( TexCoordX, Shift1 );
					vIndex2 = _mm_sll_epi16 ( _mm_and_si128 ( TexCoordX, And1 ), Shift2 );
					
					_mm_store_si128 ( (__m128i*) TexCoordXList, vIndex1 );
					_mm_store_si128 ( (__m128i*) TempList, vIndex2 );
					
					// get number of pixels remaining to draw
					Temp = EndX - x_across;
					
					// only get full 8 pixels if there are 8 or more left to draw, otherwize get remaining to draw
					Temp = ( ( Temp > 7 ) ? 7 : Temp );
					
					for ( Index = 0; Index <= Temp; Index++ )
					{
						bgr = ptr_texture [ TexCoordXList [ Index ] + TexCoordY ];
						bgr = ptr_clut [ ( clut_xoffset + ( ( bgr >> TempList [ Index ] ) & And2 ) ) & FrameBuffer_XMask ];
						TexCoordXList [ Index ] = bgr;
					}
					
					vbgr = _mm_load_si128 ( (const __m128i*) TexCoordXList );
					
					
					/*
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 0 ) + TexCoordY ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 0 ) ) & And2 ) ) & FrameBuffer_XMask ], 0 );
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 1 ) + TexCoordY ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 1 ) ) & And2 ) ) & FrameBuffer_XMask ], 1 );
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 2 ) + TexCoordY ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 2 ) ) & And2 ) ) & FrameBuffer_XMask ], 2 );
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 3 ) + TexCoordY ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 3 ) ) & And2 ) ) & FrameBuffer_XMask ], 3 );
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 4 ) + TexCoordY ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 4 ) ) & And2 ) ) & FrameBuffer_XMask ], 4 );
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 5 ) + TexCoordY ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 5 ) ) & And2 ) ) & FrameBuffer_XMask ], 5 );
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 6 ) + TexCoordY ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 6 ) ) & And2 ) ) & FrameBuffer_XMask ], 6 );
					bgr = ptr_texture [ _mm_extract_epi16 ( vIndex1, 7 ) + TexCoordY ];
					vbgr = _mm_insert_epi16 ( vbgr, ptr_clut [ ( clut_xoffset + ( ( bgr >> _mm_extract_epi16 ( vIndex2, 7 ) ) & And2 ) ) & FrameBuffer_XMask ], 7 );
					*/
				}
				else
				{
					_mm_store_si128 ( (__m128i*) TexCoordXList, TexCoordX );
					
					// get number of pixels remaining to draw
					Temp = EndX - x_across;
					
					// only get full 8 pixels if there are 8 or more left to draw, otherwize get remaining to draw
					Temp = ( ( Temp > 7 ) ? 7 : Temp );
					
					for ( Index = 0; Index <= Temp; Index++ )
					{
						TexCoordXList [ Index ] = ptr_texture [ TexCoordXList [ Index ] + TexCoordY ];
					}
					
					vbgr = _mm_load_si128 ( (const __m128i*) TexCoordXList );
					
					
					/*
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 0 ) + TexCoordY ], 0 );
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 1 ) + TexCoordY ], 1 );
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 2 ) + TexCoordY ], 2 );
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 3 ) + TexCoordY ], 3 );
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 4 ) + TexCoordY ], 4 );
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 5 ) + TexCoordY ], 5 );
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 6 ) + TexCoordY ], 6 );
					vbgr = _mm_insert_epi16 ( vbgr, ptr_texture [ _mm_extract_epi16 ( vIndex1, 7 ) + TexCoordY ], 7 );
					*/
				}
#else
					//TexCoordX = (u8) ( ( iU & ~( TWW << 3 ) ) | ( ( TWX & TWW ) << 3 ) );
					TexCoordX = (u8) ( ( iU & Not_TWW ) | ( TWXTWW ) );
					
					//bgr = VRAM [ TextureOffset + ( TexCoordX >> Shift1 ) + ( TexCoordY << 10 ) ];
					//bgr = ptr_texture [ ( TexCoordX >> Shift1 ) + ( TexCoordY << 10 ) ];
					bgr = ptr_texture [ ( TexCoordX >> Shift1 ) + TexCoordY ];
					
					if ( Shift1 )
					{
						//TexelIndex = ( ( bgr >> ( ( TexCoordX & And1 ) << Shift2 ) ) & And2 );
						//bgr = VRAM [ ( ( ( clut_x << 4 ) + TexelIndex ) & FrameBuffer_XMask ) + ( clut_y << 10 ) ];
						bgr = ptr_clut [ ( clut_xoffset + ( ( bgr >> ( ( TexCoordX & And1 ) << Shift2 ) ) & And2 ) ) & FrameBuffer_XMask ];
					}
#endif

					
#ifdef _ENABLE_SSE2_SPRITE_NONTEMPLATE
				DestPixel = _mm_loadu_si128 ((__m128i const*) ptr);
				vbgr_temp = vbgr;
				if ( !tge ) vbgr_temp = vColorMultiply1624 ( vbgr_temp, color_add_r, color_add_g, color_add_b );
				if ( command_abe )
				{
					vbgr_temp_transparent = vSemiTransparency16( DestPixel, vbgr_temp, GPU_CTRL_Read.ABR );
					vbgr_select = _mm_srai_epi16 ( vbgr, 15 );
					vbgr_temp = _mm_or_si128 ( _mm_andnot_si128( vbgr_select, vbgr_temp ), _mm_and_si128 ( vbgr_select, vbgr_temp_transparent ) );
				}
				vbgr_temp = _mm_or_si128 ( _mm_or_si128 ( vbgr_temp, SetPixelMask ), _mm_and_si128 ( vbgr, tMask ) );
				_mm_maskmoveu_si128 ( vbgr_temp, _mm_andnot_si128 ( _mm_and_si128 ( _mm_srai_epi16 ( DestPixel, 15 ), PixelMask ), _mm_andnot_si128 ( _mm_cmpeq_epi16 ( vbgr, _mm_setzero_si128 () ), _mm_cmplt_epi16 ( vx_across, vEndX ) ) ), (char*) ptr );
				vx_across = _mm_add_epi16 ( vx_across, vVectorSize );
				viU = _mm_add_epi16 ( viU, vVectorSize );
#else
					if ( bgr )
					{
						// read pixel from frame buffer if we need to check mask bit
						DestPixel = *ptr;	//VRAM [ x_across + ( Line << 10 ) ];
						
						bgr_temp = bgr;
			
						if ( !tge )
						{
							// brightness calculation
							//bgr_temp = Color24To16 ( ColorMultiply24 ( Color16To24 ( bgr_temp ), color_add ) );
							bgr_temp = ColorMultiply1624 ( bgr_temp, color_add );
						}
						
						// semi-transparency
						if ( command_abe && ( bgr & 0x8000 ) )
						{
							bgr_temp = SemiTransparency16 ( DestPixel, bgr_temp, GPU_CTRL_Read.ABR );
						}
						
						// check if we should set mask bit when drawing
						//if ( GPU_CTRL_Read.MD ) bgr_temp |= 0x8000;
						bgr_temp |= SetPixelMask | ( bgr & 0x8000 );

						// draw pixel if we can draw to mask pixels or mask bit not set
						//if ( ! ( DestPixel & PixelMask ) ) VRAM [ x_across + ( Line << 10 ) ] = bgr_temp;
						if ( ! ( DestPixel & PixelMask ) ) *ptr = bgr_temp;
					}
#endif
					
				/////////////////////////////////////////////////////////
				// interpolate texture coords across
				//iU++;	//+= dU_across;
				iU += c_iVectorSize;
				
				// update pointer for pixel out
				//ptr++;
				ptr += c_iVectorSize;
					
			}
		
		/////////////////////////////////////////////////////////
		// interpolate texture coords down
		iV++;	//+= dV_left;
	}
}

#endif


////////////////////////////////////////////////////////////////////////////////////////////


#ifndef EXCLUDE_TRIANGLE_MONO_NONTEMPLATE

//void GPU::Draw_MonoTriangle_20 ( u32 Coord0, u32 Coord1, u32 Coord2 )
void GPU::Draw_MonoTriangle_20 ( DATA_Write_Format* p_inputdata, u32 ulThreadNum )
{
	u64 NumPixels;
	
#ifdef USE_TEMPLATES_PS1_TRIANGLE
	NumPixels = Select_Triangle_Renderer_t ( p_inputdata, ulThreadNum );
#else
	//DrawTriangle_Mono ( Coord0, Coord1, Coord2 );
	NumPixels = DrawTriangle_Mono_th ( p_inputdata, ulThreadNum );
#endif
	
	if ( !ulThreadNum )
	{
		
	// *** TODO *** calculate cycles to draw here
	// check for alpha blending - add in an extra cycle for this for now
	if ( command_abe )
	{
		//BusyCycles += NumberOfPixelsDrawn * dAlphaBlending_CyclesPerPixel;
		BusyCycles += NumPixels * dAlphaBlending_CyclesPerPixel;
	}
	
	// add in cycles to draw mono-triangle
	//BusyCycles += NumberOfPixelsDrawn * dMonoTriangle_20_CyclesPerPixel;
	BusyCycles += NumPixels * dMonoTriangle_20_CyclesPerPixel;
	
#ifdef ENABLE_DRAW_OVERHEAD
	BusyCycles += DrawOverhead_Cycles;
#endif
	}	// end if ( !ulThreadNum )
}

void GPU::Draw_MonoRectangle_28 ()
//void GPU::Draw_MonoRectangle_28 ( DATA_Write_Format* p_inputdata, u32 ulThreadNum )
{
	//Draw_MonoTriangle_20 ( 0, 1, 2 );
	//Draw_MonoTriangle_20 ( p_inputdata, ulThreadNum );
	
	//p_inputdata [ 8 ].Value = p_inputdata [ 11 ].Value;
	
	//Draw_MonoTriangle_20 ( 1, 2, 3 );
	//Draw_MonoTriangle_20 ( p_inputdata, ulThreadNum );
}
//
#endif


#ifndef EXCLUDE_TRIANGLE_GRADIENT_NONTEMPLATE

//void GPU::Draw_GradientTriangle_30 ( u32 Coord0, u32 Coord1, u32 Coord2 )
void GPU::Draw_GradientTriangle_30 ( DATA_Write_Format* p_inputdata, u32 ulThreadNum )
{
	u64 NumPixels;
	
	/*
	//if ( ( bgr0 & 0x00ffffff ) == ( bgr1 & 0x00ffffff ) && ( bgr0 & 0x00ffffff ) == ( bgr2 & 0x00ffffff ) )
	if ( gbgr [ Coord0 ] == gbgr [ Coord1 ] && gbgr [ Coord0 ] == gbgr [ Coord2 ] )
	{
		//GetBGR ( Buffer [ 0 ] );
		//DrawTriangle_Mono ();
		gbgr [ 0 ] = gbgr [ Coord0 ];
		DrawTriangle_Mono ( Coord0, Coord1, Coord2 );
	}
	else
	{
	*/
#ifdef USE_TEMPLATES_PS1_TRIANGLE
	NumPixels = Select_Triangle_Renderer_t ( p_inputdata, ulThreadNum );
#else
		//DrawTriangle_Gradient ( Coord0, Coord1, Coord2 );
		NumPixels = DrawTriangle_Gradient_th ( p_inputdata, ulThreadNum );
#endif
	//}
	
	if ( !ulThreadNum )
	{
		
	// *** TODO *** calculate cycles to draw here
	// check for alpha blending - add in an extra cycle for this for now
	if ( command_abe )
	{
		//BusyCycles += NumberOfPixelsDrawn * dAlphaBlending_CyclesPerPixel;
		BusyCycles += NumPixels * dAlphaBlending_CyclesPerPixel;
	}
	
	// add in cycles to draw mono-triangle
	//BusyCycles += NumberOfPixelsDrawn * dGradientTriangle_30_CyclesPerPixel;
	BusyCycles += NumPixels * dGradientTriangle_30_CyclesPerPixel;
	
#ifdef ENABLE_DRAW_OVERHEAD
	BusyCycles += DrawOverhead_Cycles;
#endif
	}	// end if ( !ulThreadNum )
}


void GPU::Draw_GradientRectangle_38 ()
//void GPU::Draw_GradientRectangle_38 ( DATA_Write_Format* p_inputdata, u32 ulThreadNum )
{
	
	//Draw_GradientTriangle_30 ( 0, 1, 2 );
	//Draw_GradientTriangle_30 ( p_inputdata, ulThreadNum );
	
	//p_inputdata [ 7 ].Value = ( p_inputdata [ 7 ].Value & ~0xffffff ) | ( p_inputdata [ 13 ].Value & 0xffffff );
	//p_inputdata [ 8 ].Value = p_inputdata [ 14 ].Value;
	
	//Draw_GradientTriangle_30 ( 1, 2, 3 );
	//Draw_GradientTriangle_30 ( p_inputdata, ulThreadNum );

}

#endif


#ifndef EXCLUDE_TRIANGLE_TEXTURE_NONTEMPLATE

//void GPU::Draw_TextureTriangle_24 ( u32 Coord0, u32 Coord1, u32 Coord2 )
void GPU::Draw_TextureTriangle_24 ( DATA_Write_Format* p_inputdata, u32 ulThreadNum )
{

	u64 NumPixelsDrawn;
	
	u32 tge;
	tge = command_tge;
	
	//if ( ( bgr & 0x00ffffff ) == 0x00808080 )
	if ( gbgr [ 0 ] == 0x00808080 )
	{
		command_tge = 1;
	}
	
#ifdef USE_TEMPLATES_PS1_TRIANGLE
	NumPixelsDrawn = Select_Triangle_Renderer_t ( p_inputdata, ulThreadNum );
#else
	//DrawTriangle_Texture ( Coord0, Coord1, Coord2 );
	//NumPixelsDrawn = DrawTriangle_Texture_th ( p_inputdata, 0 );
	NumPixelsDrawn = DrawTriangle_Texture_th ( p_inputdata, ulThreadNum );
#endif
	
	if ( !ulThreadNum )
	{
	// restore tge
	command_tge = tge;
	
	// check for alpha blending - add in an extra cycle for this for now
	if ( command_abe )
	{
		//BusyCycles += NumberOfPixelsDrawn * dAlphaBlending_CyclesPerPixel;
		BusyCycles += NumPixelsDrawn * dAlphaBlending_CyclesPerPixel;
	}
	
	// check for brightness calculation
	if ( !command_tge )
	{
		//BusyCycles += NumberOfPixelsDrawn * dBrightnessCalculation_CyclesPerPixel;
		BusyCycles += NumPixelsDrawn * dBrightnessCalculation_CyclesPerPixel;
	}
	
	//switch ( tpage_tp )
	switch ( GPU_CTRL_Read.TP )
	{
		case 0:		// 4-bit clut
			//BusyCycles += NumberOfPixelsDrawn * dTextureTriangle4_24_CyclesPerPixel;
			BusyCycles += NumPixelsDrawn * dTextureTriangle4_24_CyclesPerPixel;
			break;
			
		case 1:		// 8-bit clut
			//BusyCycles += NumberOfPixelsDrawn * dTextureTriangle8_24_CyclesPerPixel;
			BusyCycles += NumPixelsDrawn * dTextureTriangle8_24_CyclesPerPixel;
			break;
			
		case 2:		// 15-bit color
			//BusyCycles += NumberOfPixelsDrawn * dTextureTriangle16_24_CyclesPerPixel;
			BusyCycles += NumPixelsDrawn * dTextureTriangle16_24_CyclesPerPixel;
			break;
	}
	
#ifdef ENABLE_DRAW_OVERHEAD
	BusyCycles += DrawOverhead_Cycles;
#endif
	}	// end if ( !ulThreadNum )
}

void GPU::Draw_TextureRectangle_2c ()
{
	//Draw_TextureTriangle_24 ( 0, 1, 2 );
	//Draw_TextureTriangle_24 ( p_inputdata, ulThreadNum );

	//p_inputdata [ 7 ].Value = ( p_inputdata [ 7 ].Value & ~0xffffff ) | ( p_inputdata [ 13 ].Value & 0xffffff );
	//p_inputdata [ 8 ].Value = p_inputdata [ 14 ].Value;
	
	//Draw_TextureTriangle_24 ( 1, 2, 3 );
	//Draw_TextureTriangle_24 ( p_inputdata, ulThreadNum );

}

#endif



#ifndef EXCLUDE_TRIANGLE_TEXTUREGRADIENT_NONTEMPLATE

//void GPU::Draw_TextureGradientTriangle_34 ( u32 Coord0, u32 Coord1, u32 Coord2 )
void GPU::Draw_TextureGradientTriangle_34 ( DATA_Write_Format* p_inputdata, u32 ulThreadNum )
{
	u64 NumPixels;
	u32 tge;
	tge = command_tge;
	
	/*
	//if ( ( bgr0 & 0x00ffffff ) == ( bgr1 & 0x00ffffff ) && ( bgr0 & 0x00ffffff ) == ( bgr2 & 0x00ffffff ) )
	if ( gbgr [ Coord0 ] == gbgr [ Coord1 ] && gbgr [ Coord0 ] == gbgr [ Coord2 ] )
	{
		//if ( ( bgr0 & 0x00ffffff ) == 0x00808080 )
		if ( gbgr [ Coord0 ] == 0x00808080 )
		{
			command_tge = 1;
		}
		else
		{
			//bgr = bgr0;
			gbgr [ 0 ] = gbgr [ Coord0 ];
		}
		
		//DrawTriangle_Texture ( Coord0, Coord1, Coord2 );
		NumPixels = DrawTriangle_Texture_th ( p_inputdata, ulThreadNum );
	}
	else
	{
		if ( command_tge )
		{
			//DrawTriangle_Texture ( Coord0, Coord1, Coord2 );
			NumPixels = DrawTriangle_Texture_th ( p_inputdata, ulThreadNum );
		}
		else
		{
		*/
		
#ifdef USE_TEMPLATES_PS1_TRIANGLE
	NumPixels = Select_Triangle_Renderer_t ( p_inputdata, ulThreadNum );
#else
			//DrawTriangle_TextureGradient ( Coord0, Coord1, Coord2 );
			NumPixels = DrawTriangle_TextureGradient_th ( p_inputdata, ulThreadNum );
#endif
	
		/*
		}
	}
	*/
	
	if ( !ulThreadNum )
	{
	// restore tge
	command_tge = tge;

	// check for alpha blending - add in an extra cycle for this for now
	if ( command_abe )
	{
		BusyCycles += NumPixels * dAlphaBlending_CyclesPerPixel;
	}
	
	// check for brightness calculation
	if ( !command_tge )
	{
		BusyCycles += NumPixels * dBrightnessCalculation_CyclesPerPixel;
	}

	//switch ( tpage_tp )
	switch ( GPU_CTRL_Read.TP )
	{
		case 0:		// 4-bit clut
			BusyCycles += NumPixels * dTextureTriangle4_34_Gradient_CyclesPerPixel;
			break;
			
		case 1:		// 8-bit clut
			BusyCycles += NumPixels * dTextureTriangle8_34_Gradient_CyclesPerPixel;
			break;
			
		case 2:		// 15-bit color
			BusyCycles += NumPixels * dTextureTriangle16_34_Gradient_CyclesPerPixel;
			break;
	}
	
#ifdef ENABLE_DRAW_OVERHEAD
	BusyCycles += DrawOverhead_Cycles;
#endif
	}	// end if ( !ulThreadNum )
}


void GPU::Draw_TextureGradientRectangle_3c ()
//void GPU::Draw_TextureGradientRectangle_3c ( DATA_Write_Format* p_inputdata, u32 ulThreadNum )
{
	//Draw_TextureGradientTriangle_34 ( 0, 1, 2 );
	//Draw_TextureGradientTriangle_34 ( p_inputdata, ulThreadNum );
	
	//p_inputdata [ 7 ].Value = ( p_inputdata [ 7 ].Value & ~0xffffff ) | ( ( p_inputdata [ 5 ].Value ) & 0xffffff );
	//p_inputdata [ 8 ].Value = p_inputdata [ 6 ].Value;
	//p_inputdata [ 9 ].Value = ( p_inputdata [ 9 ].Value & ~0xffff ) | ( ( p_inputdata [ 15 ].Value >> 16 ) & 0xffff );
	
	//Draw_TextureGradientTriangle_34 ( 1, 2, 3 );
	//Draw_TextureGradientTriangle_34 ( p_inputdata, ulThreadNum );

}

#endif




#ifndef EXCLUDE_SPRITE_NONTEMPLATE

void GPU::Draw_Sprite_64 ( DATA_Write_Format* p_inputdata, u32 ulThreadNum )
{
	u64 NumPixels;
	
#ifdef USE_TEMPLATES_PS1_RECTANGLE
	NumPixels = Select_Sprite_Renderer_t ( p_inputdata, ulThreadNum );
#else
	//DrawSprite ();
	NumPixels = DrawSprite_th ( p_inputdata, ulThreadNum );
#endif

	if ( !ulThreadNum )
	{
	tpage_tx = GPU_CTRL_Read.TX;
	tpage_ty = GPU_CTRL_Read.TY;
	tpage_tp = GPU_CTRL_Read.TP;
	
	// set number of cycles it takes to draw sprite
	switch ( tpage_tp )
	{
		case 0:		// 4-bit clut
			//BusyCycles = NumberOfPixelsDrawn * dSprite4_64_Cycles;
			BusyCycles = NumPixels * dSprite4_64_Cycles;
			break;
			
		case 1:		// 8-bit clut
			//BusyCycles = NumberOfPixelsDrawn * dSprite8_64_Cycles;
			BusyCycles = NumPixels * dSprite8_64_Cycles;
			break;
			
		case 2:		// 15-bit color
			//BusyCycles = NumberOfPixelsDrawn * dSprite16_64_Cycles;
			BusyCycles = NumPixels * dSprite16_64_Cycles;
			break;
	}

	}	// end if ( !ulThreadNum )
}

#endif



#ifndef EXCLUDE_SPRITE8_NONTEMPLATE

void GPU::Draw_Sprite8x8_74 ()
{
	static const u32 SpriteSize = 8;
	
	//static const int CyclesPerSprite8x8_4bit = 121;
	//static const int CyclesPerSprite8x8_8bit = 121;
	//static const int CyclesPerSprite8x8_16bit = 212;
	

	//static const int TextureSizeX = 256;
	//static const int TextureSizeY = 32;


	//tpage_tx = GPU_CTRL_Read.TX;
	//tpage_ty = GPU_CTRL_Read.TY;
	tpage_tp = GPU_CTRL_Read.TP;
	//tpage_abr = GPU_CTRL_Read.ABR;
	
	w = 8; h = 8;
	DrawSprite ();

	// set number of cycles it takes to draw 8x8 sprite
	switch ( tpage_tp )
	{
		case 0:		// 4-bit clut
			BusyCycles = dCyclesPerSprite8x8_74_4bit;	//121;
			break;
			
		case 1:		// 8-bit clut
			BusyCycles = dCyclesPerSprite8x8_74_8bit;	//121;
			break;
			
		case 2:		// 15-bit color
			BusyCycles = dCyclesPerSprite8x8_74_16bit;	//212;
			break;
	}


}

#endif


#ifndef EXCLUDE_SPRITE16_NONTEMPLATE

void GPU::Draw_Sprite16x16_7c ()
{
	static const u32 SpriteSize = 16;
	
	//static const int CyclesPerSprite16x16_4bit = 308;
	//static const int CyclesPerSprite16x16_8bit = 484;
	//static const int CyclesPerSprite16x16_16bit = 847;

	//static const int TextureSizeX = 256;
	//static const int TextureSizeY = 32;

	//tpage_tx = GPU_CTRL_Read.TX;
	//tpage_ty = GPU_CTRL_Read.TY;
	tpage_tp = GPU_CTRL_Read.TP;
	//tpage_abr = GPU_CTRL_Read.ABR;
	
	w = 16; h = 16;
	DrawSprite ();

	// set number of cycles it takes to draw 16x16 sprite
	switch ( tpage_tp )
	{
		case 0:		// 4-bit clut
			BusyCycles = dCyclesPerSprite16x16_7c_4bit;	//308;
			break;
			
		case 1:		// 8-bit clut
			BusyCycles = dCyclesPerSprite16x16_7c_8bit;	//484;
			break;
			
		case 2:		// 15-bit color
			BusyCycles = dCyclesPerSprite16x16_7c_16bit;	//847;
			break;
	}


}

#endif



void GPU::Draw_MonoLine_40 ()
{
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_LINE
			debug << "\r\nDraw_MonoLine_40";
#endif

	static constexpr double dMonoLine_40_CyclesPerPixel = 1.0L;

	s32 distance, x_distance, y_distance;
	
	//s64 rc;
	
	u16 *ptr16;
	
	s64 dxdc;
	s64 dydc;
	s64 line_x0, line_y0;
	s64 line_x1, line_y1;
	
	s64 line_x, line_y;
	
	s64 Temp, Temp0, Temp1;
	s64 dx, dy;
	
	s64 TestTop, TestBottom, TestLeft, TestRight;
	
	s64 dxdy, dydx;
	
	//s64 Start, End;

	// new local variables
	s64 x, y, x0, x1, y0, y1;
	u32 bgr, bgr2;
	s64 iX, iY;
	
	u32 DestPixel, PixelMask = 0, SetPixelMask = 0;
	
	NumberOfPixelsDrawn = 0;

	if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
	if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;
	
	// get color(s)
	bgr = gbgr [ 0 ];
	
	// ?? convert to 16-bit ?? (or should leave 24-bit?)
	bgr = ( ( bgr & ( 0xf8 << 0 ) ) >> 3 ) | ( ( bgr & ( 0xf8 << 8 ) ) >> 6 ) | ( ( bgr & ( 0xf8 << 16 ) ) >> 9 );
	
	// get x-values
	x0 = gx [ 0 ];
	x1 = gx [ 1 ];
	
	// get y-values
	y0 = gy [ 0 ];
	y1 = gy [ 1 ];
	
	//////////////////////////////////////////
	// get coordinates on screen
	x0 = DrawArea_OffsetX + x0;
	y0 = DrawArea_OffsetY + y0;
	x1 = DrawArea_OffsetX + x1;
	y1 = DrawArea_OffsetY + y1;

	// make sure line might be on screen
	if ( ( ( x1 <= ((s32)DrawArea_TopLeftX) ) && ( x0 < ((s32)DrawArea_TopLeftX) ) ) ||
		( ( x1 >= ((s32)DrawArea_BottomRightX) ) && ( x0 > ((s32)DrawArea_BottomRightX) ) ) ||
		( ( y1 <= ((s32)DrawArea_TopLeftY) ) && ( y0 < ((s32)DrawArea_TopLeftY) ) ) ||
		( ( y1 >= ((s32)DrawArea_BottomRightY) ) && ( y0 > ((s32)DrawArea_BottomRightY) ) ) ) return;
		
	// get size of line
	x_distance = _Abs( x1 - x0 );
	y_distance = _Abs( y1 - y0 );
	//if ( x_distance > y_distance ) distance = x_distance; else distance = y_distance;

	// if we don't draw the final point, then a distance of zero does not get drawn
	// if both x and y distance are zero, then do not draw
	//if ( !distance ) return;
	if ( ! ( x_distance | y_distance ) ) return;
	
	// x.48 fixed point
	if ( x_distance ) dx = ( 1LL << 32 ) / x_distance; else dx = 0;
	if ( y_distance ) dy = ( 1LL << 32 ) / y_distance; else dy = 0;
	
	// x.48 fixed point
	dxdy = ( ( (s64) ( x1 - x0 ) ) * dy );
	dydx = ( ( (s64) ( y1 - y0 ) ) * dx );
	
	/*
	if ( x_distance > y_distance )
	{
		distance = x_distance;
		
		
		//rc = ( 1LL << 48 ) / distance;
		//dydc = ( ( (s64) ( y1 - y0 ) ) * rc ) >> 32;
		
		
		
		
	}
	else
	{
		distance = y_distance;
		
		if ( y1 > y0 ) dydc = 1; else dydc = -1;
		
		rc = ( 1LL << 48 ) / distance;
		dxdc = ( ( (s64) ( x1 - x0 ) ) * rc ) >> 32;
	}
	*/
	
	// going from y0 to y1, get the change in y for every change in count
	/*
	if ( distance )
	{
		rc = ( 1LL << 48 ) / distance;
		
		//dxdc = ( ( (s64)x1 - (s64)x0 ) << 32 ) / distance;
		//dydc = ( ( (s64)y1 - (s64)y0 ) << 32 ) / distance;
		dxdc = ( ( (s64) ( x1 - x0 ) ) * rc ) >> 32;
		dydc = ( ( (s64) ( y1 - y0 ) ) * rc ) >> 32;
	}
	*/
	
	// init line x,y
	//line_x = ( ( (s64) x0 ) << 32 );
	//line_y = ( ( (s64) y0 ) << 32 );
	//line_x = ( x0 << 16 );
	//line_y = ( y0 << 16 );
	
	
	// close in from the starting point towards ending point
	
	// close in from the ending point towards starting point
	

	
	
	line_x0 = ( x0 << 16 );
	line_y0 = ( y0 << 16 );
	
	line_x1 = ( x1 << 16 );
	line_y1 = ( y1 << 16 );

	// initial fractional part
	line_x0 |= 0x8000;
	line_y0 |= 0x8000;
	line_x1 |= 0x8000;
	line_y1 |= 0x8000;

#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_LINE
			debug << "\r\nBeforeClip:";
			debug << hex << " line_x0=" << line_x0 << " line_y0=" << line_y0 << " line_x1=" << line_x1 << " line_y1=" << line_y1;
			debug << dec << " x0=" << (line_x0>>16) << " y0=" << (line_y0>>16) << " x1=" << (line_x1>>16) << " y1=" << (line_y1>>16);
#endif

	
	// get window for the end point
	TestLeft = ( DrawArea_TopLeftX << 16 );
	TestRight = ( DrawArea_BottomRightX << 16 ) | 0xffff;
	//TestRight = ( ( DrawArea_BottomRightX + 1 ) << 16 );
	TestTop = ( DrawArea_TopLeftY << 16 );
	TestBottom = ( DrawArea_BottomRightY << 16 ) | 0xffff;
	//TestBottom = ( ( DrawArea_BottomRightY + 1 ) << 16 );
	
	
	// if start coord is off to the left or right, then fix
	if ( line_x0 < TestLeft && x1 > x0 )
	{
		// line must be headed to the right (dx positive)
		// but would have to be headed to the right or would not have reached here
		//if ( x1 <= x0 ) return;
		
		Temp = TestLeft - line_x0;
		line_y0 += ( ( Temp * dydx ) >> 32 );
		line_x0 = TestLeft;
	}
	else if ( line_x0 > TestRight && x1 < x0 )
	{
		// line must be headed to the left (dx negative)
		// but would have to be headed to the left or would not have reached here
		//if ( x1 >= x0 ) return;
		
		//Temp = TestRight - line_x0;
		Temp = line_x0 - TestRight;
		line_y0 += ( ( Temp * dydx ) >> 32 );
		line_x0 = TestRight;
	}
	

	// if start coord is off to the top or bottom, then fix
	if ( line_y0 < TestTop && y1 > y0 )
	{
		// line must be headed down (dy positive)
		// but would have to be headed down or would not have reached here
		//if ( y1 <= y0 ) return;
		
		Temp = TestTop - line_y0;
		line_x0 += ( ( Temp * dxdy ) >> 32 );
		line_y0 = TestTop;
	}
	else if ( line_y0 > TestBottom && y1 < y0 )
	{
		// line must be headed up (dy negative)
		// but would have to be headed up or would not have reached here
		//if ( y1 >= y0 ) return;
		
		//Temp = TestBottom - line_y0;
		Temp = line_y0 - TestBottom;
		line_x0 += ( ( Temp * dxdy ) >> 32 );
		line_y0 = TestBottom;
	}
	
	// round to nearest
	//line_x += 0x8000;
	//line_y += 0x8000;
	
	// make sure starting point is within draw window
	//x = line_x >> 16;
	//y = line_y >> 16;
	//if ( ( x < ((s32)DrawArea_TopLeftX) ) || ( x > ((s32)DrawArea_BottomRightX) ) || ( y < ((s32)DrawArea_TopLeftY) ) || ( y > ((s32)DrawArea_BottomRightY) ) ) return;
	// but also need to check again later, because could still be offscreen
	if ( ( line_x0 < TestLeft ) || ( line_x0 > TestRight ) || ( line_y0 < TestTop ) || ( line_y0 > TestBottom ) ) return;

	
	// if line endpoint is off, then fix
	// just need to trace back to exact point exact line is out of window
	if ( line_x1 < TestLeft )
	{
		//Temp = TestLeft - line_x1;
		Temp = line_x1 - TestLeft;
		line_y1 += ( ( Temp * dydx ) >> 32 );
		line_x1 = TestLeft;
	}
	else if ( line_x1 > TestRight )
	{
		Temp = TestRight - line_x1;
		line_y1 += ( ( Temp * dydx ) >> 32 );
		line_x1 = TestRight;
	}
	

	if ( line_y1 < TestTop )
	{
		//Temp = TestTop - line_y1;
		Temp = line_y1 - TestTop;
		line_x1 += ( ( Temp * dxdy ) >> 32 );
		line_y1 = TestTop;
	}
	else if ( line_y1 > TestBottom )
	{
		Temp = TestBottom - line_y1;
		line_x1 += ( ( Temp * dxdy ) >> 32 );
		line_y1 = TestBottom;
	}
	
	// set end point to draw to
	//line_x1 = line_x;
	//line_y1 = line_y;
	
#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_LINE
			debug << "\r\nAfterClip1:";
			debug << hex << " line_x0=" << line_x0 << " line_y0=" << line_y0 << " line_x1=" << line_x1 << " line_y1=" << line_y1;
			debug << dec << " x0=" << (line_x0>>16) << " y0=" << (line_y0>>16) << " x1=" << (line_x1>>16) << " y1=" << (line_y1>>16);
#endif

	// get final distance to draw
	if ( x_distance > y_distance )
	{
		// round to nearest
		//line_x2 += 0x8000;
		
		// set dxdc,dydc
		if ( x1 > x0 ) dxdc = ( 1 << 16 ); else dxdc = ( -1 << 16 );
		
		// x.16 fixed point
		dydc = ( dydx >> 16 );
		
		// check if pixel is off 
		
		if ( x1 > x0 )
		{
			// line is going to right //
			
			// make sure start x is at .5 interval
			line_x = ( ( line_x0 + 0x7fff ) | 0x8000 ) & ~0x7fff;
			
			// make sure end x is at .5 interval
			line_y = ( ( line_x1 - 0x8000 ) | 0x8000 ) & ~0x7fff;
			
			// get difference
			Temp0 = line_x - line_x0;
			
			// get difference
			//Temp = line_x1 - line_y;
			Temp1 = line_y - line_x1;
		}
		else
		{
			// line is going to left //
			
			// make sure end x is at .5 interval
			line_x = ( ( line_x0 - 0x8000 ) | 0x8000 ) & ~0x7fff;
			
			// make sure start x is at .5 interval
			line_y = ( ( line_x1 + 0x7fff ) | 0x8000 ) & ~0x7fff;
			
			// get difference
			Temp0 = line_x0 - line_x;
			//Temp0 = line_x - line_x0;
			
			// get difference
			Temp1 = line_x1 - line_y;
			//Temp1 = line_y - line_x1;
		}

		
		// update y
		line_y0 += ( Temp0 * dydx ) >> 32;
		line_x0 = line_x;
		
		// update y
		line_y1 += ( Temp1 * dydx ) >> 32;
		line_x1 = line_y;
		
		// distance is in x direction
		distance = _Abs ( ( line_x0 >> 16 ) - ( line_x1 >> 16 ) );
		
		// can calculate rgb start here using drdc,dgdc,dbdc
	}
	else
	{
		// round to nearest
		//line_y2 += 0x8000;
		
		// set dxdc,dydc
		if ( y1 > y0 ) dydc = ( 1 << 16 ); else dydc = ( -1 << 16 );
		
		// x.16 fixed point
		dxdc = ( dxdy >> 16 );
		
		if ( y1 > y0 )
		{
			// line is going to bottom //
			
			// make sure start y is at .5 interval
			line_x = ( ( line_y0 + 0x7fff ) | 0x8000 ) & ~0x7fff;
			
			// make sure end y is at .5 interval
			line_y = ( ( line_y1 - 0x8000 ) | 0x8000 ) & ~0x7fff;
			
			// get difference
			Temp0 = line_x - line_y0;
			
			// get difference
			//Temp = line_x1 - line_y;
			Temp1 = line_y - line_y1;
		}
		else
		{
			// line is going to top //
			
			// make sure start y is at .5 interval
			line_x = ( ( line_y0 - 0x8000 ) | 0x8000 ) & ~0x7fff;
			
			// make sure end y is at .5 interval
			line_y = ( ( line_y1 + 0x7fff ) | 0x8000 ) & ~0x7fff;
			
			// get difference
			Temp0 = line_y0 - line_x;
			//Temp0 = line_x - line_x0;
			
			// get difference
			Temp1 = line_y1 - line_y;
			//Temp1 = line_y - line_x1;
		}

		// update x
		line_x0 += ( Temp0 * dxdy ) >> 32;
		line_y0 = line_x;
		
		// update x
		line_x1 += ( Temp1 * dxdy ) >> 32;
		line_y1 = line_y;
		
		// distance is in y direction
		distance = _Abs ( ( line_y0 >> 16 ) - ( line_y1 >> 16 ) );
		
		// can calculate rgb start here using drdc,dgdc,dbdc
	}

	// if starting point is outside of window, then update
	//if ( line_x0 >= TestRight || line_y0 >= TestBottom )
	//{
	//	line_x0 += dxdc;
	//	line_y0 += dydc;
	//}
	
	// if ending point is outside of window, then update
	//if ( line_x1 >= TestRight || line_y1 >= TestBottom )
	//{
	//	line_x1 -= dxdc;
	//	line_y1 -= dydc;
	//}
	
	
	// if the last point on screen is not the endpoint for entire line, then include it
	if ( x1 != ( line_x1 >> 16 ) || y1 != ( line_y1 >> 16 ) ) distance++;

#if defined INLINE_DEBUG_EXECUTE || defined INLINE_DEBUG_RUN_LINE
			debug << "\r\nAfterClip2:";
			debug << hex << " line_x0=" << line_x0 << " line_y0=" << line_y0 << " line_x1=" << line_x1 << " line_y1=" << line_y1 << " bgr0=" << hex << gbgr[0];
			debug << dec << " x0=" << (line_x0>>16) << " y0=" << (line_y0>>16) << " x1=" << (line_x1>>16) << " y1=" << (line_y1>>16) << "\r\ndistance=" << distance;
			debug << hex << " dxdc=" << dxdc << " dydc=" << dydc;
#endif

	// if there is nothing to draw, then done
	if ( !distance ) return;
	
	// draw the line
	// note: the final point should probably not be drawn
	//for ( u32 i = 0; i <= distance; i++ )
	for ( u32 i = 0; i < distance; i++ )
	{
		// get x coord
		//iX = ( _Round( line_x ) >> 32 );
		iX = ( line_x0 >> 16 );
		
		// get y coord
		//iY = ( _Round( line_y ) >> 32 );
		iY = ( line_y0 >> 16 );
		
		//if ( iX >= DrawArea_TopLeftX && iY >= DrawArea_TopLeftY
		//&& iX <= DrawArea_BottomRightX && iY <= DrawArea_BottomRightY )
		//{
			bgr2 = bgr;
			
			ptr16 = & ( VRAM [ iX + ( iY << 10 ) ] );
			
			// read pixel from frame buffer if we need to check mask bit
			DestPixel = *ptr16;
		
			// semi-transparency
			if ( command_abe )
			{
				bgr2 = SemiTransparency16 ( DestPixel, bgr2, GPU_CTRL_Read.ABR );
			}
				
			// draw point
			
			// check if we should set mask bit when drawing
			//if ( GPU_CTRL_Read.MD ) bgr2 |= 0x8000;

			// draw pixel if we can draw to mask pixels or mask bit not set
			if ( ! ( DestPixel & PixelMask ) ) *ptr16 = ( bgr2 | SetPixelMask );
			
			/////////////////////////////////////////////////////
			// update number of cycles used to draw polygon
			//NumberOfPixelsDrawn++;
		//}
		
		line_x0 += dxdc;
		line_y0 += dydc;
	}
	
	NumberOfPixelsDrawn = distance;
	BusyCycles += NumberOfPixelsDrawn * dMonoLine_40_CyclesPerPixel;
	
#ifdef ENABLE_DRAW_OVERHEAD
	BusyCycles += DrawOverhead_Cycles;
#endif
}



void GPU::Draw_ShadedLine_50 ()
{
	static constexpr double dShadedLine_50_CyclesPerPixel = 2.0L;

	s32 distance, x_distance, y_distance;
	
	s64 dx;
	s64 dy;
	
	s64 dxdy, dydx;
	
	s64 dxdc;
	s64 dydc;
	s64 drdc, dgdc, dbdc;
	s64 line_x, line_y;
	s64 line_r, line_g, line_b;
	
	s64 line_x0, line_y0;
	s64 line_x1, line_y1;
	
	s64 TestTop, TestBottom, TestLeft, TestRight;
	
	s64 Temp, Temp0, Temp1, TempX;
	
	//s64 rc;
	u16* ptr16;
	
	// new local variables
	s32 x0, x1, y0, y1;
	u32 bgr;
	s64 iX, iY;
	s32 r0, r1, g0, g1, b0, b1;
	
	s64* DitherArray;
	s64 DitherValue;
	
	
	s64 Red, Green, Blue;
	s64 iR, iG, iB;

	u32 DestPixel, PixelMask = 0, SetPixelMask = 0;

	
	if ( GPU_CTRL_Read.DTD )
	{
		DitherArray = (s64*) c_iDitherValues24;
	}
	
	NumberOfPixelsDrawn = 0;

	if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
	if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;
	
	// get the x-values
	x0 = gx [ 0 ];
	x1 = gx [ 1 ];
	
	// get the y-values
	y0 = gy [ 0 ];
	y1 = gy [ 1 ];
	
	// get color components
	//r0 = bgr0 & 0xff;
	//r1 = bgr1 & 0xff;
	//g0 = ( bgr0 >> 8 ) & 0xff;
	//g1 = ( bgr1 >> 8 ) & 0xff;
	//b0 = ( bgr0 >> 16 ) & 0xff;
	//b1 = ( bgr1 >> 16 ) & 0xff;
	r0 = gr [ 0 ];
	r1 = gr [ 1 ];
	g0 = gg [ 0 ];
	g1 = gg [ 1 ];
	b0 = gb [ 0 ];
	b1 = gb [ 1 ];
	
	line_r = r0 << 16;
	line_g = g0 << 16;
	line_b = b0 << 16;
	
	// go ahead and add the intial fractional part
	line_r |= 0x8000;
	line_g |= 0x8000;
	line_b |= 0x8000;
	
	//////////////////////////////////////////
	// get coordinates on screen
	x0 = DrawArea_OffsetX + x0;
	y0 = DrawArea_OffsetY + y0;
	x1 = DrawArea_OffsetX + x1;
	y1 = DrawArea_OffsetY + y1;
	
	
	// make sure line might be on screen
	if ( ( ( x1 <= ((s32)DrawArea_TopLeftX) ) && ( x0 < ((s32)DrawArea_TopLeftX) ) ) ||
		( ( x1 >= ((s32)DrawArea_BottomRightX) ) && ( x0 > ((s32)DrawArea_BottomRightX) ) ) ||
		( ( y1 <= ((s32)DrawArea_TopLeftY) ) && ( y0 < ((s32)DrawArea_TopLeftY) ) ) ||
		( ( y1 >= ((s32)DrawArea_BottomRightY) ) && ( y0 > ((s32)DrawArea_BottomRightY) ) ) ) return;

	// get size of line
	x_distance = _Abs( x1 - x0 );
	y_distance = _Abs( y1 - y0 );
	//if ( x_distance > y_distance ) distance = x_distance; else distance = y_distance;

	// if we don't draw the final point, then a distance of zero does not get drawn
	// if both x and y distance are zero, then do not draw
	//if ( !distance ) return;
	if ( ! ( x_distance | y_distance ) ) return;
	
	// x.48 fixed point
	if ( x_distance ) dx = ( 1LL << 32 ) / x_distance; else dx = 0;
	if ( y_distance ) dy = ( 1LL << 32 ) / y_distance; else dy = 0;
	
	// x.32 fixed point
	dxdy = ( ( (s64) ( x1 - x0 ) ) * dy );
	dydx = ( ( (s64) ( y1 - y0 ) ) * dx );

	//drdx = ( ( (s64) ( r1 - r0 ) ) * dx ) >> 16;
	//dgdx = ( ( (s64) ( g1 - g0 ) ) * dx ) >> 16;
	//dbdx = ( ( (s64) ( b1 - b0 ) ) * dx ) >> 16;

	//drdy = ( ( (s64) ( r1 - r0 ) ) * dy ) >> 16;
	//dgdy = ( ( (s64) ( g1 - g0 ) ) * dy ) >> 16;
	//dbdy = ( ( (s64) ( b1 - b0 ) ) * dy ) >> 16;
	
	
	line_x0 = ( x0 << 16 );
	line_y0 = ( y0 << 16 );
	
	line_x1 = ( x1 << 16 );
	line_y1 = ( y1 << 16 );

	// initial fractional part
	line_x0 |= 0x8000;
	line_y0 |= 0x8000;
	line_x1 |= 0x8000;
	line_y1 |= 0x8000;
	
	
	// get window for the end point
	TestLeft = ( DrawArea_TopLeftX << 16 );
	TestRight = ( DrawArea_BottomRightX << 16 ) | 0xffff;
	//TestRight = ( ( DrawArea_BottomRightX + 1 ) << 16 );
	TestTop = ( DrawArea_TopLeftY << 16 );
	TestBottom = ( DrawArea_BottomRightY << 16 ) | 0xffff;
	//TestBottom = ( ( DrawArea_BottomRightY + 1 ) << 16 );
	
	
	// if start coord is off to the left or right, then fix
	if ( line_x0 < TestLeft && x1 > x0 )
	{
		// line must be headed to the right (dx positive)
		// but would have to be headed to the right or would not have reached here
		//if ( x1 <= x0 ) return;
		
		Temp = TestLeft - line_x0;
		line_y0 += ( ( Temp * dydx ) >> 32 );
		line_x0 = TestLeft;
	}
	else if ( line_x0 > TestRight && x1 < x0 )
	{
		// line must be headed to the left (dx negative)
		// but would have to be headed to the left or would not have reached here
		//if ( x1 >= x0 ) return;
		
		//Temp = TestRight - line_x0;
		Temp = line_x0 - TestRight;
		line_y0 += ( ( Temp * dydx ) >> 32 );
		line_x0 = TestRight;
	}
	

	// if start coord is off to the top or bottom, then fix
	if ( line_y0 < TestTop && y1 > y0 )
	{
		// line must be headed down (dy positive)
		// but would have to be headed down or would not have reached here
		//if ( y1 <= y0 ) return;
		
		Temp = TestTop - line_y0;
		line_x0 += ( ( Temp * dxdy ) >> 32 );
		line_y0 = TestTop;
	}
	else if ( line_y0 > TestBottom && y1 < y0 )
	{
		// line must be headed up (dy negative)
		// but would have to be headed up or would not have reached here
		//if ( y1 >= y0 ) return;
		
		//Temp = TestBottom - line_y0;
		Temp = line_y0 - TestBottom;
		line_x0 += ( ( Temp * dxdy ) >> 32 );
		line_y0 = TestBottom;
	}
	
	// round to nearest
	//line_x += 0x8000;
	//line_y += 0x8000;
	
	// make sure starting point is within draw window
	//x = line_x >> 16;
	//y = line_y >> 16;
	//if ( ( x < ((s32)DrawArea_TopLeftX) ) || ( x > ((s32)DrawArea_BottomRightX) ) || ( y < ((s32)DrawArea_TopLeftY) ) || ( y > ((s32)DrawArea_BottomRightY) ) ) return;
	// but also need to check again later, because could still be offscreen
	if ( ( line_x0 < TestLeft ) || ( line_x0 > TestRight ) || ( line_y0 < TestTop ) || ( line_y0 > TestBottom ) ) return;

	
	// if line endpoint is off, then fix
	// just need to trace back to exact point exact line is out of window
	if ( line_x1 < TestLeft )
	{
		//Temp = TestLeft - line_x1;
		Temp = line_x1 - TestLeft;
		line_y1 += ( ( Temp * dydx ) >> 32 );
		line_x1 = TestLeft;
	}
	else if ( line_x1 > TestRight )
	{
		Temp = TestRight - line_x1;
		line_y1 += ( ( Temp * dydx ) >> 32 );
		line_x1 = TestRight;
	}
	

	if ( line_y1 < TestTop )
	{
		//Temp = TestTop - line_y1;
		Temp = line_y1 - TestTop;
		line_x1 += ( ( Temp * dxdy ) >> 32 );
		line_y1 = TestTop;
	}
	else if ( line_y1 > TestBottom )
	{
		Temp = TestBottom - line_y1;
		line_x1 += ( ( Temp * dxdy ) >> 32 );
		line_y1 = TestBottom;
	}
	
	// set end point to draw to
	//line_x1 = line_x;
	//line_y1 = line_y;
	

	// get final distance to draw
	if ( x_distance > y_distance )
	{
		// round to nearest
		//line_x2 += 0x8000;
		
		// set dxdc,dydc
		if ( x1 > x0 ) dxdc = ( 1 << 16 ); else dxdc = ( -1 << 16 );
		
		// x.16 fixed point
		dydc = ( dydx >> 16 );
		
		// color values
		drdc = ( ( r1 - r0 ) * dx ) >> 16;
		dgdc = ( ( g1 - g0 ) * dx ) >> 16;
		dbdc = ( ( b1 - b0 ) * dx ) >> 16;
		
		// check if pixel is off 
		
		if ( x1 > x0 )
		{
			// line is going to right //
			
			// make sure start x is at .5 interval
			line_x = ( ( line_x0 + 0x7fff ) | 0x8000 ) & ~0x7fff;
			
			// make sure end x is at .5 interval
			line_y = ( ( line_x1 - 0x8000 ) | 0x8000 ) & ~0x7fff;
			
			// get difference
			Temp0 = line_x - line_x0;
			
			// get difference
			//Temp = line_x1 - line_y;
			Temp1 = line_y - line_x1;
			
			// get pixels since start of line
			TempX = ( line_x >> 16 ) - x0;
		}
		else
		{
			// line is going to left //
			
			// make sure end x is at .5 interval
			line_x = ( ( line_x0 - 0x8000 ) | 0x8000 ) & ~0x7fff;
			
			// make sure start x is at .5 interval
			line_y = ( ( line_x1 + 0x7fff ) | 0x8000 ) & ~0x7fff;
			
			// get difference
			Temp0 = line_x0 - line_x;
			//Temp0 = line_x - line_x0;
			
			// get difference
			Temp1 = line_x1 - line_y;
			//Temp1 = line_y - line_x1;
			
			// get pixels since start of line
			TempX = x0 - ( line_x >> 16 );
		}

		
		// update y
		line_y0 += ( Temp0 * dydx ) >> 32;
		line_x0 = line_x;
		
		// update y
		line_y1 += ( Temp1 * dydx ) >> 32;
		line_x1 = line_y;
		
		// distance is in x direction
		distance = _Abs ( ( line_x0 >> 16 ) - ( line_x1 >> 16 ) );
		
		line_r += TempX * drdc;
		line_g += TempX * dgdc;
		line_b += TempX * dbdc;
	}
	else
	{
		// round to nearest
		//line_y2 += 0x8000;
		
		// set dxdc,dydc
		if ( y1 > y0 ) dydc = ( 1 << 16 ); else dydc = ( -1 << 16 );
		
		// x.16 fixed point
		dxdc = ( dxdy >> 16 );
		
		// color values
		drdc = ( ( r1 - r0 ) * dy ) >> 16;
		dgdc = ( ( g1 - g0 ) * dy ) >> 16;
		dbdc = ( ( b1 - b0 ) * dy ) >> 16;
		
		if ( y1 > y0 )
		{
			// line is going to bottom //
			
			// make sure start y is at .5 interval
			line_x = ( ( line_y0 + 0x7fff ) | 0x8000 ) & ~0x7fff;
			
			// make sure end y is at .5 interval
			line_y = ( ( line_y1 - 0x8000 ) | 0x8000 ) & ~0x7fff;
			
			// get difference
			Temp0 = line_x - line_y0;
			
			// get difference
			//Temp = line_x1 - line_y;
			Temp1 = line_y - line_y1;
			
			// get pixels since start of line
			TempX = ( line_x >> 16 ) - y0;
		}
		else
		{
			// line is going to top //
			
			// make sure start y is at .5 interval
			line_x = ( ( line_y0 - 0x8000 ) | 0x8000 ) & ~0x7fff;
			
			// make sure end y is at .5 interval
			line_y = ( ( line_y1 + 0x7fff ) | 0x8000 ) & ~0x7fff;
			
			// get difference
			Temp0 = line_y0 - line_x;
			//Temp0 = line_x - line_x0;
			
			// get difference
			Temp1 = line_y1 - line_y;
			//Temp1 = line_y - line_x1;
			
			// get pixels since start of line
			TempX = y0 - ( line_x >> 16 );
		}

		// update x
		line_x0 += ( Temp0 * dxdy ) >> 32;
		line_y0 = line_x;
		
		// update x
		line_x1 += ( Temp1 * dxdy ) >> 32;
		line_y1 = line_y;
		
		// distance is in y direction
		distance = _Abs ( ( line_y0 >> 16 ) - ( line_y1 >> 16 ) );
		
		// can calculate rgb start here using drdc,dgdc,dbdc
		//Temp = ( ( line_y0 >> 16 ) - y0 );
		line_r += TempX * drdc;
		line_g += TempX * dgdc;
		line_b += TempX * dbdc;
	}

	// if starting point is outside of window, then update
	//if ( line_x0 >= TestRight || line_y0 >= TestBottom )
	//{
	//	line_x0 += dxdc;
	//	line_y0 += dydc;
	//}
	
	// if ending point is outside of window, then update
	//if ( line_x1 >= TestRight || line_y1 >= TestBottom )
	//{
	//	line_x1 -= dxdc;
	//	line_y1 -= dydc;
	//}
	
	
	// if the last point on screen is not the endpoint for entire line, then include it
	if ( x1 != ( line_x1 >> 16 ) || y1 != ( line_y1 >> 16 ) ) distance++;


	// draw the line
	// note: the final point should probably not be drawn
	//for ( u32 i = 0; i <= distance; i++ )
	for ( u32 i = 0; i < distance; i++ )
	{
		// get coords
		//iX = ( _Round( line_x ) >> 32 );
		//iY = ( _Round( line_y ) >> 32 );
		iX = ( line_x0 >> 16 );
		iY = ( line_y0 >> 16 );
		
		//if ( iX >= DrawArea_TopLeftX && iY >= DrawArea_TopLeftY
		//&& iX <= DrawArea_BottomRightX && iY <= DrawArea_BottomRightY )
		//{
			if ( GPU_CTRL_Read.DTD )
			{
				DitherValue = DitherArray [ ( iX & 0x3 ) + ( ( iY & 0x3 ) << 2 ) ];
				
				iR = line_r << 8;
				iG = line_g << 8;
				iB = line_b << 8;
				
				// perform dither
				Red = iR + DitherValue;
				Green = iG + DitherValue;
				Blue = iB + DitherValue;
				
				// perform shift
				Red >>= 27;
				Green >>= 27;
				Blue >>= 27;
				
				// if dithering, perform signed clamp to 5 bits
				Red = AddSignedClamp<s64,5> ( Red );
				Green = AddSignedClamp<s64,5> ( Green );
				Blue = AddSignedClamp<s64,5> ( Blue );
				
				bgr = Red | ( Green << 5 ) | ( Blue << 10 );
			}
			else
			{
				// perform shift
				//Red = ( iR >> 27 );
				//Green = ( iG >> 27 );
				//Blue = ( iB >> 27 );
				
				bgr = ( line_r >> 19 ) | ( ( line_g >> 19 ) << 5 ) | ( ( line_b >> 19 ) << 10 );
			}
			
			//bgr = ( line_r >> 19 ) | ( ( line_g >> 19 ) << 5 ) | ( ( line_b >> 19 ) << 10 );
			
			ptr16 = & ( VRAM [ iX + ( iY << 10 ) ] );
			
			// read pixel from frame buffer if we need to check mask bit
			DestPixel = *ptr16;
		
			// semi-transparency
			if ( command_abe )
			{
				bgr = SemiTransparency16 ( DestPixel, bgr, GPU_CTRL_Read.ABR );
			}
				
			// draw point
			
			// check if we should set mask bit when drawing
			//if ( GPU_CTRL_Read.MD ) bgr |= 0x8000;

			// draw pixel if we can draw to mask pixels or mask bit not set
			if ( ! ( DestPixel & PixelMask ) ) *ptr16 = ( bgr | SetPixelMask );
			
			/////////////////////////////////////////////////////
			// update number of cycles used to draw polygon
			//NumberOfPixelsDrawn++;
		//}
		
		line_x0 += dxdc;
		line_y0 += dydc;
		line_r += drdc;
		line_g += dgdc;
		line_b += dbdc;
	}
	
	NumberOfPixelsDrawn = distance;
	BusyCycles += NumberOfPixelsDrawn * dShadedLine_50_CyclesPerPixel;
	
#ifdef ENABLE_DRAW_OVERHEAD
	BusyCycles += DrawOverhead_Cycles;
#endif
}



void GPU::Set_ScreenSize ( int _width, int _height )
{
	DisplayOutput_Window->OpenGL_MakeCurrentWindow ();
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glOrtho (0, _width, _height, 0, 0, 1);
	glMatrixMode (GL_MODELVIEW);
	glDisable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT);
	DisplayOutput_Window->OpenGL_ReleaseWindow ();
}

/*
// as long as pixel mask is zero
int GPU::Flush_TransferIn ( u64 ullReadIdx )
{
	int iStartX, iStartY;
	int iSrcX, iSrcY;
	int iDstX, iDstY;
	int iWidth, iHeight;
	int iFlags;
	u64 uIndex;

	int iCurX, iCurY;
	int iCurWidth;
	int iRun1, iRun2, iRun3;
	int iLeftX, iRightX;

	uIndex = ( ullReadIdx & c_ulInputBuffer_Mask ) << 4;

	iFlags = inputdata [ uIndex + 0 ];

	// nocash psx specifications: transfer/move vram-to-vram use masking
	// ME is bit 12
	//if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
	PixelMask = ( iFlags & 0x1000 ) << 3;
	
	// MD is bit 11
	//if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;
	SetPixelMask = ( iFlags & 0x0800 ) << 4;

	if ( PixelMask )
	{
		// need to copy command into hw buffer instead here //
		// return count of commands handled
		return 0;
	}

	iDstX = int( inputdata [ uIndex + 1 ] );
	iDstY = int( inputdata [ uIndex + 2 ] );
	iWidth = int( inputdata [ uIndex + 3 ] );
	iHeight = int( inputdata [ uIndex + 4 ] );
	//iX = int( inputdata [ uIndex + 5 ] );
	//iY = int( inputdata [ uIndex + 6 ] );
	iSrcX = int( inputdata [ uIndex + 5 ] );
	iSrcY = int( inputdata [ uIndex + 6 ] );
	
	BS = int( inputdata [ uIndex + 7 ] & 0xf );

	// count is per 2 pixels
	BS <<= 1;
	
	xximagepixelstart = iSrcX + iSrcY * iWidth;


	// Xsiz=((Xsiz-1) AND 3FFh)+1
	iWidth = ( ( iWidth - 1 ) & 0x3ff ) + 1;
	
	// Ysiz=((Ysiz-1) AND 1FFh)+1
	iHeight = ( ( iHeight - 1 ) & 0x1ff ) + 1;

	iMaxX = iDstX + iWidth;
	iMaxY = iDstY + iHeight;
	
	// xpos & 0x3ff
	//sX &= 0x3ff;
	iSrcX = sX & 0x3ff;
	//dX &= 0x3ff;
	iDstX = dX & 0x3ff;
	
	// ypos & 0x1ff
	//sY &= 0x1ff;
	iSrcY = sY & 0x1ff;
	//dY &= 0x1ff;
	iDstY = dY & 0x1ff;

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo1);

	// copy the pixels into hardware buffer
	for ( iCount = 0; iCount < BS; iCount++ )
	{
		uPixel32 = ((u16*)inputdata) [ ( uIndex << 1 ) + 16 + iCount ];
		p_uHwInputData32 [ ( ( ullReadIdx & c_ulInputBuffer_Mask ) << 4 ) + iCount ] = uPixel32 | SetPixelMask;
	}

	// copy pixels into VRAM
	iCount = BS;
	iCurX = iSrcX;
	iCurY = iSrcY;

	while ( iCount )
	{
		iRun1 = iCount;
		if ( ( iCurX + iRun1 ) > iMaxX ) iRun1 = iMaxX - iCurX;
		if ( ( ( iCurX & c_lFrameBuffer_Width_Mask ) + iRun1 ) > c_lFrameBuffer_Width ) iRun1 = c_lFrameBuffer_Width - iCurX;
		
		glCopyBufferSubData ( GL_COPY_READ_BUFFER, GL_SHADER_STORAGE_BUFFER, ( ullReadIdx & c_ulInputBuffer_Mask ) << 4, ( ( iCurX & FrameBuffer_XMask ) + ( ( iCurY & FrameBuffer_YMask ) << 10 ) ) << 2, iRun1 << 2 );

		if ( iCurX >= iMaxX )
		{
			iCurX = 0;
			iCurY++;
		}

		iCount -= iRun1;
	}


	// at end return count of commands handled
	return 1;
}

// as long as pixel mask is zero and set pixel mask is zero
int GPU::Flush_TransferMove ( u64 ullReadIdx )
{
	int iStartX, iStartY;
	int iSrcX, iSrcY;
	int iDstX, iDstY;
	int iWidth, iHeight;
	int iFlags;
	u64 uIndex;

	int iCurX, iCurY;
	int iCurWidth;
	int iRun1, iRun2, iRun3;
	int iLeftX, iRightX;

	uIndex = ( ullReadIdx & c_ulInputBuffer_Mask ) << 4;

	iFlags = inputdata [ uIndex + 0 ];

	// nocash psx specifications: transfer/move vram-to-vram use masking
	// ME is bit 12
	//if ( GPU_CTRL_Read.ME ) PixelMask = 0x8000;
	PixelMask = ( iFlags & 0x1000 ) << 3;
	
	// MD is bit 11
	//if ( GPU_CTRL_Read.MD ) SetPixelMask = 0x8000;
	SetPixelMask = ( iFlags & 0x0800 ) << 4;

	if ( PixelMask || SetPixelMask )
	{
		// don't have a solution for this yet-> send command to hardware
		// return count of commands handled
		return 0;
	}

	iWidth = inputdata [ uIndex + 10 ];
	iHeight = iWidth >> 16;

	// Xsiz=((Xsiz-1) AND 3FFh)+1
	iWidth = ( ( iWidth - 1 ) & 0x3ff ) + 1;
	
	// Ysiz=((Ysiz-1) AND 1FFh)+1
	iHeight = ( ( iHeight - 1 ) & 0x1ff ) + 1;

	
	iSrcX = inputdata [ uIndex + 8 ];
	iSrcY = sX >> 16;
	iDstX = inputdata [ uIndex + 9 ];
	iDstY = iDstX >> 16;

	
	// xpos & 0x3ff
	//sX &= 0x3ff;
	iSrcX = sX & 0x3ff;
	//dX &= 0x3ff;
	iDstX = dX & 0x3ff;
	
	// ypos & 0x1ff
	//sY &= 0x1ff;
	iSrcY = sY & 0x1ff;
	//dY &= 0x1ff;
	iDstY = dY & 0x1ff;

	iLeftX = iSrcX;
	iRightX = iDstX;
	if ( iLeftX > iRightX )
	{
		iLeftX = iDstX;
		iRightX = iSrcX;
	}

	iRun1 = iWidth;
	iRun2 = 0;
	iRun3 = 0;
	if ( ( iRightX + iWidth ) >= FrameBuffer_Width )
	{
		iRun1 = iWidth - FrameBuffer_Width;

		iRun2 = iWidth - iRun1;
		if ( iLeftX != iRightX )
		{
			if ( ( iLeftX + iWidth ) >= FrameBuffer_Width )
			{
				iRun2 = iRightX - iLeftX;
				iRun3 = iWidth - ( iRun1 + iRun2 );
			}
		}
	}



	// put in the move commands
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo1);
	for ( iCurY = 0; iCurY < iHeight; iCurY++ )
	{
		iCurX = 0;
		glCopyBufferSubData ( GL_SHADER_STORAGE_BUFFER, GL_SHADER_STORAGE_BUFFER, ( ( ( iSrcX + iCurX ) & FrameBuffer_XMask ) + ( ( ( iSrcY + iCurY ) & FrameBuffer_YMask ) << 10 ) ) << 2, ( ( ( iDstX + iCurX ) & FrameBuffer_XMask ) + ( ( ( iDstY + iCurY ) & FrameBuffer_YMask ) << 10 ) ) << 2, iRun1 << 2 );

		if ( iRun2 )
		{
			iCurX += iRun1;
			glCopyBufferSubData ( GL_SHADER_STORAGE_BUFFER, GL_SHADER_STORAGE_BUFFER, ( ( ( iSrcX + iCurX ) & FrameBuffer_XMask ) + ( ( ( iSrcY + iCurY ) & FrameBuffer_YMask ) << 10 ) ) << 2, ( ( ( iDstX + iCurX ) & FrameBuffer_XMask ) + ( ( ( iDstY + iCurY ) & FrameBuffer_YMask ) << 10 ) ) << 2, iRun2 << 2 );
			if ( iRun3 )
			{
				iCurX += iRun2;
				glCopyBufferSubData ( GL_SHADER_STORAGE_BUFFER, GL_SHADER_STORAGE_BUFFER, ( ( ( iSrcX + iCurX ) & FrameBuffer_XMask ) + ( ( ( iSrcY + iCurY ) & FrameBuffer_YMask ) << 10 ) ) << 2, ( ( ( iDstX + iCurX ) & FrameBuffer_XMask ) + ( ( ( iDstY + iCurY ) & FrameBuffer_YMask ) << 10 ) ) << 2, iRun3 << 2 );
			}
		}


		}


	}

	// return count of commands handled
	return 1;
}
*/

// send circular buffer data with draw commands to gpu hardware
// starting from ullReadIdx, up to but not including ullWriteIdx
//#ifdef ENABLE_HWPIXEL_INPUT
//void GPU::FlushToHardware ( u64 ullReadIdx, u64 ullWriteIdx, u64 ullReadPixelIdx, u64 ullWritePixelIdx )
//#else
//void GPU::FlushToHardware ( u64 ullReadIdx, u64 ullWriteIdx )
//#endif
void GPU::FlushToHardware ( u64 ullReadIdx, u64 ullWriteIdx, u64 ullReadPixelIdx, u64 ullWritePixelIdx )
{
	u64 ullCircularBufferEdge;
	u64 ullSourceIndex;
	u64 ullSourceIndex2;
	u64 ullTransferByteCount;
	u64 ullTransferByteCount2;

	// check to make sure there is data to send to gpu hardware
	if ( ullReadIdx >= ullWriteIdx )
	{
		// nothing to send to gpu hardware //
		return;
	}

	DisplayOutput_Window->OpenGL_MakeCurrentWindow ();

	// first flush data/pixel input buffer //
#ifdef ENABLE_HWPIXEL_INPUT

	ullCircularBufferEdge = ( ( ullReadPixelIdx | c_ullPixelInBuffer_Mask ) + 1 );

	glBindBuffer ( GL_COPY_READ_BUFFER, ssbo_sinputpixels );
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, ssbo_inputpixels);
	if( ullWritePixelIdx <= ullCircularBufferEdge )
	{
		// first copy data into hardware buffer
		ullSourceIndex = ( ullReadPixelIdx & c_ullPixelInBuffer_Mask ) << 0;
		ullTransferByteCount = 1 * 4 * ( ullWritePixelIdx - ullReadPixelIdx );

		memcpy( & p_ulHwPixelInBuffer32 [ ullSourceIndex ], & ulPixelInBuffer32 [ ullSourceIndex ], ullTransferByteCount );

		// now dispatch a copy command
		glCopyBufferSubData ( GL_COPY_READ_BUFFER, GL_SHADER_STORAGE_BUFFER, ullSourceIndex << 2, 0, ullTransferByteCount );

	}
	else
	{
		// circular buffer is wrapping around //

		//cout << "***BUFFER WRAP***";

		ullSourceIndex = ( ullReadPixelIdx & c_ullPixelInBuffer_Mask ) << 0;
		ullTransferByteCount = 1 * 4 * ( ullCircularBufferEdge - ullReadPixelIdx );

		ullSourceIndex2 = 0;
		ullTransferByteCount2 = 1 * 4 * ( ullWritePixelIdx & c_ullPixelInBuffer_Mask );

		// first copy the data in
		memcpy( & p_ulHwPixelInBuffer32 [ ullSourceIndex ], & ulPixelInBuffer32 [ ullSourceIndex ], ullTransferByteCount );
		memcpy( & p_ulHwPixelInBuffer32 [ ullSourceIndex2 ], & ulPixelInBuffer32 [ ullSourceIndex2 ], ullTransferByteCount2 );

		// now copy the buffers
		glCopyBufferSubData ( GL_COPY_READ_BUFFER, GL_SHADER_STORAGE_BUFFER, ullSourceIndex << 2, 0, ullTransferByteCount );
		glCopyBufferSubData ( GL_COPY_READ_BUFFER, GL_SHADER_STORAGE_BUFFER, ullSourceIndex2 << 2, ullTransferByteCount, ullTransferByteCount2 );

	}
#endif


	// now flush the commands //

	//cout << "\nSending Flush to gpu render. ReadIdx= " << dec << ulInputBuffer_ReadIndex << " WriteIdx= " << ulInputBuffer_WriteIndex;
	ullCircularBufferEdge = ( ( ullReadIdx | c_ulInputBuffer_Mask ) + 1 );

	glBindBuffer ( GL_COPY_READ_BUFFER, ssbo_sinputdata );
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo1);
	if( ullWriteIdx <= ullCircularBufferEdge )
	{
		// first copy data into hardware buffer
		ullSourceIndex = ( ullReadIdx & c_ulInputBuffer_Mask ) << 4;
		ullTransferByteCount = 16 * 4 * ( ullWriteIdx - ullReadIdx );

		memcpy( & p_uHwInputData32 [ ullSourceIndex ], & inputdata [ ullSourceIndex ], ullTransferByteCount );

		// now dispatch a copy command
		glCopyBufferSubData ( GL_COPY_READ_BUFFER, GL_SHADER_STORAGE_BUFFER, ullSourceIndex << 2, 0, ullTransferByteCount );

		glDispatchCompute(1, 1, 1);
		glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );
	}
	else
	{
		// circular buffer is wrapping around //

		//cout << "***BUFFER WRAP***";

		ullSourceIndex = ( ullReadIdx & c_ulInputBuffer_Mask ) << 4;
		ullTransferByteCount = 16 * 4 * ( ullCircularBufferEdge - ullReadIdx );

		ullSourceIndex2 = 0;
		ullTransferByteCount2 = 16 * 4 * ( ullWriteIdx & c_ulInputBuffer_Mask );

		// first copy the data in
		memcpy( & p_uHwInputData32 [ ullSourceIndex ], & inputdata [ ullSourceIndex ], ullTransferByteCount );
		memcpy( & p_uHwInputData32 [ ullSourceIndex2 ], & inputdata [ ullSourceIndex2 ], ullTransferByteCount2 );

		// now copy the buffers
		glCopyBufferSubData ( GL_COPY_READ_BUFFER, GL_SHADER_STORAGE_BUFFER, ullSourceIndex << 2, 0, ullTransferByteCount );
		glCopyBufferSubData ( GL_COPY_READ_BUFFER, GL_SHADER_STORAGE_BUFFER, ullSourceIndex2 << 2, ullTransferByteCount, ullTransferByteCount2 );

		glDispatchCompute(1, 1, 1);
		glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );
	}

	//glFlush ();


	DisplayOutput_Window->OpenGL_ReleaseWindow ();
}

/*
void GPU::Flush_HwCommands ( u64 ullReadIdx, u64 ullWriteIdx )
{
	u64 ullIdx;
	int iCount;
	int iRet;

	// loop through commands
	iCount = 0;
	for ( ullIdx = ullReadIdx; ullIdx < ullWriteIdx; ullIdx++ )
	{
		uIndex = ( ullIdx & c_ulInputBuffer_Mask ) << 4;

		uCommand = inputdata [ uIndex + 7 ] >> 24;

		if ( ( uCommand > 0x80 ) && !( inputdata [ uIndex + 0 ] & 0x1000 ) )
		{
			// pixel transfer in //

			// flush the previous commands at max settings
			// btw ullIdx and ullReadIdx, not including ullIdx
			FlushToHardware( ullReadIdx, ullIdx, iCount );
			iCount = ullIdx - ullReadIdx;

			// 
			Flush_TransferIn ( ullIdx, iCount );
		}

	}

	if ( ullReadIdx != ullWriteIdx )
	{
		FlushToHardware ( ullIdx, ullWriteIdx );
	}
}
*/

void GPU::Flush ()
{



	if ( _GPU->ulNumberOfThreads_Created )
	{
		if ( ulInputBuffer_WriteIndex != ulInputBuffer_TargetIndex )
		{
			// send the command to the other thread
			//Lock_ExchangeAdd32 ( (long&) ulInputBuffer_Count, ulNumberOfThreads );
			x64ThreadSafe::Utilities::Lock_Exchange64 ( (long long&) ulInputBuffer_TargetIndex, ulInputBuffer_WriteIndex );
			
			// trigger event
			if ( !SetEvent ( ghEvent_PS1GPU_Update ) )
			{
				cout << "\nUnable to set PS1 GPU UPDATE event. " << GetLastError ();
			}
		}
	}
	else
	{
		if ( _GPU->bEnable_OpenCL )
		{
			if ( ulInputBuffer_WriteIndex != ulInputBuffer_ReadIndex )
			{

				FlushToHardware ( ulInputBuffer_ReadIndex, ulInputBuffer_WriteIndex, ullPixelInBuffer_ReadIndex, ullPixelInBuffer_WriteIndex );
				
				ulInputBuffer_TargetIndex = ulInputBuffer_WriteIndex;
				ulInputBuffer_ReadIndex = ulInputBuffer_WriteIndex;

				ullPixelInBuffer_ReadIndex = ullPixelInBuffer_WriteIndex;
				ullPixelInBuffer_TargetIndex = ullPixelInBuffer_WriteIndex;
			}
		}	// end if ( bEnable_OpenCL )

	}
}

void GPU::Finish ()
{

	if ( _GPU->ulNumberOfThreads_Created )
	{
		if ( ulInputBuffer_WriteIndex != ulInputBuffer_ReadIndex )
		{
			if ( ulInputBuffer_WriteIndex != ulInputBuffer_TargetIndex )
			{
				// send the command to the other thread
				//Lock_ExchangeAdd32 ( (long&) ulInputBuffer_Count, ulNumberOfThreads );
				x64ThreadSafe::Utilities::Lock_Exchange64 ( (long long&) ulInputBuffer_TargetIndex, ulInputBuffer_WriteIndex );
				
				if ( !ResetEvent ( ghEvent_PS1GPU_Finish ) )
				{
					cout << "\nUnable to reset finish event before update. " << GetLastError ();
				}
				
				// trigger event
				if ( !SetEvent ( ghEvent_PS1GPU_Update ) )
				{
					cout << "\nUnable to set PS1 GPU UPDATE event. " << GetLastError ();
				}
			}
			
			// wait for the other thread to complete
			//while ( ulInputBuffer_WriteIndex != ulInputBuffer_ReadIndex );
			//while ( ulInputBuffer_WriteIndex != x64ThreadSafe::Utilities::Lock_ExchangeAdd64 ( (long long&)ulInputBuffer_ReadIndex, 0 ) )
			//{
				WaitForSingleObject ( ghEvent_PS1GPU_Finish, INFINITE );
				
				// trigger event ??
				//if ( !SetEvent ( ghEvent_PS1GPU_Update ) )
				//{
				//	cout << "\nUnable to set PS1 GPU UPDATE event. " << GetLastError ();
				//}
			//}
			//while ( MsgWaitForMultipleObjectsEx ( 1, &ghEvent_PS1GPU_Finish, INFINITE, QS_ALLINPUT, MWMO_ALERTABLE ) > WAIT_OBJECT_0 )
			//MsgWaitForMultipleObjects ( 1, &ghEvent_PS1GPU_Finish, FALSE, INFINITE, QS_ALLINPUT );
			//{
			//	SetEvent ( ghEvent_PS1GPU_Update );
			//	WindowClass::DoEventsNoWait ();
			//}
		}
	}
	else
	{
		if ( _GPU->bEnable_OpenCL )
		{
			if ( ulInputBuffer_WriteIndex != ulInputBuffer_ReadIndex )
			{

				FlushToHardware ( ulInputBuffer_ReadIndex, ulInputBuffer_WriteIndex, ullPixelInBuffer_ReadIndex, ullPixelInBuffer_WriteIndex );

				ulInputBuffer_TargetIndex = ulInputBuffer_WriteIndex;
				ulInputBuffer_ReadIndex = ulInputBuffer_WriteIndex;

				ullPixelInBuffer_ReadIndex = ullPixelInBuffer_WriteIndex;
				ullPixelInBuffer_TargetIndex = ullPixelInBuffer_WriteIndex;
			}
		}	// end if ( bEnable_OpenCL )

	}	// end if else if ( _GPU->ulNumberOfThreads_Created )

	// after GPU::Finish, readidx=writeidx=targetidx
	ulInputBuffer_ReadIndex = ulInputBuffer_WriteIndex;
	ulInputBuffer_TargetIndex = ulInputBuffer_WriteIndex;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GPU::DebugWindow_Enable ()
{

#ifndef _CONSOLE_DEBUG_ONLY_

	static constexpr char* DebugWindow_Caption = "PS1 FrameBuffer Debug Window";
	static const int DebugWindow_X = 10;
	static const int DebugWindow_Y = 10;
	static const int DebugWindow_Width = 1024;
	static const int DebugWindow_Height = 512;
	
	int i;
	long xsize, ysize;
	stringstream ss;
	
	cout << "\nGPU::DebugWindow_Enable";
	
	if ( !DebugWindow_Enabled )
	{
		// create the main debug window
		xsize = DebugWindow_Width;
		ysize = DebugWindow_Height;
		FrameBuffer_DebugWindow = new WindowClass::Window ();
		FrameBuffer_DebugWindow->GetRequiredWindowSize ( &xsize, &ysize, FALSE );
		FrameBuffer_DebugWindow->Create ( DebugWindow_Caption, DebugWindow_X, DebugWindow_Y, xsize /*DebugWindow_Width*/, ysize /*DebugWindow_Height + 50*/ );
		FrameBuffer_DebugWindow->DisableCloseButton ();
		
		cout << "\nFramebuffer: xsize=" << xsize << "; ysize=" << ysize;
		FrameBuffer_DebugWindow->GetWindowSize ( &xsize, &ysize );
		cout << "\nWindow Size. xsize=" << xsize << "; ysize=" << ysize;
		FrameBuffer_DebugWindow->GetViewableArea ( &xsize, &ysize );
		cout << "\nViewable Size. xsize=" << xsize << "; ysize=" << ysize;
		
		cout << "\nCreated main debug window";
		
		/////////////////////////////////////////////////////////
		// enable opengl for the frame buffer window
		FrameBuffer_DebugWindow->EnableOpenGL ();
		glMatrixMode (GL_PROJECTION);
		glLoadIdentity ();
		glOrtho (0, DebugWindow_Width, DebugWindow_Height, 0, 0, 1);
		glMatrixMode (GL_MODELVIEW);

		glDisable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT);

		// this window is no longer the window we want to draw to
		FrameBuffer_DebugWindow->OpenGL_ReleaseWindow ();
		
		DebugWindow_Enabled = true;
		
		cout << "\nEnabled opengl for frame buffer window";

		// update the value lists
		DebugWindow_Update ();
	}
	
		cout << "\n->GPU::DebugWindow_Enable";

#endif

}

void GPU::DebugWindow_Disable ()
{

#ifndef _CONSOLE_DEBUG_ONLY_

	if ( DebugWindow_Enabled )
	{
		delete FrameBuffer_DebugWindow;
	
		// disable debug window
		DebugWindow_Enabled = false;
	}
	
#endif

}

void GPU::DebugWindow_Update ()
{

#ifndef _CONSOLE_DEBUG_ONLY_

	if ( DebugWindow_Enabled )
	{
		_GPU->Draw_FrameBuffer ();
	}
	
#endif

}

