#ifndef __GLMETASEQ_H__
#define __GLMETASEQ_H__

/*=========================================================================================
	C / C ++ header which model you have created a (* .mqo) summarizes the function to read on the OpenGL in Metasequoia
=========================================================================================*/

/*
GLMetaseq
The MIT License
Copyright (c) 2009 Sunao Hashimoto and Keisuke Konishi

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.


GLMetaseq
MIT License
Copyright (c) 2009 Sunao Hashimoto and Keisuke Konishi

/*
----------------------------------------------------------------------------------------
	1. Notes on using this header
----------------------------------------------------------------------------------------
  
	●Readable image format of texture bmp, tga, jpeg, png
	But
	jpeg read in the JPEG library (libjpeg.lib, jpeglib.h) is required separately
	To enable the reading of the jpeg is making the DEF_USE_LIBJPEG of this header to 1
	PNG library to read the png (libpng.lib, zlib.lib, png.h, zlib.h) is required separately
	To enable the reading of the png is making the DEF_USE_LIBPNG of this header to 1

	●The size of the texture image is limited to a "square of one side 2 of the n-th power size (64, 128, 256 ...)"


----------------------------------------------------------------------------------------
	2. If you want to load and display how to use (1) one of MQO file
----------------------------------------------------------------------------------------

	(1) Initialization (in the case of ARToolKit, is used after the argInit ())

		mqoInit();

	(2) Reading of the model from a file

		MQO_MODEL model;
		model = mqoCreateModel( "mario.mqo", 1.0 );

	(3) Call model
		
		mqoCallModel( model );

	(4) Model erasure of

		mqoDeleteModel( model );

	(5) End processing (Please do at the end of the program)

		mqoCleanup();

----------------------------------------------------------------------------------------
	3. How to use (2) If you want to view by reading the serial number file
----------------------------------------------------------------------------------------

	(1) Initialization (in the case of ARToolKit, is used after the argInit ())

		mqoInit();

	(2) Creating a serial number sequence
	
	  　Example: I read the mario0.mqo ~ mario9.mqo

		MQO_SEQUENCE seq;
		seq = mqoCreateSequence( "mario%d.mqo", 10, 1.0 );

	(3) Call of the specified frame of the sequence number sequence (i is the frame number)
		
		mqoCallSequence( seq, i );

	(4) Erasure of the serial number sequence

		mqoDeleteSequence( seq );

	(5) End processing (Please do at the end of the program)

		mqoCleanup();

----------------------------------------------------------------------------------------
	4. Main Specifications
----------------------------------------------------------------------------------------

	●Display function
	? The supported versions of MQO file that is "Metasequoia Ver1.0 / 2.0 ~ 2.4"

	- Correspondingly, it has material information
		Color (light)
		Texture map (bump map non-compliant / UV mapping only)

- Correspondingly, are the object information
Switching of display / hide
The presence or absence of smoothing
Smoothing angle when determining the vertex normals
Vertex information
Surface information (vertex color not supported)

And surfaces, the mirror surface and rotating body is non-compliant
・ Metaball non-compliant

● Specifications
Chunk name, are supposed to be does not distinguish between uppercase and lowercase letters in principle but
We've distinguished.
? The path of the texture you're using a multi-byte character
That among the '\' (0x5c) や '/' (0x2f) is not read well texture and has entered.
- In the non-colored materials are not compatible.
Object chunk → face chunk of material index (M (% d)) is not supported to -1.

*/



/*=========================================================================
[Set by the user at any]
=========================================================================*/

#define MAX_TEXTURE				100			// Maximum handling number of texture
#define MAX_OBJECT				50			// The maximum number of objects in one of MQO file
#define SIZE_STR				256			// String buffer size
#define DEF_IS_LITTLE_ENDIAN	1			// Endian designation (intel system = 1)
#define DEF_USE_LIBJPEG			0			// use of libjpeg (1: use 0: Not used)
#define DEF_USE_LIBPNG			1			// use of libpng (1: use 0: Not used)



/*=========================================================================
【コンパイルオプション】
=========================================================================*/

// I use JPEG
#ifdef D_JPEG
	#undef	DEF_USE_LIBJPEG
	#define	DEF_USE_LIBJPEG 1
#endif

// I do not want to use the JPEG
#ifdef D_NO_JPEG
	#undef	DEF_USE_LIBJPEG
	#define	DEF_USE_LIBJPEG 0
#endif

// I use PNG
#ifdef D_PNG
	#undef	DEF_USE_LIBPNG
	#define	DEF_USE_LIBPNG 1
#endif

// I do not want to use the PNG
#ifdef D_NO_PNG
	#undef	DEF_USE_LIBPNG
	#define	DEF_USE_LIBPNG 0
#endif


/*=========================================================================
[Header]
=========================================================================*/

#ifdef WIN32
	#include <windows.h>
#else
	#ifndef MAX_PATH
		#define MAX_PATH    256
	#endif
	#ifndef TRUE
		#define TRUE    (1==1)
	#endif
	#ifndef FALSE
	    #define FALSE   (1!=1)
	#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef __APPLE__
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
	#include <GLUT/glut.h>
	#include <OpenGL/glext.h>
#else
	#include <GL/gl.h>
	#include <GL/glu.h>
	#include <GL/glut.h>
//	#include <gl/glext.h>
#endif


/*=========================================================================
[Function settings] libjpeg use setting
=========================================================================*/

#if DEF_USE_LIBJPEG

	#define XMD_H // It prevents the redefinition error of INT16 and INT32
	#ifdef FAR
		#undef FAR
	#endif

	#include "jpeglib.h"
	#pragma comment(lib,"libjpeg.lib")

#endif


/*=========================================================================
[Function settings] libpng use setting
=========================================================================*/

#if DEF_USE_LIBPNG

	#include "png.h"
	#include "zlib.h"
	#pragma comment(lib,"libpng.lib")
	#pragma comment(lib,"zlib.lib")

#endif


/*=========================================================================
[Macro definitions] maximum macro
=========================================================================*/

#ifndef MAX
	#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#endif


/*=========================================================================
[Type definition] TGA format
=========================================================================*/

#define DEF_TGA_COLOR_MAP_FLAG_VALID	1
#define DEF_TGA_TYPE_NON				0
#define DEF_TGA_TYPE_INDEX				1
#define DEF_TGA_TYPE_FULL				2
#define DEF_TGA_TYPE_MONO				3
#define DEF_TGA_TYPE_RLEINDEX			9
#define DEF_TGA_TYPE_RLEFULL			10
#define DEF_TGA_TYPE_RLEMONO			11
#define DEF_TGA_BIT_INFO_RIGHT_TO_LEFT	0x00
#define DEF_TGA_BIT_INFO_LEFT_TO_RIGHT	0x10
#define DEF_TGA_BIT_INFO_DOWN_TO_TOP	0x00
#define DEF_TGA_BIT_INFO_TOP_TO_DOWN	0x20

typedef struct {
	unsigned char	id;
	unsigned char	color_map_flag;
	unsigned char	type;
	unsigned short	color_map_entry;
	unsigned char	color_map_entry_size;
	unsigned short	x;
	unsigned short	y;
	unsigned short	width;
	unsigned short	height;
	unsigned char	depth;
	unsigned char	bit_info;
} STR_TGA_HEAD;


/*=========================================================================
[Type definition] color structure for OpenGL (4 colors float)
=========================================================================*/
typedef struct {
	GLfloat r;
	GLfloat g;
	GLfloat b;
	GLfloat a;
} glCOLOR4f;


/*=========================================================================
[Type definition] 2-dimensional coordinate structure for OpenGL (float)
=========================================================================*/
typedef struct {
	GLfloat x;
	GLfloat y;
} glPOINT2f;


/*=========================================================================
[Type definition three-dimensional coordinate structure for OpenGL (float)
=========================================================================*/
typedef struct tag_glPOINT3f {
	GLfloat x;
	GLfloat y;
	GLfloat z;
} glPOINT3f;


/*=========================================================================
[Type definition] surface information structure
=========================================================================*/
typedef struct {
	int			n;		// The number of vertices of the one face (3-4)
	int			m;		// Surface material number of
	int			v[4];	// Array that contains the vertex number
	glPOINT2f	uv[4];	// UV map
} MQO_FACE;


/*=========================================================================
[Type definition] material information structure (used to read information from a file)
=========================================================================*/
typedef struct {
	glCOLOR4f	col;				// Color
	GLfloat		dif[4];				// Diffusion light
	GLfloat		amb[4];				// Surrounding light
	GLfloat		emi[4];				// Self-lighting
	GLfloat		spc[4];				// Reflected light
	GLfloat		power;				// The strength of the reflected light
	int			useTex;				// The presence or absence of texture
	char		texFile[SIZE_STR];	// Texture file
	char		alpFile[SIZE_STR];	// Alpha texture file
	GLuint		texName;			// Texture name
} MQO_MATDATA;


/*=========================================================================
[Type definition] object structure (part one of the data)
=========================================================================*/
typedef struct {
	char		objname[SIZE_STR];	// Part name
	int			visible;			// Visible
	int			shading;			// Shading (0: Flat / 1: glow)
	float		facet;				// Smoothing angle
	int			n_face;				// Number of surfaces
	int			n_vertex;			// The number of vertices
	MQO_FACE	*F;					// Surface
	glPOINT3f	*V;					// Vertex
} MQO_OBJDATA;


/*=========================================================================
[Type definition] texture pool
=========================================================================*/
typedef struct {
	GLuint			texture_id;			// Texture ID
	int				texsize;			// Texture size
	char			texfile[MAX_PATH];	// Texture file
	char			alpfile[MAX_PATH];	// Alpha texture file
	unsigned char	alpha;				// Alpha
} TEXTURE_POOL;


/*=========================================================================
[Type definition] vertex data (when the texture is used)
=========================================================================*/
typedef struct {		
	GLfloat point[3];	// Vertex array (x, y, z)
	GLfloat normal[3];	// Normal array (x, y, z)
	GLfloat uv[2];		// UV array (u, v)
} VERTEX_TEXUSE;


/*=========================================================================
[Type definition] vertex data (when the texture is not used)
=========================================================================*/
typedef struct {
	GLfloat point[3];	// Vertex array (x, y, z)
	GLfloat normal[3];	// Normal array (x, y, z)
} VERTEX_NOTEX;


/*=========================================================================
[Type definition] material information (I have the material by the vertex array)
=========================================================================*/
typedef struct {
	int				isValidMaterialInfo;// Enable / Disable of material information
	int				isUseTexture;		// The presence or absence of texture: USE_TEXTURE / NOUSE_TEXTURE
	GLuint			texture_id;			// Texture of the name (OpenGL)
	GLuint			VBO_id;				// Use only when you are corresponding ID of the vertex buffer (OpenGL)
	int				datanum;			// The number of vertices
	GLfloat			color[4];			// Color arrangement (r, g, b, a)
	GLfloat			dif[4];				// Diffusion light
	GLfloat			amb[4];				// Surrounding light
	GLfloat			emi[4];				// Self-lighting
	GLfloat			spc[4];				// Reflected light
	GLfloat			power;				// The strength of the reflected light
	VERTEX_NOTEX	*vertex_p;			// Vertex array at the time of the polygon only
	VERTEX_TEXUSE	*vertex_t;			// Vertex array at the time of texture use
} MQO_MATERIAL;


/*=========================================================================
[Type definition] internal object (managing one of the parts)
=========================================================================*/
typedef struct {
	char			objname[SIZE_STR];		// Object name
	int				isVisible;				// 0: Hide Others: Display
	int				isShadingFlat;			// Shading mode
	int				matnum;					// The number of used material
	MQO_MATERIAL	*mat;					// Materials array
} MQO_INNER_OBJECT;


/*=========================================================================
[Type definition (managing one of the models) MQO object ※ MQO_MODEL entities
=========================================================================*/
typedef struct {
	unsigned char		alpha;				// Vertex array created during the specified alpha value (for reference)
	int					objnum;				// Internal number of objects
	MQO_INNER_OBJECT	obj[MAX_OBJECT];	// Internal object array
} MQO_OBJECT;


/*=========================================================================
[Type definition] MQO_MODEL structure
=========================================================================*/
typedef MQO_OBJECT * MQO_MODEL;		// MQO_MODEL the address to your own format structure


/*=========================================================================
[Type definition] MQO sequence
=========================================================================*/
typedef struct {
	MQO_MODEL	model;		// Model
	int			n_frame;	// The number of frames
} MQO_SEQUENCE;


/*=========================================================================
Definition of VBO Extension from the [type definition] glext.h
=========================================================================*/
#ifdef WIN32
	#define GL_ARRAY_BUFFER_ARB	0x8892
	#define GL_STATIC_DRAW_ARB	0x88E4
	typedef void (APIENTRY * PFNGLBINDBUFFERARBPROC)    (GLenum target, GLuint buffer);
	typedef void (APIENTRY * PFNGLDELETEBUFFERSARBPROC) (GLsizei n, const GLuint *buffers);
	typedef void (APIENTRY * PFNGLGENBUFFERSARBPROC)    (GLsizei n, GLuint *buffers);
	typedef void (APIENTRY * PFNGLBUFFERDATAARBPROC)    (GLenum target, int size, const GLvoid *data, GLenum usage);
#endif


/*=========================================================================
[Global variable definition]
=========================================================================*/

#ifdef __GLMETASEQ_C__
	#define __GLMETASEQ_C__EXTERN
#else
	#define __GLMETASEQ_C__EXTERN extern
#endif

__GLMETASEQ_C__EXTERN int g_isVBOSupported;	// Support the presence or absence of OpenGL vertex buffer of

#ifdef WIN32	
	// VBO Extension 関数のポインタ
	__GLMETASEQ_C__EXTERN PFNGLGENBUFFERSARBPROC glGenBuffersARB;		// VBO name generation
	__GLMETASEQ_C__EXTERN PFNGLBINDBUFFERARBPROC glBindBufferARB;		// VBO Tied
	__GLMETASEQ_C__EXTERN PFNGLBUFFERDATAARBPROC glBufferDataARB;		// VBO Data load
	__GLMETASEQ_C__EXTERN PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB;	// VBO Delete
#endif

#undef __GLMETASEQ_C__EXTERN


/*=========================================================================
[Function declaration]
=========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif


// Initialization
void mqoInit(void);

// End processing
void mqoCleanup(void);

// Model generation
MQO_MODEL	 mqoCreateModel(char *filename, double scale);

// Sequence generation
MQO_SEQUENCE mqoCreateSequence(const char *format, int n_file, double scale);

// Sequence generation (extended version)
MQO_SEQUENCE mqoCreateSequenceEx(const char *format, int n_file, double scale,
								 int fade_inout, unsigned char alpha);

// Model call
void mqoCallModel(MQO_MODEL model);

// Sequence call
void mqoCallSequence(MQO_SEQUENCE seq, int i);

// Delete the model
void mqoDeleteModel(MQO_MODEL model);

// Delete the sequence
void mqoDeleteSequence(MQO_SEQUENCE seq);


#ifdef __cplusplus
}
#endif




#endif	// -- end of header --

