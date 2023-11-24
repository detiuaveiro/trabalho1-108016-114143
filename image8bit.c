/// image8bit - A simple image processing module.
///
/// This module is part of a programming project
/// for the course AED, DETI / UA.PT
///
/// You may freely use and modify this code, at your own risk,
/// as long as you give proper credit to the original and subsequent authors.
///
/// João Manuel Rodrigues <jmr@ua.pt>
/// 2013, 2023

// Student authors (fill in below):
// NMec:108016  Name:Fábio Alves
// NMec:114143 Name:André Alexandre
// 
// 
// Date:
//

#include "image8bit.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "instrumentation.h"

// The data structure
//
// An image is stored in a structure containing 3 fields:
// Two integers store the image width and height.
// The other field is a pointer to an array that stores the 8-bit gray
// level of each pixel in the image.  The pixel array is one-dimensional
// and corresponds to a "raster scan" of the image from left to right,
// top to bottom.
// For example, in a 100-pixel wide image (img->width == 100),
//   pixel position (x,y) = (33,0) is stored in img->pixel[33];
//   pixel position (x,y) = (22,1) is stored in img->pixel[122].
// 
// Clients should use images only through variables of type Image,
// which are pointers to the image structure, and should not access the
// structure fields directly.

// Maximum value you can store in a pixel (maximum maxval accepted)
const uint8 PixMax = 255;

// Internal structure for storing 8-bit graymap images
struct image {
  int width;
  int height;
  int maxval;   // maximum gray value (pixels with maxval are pure WHITE)
  uint8* pixel; // pixel data (a raster scan)
};


// This module follows "design-by-contract" principles.
// Read `Design-by-Contract.md` for more details.

/// Error handling functions

// In this module, only functions dealing with memory allocation or file
// (I/O) operations use defensive techniques.
// 
// When one of these functions fails, it signals this by returning an error
// value such as NULL or 0 (see function documentation), and sets an internal
// variable (errCause) to a string indicating the failure cause.
// The errno global variable thoroughly used in the standard library is
// carefully preserved and propagated, and clients can use it together with
// the ImageErrMsg() function to produce informative error messages.
// The use of the GNU standard library error() function is recommended for
// this purpose.
//
// Additional information:  man 3 errno;  man 3 error;

// Variable to preserve errno temporarily
static int errsave = 0;

// Error cause
static char* errCause;

/// Error cause.
/// After some other module function fails (and returns an error code),
/// calling this function retrieves an appropriate message describing the
/// failure cause.  This may be used together with global variable errno
/// to produce informative error messages (using error(), for instance).
///
/// After a successful operation, the result is not garanteed (it might be
/// the previous error cause).  It is not meant to be used in that situation!
char* ImageErrMsg() { ///
  return errCause;
}


// Defensive programming aids
//
// Proper defensive programming in C, which lacks an exception mechanism,
// generally leads to possibly long chains of function calls, error checking,
// cleanup code, and return statements:
//   if ( funA(x) == errorA ) { return errorX; }
//   if ( funB(x) == errorB ) { cleanupForA(); return errorY; }
//   if ( funC(x) == errorC ) { cleanupForB(); cleanupForA(); return errorZ; }
//
// Understanding such chains is difficult, and writing them is boring, messy
// and error-prone.  Programmers tend to overlook the intricate details,
// and end up producing unsafe and sometimes incorrect programs.
//
// In this module, we try to deal with these chains using a somewhat
// unorthodox technique.  It resorts to a very simple internal function
// (check) that is used to wrap the function calls and error tests, and chain
// them into a long Boolean expression that reflects the success of the entire
// operation:
//   success = 
//   check( funA(x) != error , "MsgFailA" ) &&
//   check( funB(x) != error , "MsgFailB" ) &&
//   check( funC(x) != error , "MsgFailC" ) ;
//   if (!success) {
//     conditionalCleanupCode();
//   }
//   return success;
// 
// When a function fails, the chain is interrupted, thanks to the
// short-circuit && operator, and execution jumps to the cleanup code.
// Meanwhile, check() set errCause to an appropriate message.
// 
// This technique has some legibility issues and is not always applicable,
// but it is quite concise, and concentrates cleanup code in a single place.
// 
// See example utilization in ImageLoad and ImageSave.
//
// (You are not required to use this in your code!)


// Check a condition and set errCause to failmsg in case of failure.
// This may be used to chain a sequence of operations and verify its success.
// Propagates the condition.
// Preserves global errno!
static int check(int condition, const char* failmsg) {
  errCause = (char*)(condition ? "" : failmsg);
  return condition;
}


/// Init Image library.  (Call once!)
/// Currently, simply calibrate instrumentation and set names of counters.
void ImageInit(void) { ///
  InstrCalibrate();
  InstrName[0] = "pixmem";  // InstrCount[0] will count pixel array acesses
  // Name other counters here...
  
}

// Macros to simplify accessing instrumentation counters:
#define PIXMEM InstrCount[0]
// Add more macros here...

// TIP: Search for PIXMEM or InstrCount to see where it is incremented!


/// Image management functions

/// Create a new black image.
///   width, height : the dimensions of the new image.
///   maxval: the maximum gray level (corresponding to white).
/// Requires: width and height must be non-negative, maxval > 0.
/// 
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageCreate(int width, int height, uint8 maxval) { ///
  assert (width >= 0);   
  assert (height >= 0);   
  assert (0 < maxval && maxval <= PixMax);
  // Insert your code here!
  Image newImage = (Image)malloc(sizeof(*newImage));    // criamos nova imagem, malloc aloca um bloco de memória
  if (newImage == NULL) {   // verificamos se ouve alguma falha ao criar a imagem
    errCause = "Falha ao alocar memória";
    return NULL;
  }

  newImage->width = width;
  newImage->height = height;
  newImage->maxval = maxval;
  newImage->pixel = (uint8_t*)calloc(width * height, sizeof(uint8_t));  // estamos a alucar memória para o campo do pixel do objeto newImage.
                                                                        // calloc é usada para alocar memória para uma matriz de uint8_t com tamanho de largura * altura, e inicia-os a 0;
  if (newImage->pixel == NULL) {  // verificamos se ocorreu algum erro a alocar
    errCause = "Falha ao alocar memória";
    free(newImage);  // usamos free() para desalocar o espaço previamente criado, visto que ocorreu um erro no pixel
    return NULL;
  }
  return newImage;
  
}

/// Destroy the image pointed to by (*imgp).
///   imgp : address of an Image variable.
/// If (*imgp)==NULL, no operation is performed.
/// Ensures: (*imgp)==NULL.
/// Should never fail, and should preserve global errno/errCause.
void ImageDestroy(Image* imgp) { ///
  assert (imgp != NULL);
  // Insert your code here!
  free((*imgp)->pixel);   // Desalocamos o espaço na memória do pixel
  free(*imgp);  // Desalocamos o espaço na memória da imagem
  imgp = NULL;   // "Apagamos" a imagem
  
}


/// PGM file operations

// See also:
// PGM format specification: http://netpbm.sourceforge.net/doc/pgm.html

// Match and skip 0 or more comment lines in file f.
// Comments start with a # and continue until the end-of-line, inclusive.
// Returns the number of comments skipped.
static int skipComments(FILE* f) {
  char c;
  int i = 0;
  while (fscanf(f, "#%*[^\n]%c", &c) == 1 && c == '\n') {
    i++;
  }
  return i;
}

/// Load a raw PGM file.
/// Only 8 bit PGM files are accepted.
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageLoad(const char* filename) { ///
  int w, h;
  int maxval;
  char c;
  FILE* f = NULL;
  Image img = NULL;

  int success = 
  check( (f = fopen(filename, "rb")) != NULL, "Open failed" ) &&
  // Parse PGM header
  check( fscanf(f, "P%c ", &c) == 1 && c == '5' , "Invalid file format" ) &&
  skipComments(f) >= 0 &&
  check( fscanf(f, "%d ", &w) == 1 && w >= 0 , "Invalid width" ) &&
  skipComments(f) >= 0 &&
  check( fscanf(f, "%d ", &h) == 1 && h >= 0 , "Invalid height" ) &&
  skipComments(f) >= 0 &&
  check( fscanf(f, "%d", &maxval) == 1 && 0 < maxval && maxval <= (int)PixMax , "Invalid maxval" ) &&
  check( fscanf(f, "%c", &c) == 1 && isspace(c) , "Whitespace expected" ) &&
  // Allocate image
  (img = ImageCreate(w, h, (uint8)maxval)) != NULL &&
  // Read pixels
  check( fread(img->pixel, sizeof(uint8), w*h, f) == w*h , "Reading pixels" );
  PIXMEM += (unsigned long)(w*h);  // count pixel memory accesses

  // Cleanup
  if (!success) {
    errsave = errno;
    ImageDestroy(&img);
    errno = errsave;
  }
  if (f != NULL) fclose(f);
  return img;
}

/// Save image to PGM file.
/// On success, returns nonzero.
/// On failure, returns 0, errno/errCause are set appropriately, and
/// a partial and invalid file may be left in the system.
int ImageSave(Image img, const char* filename) { ///
  assert (img != NULL);
  int w = img->width;
  int h = img->height;
  uint8 maxval = img->maxval;
  FILE* f = NULL;

  int success =
  check( (f = fopen(filename, "wb")) != NULL, "Open failed" ) &&
  check( fprintf(f, "P5\n%d %d\n%u\n", w, h, maxval) > 0, "Writing header failed" ) &&
  check( fwrite(img->pixel, sizeof(uint8), w*h, f) == w*h, "Writing pixels failed" ); 
  PIXMEM += (unsigned long)(w*h);  // count pixel memory accesses

  // Cleanup
  if (f != NULL) fclose(f);
  return success;
}


/// Information queries

/// These functions do not modify the image and never fail.

/// Get image width
int ImageWidth(Image img) { ///
  assert (img != NULL);
  return img->width;
}

/// Get image height
int ImageHeight(Image img) { ///
  assert (img != NULL);
  return img->height;
}

/// Get image maximum gray level
int ImageMaxval(Image img) { ///
  assert (img != NULL);
  return img->maxval;
}

/// Pixel statsS
/// Find the minimum and maximum gray levels in image.
/// On return,
/// *min is set to the minimum gray level in the image,
/// *max is set to the maximum.
void ImageStats(Image img, uint8* min, uint8* max) { ///
  assert (img != NULL);
  // Insert your code here!  
  for (int y = 0; y < (img->height); y++) {    // percorremos todos os pixeis da imagem
    for (int x = 0; x < (img->width); x++) {
      int currentpixel = ImageGetPixel(img, x, y);
      if (currentpixel < *min) {    // verificamos se o currentpixel é menor que o antigo min
        *min = currentpixel;
      } else if (currentpixel > *max) {   // verificamos se o currentpixel é maior que antigo max
        *max = currentpixel;
      }
    }
  }
}

/// Check if pixel position (x,y) is inside img.
int ImageValidPos(Image img, int x, int y) { ///
  assert (img != NULL);
  return (0 <= x && x < img->width) && (0 <= y && y < img->height);
}

/// Check if rectangular area (x,y,w,h) is completely inside img.
int ImageValidRect(Image img, int x, int y, int w, int h) { ///
  assert (img != NULL);
  // Insert your code here!
  return (0 <= x && x + w <= img->width) && (0 <= y && y + h <= img->height);
}

/// Pixel get & set operations

/// These are the primitive operations to access and modify a single pixel
/// in the image.
/// These are very simple, but fundamental operations, which may be used to 
/// implement more complex operations.

// Transform (x, y) coords into linear pixel index.
// This internal function is used in ImageGetPixel / ImageSetPixel. 
// The returned index must satisfy (0 <= index < img->width*img->height)
static inline int G(Image img, int x, int y) {
  assert(img != NULL);
  int index;
  // Insert your code here!
  index = y * img->width + x;   // calcula o índice do primeiro pixel na linha y da imagem
  assert (0 <= index && index < img->width*img->height);   // asseguramos que o index é menor que o número total de pixeis da imagem, considerando o pixel (0,0)
  return index;
}

/// Get the pixel (level) at position (x,y).
uint8 ImageGetPixel(Image img, int x, int y) { ///
  assert (img != NULL);
  assert (ImageValidPos(img, x, y));
  PIXMEM += 1;  // count one pixel access (read)
  return img->pixel[G(img, x, y)];
} 

/// Set the pixel at position (x,y) to new level.
void ImageSetPixel(Image img, int x, int y, uint8 level) { ///
  assert (img != NULL);
  assert (ImageValidPos(img, x, y));
  PIXMEM += 1;  // count one pixel access (store)
  img->pixel[G(img, x, y)] = level;
} 


/// Pixel transformations

/// These functions modify the pixel levels in an image, but do not change
/// pixel positions or image geometry in any way.
/// All of these functions modify the image in-place: no allocation involved.
/// They never fail.


/// Transform image to negative image.
/// This transforms dark pixels to light pixels and vice-versa,
/// resulting in a "photographic negative" effect.
void ImageNegative(Image img) { ///
  assert (img != NULL);  
  // Insert your code here!
  double invlevel;
  for (int y = 0; y < img->height; y++) {    // percorremos a imagem
    for (int x = 0; x < img->width; x++) {
      uint8 currentpixel = ImageGetPixel(img, x, y);
      invlevel = (uint8)((255 - currentpixel)+0.5);   // o valor negativo do pixel vai ser o seu l(pixel data - 255), o 0.5 é para arrendamento de inteiros
      ImageSetPixel(img, x, y, invlevel);   // mudamos o pixel na imagem para o pixel negativo
    }
  }
}

/// Apply threshold to image.
/// Transform all pixels with level<thr to black (0) and
/// all pixels with level>=thr to white (maxval).
void ImageThreshold(Image img, uint8 thr) { ///
  assert (img != NULL);
  // Insert your code here!
  uint8 thrlevel;
  for (int y = 0; y < img->height; y++) {   // percorremos a imagem
    for (int x = 0; x < img->width; x++) {
      uint8 currentpixel = ImageGetPixel(img, x, y);    // guardamos o pixel data 
      if (currentpixel<thr){
        thrlevel=0;   // thr fica com o valor para um pixel totalmente escuro
      }
      else{
        thrlevel=img->maxval;   // thr fica com o valor para um pixel totalmente branco
      }
      ImageSetPixel(img, x, y, thrlevel);    // mudamos o pixel com o novo pixel
    }
  }
}

/// Brighten image by a factor.
/// Multiply each pixel level by a factor, but saturate at maxval.
/// This will brighten the image if factor>1.0 and
/// darken the image if factor<1.0.
void ImageBrighten(Image img, double factor) { ///
  assert (img != NULL);
  assert (factor >= 0.0);
  uint8 brighpixel;
  // Insert your code here!
  for (int y = 0; y < img->height; y++) {    // percorremos a imagem
    for (int x = 0; x < img->width; x++) {
      uint8 currentpixel = ImageGetPixel(img, x, y);
      brighpixel = (uint8)((factor*currentpixel)+0.5);    // multiplicamos o valor do pixel pelo fator, quanto maior o fator mais claro vai ficar, 0.5 serve para arrendamento de int
      if (brighpixel > (img->maxval)) {  // Não pode ser mais que o maxval ou seja 255, neste caso
        brighpixel = 255;     
      }
      ImageSetPixel(img, x, y, brighpixel);   // mudamos o pixel com o novo pixel
    }
  }
}


/// Geometric transformations

/// These functions apply geometric transformations to an image,
/// returning a new image as a result.
/// 
/// Success and failure are treated as in ImageCreate:
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.

// Implementation hint: 
// Call ImageCreate whenever you need a new image!

/// Rotate an image.
/// Returns a rotated version of the image.
/// The rotation is 90 degrees anti-clockwise.
/// Ensures: The original img is not modified.
/// 
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageRotate(Image img) { ///
  assert (img != NULL);
  // Insert your code here!
  int nimage_height = img->width;
  int nimage_width = img->height;
  int nimage_maxval = img->maxval;
  Image nimage = ImageCreate(nimage_height, nimage_width, nimage_maxval);  // criamos uma nova imagem com os mesmos dados da outra
  if (nimage == NULL) {
    errCause = "Falha ao gerar nova imagem";    // caso o processo de criação da imagem não tenha corrido bem
    return NULL;
  }
  for (int y=0;y<nimage_height;y++){    // percorremos a nova imagem  
    for (int x=0;x<nimage_width;x++){
      // para ocorrer rotação, definimos os pixeis da nova imagem ao contrário, ou seja y,x em vez de x,y
      // pois ao ocorrer rotação antihorária, as coordenadas trocam-se
      // no entanto fornecemos o valor do pixel da imagem original
      // na coordenada x damos o valor espelhado horizontalmente (width - 1 - j) pois a rotação é -90º
      // na coordenada y damos o valor original pois se a rotação é -90º, a coordenada é a mesma
      ImageSetPixel(nimage, y, x, ImageGetPixel(img, nimage_width -1 - x , y));                                   
    }
  }
  return nimage;
}

/// Mirror an image = flip left-right.
/// Returns a mirrored version of the image.
/// Ensures: The original img is not modified.
/// 
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageMirror(Image img) { ///
  assert (img != NULL);
  // Insert your code here!
  int nimage_width = img->width;
  int nimage_height= img->height;
  int nimage_maxval = img->maxval;
  Image nimage = ImageCreate(nimage_width, nimage_height, nimage_maxval);
  if (nimage == NULL) {
    errCause = "Falha ao gerar nova imagem";
    return NULL;
  }
  for (int y=0;y<(img->height);y++){   // percorremos a imagem
    for (int x=0;x<(img->width);x++){
      // nos argumentos da ImageGetPixel metemos nimage_width-x-1 para espelhar a imagem horizontalmente
      ImageSetPixel(nimage, x, y, ImageGetPixel(img, nimage_width-x-1 , y));  /// -1 porque para altura de 4, temos 0,1,2,3
    }
  }
  return nimage;
  
}

/// Crop a rectangular subimage from img.
/// The rectangle is specified by the top left corner coords (x, y) and
/// width w and height h.
/// Requires:
///   The rectangle must be inside the original image.
/// Ensures:
///   The original img is not modified.
///   The returned image has width w and height h.
/// 
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageCrop(Image img, int x, int y, int w, int h) { ///
  assert (img != NULL);
  assert (ImageValidRect(img, x, y, w, h));
  // Insert your code here!
  int nmaxval = img->maxval;
  Image nimage = ImageCreate(w, h, nmaxval);   // criamos uma nova imagem, que vai ser a imagem cortada
  if (nimage == NULL) {
    errCause = "Falha ao gerar nova imagem";
    return NULL;
  }
  for (int i = 0; i < h; i++) {  // percorremos a imagem
    for (int j = 0; j < w; j++) {
      // as coordenadas x e y para a ImagegetPixel são x+j e y+i pois temos de adicionar a coordenada top-left do retangulo
      ImageSetPixel(nimage, j, i, ImageGetPixel(img, x + j, y + i));
    }
  }
  return nimage;
}


/// Operations on two images

/// Paste an image into a larger image.
/// Paste img2 into position (x, y) of img1.
/// This modifies img1 in-place: no allocation involved.
/// Requires: img2 must fit inside img1 at position (x, y).
void ImagePaste(Image img1, int x, int y, Image img2) { ///
  assert (img1 != NULL);
  assert (img2 != NULL);
  assert (ImageValidRect(img1, x, y, img2->width, img2->height));
  // Insert your code here!
  for (int i = 0; i < img2->height; i++) {   // percorremos a imagem
    for (int j = 0; j < img2->width; j++) {
      if ((i+y) < img1->height && (j+x) < img1->width) {  // verificamos que estamos dentro da imagem
        ImageSetPixel(img1, j+x, i+y, ImageGetPixel(img2, j, i));   // colocamos o pixel tendo em conta as coordenadas top-left da img2
      }
    }
  }
}


/// Blend an image into a larger image.
/// Blend img2 into position (x, y) of img1.
/// This modifies img1 in-place: no allocation involved.
/// Requires: img2 must fit inside img1 at position (x, y).
/// alpha usually is in [0.0, 1.0], but values outside that interval
/// may provide interesting effects.  Over/underflows should saturate.
void ImageBlend(Image img1, int x, int y, Image img2, double alpha) { ///
  assert (img1 != NULL);
  assert (img2 != NULL);
  assert (alpha >= 0.0 && alpha <= 1.0);
  assert (ImageValidRect(img1, x, y, img2->width, img2->height));
  // Insert your code here!
  for (int i = 0; i < img2->height; i++) {   // percorremos a imagem
    for (int j = 0; j < img2->width; j++) {
      if ((i+y) < img1->height && (j+x) < img1->width) {   // verificamos se as coordenadas da img1 e da img2 somadas estão dentro dos limites
        uint8 pixel1 = ImageGetPixel(img1, j+x, i+y);   // j+x e i+y pois estamos a selecionar o pixel da img1 correspondente ao da img2
        uint8 pixel2 = ImageGetPixel(img2, j, i);  // vamos buscar o pixel na img2
        uint8 newpixel = (uint8)(((1.0 - alpha) * pixel1 + alpha * pixel2)+0.5);   // Se alpha for 0.0, o resultado será idêntico a pixel1, se alpha for 1.0, o resultado será idêntico a pixel2, e para valores intermediários de alpha, o resultado será uma combinação ponderada dos dois pixels.
        if (newpixel > 255) {   // overflow aturate
          newpixel = 255;
        } else if (newpixel < 0){   // underflow
          newpixel = 0;
        }
        ImageSetPixel(img1, j+x, i+y, newpixel);  // mudamos o pixel, temos j+x e i+y tendo em consideração a posição da img2 na img1
      }
    }
  }
}




/// Compare an image to a subimage of a larger image.
/// Returns 1 (true) if img2 matches subimage of img1 at pos (x, y).
/// Returns 0, otherwise.
int ImageMatchSubImage(Image img1, int x, int y, Image img2) { 
  assert (img1 != NULL);
  assert (img2 != NULL);
  assert (ImageValidPos(img1, x, y));
  // Insert your code here!
  if (!ImageValidRect(img1,x,y,img2->width,img2->height)){    //vemos se a area retangular da img2 está contida na img1
    return 0;
  }
  for(int i=0; i<img2->height; i++){  // percorremos a imagem
    for(int j=0; j<img2->width; j++){
      if (ImageGetPixel(img1,j+x,i+y)!=ImageGetPixel(img2,j,i)){   // se as imagens não coincidirem, j+x e i+y pois temos de ter em consideração a posição da img2 em relação à img1
        return 0;
      }
    }
  }
  InstrCount[0] += 1;  // to count addition
  return 1;   // se coincidirem retorna 1
}


/// Locate a subimage inside another image.
/// Searches for img2 inside img1.
/// If a match is found, returns 1 and matching position is set in vars (*px, *py).
/// If no match is found, returns 0 and (*px, *py) are left untouched.
int ImageLocateSubImage(Image img1, int* px, int* py, Image img2) { ///
  assert (img1 != NULL);
  assert (img2 != NULL);
  void InstrCalibrate();
  void InstrReset();
  InstrName[0]="ncomp";
  // Insert your code here!
  for(int i=0; i<img1->height-img2->height; i++){    // percorremos a imagem
    for(int j=0; j<img1->width-img2->width; j++){
      if(ImageMatchSubImage(img1,j,i,img2)) {   // Comparamos a img1 com a subimage img2
        if(px != NULL){
          *px = j;   // *px toma a posição da coordenada x
        }
        if(py != NULL){
          *py = i;   // *py toma a posição da coordenada y
        }
        InstrPrint();  // to show time and counters
        return 1;   
      }
    }
  }
  InstrPrint();
  return 0;
}


/// Filtering

/// Blur an image by a applying a (2dx+1)x(2dy+1) mean filter.
/// Each pixel is substituted by the mean of the pixels in the rectangle
/// [x-dx, x+dx]x[y-dy, y+dy].
/// The image is changed in-place.
void ImageBlur(Image img, int dx, int dy) { ///
  // Insert your code here!
  assert(img != NULL);
  Image imgToBlur = ImageCreate(img->width, img->height,img->maxval);   // criamos uma imagem temporaria
  for(int i=0; i<img->height; i++){   // percorremos a imagem original
    for(int j=0; j<img->width; j++){
      double soma=0;
      double count=0;
      double media=0;
 
      for (int x=(j-dx); x<=(j+dx); x++) {   // percorremos o retangulo dentro da "área afetada pelo blur"
        for (int y=(i-dy); y<=(i+dy); y++) {
          if (ImageValidPos(img,x,y)) {   // se a posição for valida:
            count++;                      // +1 para o count pois temos +1 pixel
            soma += ImageGetPixel(img,x,y);  // e na soma somamos o valor do pixel
          }
        }
      }
      if (count != 0) {   // se count for diferente de 0, calculamos a média
        media = (soma/count)+0.5;   // com 0.5 para o arredondamento às unidades
      } else {
        media = 0;  // caso seja 0 a média também é 0
      }
      // não podemos alterar a imagem original pois isso ia alterar os valores dos pixeis na altura de percorrer o retangulo da "área afetada pelo blur"
      ImageSetPixel(imgToBlur,j,i,(uint8)media);   // definimos o pixel com o valor da média, na imagem temporaria
    }
  }
  for(int i=0; i<imgToBlur->height; i++){   // percorremos a imagem temporaria
    for(int j=0; j<imgToBlur->width; j++){
      ImageSetPixel(img,j,i,ImageGetPixel(imgToBlur,j,i));   // mudamos os valores na imagem original
    }
  }
  ImageDestroy(&imgToBlur);  // destruimos a imagem temporaria
}

