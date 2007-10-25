/* QuesoGLC
 * A free implementation of the OpenGL Character Renderer (GLC)
 * Copyright (c) 2002, 2004-2007, Bertrand Coconnier
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* $Id$ */

/** \file
 * header of the object __GLCcontext which is used to manage the contexts.
 */

#ifndef __glc_ocontext_h
#define __glc_ocontext_h

#ifndef __WIN32__
#include <pthread.h>
#else
#include <windows.h>
#endif

#include <ft2build.h>
#include FT_FREETYPE_H
#ifdef FT_CACHE_H
#include FT_CACHE_H
#endif
#include FT_LIST_H

#include "oarray.h"
#include "except.h"

#define GLC_MAX_MATRIX_STACK_DEPTH	32
#define GLC_MAX_ATTRIB_STACK_DEPTH	16

typedef struct __GLCcontextRec __GLCcontext;
typedef struct __GLCtextureRec __GLCtexture;
typedef struct __GLCenableStateRec __GLCenableState;
typedef struct __GLCrenderStateRec __GLCrenderState;
typedef struct __GLCstringStateRec __GLCstringState;
typedef struct __GLCglStateRec __GLCglState;
typedef struct __GLCattribStackLevelRec  __GLCattribStackLevel;
typedef struct __GLCthreadAreaRec __GLCthreadArea;
typedef struct __GLCcommonAreaRec  __GLCcommonArea;
typedef struct __GLCfontRec __GLCfont;

struct __GLCtextureRec {
  GLuint id;
  GLsizei width;
  GLsizei heigth;
  GLuint bufferObjectID;
};

struct __GLCenableStateRec {
  GLboolean autoFont;		/* GLC_AUTO_FONT */
  GLboolean glObjects;		/* GLC_GLOBJECTS */
  GLboolean mipmap;		/* GLC_MIPMAP */
  GLboolean hinting;		/* GLC_HINTING_QSO */
  GLboolean extrude;		/* GLC_EXTRUDE_QSO */
  GLboolean kerning;		/* GLC_KERNING_QSO */
};

struct __GLCrenderStateRec {
  GLfloat resolution;		/* GLC_RESOLUTION */
  GLint renderStyle;		/* GLC_RENDER_STYLE */
};

struct __GLCstringStateRec {
  GLint replacementCode;	/* GLC_REPLACEMENT_CODE */
  GLint stringType;		/* GLC_STRING_TYPE */
  GLCfunc callback;		/* Callback function GLC_OP_glcUnmappedCode */
  GLvoid* dataPointer;		/* GLC_DATA_POINTER */
};

struct __GLCglStateRec {
  GLboolean texture2D;
  GLint textureID;
  GLint textureEnvMode;
  GLint bufferObjectID;
  GLboolean blend;
  GLint blendSrc;
  GLint blendDst;
  GLboolean vertexArray;
  GLboolean normalArray;
  GLboolean colorArray;
  GLboolean indexArray;
  GLboolean texCoordArray;
  GLboolean edgeFlagArray;
};

struct __GLCattribStackLevelRec {
  GLbitfield attribBits;
  __GLCenableState enableState;
  __GLCrenderState renderState;
  __GLCstringState stringState;
  __GLCglState glState;
};

struct __GLCcontextRec {
  FT_ListNodeRec node;

  GLboolean isCurrent;
  GLCchar *buffer;
  GLint bufferSize;

  FT_Library library;
#ifdef FT_CACHE_H
  FTC_Manager cache;
#endif
  FcConfig *config;

  GLint id;			/* Context ID */
  GLboolean isInGlobalCommand;	/* Is in a global command ? */
  GLboolean pendingDelete;	/* Is there a pending deletion ? */
  __GLCenableState enableState;
  __GLCrenderState renderState;
  __GLCstringState stringState;
  FT_ListRec currentFontList;	/* GLC_CURRENT_FONT_LIST */
  FT_ListRec fontList;		/* GLC_FONT_LIST */
  __GLCarray* masterHashTable;
  __GLCarray* catalogList;	/* GLC_CATALOG_LIST */
  __GLCarray* measurementBuffer;
  GLfloat measurementStringBuffer[12];
  GLboolean isInCallbackFunc;	/* Is a callback function executing ? */
  GLint lastFontID;
  __GLCarray* vertexArray;	/* Array of vertices */
  __GLCarray* controlPoints;	/* Array of control points */
  __GLCarray* endContour;	/* Array of contour limits */
  __GLCarray* vertexIndices;	/* Array of vertex indices */
  __GLCarray* geomBatches;	/* Array of geometric batches */

  GLEWContext glewContext;	/* GLEW context for OpenGL extensions */
  __GLCtexture texture;		/* Texture for immediate mode rendering */

  __GLCtexture atlas;
  FT_ListRec atlasList;
  int atlasWidth;
  int atlasHeight;
  int atlasCount;

  GLfloat* bitmapMatrix;	/* GLC_BITMAP_MATRIX */
  GLfloat bitmapMatrixStack[4*GLC_MAX_MATRIX_STACK_DEPTH];
  GLint bitmapMatrixStackDepth;

  __GLCattribStackLevel attribStack[GLC_MAX_ATTRIB_STACK_DEPTH];
  GLint attribStackDepth;
};

struct __GLCthreadAreaRec {
  __GLCcontext* currentContext;
  GLCenum errorState;
  GLint lockState;
  FT_ListRec exceptionStack;
  __glcException failedTry;
};

struct __GLCcommonAreaRec {
  GLint versionMajor;		/* GLC_VERSION_MAJOR */
  GLint versionMinor;		/* GLC_VERSION_MINOR */

  FT_ListRec contextList;
#ifndef __WIN32__
  pthread_mutex_t mutex;	/* For concurrent accesses to the common
				   area */
#ifndef HAVE_TLS
  pthread_key_t threadKey;
  pthread_t threadID;
  pthread_once_t __glcInitThreadOnce;
#endif /* HAVE_TLS */
#else /* __WIN32__ */
  CRITICAL_SECTION section;
  DWORD threadKey;
  DWORD threadID;
  LONG __glcInitThreadOnce;
#endif

  /* Evil hack : we use the FT_MemoryRec_ structure definition which is
   * supposed not to be exported by FreeType headers. So this definition may
   * fail if the guys of FreeType decide not to expose FT_MemoryRec_ anymore.
   * However, this has not happened yet so we still rely on FT_MemoryRec_ ...
   */
  struct FT_MemoryRec_ memoryManager;
};

extern __GLCcommonArea __glcCommonArea;
#ifdef HAVE_TLS
extern __thread __GLCthreadArea __glcTlsThreadArea
    __attribute__((tls_model("initial-exec")));
#else
extern __GLCthreadArea* __glcThreadArea;
#endif

__GLCcontext* __glcContextCreate(GLint inContext);
void __glcContextDestroy(__GLCcontext *This);
__GLCfont* __glcContextGetFont(__GLCcontext *This, GLint code);
GLCchar* __glcContextQueryBuffer(__GLCcontext *This, int inSize);
void __glcContextAppendCatalog(__GLCcontext* This, const GLCchar* inCatalog);
void __glcContextPrependCatalog(__GLCcontext* This, const GLCchar* inCatalog);
void __glcContextRemoveCatalog(__GLCcontext* This, GLint inIndex);
GLCchar8* __glcContextGetCatalogPath(__GLCcontext* This, GLint inIndex);
#endif /* __glc_ocontext_h */
