
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
MIT License
Copyright (c) 2009 Sunao Hashimoto and Keisuke Konishi

*/

/*=========================================================================
[Global variable valid only within this source]
=========================================================================*/

static TEXTURE_POOL l_texPool[MAX_TEXTURE];		// Texture pool
static int			l_texPoolnum;				// Number of texture
static int			l_GLMetaseqInitialized = 0;	// Initialization flag


/*=========================================================================
[Function declaration]
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
[Function] endianConverter
[Applications] endian conversion
[Argument]
		addr	Address
		size	Size

[Return value] None
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
[Function] TGAHeaderEndianConverter
[Applications]TGA of the header of endian conversion
[Argument]
		tgah	TGA header of

[Return value] None
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
[Function]IsExtensionSupported
[Applications]Examine whether the extension of OpenGL is supported
[Argument]
		szTargetExtension	Name of the extension

[Return value] 1: are supported, 0: that is not
=========================================================================*/

int IsExtensionSupported( char* szTargetExtension )
{
	const unsigned char *pszExtensions = NULL;
	const unsigned char *pszStart;
	unsigned char *pszWhere, *pszTerminator;

	// Extension To investigate if the name is correct (NULL or blank NG)
	pszWhere = (unsigned char *) strchr( szTargetExtension, ' ' );
	if ( pszWhere || *szTargetExtension == (char)NULL )
		return 0;

	// Extension I income of string
	pszExtensions = glGetString( GL_EXTENSIONS );

	// Find out if there is extension necessary in a string
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
[Function]mqoInit
[Applications]Initialization of Metasequoia loader
[Argument] No
[Return value] None
=========================================================================*/

void mqoInit(void)
{
	// Texture pool initialization
	memset(l_texPool,0,sizeof(l_texPool));
	l_texPoolnum = 0;

	// Check the vertex buffer of support
	g_isVBOSupported = IsExtensionSupported("GL_ARB_vertex_buffer_object");
//	g_isVBOSupported = 0;

#ifdef WIN32
	glGenBuffersARB = NULL;
	glBindBufferARB = NULL;
	glBufferDataARB = NULL;
	glDeleteBuffersARB = NULL;

	if ( g_isVBOSupported ) {
		// printf("OpenGL : It is used because it supports the vertex buffer\n");
		// I income pointers GL function
		glGenBuffersARB = (PFNGLGENBUFFERSARBPROC) wglGetProcAddress("glGenBuffersARB");
		glBindBufferARB = (PFNGLBINDBUFFERARBPROC) wglGetProcAddress("glBindBufferARB");
		glBufferDataARB = (PFNGLBUFFERDATAARBPROC) wglGetProcAddress("glBufferDataARB");
		glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC) wglGetProcAddress("glDeleteBuffersARB");
	}
#endif

	// Initialization flag
	l_GLMetaseqInitialized = 1;
}


/*=========================================================================
[Function]mqoCleanup
[Applications]End processing of Metasequoia loader
[Argument]None
[Return value] None
=========================================================================*/

void mqoCleanup(void)
{
	mqoClearTexturePool();	// Clear texture pool
}


/*=========================================================================
[Function]mqoSetTexturePool
[Applications]Read a texture to texture pool
[Argument]
		texfile		Texture file name
		alpfile		Alpha file name
		alpha		Alpha

[Return value] texture ID
[Specification] texture read if it has not already been loaded, texture registration
It returns those registered if already loaded.
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
	if ( pos < l_texPoolnum ) { //Already Loaded
		return  l_texPool[pos].texture_id;
	}
	if ( MAX_TEXTURE <= pos ) {
		printf("%s:mqoSetTexturePool Texture read out of space\n",__FILE__);
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
	glGenTextures(1,&l_texPool[pos].texture_id);			// Generate texture
	glBindTexture(GL_TEXTURE_2D,l_texPool[pos].texture_id);	// Assignment of texture

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, l_texPool[pos].texsize, l_texPool[pos].texsize,
					0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	l_texPoolnum = pos+1;

	//If you register, the read buffer is unnecessary
	free(image);
	glBindTexture(GL_TEXTURE_2D,0);	// Assignment of default texture

	return l_texPool[pos].texture_id;
}


/*=========================================================================
[Function]mqoClearTexturePool()
[Applications]テクスチャプールの開放
[Argument]None
[Return value] None
=========================================================================*/

void mqoClearTexturePool()
{
	int pos;
	for ( pos = 0; pos < l_texPoolnum; pos++ ) {
		glDeleteTextures(1, &l_texPool[pos].texture_id);	// Remove texture information
	}

	memset(l_texPool,0,sizeof(l_texPool));
	l_texPoolnum = 0;
}


/*=========================================================================
[Function] mqoLoadTextureEx
To create a texture image from the [Applications] file
[Argument]
texfile file name
alpfile alpha file name
Return the tex_size size of the texture (the length of one side)

[Return value] pointer to the texture image (failure is NULL)
[Specification] 24bit bitmap, and 8,24,32bitTGA
Size is limited to "one side 2 of the n-th power of the square"
libjpeg, libpng (external library) if there is JPEG, PNG of readable
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
	for ( fl = 0; fl < 2; fl++ ) {//Texture = fl = 0 α = fl = 1
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
		if ( fl == 1 ) { //Reading of alpha TGAorPNG
			if ( ! (isTGA || isPNG) ) {
				printf("It is not possible to correspond to alpha of file %s\n",filename[fl]);
				break;
			}
		}
		if ( fp != NULL ) fclose(fp);
		if ( (fp=fopen(filename[fl],"rb"))==NULL ) {
			printf("%s:Texture read error[%s]\n",__FILE__,filename[fl]);
			continue;
		}
		// Header of load
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
			jpeg_create_decompress( &cinfo );	//Generation information thaw
			jpeg_stdio_src( &cinfo, fp );		//Read file specification
			jpeg_read_header( &cinfo, TRUE );	//jpeg header read
			jpeg_start_decompress( &cinfo );	//Thawing start

			if ( cinfo.out_color_components == 3 && cinfo.out_color_space == JCS_RGB ) {
				if ( jpegimage != NULL ) {
					for (i = 0; i < cinfo.output_height; i++) free(jpegimage[i]);            // Two lines will release a two-dimensional array or less
					free(jpegimage);
				}
				//Creating a read data array
				jpegimage = (JSAMPARRAY)malloc( sizeof( JSAMPROW ) * cinfo.output_height );
				for ( i = 0; i < cinfo.output_height; i++ ) {
					jpegimage[i] = (JSAMPROW)malloc( sizeof( JSAMPLE ) * cinfo.out_color_components * cinfo.output_width );
				}
				//Decompress data read
				while( cinfo.output_scanline < cinfo.output_height ) {
					jpeg_read_scanlines( &cinfo,
						jpegimage + cinfo.output_scanline,
						cinfo.output_height - cinfo.output_scanline
					);
				}
				size = width[fl] = cinfo.output_width;
			}

			jpeg_finish_decompress( &cinfo );	//Thawing end
			jpeg_destroy_decompress( &cinfo );	//Decompression information release
			if ( !(cinfo.out_color_components == 3 && cinfo.out_color_space == JCS_RGB) ) {
				printf("The JPEG support can not format %s\n",filename[fl]);
			}
#else
			printf("Format This texture can not cope %s\n",filename[fl]);
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
			png_ptr = png_create_read_struct(                       // It will ensure and initialize the png_ptr structure
							PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
			info_ptr = png_create_info_struct(png_ptr);             // It will ensure and initialize the info_ptr structure
			png_init_io(png_ptr, fp);                               // It will inform the fp in libpng
			png_read_info(png_ptr, info_ptr);                       // It will read the header of PNG files
			png_get_IHDR(png_ptr, info_ptr, &pngwidth, &pngheight,        // Get IHDR chunk information
							&bit_depth, &color_type, &interlace_type,
							&j,&k);
			if ( pngimage != NULL ) {
				for (i = 0; i < pngheight; i++) free(pngimage[i]);            // Two lines will release a two-dimensional array or less
				free(pngimage);
			}
			pngimage = (png_bytepp)malloc(pngheight * sizeof(png_bytep)); // The following three lines to ensure a two-dimensional array
			i = png_get_rowbytes(png_ptr, info_ptr);
			pngdepth = i / pngwidth;
			for (i = 0; i < pngheight; i++)
					pngimage[i] = (png_bytep)malloc(png_get_rowbytes(png_ptr, info_ptr));
			png_read_image(png_ptr, pngimage);                         // It will read the image data

			png_destroy_read_struct(                                // It will release two of the memory of structure
	        &png_ptr, &info_ptr, (png_infopp)NULL);
			size = width[fl] = pngwidth;
#else
			printf("Format This texture can not cope %s\n",filename[fl]);
			continue;
#endif
		}
		if ( width[fl] == -1 ) {//Size come up here is not specified = bitmap
			fseek(fp,14+4,SEEK_SET);		// Seek to a position where the image width is stored
			fread(&size,sizeof(int),1,fp);	// Acquisition only BiWidth of information
			fseek(fp,14+40,SEEK_SET);		// Seek to the position where the pixel data is stored
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
		if ( fl == 1 && isTGA ) { //8-bit monochrome or32-bit full of reading of alpha TGA
			if ( !(
				(tgah.depth == 8 && tgah.type == DEF_TGA_TYPE_MONO) ||
				(tgah.depth == 32 && tgah.type == DEF_TGA_TYPE_FULL) 
				) ) {
				break;
			}
		}
		if ( fl == 1 && isPNG ) { //Reading of alpha PNG of true color + alpha or gray scale + alpha
#if DEF_USE_LIBPNG
			if ( !(
				(color_type== 6 ) ||
				(color_type== 4 ) 
				) ) {
				break;
			}
#endif
		}

		// Secure memory
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
						if ( color_type == 6 ) { //True color + alpha
							pRead[3]= pngimage[size-1-y][x*pngdepth+3];
						}
						if ( color_type == 4 ) { //Gray scale + alpha
							pRead[3]= pngimage[size-1-y][x*pngdepth+1];
						}
						if ( alpha < pRead[3] ) pRead[3] = alpha;
#endif
					}
					if ( isTGA ) {
						if ( tgah.depth == 32 ) { //To skip the data you do not need
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
		for (uy = 0; uy < pngheight; uy++) free(pngimage[uy]);            // Two lines will release a two-dimensional array or less
		free(pngimage);
	}
#endif
#if DEF_USE_LIBJPEG
	if ( jpegimage != NULL ) {
		unsigned int uy;
		for (uy = 0; uy < cinfo.output_height; uy++) free(jpegimage[uy]);            // Two lines will release a two-dimensional array or less
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
[Function] mqoLoadFile
It reads the data from the [Applications] Metasequoia file (* .mqo)
[Argument]
mqoobj MQO object
The path of the file filename
scale enlargement factor
alpha alpha

[Return value] Success: 1 / Failure: 0
=========================================================================*/

int mqoLoadFile( MQO_OBJECT *mqoobj, char *filename, double scale, unsigned char alpha)
{
	FILE			*fp;
	MQO_OBJDATA		obj[MAX_OBJECT];
	MQO_MATDATA		*M = NULL;

	char	buf[SIZE_STR];		// String read buffer
	char	path_dir[SIZE_STR];	// Directory path
	char	path_tex[SIZE_STR];	// The path of the texture file
	char	path_alp[SIZE_STR];	// The path of the alpha texture file
	int		n_mat = 0;			// Number of Materials
	int		n_obj = 0;			// The number of objects
	int		i;

	// Reading of Material and Object
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

	// Acquisition of path
	mqoGetDirectory(filename, path_dir);

	// Registration of texture
	for (i=0; i<n_mat; i++) {
		if (M[i].useTex) {

			if (strstr(M[i].texFile,":")) {
				strcpy(path_tex, M[i].texFile);	// In the case of an absolute path
			} else {
				sprintf(path_tex,"%s%s",path_dir,M[i].texFile);	// In the case of a relative path
			}

			if ( M[i].alpFile[0] != (char)0 ) {
				if (strstr(M[i].texFile,":")) {
					strcpy(path_alp, M[i].alpFile);	// In the case of an absolute path
				} else {
					sprintf(path_alp,"%s%s",path_dir,M[i].alpFile);	// In the case of a relative path
				}
				M[i].texName = mqoSetTexturePool(path_tex,path_alp,alpha);
			}
			else {
				M[i].texName = mqoSetTexturePool(path_tex,NULL,alpha);
			}
		}
	}

	mqoMakeObjectsEx( mqoobj, obj, n_obj, M, n_mat, scale, alpha );

	// The opening of the object of data
	for (i=0; i<n_obj; i++) {
		free(obj[i].V);
		free(obj[i].F);
	}

	// The opening of the material
	free(M);

	return 1;
}


/*=========================================================================
[Function] mqoCreateList
You want to specify the number of securing the [Applications] MQO object
[Argument] num MQO number of objects

[Return value] MQO object
=========================================================================*/

MQO_OBJECT* mqoCreateList(int num)
{
	MQO_OBJECT *obj;

	// Initialization I had not been initialized
	if ( ! l_GLMetaseqInitialized ) mqoInit();

	// Area secure and initialization
	obj = (MQO_OBJECT *)malloc(sizeof(MQO_OBJECT)*num);
	memset(obj, 0, sizeof(MQO_OBJECT)*num);

	return obj;
}


/*=========================================================================
[Function] mqoCreateListObject
Create a MQO object array from the [Applications] Metasequoia file (* .mqo)

[Argument] mqoobj MQO object
i read destination number (i th I read the MQO file)
The path of the file filename
scale enlargement factor
alpha alpha specification (specify the overall alpha value (0-255))

[Return value] Status negative: Abnormal 0: Normal
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
[Function] mqoCallListObject
I call [Applications] the MQO object on the screen of the OpenGL
[Argument]
mqoobj MQO object array
num sequence number (0 ~)

[Return value] None
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
		//Metaseko is right around the sequence of vertices is viewed from the surface
		glGetIntegerv(GL_FRONT_FACE,&intFrontFace);
		glFrontFace(GL_CW);
		dalpha = (double)mqoobj[num].alpha/(double)255;

		for ( o=0; o<mqoobj[num].objnum; o++ ) {	// Internal object loop

			obj = &mqoobj[num].obj[o];
			if ( ! obj->isVisible ) continue;
			glShadeModel(((obj->isShadingFlat))?GL_FLAT:GL_SMOOTH);

			for ( m = 0; m < obj->matnum; m++ ) {	//Materials loop

				mat = &obj->mat[m];
				if ( mat->datanum == 0 ) continue;

				if ( mat->isValidMaterialInfo ) {	// Information setting of material
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

				if ( mat->isUseTexture) {	// If there is a texture
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

					if ( g_isVBOSupported ) {	// Vertex buffer usage
						base = (char *)NULL;	// Address top NULL
						glBindBufferARB( GL_ARRAY_BUFFER_ARB, mat->VBO_id ); // Tie the vertex buffer
					}
					else {
						// When the vertex array, put the address as it is
						base = (char *)mat->vertex_t[0].point;
					}

					// Set the vertex array
					offset = (int)( (char *)mat->vertex_t[0].point - (char *)mat->vertex_t[0].point );
					glVertexPointer( 3, GL_FLOAT, sizeof(VERTEX_TEXUSE) , base + offset );

					// Set the texture coordinate array
					offset = (int)((char *)mat->vertex_t[0].uv-(char *)mat->vertex_t[0].point);
					glTexCoordPointer( 2, GL_FLOAT, sizeof(VERTEX_TEXUSE) , base + offset );

					// Set the normal array
					offset = (int)((char *)mat->vertex_t[0].normal-(char *)mat->vertex_t[0].point);
					glNormalPointer( GL_FLOAT, sizeof(VERTEX_TEXUSE) , base+offset );

					// Color setting
					glColor4f(mat->color[0],mat->color[1],mat->color[2],mat->color[3]);

					// Execution drawing
					glDrawArrays( GL_TRIANGLES, 0, mat->datanum );

					glBindTexture(GL_TEXTURE_2D,bindGL_TEXTURE_2D);
					if( isGL_BLEND == GL_FALSE ) glDisable(GL_BLEND);
					if( isGL_TEXTURE_2D == GL_FALSE ) glDisable(GL_TEXTURE_2D);

					if ( g_isVBOSupported ) {						// Vertex buffer usage
						glBindBufferARB( GL_ARRAY_BUFFER_ARB, 0 );	// The vertex buffer to the default
					}

					glDisableClientState( GL_VERTEX_ARRAY );
					glDisableClientState( GL_NORMAL_ARRAY );
					glDisableClientState( GL_TEXTURE_COORD_ARRAY );
				}
				else {	// If there is no texture

					glEnableClientState( GL_VERTEX_ARRAY );
					glEnableClientState( GL_NORMAL_ARRAY );
				//	glEnableClientState( GL_COLOR_ARRAY );

					isGL_BLEND = glIsEnabled(GL_BLEND);
					glEnable(GL_BLEND);
					glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

					if ( g_isVBOSupported ) {	// Vertex buffer usage
						base = (char *)NULL;
						glBindBufferARB( GL_ARRAY_BUFFER_ARB, mat->VBO_id );
					}
					else {
						base = (char *)mat->vertex_p[0].point;
					}

					// Set the vertex array
					offset = (int)((char *)mat->vertex_p[0].point-(char *)mat->vertex_p[0].point);
					glVertexPointer( 3, GL_FLOAT, sizeof(VERTEX_NOTEX) , base+offset );

					// Set the normal array
					offset = (int)((char *)mat->vertex_p[0].normal-(char *)mat->vertex_p[0].point);
					glNormalPointer( GL_FLOAT, sizeof(VERTEX_NOTEX) , base+offset );

					// Color setting
					glColor4f(mat->color[0],mat->color[1],mat->color[2],mat->color[3]);
				//	offset = (int)((char *)mat->vertex_p[0].color-(char *)mat->vertex_p[0].point);
				//	glColorPointer(4,GL_FLOAT,sizeof(VERTEX_NOTEX),base+offset);

					// Execution drawing
					glDrawArrays( GL_TRIANGLES, 0, mat->datanum );

					if( isGL_BLEND == GL_FALSE ) glDisable(GL_BLEND);
					if ( g_isVBOSupported ) {						// Vertex buffer usage
						glBindBufferARB( GL_ARRAY_BUFFER_ARB, 0 );	// The vertex buffer to the default
					}

				//	glDisableClientState( GL_COLOR_ARRAY );
					glDisableClientState( GL_VERTEX_ARRAY );
					glDisableClientState( GL_NORMAL_ARRAY );

				}
			}
		}

		//Metaseko is right around in a sequence of vertices are viewed from the surface (back to the original setting)
		glFrontFace(intFrontFace);
	glPopMatrix();
}


/*=========================================================================
[Function] mqoGetDirectory
To extract only the directory path from a string that contains the [Applications] file name
[Argument]
* path_file path string that contains the file name (input)
* path_dir file name path string except the (output)

[Return value] None
[Specification] Example:
"C: /data/file.bmp" → "C: / data /"
"data / file.mqo" → "data /"
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
[Function] mqoSnormal
Ask the [Applications] Normal vector
[Argument]
A point on the A 3-dimensional coordinates
Point B on the B 3-dimensional coordinates
Point C on C 3-dimensional coordinate
* normal vector BA and vector BC Normal vector (right screw direction)

[Return value] None
[Specification] number of vertices that make up the surface in Metasequoia, when viewed from the display surface
It has described in a clockwise direction. That is, when the vertices A, B, C were found
It is is normal to be obtained determined by the cross product of BA and BC
=========================================================================*/

void mqoSnormal(glPOINT3f A, glPOINT3f B, glPOINT3f C, glPOINT3f *normal)
{
	double norm;
	glPOINT3f vec0,vec1;

	// Vector BA
	vec0.x = A.x - B.x; 
	vec0.y = A.y - B.y;
	vec0.z = A.z - B.z;

	// Vector BC
	vec1.x = C.x - B.x;
	vec1.y = C.y - B.y;
	vec1.z = C.z - B.z;

	// Normal vector
	normal->x = vec0.y * vec1.z - vec0.z * vec1.y;
	normal->y = vec0.z * vec1.x - vec0.x * vec1.z;
	normal->z = vec0.x * vec1.y - vec0.y * vec1.x;

	// Normalization
	norm = normal->x * normal->x + normal->y * normal->y + normal->z * normal->z;
	norm = sqrt ( norm );

	normal->x /= norm;
	normal->y /= norm;
	normal->z /= norm;
}


/*=========================================================================
[Function] mqoReadMaterial
Reading of [Applications] material information
[Argument]
fp file pointer
M Materials array

[Return value] None
[Specification] mqoCreateModel (), sub-function of mqoCreateSequence ().
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
		fgets(buf,SIZE_STR,fp);	// Line read
		if (strstr(buf,"}")) break;

		pStr = strstr(buf,"col(");	// Skip Material name
		sscanf( pStr,
				"col(%f %f %f %f) dif (%f) amb(%f) emi(%f) spc(%f) power(%f)",
				&c.r, &c.g, &c.b, &c.a, &dif, &amb, &emi, &spc, &M[i].power );

		// Vertex color
		M[i].col = c;

		// Diffusion light
		M[i].dif[0] = dif * c.r;
		M[i].dif[1] = dif * c.g;
		M[i].dif[2] = dif * c.b;
		M[i].dif[3] = c.a;

		// Surrounding light
		M[i].amb[0] = amb * c.r;
		M[i].amb[1] = amb * c.g;
		M[i].amb[2] = amb * c.b;
		M[i].amb[3] = c.a;

		// Self-lighting
		M[i].emi[0] = emi * c.r;
		M[i].emi[1] = emi * c.g;
		M[i].emi[2] = emi * c.b;
		M[i].emi[3] = c.a;

		// Reflected light
		M[i].spc[0] = spc * c.r;
		M[i].spc[1] = spc * c.g;
		M[i].spc[2] = spc * c.b;
		M[i].spc[3] = c.a;
		
		// tex: pattern mapping name
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
[Function] mqoReadVertex
[Applications] reading of vertex information
[Argument]
fp File pointer of currently open Metasequoia file
Array to store the V vertex

[Return value] None
[Specification] sub function of mqoReadObject ()
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
[Function] mqoReadBVertex
To read the vertex information of [Applications] binary format
[Argument]
fp File pointer of currently open Metasequoia file
Array to store the V vertex

[Return value] Vertex
[Specification] sub function of mqoReadObject ()
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
		sscanf(pStr,"Vector %d [%d]",&n_vertex,&size);	// Vertex, read the data size
	}
	else {
		return -1;
	}
	//Binary vertex data intel format of MQO files (Little endian)
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

	// And skip to the "}"
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
[Function] mqoReadFace
Reading of [Applications] surface information
[Argument]
fp file pointer
F plane array

[Return value] None
[Specification] sub function of mqoReadObject ()
=========================================================================*/

void mqoReadFace(FILE *fp, MQO_FACE F[])
{
	char buf[SIZE_STR];
	char *pStr;
	int  i=0;

	while (1) {
		fgets(buf,SIZE_STR,fp);
		if (strstr(buf,"}")) break;

		// Vertex that make up the surface
		sscanf(buf,"%d",&F[i].n);

		// Reading of the vertex (V)
		if ( (pStr = strstr(buf,"V(")) != NULL ) {
			switch (F[i].n) {
				case 3:
// Metaseko is right around the sequence of vertices is viewed from the surface
// There is also a method to sort at the time of reading. But, the setting of the surface
// better to change in glFrontFace is smart?
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

		// Reading of the material (M)
		F[i].m = 0;
		if ( (pStr = strstr(buf,"M(")) != NULL ) {
			sscanf(pStr,"M(%d)",&F[i].m);
		}
		else { // Surface material has not been set
			F[i].m = -1;
		}

		// Reading of UV map (UV)
		if ( (pStr = strstr(buf,"UV(")) != NULL ) {
			switch (F[i].n) {
				case 3:	// Vertex3
					sscanf(pStr,"UV(%f %f %f %f %f %f)",
									&F[i].uv[0].x, &F[i].uv[0].y,
									&F[i].uv[1].x, &F[i].uv[1].y,
									&F[i].uv[2].x, &F[i].uv[2].y
									);
					break;

				case 4:	// Vertex4
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
[Function] mqoReadObject
[Applications] reading of object information
[Argument]
fp file pointer
obj object information

[Return value] None
[Specification] one of the object information in this function is read.
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
[Function] mqoMakeArray
Creation of the [Applications] vertex array
[Argument]
mat material (it includes the vertex data in this)
matpos material number
F surface
fnum surface number
V vertex
N normal
facet smoothing angle
mcol color
scale scale
alpha alpha

[Return value] None
[Specification] Since all vertex array to triangle, square divided into triangular x2
0 3 0 0 3
□ → △ ▽
1 2 1 2 2
=========================================================================*/

void mqoMakeArray( MQO_MATERIAL *mat, int matpos, MQO_FACE F[], int fnum,glPOINT3f V[],
				   glPOINT3f N[], double facet, glCOLOR4f *mcol, double scale, unsigned char alpha )
{
	int f;
	int i;
	int dpos;
	double s;
	glPOINT3f normal;	// Normal vector
	
	dpos = 0;
	mat->color[0] = mcol->r;
	mat->color[1] = mcol->g;
	mat->color[2] = mcol->b;
	mat->color[3] = mcol->a;
	if ( mat->isUseTexture ) {
		for ( f = 0; f < fnum; f++ ){
			if ( F[f].m != matpos ) continue;
			if ( F[f].n == 3 ) {
				mqoSnormal(V[F[f].v[0]],V[F[f].v[1]],V[F[f].v[2]],&normal);	// To calculate the normal vector
				for ( i = 0; i < 3; i++ ) {
					mat->vertex_t[dpos].point[0] = V[F[f].v[i]].x*scale;
					mat->vertex_t[dpos].point[1] = V[F[f].v[i]].y*scale;
					mat->vertex_t[dpos].point[2] = V[F[f].v[i]].z*scale;
					mat->vertex_t[dpos].uv[0] = F[f].uv[i].x;
					mat->vertex_t[dpos].uv[1] = F[f].uv[i].y;
					s = acos(normal.x*N[F[f].v[i]].x + normal.y*N[F[f].v[i]].y + normal.z*N[F[f].v[i]].z);
					if ( facet < s ) {
						// I and vertex normal the surface normal when smoothing angle <of (the angle of the vertex normal and the surface normal)
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
			//4 vertex (squares) divided into three vertex (triangle) x2
			if ( F[f].n == 4 ) {
				mqoSnormal(V[F[f].v[0]],V[F[f].v[1]],V[F[f].v[2]],&normal);	// To calculate the normal vector
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
				mqoSnormal(V[F[f].v[0]],V[F[f].v[2]],V[F[f].v[3]],&normal);	// To calculate the normal vector
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
				mqoSnormal(V[F[f].v[0]],V[F[f].v[1]],V[F[f].v[2]],&normal);		// To calculate the normal vector
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
			//4 vertex (squares) divided into three vertex (triangle) x2
			if ( F[f].n == 4 ) {
				mqoSnormal(V[F[f].v[0]],V[F[f].v[1]],V[F[f].v[2]],&normal);		// To calculate the normal vector
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
				mqoSnormal(V[F[f].v[0]],V[F[f].v[2]],V[F[f].v[3]],&normal);		// To calculate the normal vector
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
[Function] mqoVertexNormal
[Applications] calculation of vertex normals
[Argument]
obj object information

[Return value] normal array
[Specification] 4 vertex of the surface is calculated by dividing the triangle
The return value is always released by the caller (free) that!
=========================================================================*/

glPOINT3f * mqoVertexNormal(MQO_OBJDATA *obj)
{
	int f;
	int v;
	int i;
	double len;
	glPOINT3f fnormal;	// Surface normal vector
	MQO_FACE *F;
	glPOINT3f *V;
	glPOINT3f *ret;
	F = obj->F;
	V = obj->V;
	ret = (glPOINT3f *)calloc(obj->n_vertex,sizeof(glPOINT3f));
	//The summation of the normal line of the surface to the apex
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
	//Normalization
	for ( v = 0; v < obj->n_vertex; v++ ) {
		if ( ret[v].x == 0 && ret[v].y == 0 && ret[v].z == 0 ) {
			//That it is not used in the surface
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
[Function] mqoMakePolygon
[Applications] polygon generation of
[Argument]
readObj the loaded object information
mqoobj MQO object
N [] normal array
M [] Materials array
n_mat number of material
scale scale
alpha alpha

[Return value] None
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

	// The number of vertices of each Material in the face
	// When M = NULL, F []. M = 0 is incoming
	if ( M == NULL ) n_mat = 1;

	mat_vnum = (int *)malloc(sizeof(int)*n_mat);
	memset(mat_vnum,0,sizeof(int)*n_mat);

	for ( f = 0; f < fnum; f++ ){
		if( F[f].m < 0 ) continue; // Surface material has not been set
		if ( F[f].n == 3 ) {
			mat_vnum[F[f].m] += 3;
		}
		if ( F[f].n == 4 ) {
// 4 vertex (squares) divided into three vertex (triangle) x2
// 0 3 0 0 3
// □ → △ ▽
// 1 2 1 2 2
// 4 vertices of the plane data
// 3 plane data x2 vertices
			mat_vnum[F[f].m] += 3*2;
		}
		if ( setObj->matnum < F[f].m+1 ) setObj->matnum = F[f].m+1;
	}

	// You want to create a material different to the vertex array
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
		printf("MQO file read: have exceeded the maximum number of objects [% d]\n",mqoobj->objnum);
		mqoobj->objnum = MAX_OBJECT-1;
	}

	free(mat_vnum);

}


/*=========================================================================
[Function] mqoMakeObjectsEx
To create a polygon model from [Applications] object of data
[Argument]
mqoobj MQO object
obj object array
The number of n_obj object
M Materials array
n_mat number of material
scale enlargement factor
alpha alpha

[Return value] None
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
[Function] mqoCreateModel
Create a MQO model from [Applications] MQO file
[Argument]
filename MQO file
scale enlargement factor (as in 1.0)

[Return value] MQO_MODEL (MQO model)
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
[Function] mqoCreateSequenceEx
You want to create a MQO sequence from [Applications] serial number MQO file
[Argument]
format file name format
n_file number of files
scale enlargement factor (as in 1.0)
fade_inout 0: as it is positive: fade-negative: fade-out
The number of frames absolute value of applying the effect
alpha alpha

[Return value] MQO_SEQUENCE (MQO sequence)
[Note] serial number start from 0
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
[Function] mqoCreateSequence
You want to create a MQO sequence from [Applications] serial number MQO file
[Argument]
The format of the format file name
n_file number of files
scale enlargement factor (as in 1.0)

[Return value] MQO_SEQUENCE (MQO sequence)
[Note] serial number start from 0
=========================================================================*/

MQO_SEQUENCE mqoCreateSequence(const char *format, int n_file, double scale)
{
	return mqoCreateSequenceEx(format, n_file, scale, 0, (unsigned char)255);
}


/*=========================================================================
[Function] mqoCallModel
Call the [Applications] MQO model on the screen of the OpenGL
[Argument]
model MQO model

[Return value] None
=========================================================================*/

void mqoCallModel(MQO_MODEL model)
{
	mqoCallListObject(model, 0);
}


/*=========================================================================
[Function] mqoCallSequence
Call [Applications] the MQO sequence in OpenGL screen
[Argument]
seq MQO sequence
i frame number

[Return value] None
Call the model of the specified frame number from among the [specification] MQO sequence
=========================================================================*/

void mqoCallSequence(MQO_SEQUENCE seq, int i)
{
	if ( i>=0 && i<seq.n_frame ) {
		mqoCallListObject(seq.model,i);
	}
}


/*=========================================================================
[Function] mqoClearObject
[Applications] Clear MQO object
[Argument]
object MQO object array
Delete from start number (0 ~)
the number you want to delete num

[Return value] None
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
					// Delete the vertex buffer
					glDeleteBuffersARB( 1, &mat->VBO_id );
				}

				// Delete the vertex array
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
[Function] mqoDeleteObject
You want to remove the [Applications] MQO object
[Argument]
object MQO object array
the number you want to delete num

[Return value] None
=========================================================================*/

void mqoDeleteObject(MQO_OBJECT object[], int num)
{
	mqoClearObject(object,0,num);
	free(object);
}


/*=========================================================================
[Function] mqoDeleteModel
You want to remove the [Applications] MQO model
[Argument]
model MQO model

[Return value] None
[Note] If there is a possibility to re-use a variable that made the removal process
That it should be assigned a NULL after the execution of this function
=========================================================================*/

void mqoDeleteModel(MQO_MODEL model)
{
	mqoDeleteObject(model,1);
}


/*=========================================================================
[Function] mqoDeleteSequence
You want to remove the [Applications] MQO sequence
[Argument]
seq MQO sequence

[Note] If there is a possibility to re-use a variable that made the removal process
That it should be assigned a NULL after the execution of this function
=========================================================================*/

void mqoDeleteSequence(MQO_SEQUENCE seq)
{
	mqoDeleteObject( seq.model, seq.n_frame );
}