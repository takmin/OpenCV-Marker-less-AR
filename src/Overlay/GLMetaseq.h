#ifndef __GLMETASEQ_H__
#define __GLMETASEQ_H__

/*=========================================================================================
	メタセコイアで作成したモデル(*.mqo)をOpenGL上に読み込む関数をまとめたC/C++用ヘッダ
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
MITライセンス
Copyright (c) 2009 Sunao Hashimoto and Keisuke Konishi

以下に定める条件に従い、本ソフトウェアおよび関連文書のファイル（以下「ソフト
ウェア」）の複製を取得するすべての人に対し、ソフトウェアを無制限に扱うことを
無償で許可します。これには、ソフトウェアの複製を使用、複写、変更、結合、掲載、
頒布、サブライセンス、および/または販売する権利、およびソフトウェアを提供する
相手に同じことを許可する権利も無制限に含まれます。 

上記の著作権表示および本許諾表示を、ソフトウェアのすべての複製または重要な部分
に記載するものとします。 

ソフトウェアは「現状のまま」で、明示であるか暗黙であるかを問わず、何らの保証
もなく提供されます。ここでいう保証とは、商品性、特定の目的への適合性、および
権利非侵害についての保証も含みますが、それに限定されるものではありません。 
作者または著作権者は、契約行為、不法行為、またはそれ以外であろうと、ソフト
ウェアに起因または関連し、あるいはソフトウェアの使用またはその他の扱いに
よって生じる一切の請求、損害、その他の義務について何らの責任も負わないもの
とします。 
*/

/*
----------------------------------------------------------------------------------------
	1. このヘッダを使う上での注意点
----------------------------------------------------------------------------------------
  
	●読み込み可能なテクスチャの画像形式はbmp，tga，jpeg，png
	　ただし
	　　jpegの読み込みにはJPEGライブラリ（libjpeg.lib, jpeglib.h）が別途必要
	　　jpegの読み込みを有効にするには，このヘッダの DEF_USE_LIBJPEG を 1 にすること

	　	pngの読み込みにはPNGライブラリ（libpng.lib, zlib.lib, png.h ,zlib.h）が別途必要
	　　pngの読み込みを有効にするには，このヘッダの DEF_USE_LIBPNG  を 1 にすること

	●テクスチャ画像のサイズは「一辺が2のn乗サイズ(64,128,256…)の正方形」に限る


----------------------------------------------------------------------------------------
	2. 使い方(1) 1つのMQOファイルを読み込んで表示する場合
----------------------------------------------------------------------------------------

	(1) 初期化（ARToolKitの場合，argInit()の後に使用）

		mqoInit();

	(2) ファイルからのモデルの読み込み

		MQO_MODEL model;
		model = mqoCreateModel( "mario.mqo", 1.0 );

	(3) モデルの呼び出し
		
		mqoCallModel( model );

	(4) モデルの消去

		mqoDeleteModel( model );

	(5) 終了処理（プログラム終了時にやってください）

		mqoCleanup();

----------------------------------------------------------------------------------------
	3. 使い方(2) 連番ファイルを読み込んで表示する場合
----------------------------------------------------------------------------------------

	(1) 初期化（ARToolKitの場合，argInit()の後に使用）

		mqoInit();

	(2) 連番シーケンスの作成
	
	  　例：mario0.mqo ～ mario9.mqo を読み込む

		MQO_SEQUENCE seq;
		seq = mqoCreateSequence( "mario%d.mqo", 10, 1.0 );

	(3) 連番シーケンスの指定フレームの呼び出し（iはフレーム番号）
		
		mqoCallSequence( seq, i );

	(4) 連番シーケンスの消去

		mqoDeleteSequence( seq );

	(5) 終了処理（プログラム終了時にやってください）

		mqoCleanup();

----------------------------------------------------------------------------------------
	4. 主な仕様
----------------------------------------------------------------------------------------

	●表示機能
	・サポートしているMQOファイルのバージョンは「Metasequoia Ver1.0/2.0～2.4」

	・対応している材質情報
	　色（光）
	　テクスチャマップ（バンプマップ非対応／UVマッピングのみ）

	・対応しているオブジェクト情報
	　表示／非表示の切り替え
	　スムージングの有無
	　頂点法線を決めるときのスムージング角
	　頂点情報
	　面情報（頂点カラー非対応）

	・曲面・鏡面・回転体には非対応
	・メタボールは非対応

	●仕様
	・チャンク名は原則として大文字と小文字の区別をしないことになっているが
	　区別してしまっている．
	・テクスチャのパスに多バイト文字をつかっていて
	　そのなかに'\'(0x5c)や'/'(0x2f)がはいっているとうまくテクスチャが読めない．
	・未着色のマテリアルに対応していない．
	　Objectチャンク→faceチャンクの材質インデックス（M(%d)）が-1に未対応．

*/



/*=========================================================================
【ユーザが任意で設定】
=========================================================================*/

#define MAX_TEXTURE				100			// テクスチャの最大取り扱い数
#define MAX_OBJECT				50			// 1個のMQOファイル内の最大オブジェクト数
#define SIZE_STR				256			// 文字列バッファのサイズ
#define DEF_IS_LITTLE_ENDIAN	1			// エンディアン指定（intel系=1）
#define DEF_USE_LIBJPEG			0			// libjpegの使用（1:使用 0:未使用）
#define DEF_USE_LIBPNG			1			// libpng の使用（1:使用 0:未使用）



/*=========================================================================
【コンパイルオプション】
=========================================================================*/

// JPEGを使用する
#ifdef D_JPEG
	#undef	DEF_USE_LIBJPEG
	#define	DEF_USE_LIBJPEG 1
#endif

// JPEGを使用しない
#ifdef D_NO_JPEG
	#undef	DEF_USE_LIBJPEG
	#define	DEF_USE_LIBJPEG 0
#endif

// PNGを使用する
#ifdef D_PNG
	#undef	DEF_USE_LIBPNG
	#define	DEF_USE_LIBPNG 1
#endif

// PNGを使用しない
#ifdef D_NO_PNG
	#undef	DEF_USE_LIBPNG
	#define	DEF_USE_LIBPNG 0
#endif


/*=========================================================================
【ヘッダ】
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
【機能設定】 libjpeg使用設定
=========================================================================*/

#if DEF_USE_LIBJPEG

	#define XMD_H // INT16とINT32の再定義エラーを防ぐ
	#ifdef FAR
		#undef FAR
	#endif

	#include "jpeglib.h"
	#pragma comment(lib,"libjpeg.lib")

#endif


/*=========================================================================
【機能設定】 libpng使用設定
=========================================================================*/

#if DEF_USE_LIBPNG

	#include "png.h"
	#include "zlib.h"
	#pragma comment(lib,"libpng.lib")
	#pragma comment(lib,"zlib.lib")

#endif


/*=========================================================================
【マクロ定義】 最大値マクロ
=========================================================================*/

#ifndef MAX
	#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#endif


/*=========================================================================
【型定義】 TGAフォーマット
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
【型定義】 OpenGL用色構造体 (4色float)
=========================================================================*/
typedef struct {
	GLfloat r;
	GLfloat g;
	GLfloat b;
	GLfloat a;
} glCOLOR4f;


/*=========================================================================
【型定義】 OpenGL用２次元座標構造体 (float)
=========================================================================*/
typedef struct {
	GLfloat x;
	GLfloat y;
} glPOINT2f;


/*=========================================================================
【型定義】 OpenGL用３次元座標構造体 (float)
=========================================================================*/
typedef struct tag_glPOINT3f {
	GLfloat x;
	GLfloat y;
	GLfloat z;
} glPOINT3f;


/*=========================================================================
【型定義】 面情報構造体
=========================================================================*/
typedef struct {
	int			n;		// 1つの面を構成する頂点の数（3～4）
	int			m;		// 面の材質番号
	int			v[4];	// 頂点番号を格納した配列
	glPOINT2f	uv[4];	// UVマップ
} MQO_FACE;


/*=========================================================================
【型定義】 材質情報構造体（ファイルから情報を読み込む際に使用）
=========================================================================*/
typedef struct {
	glCOLOR4f	col;				// 色
	GLfloat		dif[4];				// 拡散光
	GLfloat		amb[4];				// 周囲光
	GLfloat		emi[4];				// 自己照明
	GLfloat		spc[4];				// 反射光
	GLfloat		power;				// 反射光の強さ
	int			useTex;				// テクスチャの有無
	char		texFile[SIZE_STR];	// テクスチャファイル
	char		alpFile[SIZE_STR];	// アルファテクスチャファイル
	GLuint		texName;			// テクスチャ名
} MQO_MATDATA;


/*=========================================================================
【型定義】 オブジェクト構造体（パーツ１個のデータ）
=========================================================================*/
typedef struct {
	char		objname[SIZE_STR];	// パーツ名
	int			visible;			// 可視状態
	int			shading;			// シェーディング（0:フラット／1:グロー）
	float		facet;				// スムージング角
	int			n_face;				// 面数
	int			n_vertex;			// 頂点数
	MQO_FACE	*F;					// 面
	glPOINT3f	*V;					// 頂点
} MQO_OBJDATA;


/*=========================================================================
【型定義】 テクスチャプール
=========================================================================*/
typedef struct {
	GLuint			texture_id;			// テクスチャID
	int				texsize;			// テクスチャサイズ
	char			texfile[MAX_PATH];	// テクスチャファイル
	char			alpfile[MAX_PATH];	// アルファテクスチャファイル
	unsigned char	alpha;				// アルファ
} TEXTURE_POOL;


/*=========================================================================
【型定義】 頂点データ（テクスチャ使用時）
=========================================================================*/
typedef struct {		
	GLfloat point[3];	// 頂点配列 (x, y, z)
	GLfloat normal[3];	// 法線配列 (x, y, z)
	GLfloat uv[2];		// UV配列 (u, v)
} VERTEX_TEXUSE;


/*=========================================================================
【型定義】 頂点データ（テクスチャ不使用時）
=========================================================================*/
typedef struct {
	GLfloat point[3];	// 頂点配列 (x, y, z)
	GLfloat normal[3];	// 法線配列 (x, y, z)
} VERTEX_NOTEX;


/*=========================================================================
【型定義】 マテリアル情報（マテリアル別に頂点配列を持つ）
=========================================================================*/
typedef struct {
	int				isValidMaterialInfo;// マテリアル情報の有効/無効
	int				isUseTexture;		// テクスチャの有無：USE_TEXTURE / NOUSE_TEXTURE
	GLuint			texture_id;			// テクスチャの名前(OpenGL)
	GLuint			VBO_id;				// 頂点バッファのID(OpenGL)　対応してる時だけ使用
	int				datanum;			// 頂点数
	GLfloat			color[4];			// 色配列 (r, g, b, a)
	GLfloat			dif[4];				// 拡散光
	GLfloat			amb[4];				// 周囲光
	GLfloat			emi[4];				// 自己照明
	GLfloat			spc[4];				// 反射光
	GLfloat			power;				// 反射光の強さ
	VERTEX_NOTEX	*vertex_p;			// ポリゴンのみの時の頂点配列
	VERTEX_TEXUSE	*vertex_t;			// テクスチャ使用時の頂点配列
} MQO_MATERIAL;


/*=========================================================================
【型定義】 内部オブジェクト（1つのパーツを管理）
=========================================================================*/
typedef struct {
	char			objname[SIZE_STR];		// オブジェクト名
	int				isVisible;				// 0：非表示　その他：表示
	int				isShadingFlat;			// シェーディングモード
	int				matnum;					// 使用マテリアル数
	MQO_MATERIAL	*mat;					// マテリアル配列
} MQO_INNER_OBJECT;


/*=========================================================================
【型定義】 MQOオブジェクト（1つのモデルを管理）　※MQO_MODELの実体
=========================================================================*/
typedef struct {
	unsigned char		alpha;				// 頂点配列作成時に指定されたアルファ値（参照用）
	int					objnum;				// 内部オブジェクト数
	MQO_INNER_OBJECT	obj[MAX_OBJECT];	// 内部オブジェクト配列
} MQO_OBJECT;


/*=========================================================================
【型定義】 MQO_MODEL構造体
=========================================================================*/
typedef MQO_OBJECT * MQO_MODEL;		// MQO_MODELは独自形式構造体へのアドレス


/*=========================================================================
【型定義】 MQOシーケンス
=========================================================================*/
typedef struct {
	MQO_MODEL	model;		// モデル
	int			n_frame;	// フレーム数
} MQO_SEQUENCE;


/*=========================================================================
【型定義】 glext.h からの VBO Extension の定義
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
【グローバル変数定義】
=========================================================================*/

#ifdef __GLMETASEQ_C__
	#define __GLMETASEQ_C__EXTERN
#else
	#define __GLMETASEQ_C__EXTERN extern
#endif

__GLMETASEQ_C__EXTERN int g_isVBOSupported;	// OpenGLの頂点バッファのサポート有無

#ifdef WIN32	
	// VBO Extension 関数のポインタ
	__GLMETASEQ_C__EXTERN PFNGLGENBUFFERSARBPROC glGenBuffersARB;		// VBO 名前生成
	__GLMETASEQ_C__EXTERN PFNGLBINDBUFFERARBPROC glBindBufferARB;		// VBO 結びつけ
	__GLMETASEQ_C__EXTERN PFNGLBUFFERDATAARBPROC glBufferDataARB;		// VBO データロード
	__GLMETASEQ_C__EXTERN PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB;	// VBO 削除
#endif

#undef __GLMETASEQ_C__EXTERN


/*=========================================================================
【関数宣言】
=========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif


// 初期化
void mqoInit(void);

// 終了処理
void mqoCleanup(void);

// モデル生成
MQO_MODEL	 mqoCreateModel(char *filename, double scale);

// シーケンス生成
MQO_SEQUENCE mqoCreateSequence(const char *format, int n_file, double scale);

// シーケンス生成（拡張版）
MQO_SEQUENCE mqoCreateSequenceEx(const char *format, int n_file, double scale,
								 int fade_inout, unsigned char alpha);

// モデル呼び出し
void mqoCallModel(MQO_MODEL model);

// シーケンス呼び出し
void mqoCallSequence(MQO_SEQUENCE seq, int i);

// モデルの削除
void mqoDeleteModel(MQO_MODEL model);

// シーケンスの削除
void mqoDeleteSequence(MQO_SEQUENCE seq);


#ifdef __cplusplus
}
#endif




#endif	// -- end of header --

