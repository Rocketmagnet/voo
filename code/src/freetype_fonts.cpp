#include "freetype_fonts.h"
#include <iostream>
#include <GL/glu.h>
#include "font_header.h"

using namespace std;

FreetypeFont::FreetypeFont():
  height(8),
  advances(0),
  tx(0),
  ty(0),
  textures(0),
  //topLeft(0),
  //glyphSize(0),
  face(0),
  library(0)
{
}

void FreetypeFont::Initialise(int fontResolution)
{
    //cout << "LoadFont(nothing)" << endl;
    Init("", fontResolution);
}

void FreetypeFont::LoadFont(const wxFileName & fontFile, int fontResolution)
{
    //cout << "LoadFont(" << fontFile.GetFullPath() << ")" << endl;
    Init(fontFile.GetFullPath().ToAscii(), fontResolution);
}

///Returns the power of 2, greater than or equal to a
int FreetypeFont::NextP2 ( int a )
{
  int rval=1;
  while(rval<a) rval<<=1;
  return rval;
}

///Create the textures corresponding to the given character.
void FreetypeFont::MakeDlist ( FT_Face face, char ch)
{
  //The first thing we do is get FreeType to render our character
  //into a bitmap.  This actually requires a couple of FreeType commands:
  FT_Glyph glyph;
  int width, height;
  int i, j;

  if(FT_Load_Glyph( face, FT_Get_Char_Index( face, ch ), FT_LOAD_DEFAULT ))   //Load the Glyph for our character.
    throw std::runtime_error("FT_Load_Glyph failed");
    //LogTime(_T("  LoadGlyph"));


  if(FT_Get_Glyph( face->glyph, &glyph ))                                     //Move the face's glyph into a Glyph object.
    throw std::runtime_error("FT_Get_Glyph failed");
    //LogTime(_T("  GetGlyph"));

  FT_Glyph_To_Bitmap( &glyph, ft_render_mode_normal, 0, 1 );                  //Convert the glyph to a bitmap.
    //LogTime(_T("  G2B"));
  FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;
    //LogTime(_T("  bitmap"));

  FT_Bitmap& bitmap = bitmap_glyph->bitmap;                                     //This reference will make accessing the bitmap easier

  width        = NextP2( bitmap.width );                                      //Use our helper function to get the widths of
  height       = NextP2( bitmap.rows );                                       //the bitmap data that we will need in order to create
/*
  cout << "  Character " << (char)ch << endl;
  cout << "topLeft[ch] (" << bitmap_glyph->left << ", " << bitmap_glyph->top << ") " << endl;
  cout << "       left " << bitmap_glyph->left<< endl;
  cout << "        top " << bitmap_glyph->top << endl;
  cout << " bitmap_top " << face->glyph->bitmap_left << endl;
  cout << "bitmap_left " << face->glyph->bitmap_top << endl << endl;
*/


  topLeft[ch]   = Vector2D(bitmap_glyph->left, -bitmap_glyph->top);
  glyphSize[ch] = Vector2D((float)bitmap.width, (float)bitmap.rows);
  advances[ch]  = (float)face->glyph->advance.x * (1.0f/64.0f);
  tx[ch]        = (float)bitmap.width / (float)width,
  ty[ch]        = (float)bitmap.rows  / (float)height;
    //LogTime(_T("  stuff"));

  GLubyte* pixelData = new GLubyte[width * height];                             //Allocate memory for the texture data.
    //LogTime(_T("  new GLubyte"));

  //Here we fill in the data for the expanded bitmap.
  //Notice that we are using two channel bitmap (one for
  //luminosity and one for alpha), we assign
  //luminosity=1.0 and alpha to the value that we
  //find in the FreeType bitmap. 
  //We use the ?: operator so that value which we use
  //will be 0 if we are in the padding zone, and whatever
  //is the the Freetype bitmap otherwise.
  for(j=0; j <height;j++)
    for(i=0; i < width; i++)
      pixelData[i+j*width] = (i>=bitmap.width || j>=bitmap.rows) ?    0 : bitmap.buffer[i + bitmap.width*j];
    //LogTime(_T("  pixeldata"));

  //Now we just setup some texture parameters.
  glBindTexture  (GL_TEXTURE_2D, textures[ch]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    //LogTime(_T("  build textures"));

  // Pass the texture data to OpenGL, and also also GLU to calculate a
  // series of Mip Maps to make it look good when small
  gluBuild2DMipmaps( GL_TEXTURE_2D, GL_ALPHA, width, height, GL_ALPHA, GL_UNSIGNED_BYTE, pixelData );
    //LogTime(_T("  build mapmaps"));

  //With the texture created, we don't need the expanded data anymore
  delete [] pixelData;
    //LogTime(_T("  delete"));

}


void FreetypeFont::Init(const char * fname, unsigned int h)
{
    //LogTime(_T("FreetypeFont::Init"));

  //Allocate some memory to store the texture ids.
  textures  = new GLuint[128];
  advances  = new  float[128];
  tx        = new  float[128];
  ty        = new  float[128];
  topLeft   = new Vector2D[128];
  glyphSize = new Vector2D[128];

  height=h;
    //LogTime(_T("  Allocate memory"));

  
    if (FT_Init_FreeType( &library ))                                           //Create and initialize a freetype font library.
        throw std::runtime_error("FT_Init_FreeType failed");

    //LogTime(_T("  Init"));

    FT_TrueTypeEngineType lTTET = FT_Get_TrueType_Engine_Type (library);
    //LogTime(_T("  Get Type"));

                                                                                //The object in which Freetype holds information on a given font is called a "face".
  
                                                                                //This is where we load in the font information from the file.
                                                                                //Of all the places where the code might die, this is the most likely,
                                                                                //as FT_New_Face will die if the font file does not exist or is somehow broken.
    //if (FT_New_Face( library, fname, 0, &face )) 
    //    throw std::runtime_error("FT_New_Face failed (there is probably a problem with your font file)");
	//if (FT_New_Face(library, fname, 0, &face))
	//	throw std::runtime_error("FT_New_Face failed (there is probably a problem with your font file)");

	FT_New_Memory_Face(library, fontData, FONT_DATA_SIZE, 0, &face);
	
    FT_Set_Char_Size( face, h*64, 0, 96, 96);                                   // The font size is stored by Freetype using 26.6 fixed point, so shift the needed size by
                                                                                // 6 place (or multiply by 64) to convert our value to that format.
    //LogTime(_T("  Char Size"));

    glGenTextures( 128, textures );                                             // Here we ask OpenGL to allocate resources for all the textures.
    //LogTime(_T("  glGenTextures"));

    for(unsigned char i=0;i<128;i++)                                            //This is where we actually create each of the fonts character texture.
    {
        MakeDlist(face, i);
    }

  //We don't need the face information now that the display
  //lists have been created, so we free the associated resources.
  //FT_Done_Face(face);

  //Ditto for the library.
  //FT_Done_FreeType(library);
}

FreetypeFont::~FreetypeFont()
{
  Clean();
}

void FreetypeFont::Clean()
{
  if (textures)
  {
    //glDeleteTextures(128,textures);   Argh. Can't do this because the GL canvas can't SetCurrent if it's hidden.
    delete [] textures; textures  = 0;
  }
  delete [] advances;     advances  = 0;
  delete [] tx;           tx        = 0;
  delete [] ty;           ty        = 0;
  delete [] topLeft;      topLeft   = 0;
  delete [] glyphSize;    glyphSize = 0;
}

/// A fairly straight forward function that pushes
/// a projection matrix that will make object world 
/// coordinates identical to window coordinates.
void FreetypeFont::PushScreenCoordinateMatrix()
{
  glPushAttrib(GL_TRANSFORM_BIT);
  GLint	viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(viewport[0],viewport[2],viewport[1],viewport[3]);
  glPopAttrib();
}

/// Pops the projection matrix without changing the current
/// MatrixMode.
void FreetypeFont::PopProjectionMatrix()
{
  glPushAttrib(GL_TRANSFORM_BIT);
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPopAttrib();
}


Vector2D FreetypeFont::CalcSize(float textHeight, const wxString & text)
{
    unsigned int  ch;
    float sizeAdjust = textHeight/height;
    Vector2D corner;

    float totalWidth=0, maxWidth=0, totalHeight=textHeight;


    if (!textures)                          // Did we get initialised?
        return Vector2D(0,0);               // If not, then abort


    unsigned int i;
    Vector2D tl, br;
    for(i=0; i<text.Length(); i++)
    {
        ch = text[i];

        if (ch == CARRAGE_RETURN)
        {
            if (totalWidth > maxWidth)
              maxWidth = totalWidth;

            totalWidth   = 0.0f;
            if (textHeight > 3.0f)
                totalHeight += floor(textHeight * 1.3f);        // Make the text align to the pixel if it's bigger than 3 pixels high
            else
                totalHeight += textHeight * 1.3f;
        }
        else
        {
            totalWidth += advances[ch] * sizeAdjust;
        }
    }

    return Vector2D(totalWidth, totalHeight);
}

void FreetypeFont::Print(float x, float y, float textHeight, const wxString & text, bool flipOver)
{
    unsigned int  ch=0;
    float sizeAdjust = textHeight/height, kern=0;
    Vector2D cursor(Vector2D(x,y+textHeight));
    Vector2D corner;
    char prevChar;

    if (!face)
        return;

    FT_UInt         glyph_index = 0;
    //FT_GlyphSlot    slot        = face->glyph;         // a small shortcut
    FT_Bool         useKerning  = FT_HAS_KERNING( face );
    FT_UInt         previous    = 0; 

    if (!textures)          // Did we get initialised?
        return;             // If not, then abort


    glPushAttrib(GL_LIGHTING_BIT | GL_TEXTURE_BIT | GL_ENABLE_BIT);

    glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);

    unsigned int i;
    Vector2D tl, br;
    for(i=0; i<text.Length(); i++)
    {
        prevChar=ch;
        ch = text[i];
        previous = glyph_index;
        glyph_index = FT_Get_Char_Index( face, ch );

        if (ch == CARRAGE_RETURN)
        {
            if (textHeight > 3.0f)
                cursor.y += floor(textHeight * 1.3f);           // Make the text align to the pixel if it's bigger than 3 pixels high
            else
                cursor.y += (textHeight * 1.3f);

            cursor.x = x;
        }
        else
        {
            if (useKerning)
            {
                FT_Vector delta;
                FT_Get_Kerning( face, previous, glyph_index, FT_KERNING_DEFAULT, &delta );
                kern = (delta.x/64.0f)* sizeAdjust;

                cursor.x += kern;
            }

            if (!flipOver)
            {// normal (y positive going down)
                tl = cursor +   topLeft[ch]*sizeAdjust;
                br =     tl + glyphSize[ch]*sizeAdjust;
            }
            else
            {// flipped over (y positive going up)
                tl = cursor;
                tl.x += topLeft[ch].x*sizeAdjust;
                tl.y -= topLeft[ch].y*sizeAdjust;

                br.x =     tl.x + glyphSize[ch].x*sizeAdjust;
                br.y =     tl.y - glyphSize[ch].y*sizeAdjust;
            }


            glBindTexture(GL_TEXTURE_2D, textures[ch]);
            glEnable(GL_TEXTURE_2D);
            glBegin(GL_QUADS);

            glColor4f(r,g,b,a);
            //glColor4f(1.0f,1.0f,1.0f,1.0f);
                glTexCoord2d(0,      0);      glVertex3f(tl.x, tl.y, 1.0f);
                glTexCoord2d(tx[ch], 0);      glVertex3f(br.x, tl.y, 1.0f);
                glTexCoord2d(tx[ch], ty[ch]); glVertex3f(br.x, br.y, 1.0f);
                glTexCoord2d(0,      ty[ch]); glVertex3f(tl.x, br.y, 1.0f);
            glEnd();

            /*
            // Draw a box around the character
            glDisable(GL_TEXTURE_2D);
            glBegin(GL_LINE_LOOP);
                glColor4f(1.0f,1.0f,1.0f,1.0f);
                glTexCoord2d(0,      0);      glVertex3f(tl.x, tl.y, 1.0f);
                glTexCoord2d(tx[ch], 0);      glVertex3f(br.x, tl.y, 1.0f);
                glTexCoord2d(tx[ch], ty[ch]); glVertex3f(br.x, br.y, 1.0f);
                glTexCoord2d(0,      ty[ch]); glVertex3f(tl.x, br.y, 1.0f);
            glEnd();
            */
            cursor.x += advances[ch] * sizeAdjust;
        }
    }
    glPopAttrib();
}
