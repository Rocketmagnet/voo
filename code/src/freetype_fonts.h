// Requires linking with freetype235MT_D.lib for debug
// Requires linking with freetype235MT.lib for release
//
// Much of this code is taken from NeHe's lesson 43.
// http://nehe.gamedev.net
// 

#ifndef FREETYPE_FONTS_H_INCLUDED
#define FREETYPE_FONTS_H_INCLUDED


#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include <wx/wx.h>
    #include <wx/mdi.h>
#endif



//FreeType Headers
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>
#include <freetype/ftmodapi.h>

//OpenGL Headers
#if HAVE_WINDOW_H
#include <windows.h>		//(the GL headers need it, only on Windows)
#endif // #if HAVE_WINDOW_H
#include <GL/gl.h>

//Some STL headers
#include <vector>
#include <string>

//Using the STL exception library increases the
//chances that someone else using our code will corretly
//catch any exceptions that we throw.
#include <stdexcept>


//MSVC will spit out all sorts of useless warnings if
//you create vectors of strings, this pragma gets rid of them.
#pragma warning(disable: 4786) 

#include <wx/filename.h>
#include "vector3d.h"


#define FREETYPE_FONT_RESOLUTION 64
#define FLIP_OVER true
#define CARRAGE_RETURN 10

class FreetypeFont
{
public:
	FreetypeFont();
    ~FreetypeFont();

	void     LoadFont(const wxFileName & fontFile, int fontResolution = FREETYPE_FONT_RESOLUTION);
	void     Print(float x, float y, float textHeight, const wxString & text, bool flipOver=false);
    Vector2D CalcSize(float textHeight, const wxString & text);
    void     SetColour(float rd, float gn, float bl, float al) { r = rd; g = gn; b = bl; a = al; }

   	//Free all the resources assosiated with the font.
	void Clean();

private:
	//The init function will create a font of
	//of the height h from the file fname.
	void Init(const char * fname, unsigned int h);

	int  NextP2(int a);
	void MakeDlist(FT_Face face, char ch);
	void PushScreenCoordinateMatrix();
	void PopProjectionMatrix();

    FT_Face     face;
    FT_Library  library;

	float		height;		///< Holds the height of the font
    Vector2D    *topLeft;
    Vector2D    *glyphSize;

	float		*advances;	///< How much to advance in X after each character is rendered
	float		*tx, *ty;
	GLuint		*textures;	///< Holds the texture id's
    float       r, g, b, a;
};

extern unsigned int testTexture;

#endif
