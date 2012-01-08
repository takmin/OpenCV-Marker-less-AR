
#define __GLMETASEQ_C__
#include "GLMetaseq.h"

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

/*=========================================================================
【このソース内でのみ有効なグローバル変数】
=========================================================================*/

static TEXTURE_POOL l_texPool[MAX_TEXTURE];		// テクスチャプール
static int			l_texPoolnum;				// テクスチャの数
static int			l_GLMetaseqInitialized = 0;	// 初期化フラグ


/*=========================================================================
【関数宣言】
=========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

void	endianConverter(void *addr,unsigned int size);
void	TGAHeaderEndianConverter(	STR_TGA_HEAD *tgah );
int		IsExtensionSupported( char* szTargetExtension );

GLuint		mqoSetTexturePool(char *texfile, char *alpfile, unsigned char alpha );
void		mqoClearTexturePool();
GLubyte*	mqoLoadTextureEx(char *texfile,char *alpfile,int *tex_size,unsigned char alpha);
int			mqoLoadFile(MQO_OBJECT *mqoobj,char *filename,double scale,unsigned char alpha);
MQO_OBJECT*	mqoCreateList(int num);
int			mqoCreateListObject( MQO_OBJECT *obj, int id, char *filename,double scale,unsigned char alpha);

void mqoCallListObject(MQO_OBJECT object[],int num);
void mqoClearObject(MQO_OBJECT object[],int from,int num);
void mqoDeleteObject(MQO_OBJECT * object,int num);
void mqoGetDirectory(const char *path_file, char *path_dir);
void mqoSnormal(glPOINT3f A, glPOINT3f B, glPOINT3f C, glPOINT3f *normal);
void mqoReadMaterial(FILE *fp, MQO_MATDATA M[]);
void mqoReadVertex(FILE *fp, glPOINT3f V[]);
int	 mqoReadBVertex(FILE *fp,glPOINT3f V[]);
void mqoReadFace(FILE *fp, MQO_FACE F[]);
void mqoReadObject(FILE *fp, MQO_OBJDATA *obj);

void mqoMakeArray(MQO_MATERIAL *mat, int matpos,MQO_FACE F[], int fnum,glPOINT3f V[],
				  glPOINT3f N[], double facet, glCOLOR4f *mcol, double scale, unsigned char alpha );

glPOINT3f *mqoVertexNormal(MQO_OBJDATA *obj);

void mqoMakePolygon(MQO_OBJDATA *readObj, MQO_OBJECT *mqoobj,
					glPOINT3f N[], MQO_MATDATA M[], int n_mat, double scale, unsigned char alpha);

void mqoMakeObjectsEx(MQO_OBJECT *mqoobj, MQO_OBJDATA obj[], int n_obj, MQO_MATDATA M[],int n_mat,
					  double scale,unsigned char alpha);

#ifdef __cplusplus
}
#endif


/*=========================================================================
【関数】endianConverter
【用途】エンディアン変換
【引数】
		addr	アドレス
		size	サイズ

【戻値】なし
=========================================================================*/

void endianConverter(void *addr,unsigned int size)
{
	unsigned int pos;
	char c;
	if ( size <= 1 ) return;
	for ( pos = 0; pos < size/2; pos++ ) {
		c = *(((char *)addr)+pos);
		*(((char *)addr)+pos) = *(((char *)addr)+(size-1 - pos));
		*(((char *)addr)+(size-1 - pos)) = c;
	}
}


/*=========================================================================
【関数】TGAHeaderEndianConverter
【用途】TGAのヘッダのエンディアン変換
【引数】
		tgah	TGAのヘッダ

【戻値】なし
=========================================================================*/

void TGAHeaderEndianConverter(	STR_TGA_HEAD *tgah )
{
	endianConverter(&tgah->color_map_entry,sizeof(tgah->color_map_entry));
	endianConverter(&tgah->x,sizeof(tgah->x));
	endianConverter(&tgah->y,sizeof(tgah->y));
	endianConverter(&tgah->width,sizeof(tgah->width));
	endianConverter(&tgah->height,sizeof(tgah->height));
}


/*=========================================================================
【関数】IsExtensionSupported
【用途】OpenGLの拡張機能がサポートされているかどうか調べる
【引数】
		szTargetExtension	拡張機能の名前

【戻値】1：サポートされている，0：されていない
=========================================================================*/

int IsExtensionSupported( char* szTargetExtension )
{
	const unsigned char *pszExtensions = NULL;
	const unsigned char *pszStart;
	unsigned char *pszWhere, *pszTerminator;

	// Extension の名前が正しいか調べる(NULLや空白はNG）
	pszWhere = (unsigned char *) strchr( szTargetExtension, ' ' );
	if ( pszWhere || *szTargetExtension == (char)NULL )
		return 0;

	// Extension の文字列を所得する
	pszExtensions = glGetString( GL_EXTENSIONS );

	// 文字列の中に必要な extension があるか調べる
	pszStart = pszExtensions;
	for (;;)
	{
		pszWhere = (unsigned char *) strstr( (const char *) pszStart, szTargetExtension );
		if ( !pszWhere )
			break;
		pszTerminator = pszWhere + strlen( szTargetExtension );
		if ( pszWhere == pszStart || *( pszWhere - 1 ) == ' ' )
		if ( *pszTerminator == ' ' || *pszTerminator == (char)NULL )
			return 1;
		pszStart = pszTerminator;
	}
	return 0;
}


/*=========================================================================
【関数】mqoInit
【用途】メタセコイアローダの初期化
【引数】なし
【戻値】なし
=========================================================================*/

void mqoInit(void)
{
	// テクスチャプール初期化
	memset(l_texPool,0,sizeof(l_texPool));
	l_texPoolnum = 0;

	// 頂点バッファのサポートのチェック
	g_isVBOSupported = IsExtensionSupported("GL_ARB_vertex_buffer_object");
//	g_isVBOSupported = 0;

#ifdef WIN32
	glGenBuffersARB = NULL;
	glBindBufferARB = NULL;
	glBufferDataARB = NULL;
	glDeleteBuffersARB = NULL;

	if ( g_isVBOSupported ) {
		// printf("OpenGL : 頂点バッファをサポートしているので使用します\n");
		// GL 関数のポインタを所得する
		glGenBuffersARB = (PFNGLGENBUFFERSARBPROC) wglGetProcAddress("glGenBuffersARB");
		glBindBufferARB = (PFNGLBINDBUFFERARBPROC) wglGetProcAddress("glBindBufferARB");
		glBufferDataARB = (PFNGLBUFFERDATAARBPROC) wglGetProcAddress("glBufferDataARB");
		glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC) wglGetProcAddress("glDeleteBuffersARB");
	}
#endif

	// 初期化フラグ
	l_GLMetaseqInitialized = 1;
}


/*=========================================================================
【関数】mqoCleanup
【用途】メタセコイアローダの終了処理
【引数】なし
【戻値】なし
=========================================================================*/

void mqoCleanup(void)
{
	mqoClearTexturePool();	// テクスチャプールのクリア
}


/*=========================================================================
【関数】mqoSetTexturePool
【用途】テクスチャプールにテクスチャを読み込む
【引数】
		texfile		テクスチャファイル名
		alpfile		アルファファイル名
		alpha		アルファ

【戻値】テクスチャID
【仕様】テクスチャがまだ読み込まれていなければ読み込み，テクスチャ登録
		すでに読み込まれていれば登録したものを返す.
=========================================================================*/

GLuint mqoSetTexturePool(char *texfile, char *alpfile, unsigned char alpha ) 
{
	int pos;
	GLubyte *image;

	for ( pos = 0; pos < l_texPoolnum; pos++ ) {
		if ( alpha != l_texPool[pos].alpha ) {
			continue;
		}
		if ( texfile != NULL ) {
			if ( strcmp(texfile,l_texPool[pos].texfile) != 0 ) {
				continue;
			}
		}
		if ( alpfile != NULL ) {
			if ( strcmp(alpfile,l_texPool[pos].alpfile) != 0 ) {
				continue;
			}
		}
		break;
	}
	if ( pos < l_texPoolnum ) { //すでに読み込み済み
		return  l_texPool[pos].texture_id;
	}
	if ( MAX_TEXTURE <= pos ) {
		printf("%s:mqoSetTexturePool テクスチャ読み込み領域不足\n",__FILE__);
		return -1;
	}
	image = mqoLoadTextureEx(texfile,alpfile,&l_texPool[pos].texsize,alpha);
	if ( image == NULL ) {
		return -1;
	}

	if ( texfile != NULL ) strncpy(l_texPool[pos].texfile,texfile,MAX_PATH);
	if ( alpfile != NULL ) strncpy(l_texPool[pos].alpfile,alpfile,MAX_PATH);
	l_texPool[pos].alpha = alpha;

	glPixelStorei(GL_UNPACK_ALIGNMENT,4);
	glPixelStorei(GL_PACK_ALIGNMENT,4);
	glGenTextures(1,&l_texPool[pos].texture_id);			// テクスチャを生成
	glBindTexture(GL_TEXTURE_2D,l_texPool[pos].texture_id);	// テクスチャの割り当て

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, l_texPool[pos].texsize, l_texPool[pos].texsize,
					0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	l_texPoolnum = pos+1;

	//登録すれば、読み込んだバッファは不要
	free(image);
	glBindTexture(GL_TEXTURE_2D,0);	// デフォルトテクスチャの割り当て

	return l_texPool[pos].texture_id;
}


/*=========================================================================
【関数】mqoClearTexturePool()
【用途】テクスチャプールの開放
【引数】なし
【戻値】なし
=========================================================================*/

void mqoClearTexturePool()
{
	int pos;
	for ( pos = 0; pos < l_texPoolnum; pos++ ) {
		glDeleteTextures(1, &l_texPool[pos].texture_id);	// テクスチャ情報を削除
	}

	memset(l_texPool,0,sizeof(l_texPool));
	l_texPoolnum = 0;
}


/*=========================================================================
【関数】mqoLoadTextureEx
【用途】ファイルからテクスチャ画像を作成する
【引数】
		texfile		ファイル名
		alpfile		アルファファイル名
		tex_size	テクスチャのサイズ（一辺の長さ）を返す

【戻値】テクスチャ画像へのポインタ（失敗時はNULL）
【仕様】24bitビットマップ，および8,24,32bitＴＧＡ
		サイズは「一辺が2のn乗の正方形」に限定
		libjpeg,libpng（外部ライブラリ）が有ればJPEG,PNGの読み込み可能
=========================================================================*/

GLubyte* mqoLoadTextureEx(char *texfile,char *alpfile,int *tex_size,unsigned char alpha)
{
	FILE *fp;
	size_t namelen;
	char ext[4];
	char wbuf[3];
	int isTGA;
	int isPNG;
	int isJPEG;
	int other;
	int	y,x,size;
	int fl;
	char *filename[2];
	int width[2];
	int sts;
	STR_TGA_HEAD tgah;
	GLubyte	*pImage, *pRead;

#if DEF_USE_LIBJPEG
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	JSAMPARRAY jpegimage;
#endif
#if DEF_USE_LIBPNG
	unsigned char **pngimage;
	unsigned long   pngwidth, pngheight;
	int				pngdepth;
	int             color_type;
#endif

	filename[0] = texfile;
	filename[1] = alpfile;
	width[0] = -1;
	width[1] = -1;
	pImage = NULL;
	fp = NULL;
	sts = 0;
#if DEF_USE_LIBJPEG
	jpegimage = NULL;
#endif
#if DEF_USE_LIBPNG
	pngimage = NULL;
#endif
	size = - 1;
	for ( fl = 0; fl < 2; fl++ ) {//テクスチャ＝fl=0    アルファ＝fl=1
		if ( filename[fl] == NULL ) continue;
		namelen = strlen(filename[fl]);
		ext[0] = tolower(filename[fl][namelen-3]);
		ext[1] = tolower(filename[fl][namelen-2]);
		ext[2] = tolower(filename[fl][namelen-1]);
		ext[3] = 0x00;
		isTGA = (strcmp(ext,"tga")==0)?1:0;
		isPNG = (strcmp(ext,"png")==0)?1:0;
		isJPEG = (strcmp(ext,"jpg")==0)?1:0;
		/* */
		if ( (! isTGA) && (! isPNG) &&(! isJPEG) ) {
			filename[fl][namelen-3] = 'b';
			filename[fl][namelen-2] = 'm';
			filename[fl][namelen-1] = 'p';
		}
		/* */
		if ( fl == 1 ) { //アルファの読み込みはＴＧＡorＰＮＧ
			if ( ! (isTGA || isPNG) ) {
				printf("アルファのファイルに対応できない→%s\n",filename[fl]);
				break;
			}
		}
		if ( fp != NULL ) fclose(fp);
		if ( (fp=fopen(filename[fl],"rb"))==NULL ) {
			printf("%s:テクスチャ読み込みエラー[%s]\n",__FILE__,filename[fl]);
			continue;
		}
		// ヘッダのロード
		if ( isTGA ) {
			fread(&tgah,sizeof(STR_TGA_HEAD),1,fp);
#if DEF_IS_LITTLE_ENDIAN
#else
			TGAHeaderEndianConverter(&tgah);
#endif
			size = width[fl] = tgah.width;
		}
		if ( isJPEG ) {
#if DEF_USE_LIBJPEG
			unsigned int i;
			cinfo.err = jpeg_std_error( &jerr );
			jpeg_create_decompress( &cinfo );	//解凍用情報作成
			jpeg_stdio_src( &cinfo, fp );		//読み込みファイル指定
			jpeg_read_header( &cinfo, TRUE );	//jpegヘッダ読み込み
			jpeg_start_decompress( &cinfo );	//解凍開始

			if ( cinfo.out_color_components == 3 && cinfo.out_color_space == JCS_RGB ) {
				if ( jpegimage != NULL ) {
					for (i = 0; i < cinfo.output_height; i++) free(jpegimage[i]);            // 以下２行は２次元配列を解放します
					free(jpegimage);
				}
				//読み込みデータ配列の作成
				jpegimage = (JSAMPARRAY)malloc( sizeof( JSAMPROW ) * cinfo.output_height );
				for ( i = 0; i < cinfo.output_height; i++ ) {
					jpegimage[i] = (JSAMPROW)malloc( sizeof( JSAMPLE ) * cinfo.out_color_components * cinfo.output_width );
				}
				//解凍データ読み込み
				while( cinfo.output_scanline < cinfo.output_height ) {
					jpeg_read_scanlines( &cinfo,
						jpegimage + cinfo.output_scanline,
						cinfo.output_height - cinfo.output_scanline
					);
				}
				size = width[fl] = cinfo.output_width;
			}

			jpeg_finish_decompress( &cinfo );	//解凍終了
			jpeg_destroy_decompress( &cinfo );	//解凍用情報解放
			if ( !(cinfo.out_color_components == 3 && cinfo.out_color_space == JCS_RGB) ) {
				printf("JPEG 対応できないフォーマット→%s\n",filename[fl]);
			}
#else
			printf("このテクスチャは対応できないフォーマット→%s\n",filename[fl]);
			continue;
#endif
		}
		if ( isPNG ) {
#if DEF_USE_LIBPNG
			png_structp     png_ptr;
			png_infop       info_ptr;
			int             bit_depth, interlace_type;
			unsigned int             i;
			int j,k;
			png_ptr = png_create_read_struct(                       // png_ptr構造体を確保・初期化します
							PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
			info_ptr = png_create_info_struct(png_ptr);             // info_ptr構造体を確保・初期化します
			png_init_io(png_ptr, fp);                               // libpngにfpを知らせます
			png_read_info(png_ptr, info_ptr);                       // PNGファイルのヘッダを読み込みます
			png_get_IHDR(png_ptr, info_ptr, &pngwidth, &pngheight,        // IHDRチャンク情報を取得します
							&bit_depth, &color_type, &interlace_type,
							&j,&k);
			if ( pngimage != NULL ) {
				for (i = 0; i < pngheight; i++) free(pngimage[i]);            // 以下２行は２次元配列を解放します
				free(pngimage);
			}
			pngimage = (png_bytepp)malloc(pngheight * sizeof(png_bytep)); // 以下３行は２次元配列を確保します
			i = png_get_rowbytes(png_ptr, info_ptr);
			pngdepth = i / pngwidth;
			for (i = 0; i < pngheight; i++)
					pngimage[i] = (png_bytep)malloc(png_get_rowbytes(png_ptr, info_ptr));
			png_read_image(png_ptr, pngimage);                         // 画像データを読み込みます

			png_destroy_read_struct(                                // ２つの構造体のメモリを解放します
	        &png_ptr, &info_ptr, (png_infopp)NULL);
			size = width[fl] = pngwidth;
#else
			printf("このテクスチャは対応できないフォーマット→%s\n",filename[fl]);
			continue;
#endif
		}
		if ( width[fl] == -1 ) {//ココまできてサイズが指定されていない　＝　ビットマップ
			fseek(fp,14+4,SEEK_SET);		// 画像幅が格納されている位置までシーク
			fread(&size,sizeof(int),1,fp);	// BiWidthの情報だけ取得
			fseek(fp,14+40,SEEK_SET);		// 画素データが格納されている位置までシーク
#if DEF_IS_LITTLE_ENDIAN
#else
			endianConverter(&size,sizeof(int));
#endif
			width[fl] = size;

		}
		if ( width[0] != -1 && width[1] != -1 ) {
			if ( width[0] != width[1] ) {
				sts = -1;
				break;
			}
		}
		if ( fl == 1 && isTGA ) { //アルファの読み込みはＴＧＡの８ビットモノクロｏｒ３２ビットフル
			if ( !(
				(tgah.depth == 8 && tgah.type == DEF_TGA_TYPE_MONO) ||
				(tgah.depth == 32 && tgah.type == DEF_TGA_TYPE_FULL) 
				) ) {
				break;
			}
		}
		if ( fl == 1 && isPNG ) { //アルファの読み込みはＰＮＧのトゥルーカラー＋アルファｏｒグレースケール＋アルファ
#if DEF_USE_LIBPNG
			if ( !(
				(color_type== 6 ) ||
				(color_type== 4 ) 
				) ) {
				break;
			}
#endif
		}

		// メモリの確保
		if ( pImage == NULL ) {
			pImage = (GLubyte*)malloc(sizeof(unsigned char)*size*size*4);
		}
		if (pImage==NULL) return NULL;
		for (y=0; y<size; y++){
			pRead = pImage + (size-1-y)*4*size;
			for (x=0; x<size; x++) {
				other = 1;
				if ( fl == 0 ) {
					if ( isJPEG ) {
#if DEF_USE_LIBJPEG
						pRead[0]= jpegimage[size-1-y][x*3];
						pRead[1]= jpegimage[size-1-y][x*3+1];
						pRead[2]= jpegimage[size-1-y][x*3+2];
						pRead[3] = alpha;				// A
						other = 0;
#endif
					}
					if ( isPNG ) {
#if DEF_USE_LIBPNG
						if ( color_type == 2 || color_type==6 ) { 
							pRead[0]= pngimage[size-1-y][x*pngdepth];
							pRead[1]= pngimage[size-1-y][x*pngdepth+1];
							pRead[2]= pngimage[size-1-y][x*pngdepth+2];
							pRead[3] = alpha;				// A
							if ( color_type == 6 ) {
								pRead[3]= pngimage[size-1-y][x*pngdepth+3];
							}
						}
						other = 0;
#endif
					}
					if ( other )  {
						fread(&pRead[2],1,1,fp);	// B
						fread(&pRead[1],1,1,fp);	// G	
						fread(&pRead[0],1,1,fp);	// R
						pRead[3] = alpha;				// A
						if ( isTGA && tgah.depth == 32 ) {
							fread(&pRead[3],1,1,fp);	// A
							if ( alpha < pRead[3] ) pRead[3] = alpha;
						}
					}
				}
				else {
					if ( isPNG ) {
#if DEF_USE_LIBPNG
						if ( color_type == 6 ) { //トゥルーカラー＋アルファ
							pRead[3]= pngimage[size-1-y][x*pngdepth+3];
						}
						if ( color_type == 4 ) { //グレースケール＋アルファ
							pRead[3]= pngimage[size-1-y][x*pngdepth+1];
						}
						if ( alpha < pRead[3] ) pRead[3] = alpha;
#endif
					}
					if ( isTGA ) {
						if ( tgah.depth == 32 ) { //いらないデータを読み飛ばす
							fread(wbuf,3,1,fp);	// BGR
						}
						fread(&pRead[3],1,1,fp);	// A
						if ( alpha < pRead[3] ) pRead[3] = alpha;
					}
				}
				pRead+=4;
			}
		}
		fclose(fp);
		fp = NULL;
	}
	if ( sts != 0 ) {
		if ( pImage != NULL ) free(pImage);
		if ( fp != NULL ) fclose(fp);
	}
#if DEF_USE_LIBPNG
	if ( pngimage != NULL ) {
		unsigned int uy;
		for (uy = 0; uy < pngheight; uy++) free(pngimage[uy]);            // 以下２行は２次元配列を解放します
		free(pngimage);
	}
#endif
#if DEF_USE_LIBJPEG
	if ( jpegimage != NULL ) {
		unsigned int uy;
		for (uy = 0; uy < cinfo.output_height; uy++) free(jpegimage[uy]);            // 以下２行は２次元配列を解放します
		free(jpegimage);
	}
#endif
	if ( size < 0 ) {
		if ( pImage != NULL ) free(pImage);
		pImage = NULL;
	}
	*tex_size = size;

	return pImage;
}


/*=========================================================================
【関数】mqoLoadFile
【用途】メタセコイアファイル(*.mqo)からデータを読み込む
【引数】
		mqoobj		MQOオブジェクト
		filename	ファイルのパス
		scale		拡大率
		alpha		アルファ

【戻値】成功：1 ／ 失敗：0
=========================================================================*/

int mqoLoadFile( MQO_OBJECT *mqoobj, char *filename, double scale, unsigned char alpha)
{
	FILE			*fp;
	MQO_OBJDATA		obj[MAX_OBJECT];
	MQO_MATDATA		*M = NULL;

	char	buf[SIZE_STR];		// 文字列読み込みバッファ
	char	path_dir[SIZE_STR];	// ディレクトリのパス
	char	path_tex[SIZE_STR];	// テクスチャファイルのパス
	char	path_alp[SIZE_STR];	// アルファテクスチャファイルのパス
	int		n_mat = 0;			// マテリアル数
	int		n_obj = 0;			// オブジェクト数
	int		i;

	// MaterialとObjectの読み込み
	fp = fopen(filename,"rb");
	if (fp==NULL) return 0;

	mqoobj->alpha = alpha;
	memset(obj,0,sizeof(obj));

	i = 0;
	while ( !feof(fp) ) {
		fgets(buf,SIZE_STR,fp);

		// Material
		if (strstr(buf,"Material")) {
			sscanf(buf,"Material %d", &n_mat);
			M = (MQO_MATDATA*) calloc( n_mat, sizeof(MQO_MATDATA) );
			mqoReadMaterial(fp,M);
		}

		// Object
		if (strstr(buf,"Object")) {
			sscanf(buf,"Object %s", obj[i].objname);
			mqoReadObject(fp, &obj[i]);
			i++;
		}
	}
	n_obj = i;
	fclose(fp);

	// パスの取得
	mqoGetDirectory(filename, path_dir);

	// テクスチャの登録
	for (i=0; i<n_mat; i++) {
		if (M[i].useTex) {

			if (strstr(M[i].texFile,":")) {
				strcpy(path_tex, M[i].texFile);	// 絶対パスの場合
			} else {
				sprintf(path_tex,"%s%s",path_dir,M[i].texFile);	// 相対パスの場合
			}

			if ( M[i].alpFile[0] != (char)0 ) {
				if (strstr(M[i].texFile,":")) {
					strcpy(path_alp, M[i].alpFile);	// 絶対パスの場合
				} else {
					sprintf(path_alp,"%s%s",path_dir,M[i].alpFile);	// 相対パスの場合
				}
				M[i].texName = mqoSetTexturePool(path_tex,path_alp,alpha);
			}
			else {
				M[i].texName = mqoSetTexturePool(path_tex,NULL,alpha);
			}
		}
	}

	mqoMakeObjectsEx( mqoobj, obj, n_obj, M, n_mat, scale, alpha );

	// オブジェクトのデータの開放
	for (i=0; i<n_obj; i++) {
		free(obj[i].V);
		free(obj[i].F);
	}

	// マテリアルの開放
	free(M);

	return 1;
}


/*=========================================================================
【関数】mqoCreateList
【用途】MQOオブジェクトを指定数確保する
【引数】num		MQOオブジェクトの数

【戻値】MQOオブジェクト
=========================================================================*/

MQO_OBJECT* mqoCreateList(int num)
{
	MQO_OBJECT *obj;

	// 初期化されてなかったら初期化
	if ( ! l_GLMetaseqInitialized ) mqoInit();

	// 領域確保と初期化
	obj = (MQO_OBJECT *)malloc(sizeof(MQO_OBJECT)*num);
	memset(obj, 0, sizeof(MQO_OBJECT)*num);

	return obj;
}


/*=========================================================================
【関数】mqoCreateListObject
【用途】メタセコイアファイル(*.mqo)からMQOオブジェクト配列を作成する

【引数】mqoobj		MQOオブジェクト
		i			読み込み先番号（i番目にMQOファイルを読み込む）
		filename	ファイルのパス
		scale		拡大率
		alpha		アルファ指定（全体のアルファ値を指定（0～255））

【戻値】ステータス　負：異常　０：正常
=========================================================================*/

int mqoCreateListObject(MQO_OBJECT *mqoobj, int i, char *filename, double scale, unsigned char alpha )
{
	int ret;
	ret = 0;
	if ( mqoobj == (MQO_OBJECT *)NULL ) return -1;
	if (! mqoLoadFile(&mqoobj[i], filename, scale, alpha)) ret = -1; 
	return ret;
}


/*=========================================================================
【関数】mqoCallListObject
【用途】MQOオブジェクトをOpenGLの画面上に呼び出す
【引数】
		mqoobj		MQOオブジェクト配列
		num			配列番号 (0～）

【戻値】なし
=========================================================================*/

void mqoCallListObject(MQO_OBJECT mqoobj[],int num)
{

	MQO_INNER_OBJECT	*obj;
	MQO_MATERIAL		*mat;
	GLfloat				matenv[4];
	GLint				bindGL_TEXTURE_2D	= 0;
	GLboolean			isGL_TEXTURE_2D		= GL_FALSE;
	GLboolean			isGL_BLEND			= GL_FALSE;
	GLint				blendGL_SRC_ALPHA	= 0;
	GLint				intFrontFace;

	int		o, m, offset;
	double	dalpha;
	char	*base;

	if ( mqoobj == NULL) return;

	glPushMatrix();
		//メタセコは頂点の並びが表面からみて右回り
		glGetIntegerv(GL_FRONT_FACE,&intFrontFace);
		glFrontFace(GL_CW);
		dalpha = (double)mqoobj[num].alpha/(double)255;

		for ( o=0; o<mqoobj[num].objnum; o++ ) {	// 内部オブジェクトループ

			obj = &mqoobj[num].obj[o];
			if ( ! obj->isVisible ) continue;
			glShadeModel(((obj->isShadingFlat))?GL_FLAT:GL_SMOOTH);

			for ( m = 0; m < obj->matnum; m++ ) {	//マテリアルループ

				mat = &obj->mat[m];
				if ( mat->datanum == 0 ) continue;

				if ( mat->isValidMaterialInfo ) {	// マテリアルの情報設定
					memcpy(matenv,mat->dif,sizeof(matenv));
					matenv[3] *= dalpha;
					glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, matenv);
					memcpy(matenv,mat->amb,sizeof(matenv));
					matenv[3] *= dalpha;
					glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, matenv);
					memcpy(matenv,mat->spc,sizeof(matenv));
					matenv[3] *= dalpha;
					glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matenv);
					memcpy(matenv,mat->emi,sizeof(matenv));
					matenv[3] *= dalpha;
					glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, matenv);
					glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat->power);
				}

				if ( mat->isUseTexture) {	// テクスチャがある場合
					glEnableClientState( GL_VERTEX_ARRAY );
					glEnableClientState( GL_NORMAL_ARRAY );
					glEnableClientState( GL_TEXTURE_COORD_ARRAY );

					isGL_TEXTURE_2D = glIsEnabled(GL_TEXTURE_2D);
					isGL_BLEND = glIsEnabled(GL_BLEND);
					glGetIntegerv(GL_TEXTURE_BINDING_2D,&bindGL_TEXTURE_2D);
//					glGetIntegerv(GL_BLEND_SRC_ALPHA,&blendGL_SRC_ALPHA);

					glEnable(GL_TEXTURE_2D);
					glEnable(GL_BLEND);
					glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

					glBindTexture(GL_TEXTURE_2D,mat->texture_id);

					if ( g_isVBOSupported ) {	// 頂点バッファ使用
						base = (char *)NULL;	// アドレスはNULLが先頭
						glBindBufferARB( GL_ARRAY_BUFFER_ARB, mat->VBO_id ); // 頂点バッファを結びつける
					}
					else {
						// 頂点配列の時は、アドレスをそのまま入れる
						base = (char *)mat->vertex_t[0].point;
					}

					// 頂点配列を設定
					offset = (int)( (char *)mat->vertex_t[0].point - (char *)mat->vertex_t[0].point );
					glVertexPointer( 3, GL_FLOAT, sizeof(VERTEX_TEXUSE) , base + offset );

					// テクスチャ座標配列を設定
					offset = (int)((char *)mat->vertex_t[0].uv-(char *)mat->vertex_t[0].point);
					glTexCoordPointer( 2, GL_FLOAT, sizeof(VERTEX_TEXUSE) , base + offset );

					// 法線配列を設定
					offset = (int)((char *)mat->vertex_t[0].normal-(char *)mat->vertex_t[0].point);
					glNormalPointer( GL_FLOAT, sizeof(VERTEX_TEXUSE) , base+offset );

					// 色設定
					glColor4f(mat->color[0],mat->color[1],mat->color[2],mat->color[3]);

					// 描画実行
					glDrawArrays( GL_TRIANGLES, 0, mat->datanum );

					glBindTexture(GL_TEXTURE_2D,bindGL_TEXTURE_2D);
					if( isGL_BLEND == GL_FALSE ) glDisable(GL_BLEND);
					if( isGL_TEXTURE_2D == GL_FALSE ) glDisable(GL_TEXTURE_2D);

					if ( g_isVBOSupported ) {						// 頂点バッファ使用
						glBindBufferARB( GL_ARRAY_BUFFER_ARB, 0 );	// 頂点バッファをデフォルトへ
					}

					glDisableClientState( GL_VERTEX_ARRAY );
					glDisableClientState( GL_NORMAL_ARRAY );
					glDisableClientState( GL_TEXTURE_COORD_ARRAY );
				}
				else {	// テクスチャがない場合

					glEnableClientState( GL_VERTEX_ARRAY );
					glEnableClientState( GL_NORMAL_ARRAY );
				//	glEnableClientState( GL_COLOR_ARRAY );

					isGL_BLEND = glIsEnabled(GL_BLEND);
					glEnable(GL_BLEND);
					glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

					if ( g_isVBOSupported ) {	// 頂点バッファ使用
						base = (char *)NULL;
						glBindBufferARB( GL_ARRAY_BUFFER_ARB, mat->VBO_id );
					}
					else {
						base = (char *)mat->vertex_p[0].point;
					}

					// 頂点配列を設定
					offset = (int)((char *)mat->vertex_p[0].point-(char *)mat->vertex_p[0].point);
					glVertexPointer( 3, GL_FLOAT, sizeof(VERTEX_NOTEX) , base+offset );

					// 法線配列を設定
					offset = (int)((char *)mat->vertex_p[0].normal-(char *)mat->vertex_p[0].point);
					glNormalPointer( GL_FLOAT, sizeof(VERTEX_NOTEX) , base+offset );

					// 色設定
					glColor4f(mat->color[0],mat->color[1],mat->color[2],mat->color[3]);
				//	offset = (int)((char *)mat->vertex_p[0].color-(char *)mat->vertex_p[0].point);
				//	glColorPointer(4,GL_FLOAT,sizeof(VERTEX_NOTEX),base+offset);

					// 描画実行
					glDrawArrays( GL_TRIANGLES, 0, mat->datanum );

					if( isGL_BLEND == GL_FALSE ) glDisable(GL_BLEND);
					if ( g_isVBOSupported ) {						// 頂点バッファ使用
						glBindBufferARB( GL_ARRAY_BUFFER_ARB, 0 );	// 頂点バッファをデフォルトへ
					}

				//	glDisableClientState( GL_COLOR_ARRAY );
					glDisableClientState( GL_VERTEX_ARRAY );
					glDisableClientState( GL_NORMAL_ARRAY );

				}
			}
		}

		//メタセコは頂点の並びが表面からみて右回り（元の設定にもどす）
		glFrontFace(intFrontFace);
	glPopMatrix();
}


/*=========================================================================
【関数】mqoGetDirectory
【用途】ファイル名を含むパス文字列からディレクトリのパスのみを抽出する
【引数】
		*path_file	ファイル名を含むパス文字列（入力）
		*path_dir	ファイル名を除いたパス文字列（出力）

【戻値】なし
【仕様】例：
		"C:/data/file.bmp" → "C:/data/"
		"data/file.mqo"    → "data/"
=========================================================================*/

void mqoGetDirectory(const char *path_file, char *path_dir)
{
	char *pStr;
	int len;

	pStr = MAX( strrchr(path_file,'\\'), strrchr(path_file,'/') );
	len = MAX((int)(pStr-path_file)+1,0);
	strncpy(path_dir,path_file,len);
	path_dir[len] = (char)0;
}


/*=========================================================================
【関数】mqoSnormal
【用途】法線ベクトルを求める
【引数】
		A		3次元座標上の点A
		B		3次元座標上の点B
		C		3次元座標上の点C
		*normal	ベクトルBAとベクトルBCの法線ベクトル（右ねじ方向）

【戻値】なし
【仕様】メタセコイアにおいて面を構成する頂点の番号は，表示面から見て
		時計回りに記述してある．つまり，頂点A,B,C があったとき，
		求めるべき法線はBAとBCの外積によって求められる
=========================================================================*/

void mqoSnormal(glPOINT3f A, glPOINT3f B, glPOINT3f C, glPOINT3f *normal)
{
	double norm;
	glPOINT3f vec0,vec1;

	// ベクトルBA
	vec0.x = A.x - B.x; 
	vec0.y = A.y - B.y;
	vec0.z = A.z - B.z;

	// ベクトルBC
	vec1.x = C.x - B.x;
	vec1.y = C.y - B.y;
	vec1.z = C.z - B.z;

	// 法線ベクトル
	normal->x = vec0.y * vec1.z - vec0.z * vec1.y;
	normal->y = vec0.z * vec1.x - vec0.x * vec1.z;
	normal->z = vec0.x * vec1.y - vec0.y * vec1.x;

	// 正規化
	norm = normal->x * normal->x + normal->y * normal->y + normal->z * normal->z;
	norm = sqrt ( norm );

	normal->x /= norm;
	normal->y /= norm;
	normal->z /= norm;
}


/*=========================================================================
【関数】mqoReadMaterial
【用途】マテリアル情報の読み込み
【引数】
		fp		ファイルポインタ
		M		マテリアル配列

【戻値】なし
【仕様】mqoCreateModel(), mqoCreateSequence()のサブ関数．
=========================================================================*/

void mqoReadMaterial(FILE *fp, MQO_MATDATA M[])
{
	GLfloat		dif, amb, emi, spc;
	glCOLOR4f	c;
	char		buf[SIZE_STR];
	char		*pStrEnd, *pStr;
	int			len;
	int			i = 0;

	while (1) {
		fgets(buf,SIZE_STR,fp);	// 行読み込み
		if (strstr(buf,"}")) break;

		pStr = strstr(buf,"col(");	// 材質名読み飛ばし
		sscanf( pStr,
				"col(%f %f %f %f) dif (%f) amb(%f) emi(%f) spc(%f) power(%f)",
				&c.r, &c.g, &c.b, &c.a, &dif, &amb, &emi, &spc, &M[i].power );

		// 頂点カラー
		M[i].col = c;

		// 拡散光
		M[i].dif[0] = dif * c.r;
		M[i].dif[1] = dif * c.g;
		M[i].dif[2] = dif * c.b;
		M[i].dif[3] = c.a;

		// 周囲光
		M[i].amb[0] = amb * c.r;
		M[i].amb[1] = amb * c.g;
		M[i].amb[2] = amb * c.b;
		M[i].amb[3] = c.a;

		// 自己照明
		M[i].emi[0] = emi * c.r;
		M[i].emi[1] = emi * c.g;
		M[i].emi[2] = emi * c.b;
		M[i].emi[3] = c.a;

		// 反射光
		M[i].spc[0] = spc * c.r;
		M[i].spc[1] = spc * c.g;
		M[i].spc[2] = spc * c.b;
		M[i].spc[3] = c.a;
		
		// tex：模様マッピング名
		if ( (pStr = strstr(buf,"tex(")) != NULL ) {
			M[i].useTex = TRUE;

			pStrEnd = strstr(pStr,")")-1;
			len = pStrEnd - (pStr+5);
			strncpy(M[i].texFile,pStr+5,len);
			M[i].texFile[len] = (char)0;
			if ( (pStr = strstr(buf,"aplane(")) != NULL ) {
				pStrEnd = strstr(pStr,")")-1;
				len = pStrEnd - (pStr+8);
				strncpy(M[i].alpFile,pStr+8,len);
				M[i].alpFile[len] = (char)0;
			}
			else {
				M[i].alpFile[0] = (char)0;
			}

		} else {
			M[i].useTex = FALSE;
			M[i].texFile[0] = (char)0;
			M[i].alpFile[0] = (char)0;
		}

		i++;
	}

}


/*=========================================================================
【関数】mqoReadVertex
【用途】頂点情報の読み込み
【引数】
		fp		現在オープンしているメタセコイアファイルのファイルポインタ
		V		頂点を格納する配列
		
【戻値】なし
【仕様】mqoReadObject()のサブ関数
=========================================================================*/

void mqoReadVertex(FILE *fp, glPOINT3f V[])
{
	char buf[SIZE_STR];
	int  i=0;

	while (1) {
		fgets(buf,SIZE_STR,fp);
		if (strstr(buf,"}")) break;
		sscanf(buf,"%f %f %f",&V[i].x,&V[i].y,&V[i].z);
		i++;
	}
}


/*=========================================================================
【関数】mqoReadBVertex
【用途】バイナリ形式の頂点情報を読み込む
【引数】
		fp		現在オープンしているメタセコイアファイルのファイルポインタ
		V		頂点を格納する配列

【戻値】頂点数
【仕様】mqoReadObject()のサブ関数
=========================================================================*/

int mqoReadBVertex(FILE *fp, glPOINT3f V[])
{
	int n_vertex,i;
	float *wf;
	int size;
	char cw[256];
	char *pStr;

	fgets(cw,sizeof(cw),fp);
	if ( (pStr = strstr(cw,"Vector")) != NULL ) {
		sscanf(pStr,"Vector %d [%d]",&n_vertex,&size);	// 頂点数、データサイズを読み込む
	}
	else {
		return -1;
	}
	//MQOファイルのバイナリ頂点データはintel形式（リトルエディアン）
	wf = (float *)malloc(size);
	fread(wf,size,1,fp);
	for ( i = 0; i < n_vertex; i++ ) {
		V[i].x = wf[i*3+0];
		V[i].y = wf[i*3+1];
		V[i].z = wf[i*3+2];
#if DEF_IS_LITTLE_ENDIAN
#else
		endianConverter((void *)&V[i].x,sizeof(V[i].x));
		endianConverter(&V[i].y,sizeof(V[i].y));
		endianConverter(&V[i].z,sizeof(V[i].z));
#endif
	}
	free(wf);

	// "}"まで読み飛ばし
	{
		char buf[SIZE_STR];
		while (1) {
			fgets(buf,SIZE_STR,fp);
			if (strstr(buf,"}")) break;
		}
	}

	return n_vertex;
}


/*=========================================================================
【関数】mqoReadFace
【用途】面情報の読み込み
【引数】
		fp		ファイルポインタ
		F		面配列
		
【戻値】なし
【仕様】mqoReadObject()のサブ関数
=========================================================================*/

void mqoReadFace(FILE *fp, MQO_FACE F[])
{
	char buf[SIZE_STR];
	char *pStr;
	int  i=0;

	while (1) {
		fgets(buf,SIZE_STR,fp);
		if (strstr(buf,"}")) break;

		// 面を構成する頂点数
		sscanf(buf,"%d",&F[i].n);

		// 頂点(V)の読み込み
		if ( (pStr = strstr(buf,"V(")) != NULL ) {
			switch (F[i].n) {
				case 3:
//メタセコは頂点の並びが表面からみて右回り
//読み込み時に並べ替える方法もある。けど、表面の設定を
//glFrontFaceで変えるほうがスマート？
					sscanf(pStr,"V(%d %d %d)",&F[i].v[0],&F[i].v[1],&F[i].v[2]);
//					sscanf(pStr,"V(%d %d %d)",&F[i].v[2],&F[i].v[1],&F[i].v[0]);
					break;
				case 4:
					sscanf(pStr,"V(%d %d %d %d)",&F[i].v[0],&F[i].v[1],&F[i].v[2],&F[i].v[3]);
//					sscanf(pStr,"V(%d %d %d %d)",&F[i].v[3],&F[i].v[2],&F[i].v[1],&F[i].v[0]);
					break;
				default:
					break;
			}		
		}

		// マテリアル(M)の読み込み
		F[i].m = 0;
		if ( (pStr = strstr(buf,"M(")) != NULL ) {
			sscanf(pStr,"M(%d)",&F[i].m);
		}
		else { // マテリアルが設定されていない面
			F[i].m = -1;
		}

		// UVマップ(UV)の読み込み
		if ( (pStr = strstr(buf,"UV(")) != NULL ) {
			switch (F[i].n) {
				case 3:	// 頂点数3
					sscanf(pStr,"UV(%f %f %f %f %f %f)",
									&F[i].uv[0].x, &F[i].uv[0].y,
									&F[i].uv[1].x, &F[i].uv[1].y,
									&F[i].uv[2].x, &F[i].uv[2].y
									);
					break;

				case 4:	// 頂点数4
					sscanf(pStr,"UV(%f %f %f %f %f %f %f %f)",
									&F[i].uv[0].x, &F[i].uv[0].y,
									&F[i].uv[1].x, &F[i].uv[1].y,
									&F[i].uv[2].x, &F[i].uv[2].y,
									&F[i].uv[3].x, &F[i].uv[3].y
									);
					break;
				default:
					break;
			}		
		}

		i++;
	}

}


/*=========================================================================
【関数】mqoReadObject
【用途】オブジェクト情報の読み込み
【引数】
		fp		ファイルポインタ
		obj		オブジェクト情報

【戻値】なし
【仕様】この関数で１個のオブジェクト情報が読み込まれる．
=========================================================================*/

void mqoReadObject(FILE *fp, MQO_OBJDATA *obj)
{
	char buf[SIZE_STR];

	while (1) {
		fgets(buf,SIZE_STR,fp);
		if (strstr(buf,"}")) break;

		// visible
		if (strstr(buf,"visible ")) {
			sscanf(buf," visible %d", &obj->visible);
		}

		// shading
		if (strstr(buf,"shading ")) {
			sscanf(buf," shading %d", &obj->shading);
		}

		// facet
		if (strstr(buf,"facet ")) {
			sscanf(buf," facet %f", &obj->facet);
		}

		// vertex
		if (strstr(buf,"vertex ")) {
			sscanf(buf," vertex %d", &obj->n_vertex);
			obj->V = (glPOINT3f*) calloc( obj->n_vertex, sizeof(glPOINT3f) );
			mqoReadVertex(fp, obj->V);
		}
		// BVertex
		if (strstr(buf,"BVertex"))	{
			sscanf(buf," BVertex %d", &obj->n_vertex);
			obj->V = (glPOINT3f*) calloc( obj->n_vertex, sizeof(glPOINT3f) );
			mqoReadBVertex(fp,obj->V);
		}

		// face
		if (strstr(buf,"face ")) {
			sscanf(buf," face %d", &obj->n_face);
			obj->F = (MQO_FACE*) calloc( obj->n_face, sizeof(MQO_FACE) );
			mqoReadFace(fp, obj->F);
		}

	}

}


/*=========================================================================
【関数】mqoMakeArray
【用途】頂点配列の作成
【引数】
		mat		マテリアル（この中に頂点データを含む）
		matpos	材質番号
		F		面
		fnum	面数
		V		頂点
		N		法線
		facet	スムージング角
		mcol	色
		scale	スケール
		alpha	アルファ

【戻値】なし
【仕様】頂点配列はすべて三角にするので、四角は三角ｘ２に分割
		  0  3      0    0  3
		   □   →　△　　▽
		  1  2     1  2   2  
=========================================================================*/

void mqoMakeArray( MQO_MATERIAL *mat, int matpos, MQO_FACE F[], int fnum,glPOINT3f V[],
				   glPOINT3f N[], double facet, glCOLOR4f *mcol, double scale, unsigned char alpha )
{
	int f;
	int i;
	int dpos;
	double s;
	glPOINT3f normal;	// 法線ベクトル
	
	dpos = 0;
	mat->color[0] = mcol->r;
	mat->color[1] = mcol->g;
	mat->color[2] = mcol->b;
	mat->color[3] = mcol->a;
	if ( mat->isUseTexture ) {
		for ( f = 0; f < fnum; f++ ){
			if ( F[f].m != matpos ) continue;
			if ( F[f].n == 3 ) {
				mqoSnormal(V[F[f].v[0]],V[F[f].v[1]],V[F[f].v[2]],&normal);	// 法線ベクトルを計算
				for ( i = 0; i < 3; i++ ) {
					mat->vertex_t[dpos].point[0] = V[F[f].v[i]].x*scale;
					mat->vertex_t[dpos].point[1] = V[F[f].v[i]].y*scale;
					mat->vertex_t[dpos].point[2] = V[F[f].v[i]].z*scale;
					mat->vertex_t[dpos].uv[0] = F[f].uv[i].x;
					mat->vertex_t[dpos].uv[1] = F[f].uv[i].y;
					s = acos(normal.x*N[F[f].v[i]].x + normal.y*N[F[f].v[i]].y + normal.z*N[F[f].v[i]].z);
					if ( facet < s ) {
						// スムージング角　＜（頂点法線と面法線の角度）のときは面法線を頂点法線とする
						mat->vertex_t[dpos].normal[0] = normal.x;
						mat->vertex_t[dpos].normal[1] = normal.y;
						mat->vertex_t[dpos].normal[2] = normal.z;
					}
					else {
						mat->vertex_t[dpos].normal[0] = N[F[f].v[i]].x;
						mat->vertex_t[dpos].normal[1] = N[F[f].v[i]].y;
						mat->vertex_t[dpos].normal[2] = N[F[f].v[i]].z;
					}
					dpos++;
				}
			}
			//４頂点（四角）は３頂点（三角）ｘ２に分割
			if ( F[f].n == 4 ) {
				mqoSnormal(V[F[f].v[0]],V[F[f].v[1]],V[F[f].v[2]],&normal);	// 法線ベクトルを計算
				for ( i = 0; i < 4; i++ ) {
					if ( i == 3 ) continue;
					mat->vertex_t[dpos].point[0] = V[F[f].v[i]].x*scale;
					mat->vertex_t[dpos].point[1] = V[F[f].v[i]].y*scale;
					mat->vertex_t[dpos].point[2] = V[F[f].v[i]].z*scale;
					mat->vertex_t[dpos].uv[0] = F[f].uv[i].x;
					mat->vertex_t[dpos].uv[1] = F[f].uv[i].y;
					s = acos(normal.x*N[F[f].v[i]].x + normal.y*N[F[f].v[i]].y + normal.z*N[F[f].v[i]].z);
					if ( facet < s ) {
						mat->vertex_t[dpos].normal[0] = normal.x;
						mat->vertex_t[dpos].normal[1] = normal.y;
						mat->vertex_t[dpos].normal[2] = normal.z;
					}
					else {
						mat->vertex_t[dpos].normal[0] = N[F[f].v[i]].x;
						mat->vertex_t[dpos].normal[1] = N[F[f].v[i]].y;
						mat->vertex_t[dpos].normal[2] = N[F[f].v[i]].z;
					}
					dpos++;
				}
				mqoSnormal(V[F[f].v[0]],V[F[f].v[2]],V[F[f].v[3]],&normal);	// 法線ベクトルを計算
				for ( i = 0; i < 4; i++ ) {
					if ( i == 1 ) continue;
					mat->vertex_t[dpos].point[0] = V[F[f].v[i]].x*scale;
					mat->vertex_t[dpos].point[1] = V[F[f].v[i]].y*scale;
					mat->vertex_t[dpos].point[2] = V[F[f].v[i]].z*scale;
					mat->vertex_t[dpos].uv[0] = F[f].uv[i].x;
					mat->vertex_t[dpos].uv[1] = F[f].uv[i].y;
					s = acos(normal.x*N[F[f].v[i]].x + normal.y*N[F[f].v[i]].y + normal.z*N[F[f].v[i]].z);
					if ( facet < s ) {
						mat->vertex_t[dpos].normal[0] = normal.x;
						mat->vertex_t[dpos].normal[1] = normal.y;
						mat->vertex_t[dpos].normal[2] = normal.z;
					}
					else {
						mat->vertex_t[dpos].normal[0] = N[F[f].v[i]].x;
						mat->vertex_t[dpos].normal[1] = N[F[f].v[i]].y;
						mat->vertex_t[dpos].normal[2] = N[F[f].v[i]].z;
					}
					dpos++;
				}
			}
		}
	}
	else {
		if ( alpha != 255 ) {
			mat->color[3] = (double)alpha/(double)255;
		}
		for ( f = 0; f < fnum; f++ ){
			if ( F[f].m != matpos ) continue;
			if ( F[f].n == 3 ) {
				mqoSnormal(V[F[f].v[0]],V[F[f].v[1]],V[F[f].v[2]],&normal);		// 法線ベクトルを計算
				for ( i = 0; i < 3; i++ ) {
					mat->vertex_p[dpos].point[0] = V[F[f].v[i]].x*scale;
					mat->vertex_p[dpos].point[1] = V[F[f].v[i]].y*scale;
					mat->vertex_p[dpos].point[2] = V[F[f].v[i]].z*scale;
					mat->vertex_p[dpos].normal[0] = normal.x;
					mat->vertex_p[dpos].normal[1] = normal.y;
					mat->vertex_p[dpos].normal[2] = normal.z;
					s = acos(normal.x*N[F[f].v[i]].x + normal.y*N[F[f].v[i]].y + normal.z*N[F[f].v[i]].z);
					if ( facet < s ) {
						mat->vertex_p[dpos].normal[0] = normal.x;
						mat->vertex_p[dpos].normal[1] = normal.y;
						mat->vertex_p[dpos].normal[2] = normal.z;
					}
					else {
						mat->vertex_p[dpos].normal[0] = N[F[f].v[i]].x;
						mat->vertex_p[dpos].normal[1] = N[F[f].v[i]].y;
						mat->vertex_p[dpos].normal[2] = N[F[f].v[i]].z;
					}
					dpos++;
				}
			}
			//４頂点（四角）は３頂点（三角）ｘ２に分割
			if ( F[f].n == 4 ) {
				mqoSnormal(V[F[f].v[0]],V[F[f].v[1]],V[F[f].v[2]],&normal);		// 法線ベクトルを計算
				for ( i = 0; i < 4; i++ ) {
					if ( i == 3 ) continue;
					mat->vertex_p[dpos].point[0] = V[F[f].v[i]].x*scale;
					mat->vertex_p[dpos].point[1] = V[F[f].v[i]].y*scale;
					mat->vertex_p[dpos].point[2] = V[F[f].v[i]].z*scale;
					mat->vertex_p[dpos].normal[0] = normal.x;
					mat->vertex_p[dpos].normal[1] = normal.y;
					mat->vertex_p[dpos].normal[2] = normal.z;
					s = acos(normal.x*N[F[f].v[i]].x + normal.y*N[F[f].v[i]].y + normal.z*N[F[f].v[i]].z);
					if ( facet < s ) {
						mat->vertex_p[dpos].normal[0] = normal.x;
						mat->vertex_p[dpos].normal[1] = normal.y;
						mat->vertex_p[dpos].normal[2] = normal.z;
					}
					else {
						mat->vertex_p[dpos].normal[0] = N[F[f].v[i]].x;
						mat->vertex_p[dpos].normal[1] = N[F[f].v[i]].y;
						mat->vertex_p[dpos].normal[2] = N[F[f].v[i]].z;
					}
					dpos++;
				}
				mqoSnormal(V[F[f].v[0]],V[F[f].v[2]],V[F[f].v[3]],&normal);		// 法線ベクトルを計算
				for ( i = 0; i < 4; i++ ) {
					if ( i == 1 ) continue;
					mat->vertex_p[dpos].point[0] = V[F[f].v[i]].x*scale;
					mat->vertex_p[dpos].point[1] = V[F[f].v[i]].y*scale;
					mat->vertex_p[dpos].point[2] = V[F[f].v[i]].z*scale;
					mat->vertex_p[dpos].normal[0] = normal.x;
					mat->vertex_p[dpos].normal[1] = normal.y;
					mat->vertex_p[dpos].normal[2] = normal.z;
					s = acos(normal.x*N[F[f].v[i]].x + normal.y*N[F[f].v[i]].y + normal.z*N[F[f].v[i]].z);
					if ( facet < s ) {
						mat->vertex_p[dpos].normal[0] = normal.x;
						mat->vertex_p[dpos].normal[1] = normal.y;
						mat->vertex_p[dpos].normal[2] = normal.z;
					}
					else {
						mat->vertex_p[dpos].normal[0] = N[F[f].v[i]].x;
						mat->vertex_p[dpos].normal[1] = N[F[f].v[i]].y;
						mat->vertex_p[dpos].normal[2] = N[F[f].v[i]].z;
					}
					dpos++;
				}
			}
		}
	}
}


/*=========================================================================
【関数】mqoVertexNormal
【用途】頂点法線の計算
【引数】
		obj			オブジェクト情報

【戻値】法線配列
【仕様】４頂点の面は三角形に分割して計算
		戻り値は必ず呼び出し元で解放（free）すること！
=========================================================================*/

glPOINT3f * mqoVertexNormal(MQO_OBJDATA *obj)
{
	int f;
	int v;
	int i;
	double len;
	glPOINT3f fnormal;	// 面法線ベクトル
	MQO_FACE *F;
	glPOINT3f *V;
	glPOINT3f *ret;
	F = obj->F;
	V = obj->V;
	ret = (glPOINT3f *)calloc(obj->n_vertex,sizeof(glPOINT3f));
	//面の法線を頂点に足し込み
	for ( f = 0; f < obj->n_face; f++ ) {
		if ( obj->F[f].n == 3 ) {
			mqoSnormal(V[F[f].v[0]],V[F[f].v[1]],V[F[f].v[2]],&fnormal);
			for ( i = 0; i < 3; i++ ) {
				ret[F[f].v[i]].x += fnormal.x;
				ret[F[f].v[i]].y += fnormal.y;
				ret[F[f].v[i]].z += fnormal.z;
			}
		}
		if ( obj->F[f].n == 4 ) {
			mqoSnormal(V[F[f].v[0]],V[F[f].v[1]],V[F[f].v[2]],&fnormal);
			for ( i = 0; i < 4; i++ ) {
				if ( i == 3 ) continue;
				ret[F[f].v[i]].x += fnormal.x;
				ret[F[f].v[i]].y += fnormal.y;
				ret[F[f].v[i]].z += fnormal.z;
			}
			mqoSnormal(V[F[f].v[0]],V[F[f].v[2]],V[F[f].v[3]],&fnormal);
			for ( i = 0; i < 4; i++ ) {
				if ( i == 1 ) continue;
				ret[F[f].v[i]].x += fnormal.x;
				ret[F[f].v[i]].y += fnormal.y;
				ret[F[f].v[i]].z += fnormal.z;
			}
		}
	}
	//正規化
	for ( v = 0; v < obj->n_vertex; v++ ) {
		if ( ret[v].x == 0 && ret[v].y == 0 && ret[v].z == 0 ) {
			//面に使われてない点
			continue;
		}
		len = sqrt(ret[v].x*ret[v].x + ret[v].y*ret[v].y + ret[v].z*ret[v].z);
		if ( len != 0 ) {
			ret[v].x = ret[v].x/len;
			ret[v].y = ret[v].y/len;
			ret[v].z = ret[v].z/len;
		}
	}

	return ret;
}

/*=========================================================================
【関数】mqoMakePolygon
【用途】ポリゴンの生成
【引数】
		readObj		読み込んだオブジェクト情報
		mqoobj		MQOオブジェクト 
		N[]			法線配列
		M[]			マテリアル配列
		n_mat		マテリアル数
		scale		スケール
		alpha		アルファ

【戻値】なし
=========================================================================*/

void mqoMakePolygon(MQO_OBJDATA *readObj, MQO_OBJECT *mqoobj,
					glPOINT3f N[], MQO_MATDATA M[], int n_mat, double scale, unsigned char alpha)
{

	MQO_INNER_OBJECT		*setObj;
	MQO_MATERIAL			*material;
	glCOLOR4f				defcol;
	glCOLOR4f				*pcol;
	int						f, m, *mat_vnum;
	int						fnum;
	MQO_FACE				*F;
	glPOINT3f				*V;
	double					facet;


	setObj = &mqoobj->obj[mqoobj->objnum];
	strcpy(setObj->objname,readObj->objname);
	setObj->isVisible = readObj->visible;
	setObj->isShadingFlat = (readObj->shading == 0);
	F = readObj->F;
	fnum = readObj->n_face;
	V = readObj->V;
	facet = readObj->facet;

	// faceの中でのマテリアル毎の頂点の数
	// M=NULLのとき、F[].m = 0 が入ってくる
	if ( M == NULL ) n_mat = 1;

	mat_vnum = (int *)malloc(sizeof(int)*n_mat);
	memset(mat_vnum,0,sizeof(int)*n_mat);

	for ( f = 0; f < fnum; f++ ){
		if( F[f].m < 0 ) continue; // マテリアルが設定されていない面
		if ( F[f].n == 3 ) {
			mat_vnum[F[f].m] += 3;
		}
		if ( F[f].n == 4 ) {
			//４頂点（四角）は３頂点（三角）ｘ２に分割
			//  0  3      0    0  3
			//   □   →　△　　▽
			//  1  2     1  2   2
			// ４頂点の平面データは
			// ３頂点の平面データｘ２個
			mat_vnum[F[f].m] += 3*2;
		}
		if ( setObj->matnum < F[f].m+1 ) setObj->matnum = F[f].m+1;
	}

	// マテリアル別に頂点配列を作成する
	setObj->mat = (MQO_MATERIAL *)malloc(sizeof(MQO_MATERIAL)*setObj->matnum);
	memset(setObj->mat,0,sizeof(MQO_MATERIAL)*setObj->matnum);

	for ( m = 0; m < setObj->matnum; m++ ) {
		material = &setObj->mat[m];
		material->datanum = mat_vnum[m];
		material->isValidMaterialInfo = (M != NULL);

		if ( mat_vnum[m] <= 0 ) continue;
		if ( material->isValidMaterialInfo ) {
			memcpy(material->dif,M[m].dif,sizeof(material->dif));
			memcpy(material->amb,M[m].amb,sizeof(material->amb));
			memcpy(material->spc,M[m].spc,sizeof(material->spc));
			memcpy(material->emi,M[m].emi,sizeof(material->emi));
			material->power = M[m].power;
			material->isUseTexture = M[m].useTex;
			pcol = &M[m].col;
		}
		else {
			defcol.r = 1.0;
			defcol.g = 1.0;
			defcol.b = 1.0;
			defcol.a = 1.0;
			material->isUseTexture = 0;
			pcol = &defcol;
		}
		if ( material->isUseTexture ) {
			material->vertex_t = (VERTEX_TEXUSE *)calloc(material->datanum,sizeof(VERTEX_TEXUSE));
			material->texture_id = M[m].texName;
		}
		else {
			material->vertex_p = (VERTEX_NOTEX *)calloc(material->datanum,sizeof(VERTEX_NOTEX));
		}
		mqoMakeArray(material,m,F,fnum,V,N,facet,pcol,scale,alpha);
		if (g_isVBOSupported) {
			if ( material->isUseTexture ) {
				glGenBuffersARB( 1, &material->VBO_id );
				glBindBufferARB( GL_ARRAY_BUFFER_ARB, material->VBO_id  );
				glBufferDataARB( GL_ARRAY_BUFFER_ARB, material->datanum*sizeof(VERTEX_TEXUSE), material->vertex_t, GL_STATIC_DRAW_ARB );
			}
			else {
				glGenBuffersARB( 1, &material->VBO_id );
				glBindBufferARB( GL_ARRAY_BUFFER_ARB, material->VBO_id  );
				glBufferDataARB( GL_ARRAY_BUFFER_ARB, material->datanum*sizeof(VERTEX_NOTEX), material->vertex_p, GL_STATIC_DRAW_ARB );
			}
		}
	}
	mqoobj->objnum++;
	if ( MAX_OBJECT <= mqoobj->objnum ) {
		printf("MQOファイル読み込み：　最大オブジェクト数を超えました[%d]\n",mqoobj->objnum);
		mqoobj->objnum = MAX_OBJECT-1;
	}

	free(mat_vnum);

}


/*=========================================================================
【関数】mqoMakeObjectsEx
【用途】オブジェクトのデータからポリゴンモデルを作成する
【引数】
		mqoobj	MQOオブジェクト
		obj		オブジェクト配列
		n_obj	オブジェクトの個数
		M		マテリアル配列
		n_mat	マテリアルの個数
		scale	拡大率
		alpha	アルファ

【戻値】なし
=========================================================================*/

void mqoMakeObjectsEx( MQO_OBJECT *mqoobj, MQO_OBJDATA obj[], int n_obj, MQO_MATDATA M[],int n_mat,
					   double scale,unsigned char alpha)
{
	int i;
	glPOINT3f *N;
	for (i=0; i<n_obj; i++) {
		N = mqoVertexNormal(&obj[i]);
		mqoMakePolygon(
				&obj[i],
				mqoobj,
				N,
				M,n_mat,
				scale,
				alpha);
		free(N);
	}
}


/*=========================================================================
【関数】mqoCreateModel
【用途】MQOファイルからMQOモデルを作成する
【引数】
		filename	MQOファイル
		scale		拡大率（1.0でそのまま）

【戻値】MQO_MODEL（MQOモデル）
=========================================================================*/

MQO_MODEL mqoCreateModel(char *filename, double scale)
{
	MQO_MODEL ret;
	ret = mqoCreateList(1);
	if ( mqoCreateListObject(ret,1-1,filename,scale,(unsigned char)255) < 0 ) {
		mqoDeleteObject(ret,1);
		ret = NULL;
	}
	return ret;
}


/*=========================================================================
【関数】mqoCreateSequenceEx
【用途】連番のMQOファイルからMQOシーケンスを作成する
【引数】
		format		ファイル名の書式
		n_file		ファイル数
		scale		拡大率（1.0でそのまま）
		fade_inout	0:そのまま　正：フェードイン　負：フェードアウト
					絶対値は効果をかけるフレーム数
		alpha		アルファ

【戻値】MQO_SEQUENCE（MQOシーケンス）
【備考】連番は0から開始
=========================================================================*/

MQO_SEQUENCE mqoCreateSequenceEx(const char *format, int n_file, double scale,
								 int fade_inout, unsigned char alpha)
{
	MQO_SEQUENCE retSeq;
	int iret;
	int seq;
	char filename[SIZE_STR];
	short setAlpha;
	short calAlpha;
	int frames;

	retSeq.n_frame = 0;
	if ( format == NULL ) {
		return retSeq;
	}
	calAlpha = alpha;
	frames = abs(fade_inout);
	frames = MAX(frames,n_file);
	setAlpha = (fade_inout<=0)?alpha:0;

	retSeq.model = mqoCreateList(n_file);
	for ( seq = 0; seq < frames; seq++ ) {
		if ( seq < n_file ) {
			sprintf(filename,format,seq);
		}
		if ( (fade_inout !=  0) && ((frames-1) == seq) ) {
			setAlpha = (fade_inout<0)?0:calAlpha;
		}
		iret = mqoCreateListObject(retSeq.model,seq,filename,scale,(unsigned char)setAlpha);
		if ( iret == - 1 ) {
			seq--;
			mqoClearObject(retSeq.model,seq,n_file-seq);
			break;
		}
		if ( fade_inout !=  0 ) {
			if ( fade_inout<0 ) {
				if ( (n_file-seq) <= (-1*fade_inout) ) {
					setAlpha -= (calAlpha/(-1*fade_inout));
					if ( setAlpha < 0 ) setAlpha = 0;
				}
			}
			else {
				setAlpha += (calAlpha/fade_inout);
				if ( calAlpha < setAlpha ) setAlpha = calAlpha;
			}
		}
	}
	retSeq.n_frame = seq;
	return retSeq;
}


/*=========================================================================
【関数】mqoCreateSequence
【用途】連番のMQOファイルからMQOシーケンスを作成する
【引数】
		format		ファイル名のフォーマット
		n_file		ファイル数
		scale		拡大率（1.0でそのまま）

【戻値】MQO_SEQUENCE（MQOシーケンス）
【備考】連番は0から開始
=========================================================================*/

MQO_SEQUENCE mqoCreateSequence(const char *format, int n_file, double scale)
{
	return mqoCreateSequenceEx(format, n_file, scale, 0, (unsigned char)255);
}


/*=========================================================================
【関数】mqoCallModel
【用途】MQOモデルをOpenGLの画面上に呼び出す
【引数】
		model		MQOモデル

【戻値】なし
=========================================================================*/

void mqoCallModel(MQO_MODEL model)
{
	mqoCallListObject(model, 0);
}


/*=========================================================================
【関数】mqoCallSequence
【用途】MQOシーケンスをOpenGLの画面に呼び出す
【引数】
		seq		MQOシーケンス
		i		フレーム番号

【戻値】なし
【仕様】MQOシーケンスの中から指定したフレーム番号のモデルを呼び出す
=========================================================================*/

void mqoCallSequence(MQO_SEQUENCE seq, int i)
{
	if ( i>=0 && i<seq.n_frame ) {
		mqoCallListObject(seq.model,i);
	}
}


/*=========================================================================
【関数】mqoClearObject
【用途】MQOオブジェクトのクリア
【引数】
		object	MQOオブジェクト配列
		from	削除開始番号（0～）
		num		削除する個数

【戻値】なし
=========================================================================*/

void mqoClearObject( MQO_OBJECT object[], int from, int num ) 
{
	int					loop, o, m;
	MQO_INNER_OBJECT	*obj;
	MQO_MATERIAL		*mat;

	if ( object == NULL ) return;

	for ( loop = from; loop < from + num; loop++ ) {
		for ( o = 0; o < (object+from)->objnum; o++ ) {

			obj = &(object+loop)->obj[o];
			for ( m = 0; m < obj->matnum; m++ ) {
				mat = &obj->mat[m];
				if ( mat->datanum <= 0 ) continue;
				if ( g_isVBOSupported ) {
					// 頂点バッファの削除
					glDeleteBuffersARB( 1, &mat->VBO_id );
				}

				// 頂点配列の削除
				if ( mat->isUseTexture ) {
					if ( mat->vertex_t != NULL ) {
						free(mat->vertex_t);
						mat->vertex_t = NULL;
					}
				}
				else {
					if ( mat->vertex_p != NULL ) {
						free(mat->vertex_p);
						mat->vertex_p = NULL;
					}
				}
			}
			if ( obj->mat != NULL ) {
				free(obj->mat);
				obj->mat = NULL;
			}
			obj->matnum = 0;
		}
	}

}


/*=========================================================================
【関数】mqoDeleteObject
【用途】MQOオブジェクトを削除する
【引数】
		object	MQOオブジェクト配列
		num		削除する個数

【戻値】なし
=========================================================================*/

void mqoDeleteObject(MQO_OBJECT object[], int num)
{
	mqoClearObject(object,0,num);
	free(object);
}


/*=========================================================================
【関数】mqoDeleteModel
【用途】MQOモデルを削除する
【引数】
		model	MQOモデル

【戻値】なし
【備考】削除処理を行った変数を再利用する可能性がある場合は
		この関数の実行後にNULLを代入しておくこと
=========================================================================*/

void mqoDeleteModel(MQO_MODEL model)
{
	mqoDeleteObject(model,1);
}


/*=========================================================================
【関数】mqoDeleteSequence
【用途】MQOシーケンスを削除する
【引数】
		seq		MQOシーケンス

【備考】削除処理を行った変数を再利用する可能性がある場合は
		この関数の実行後にNULLを代入しておくこと
=========================================================================*/

void mqoDeleteSequence(MQO_SEQUENCE seq)
{
	mqoDeleteObject( seq.model, seq.n_frame );
}