/*
 * color-drawing.c - demonstrate drawing of pixels, lines, arcs, etc, using
 *		      different foreground colors, in a window.
 */

/*
 * The first thing we need to do is include the grpc classes and objects
 * along with identifying what we are using from them. Putting this later 
 * in teh includes causes an error
 */

//#include <grpc/support/log.h>
//#include <grpcpp/grpcpp.h>
//
//using grpc::Channel;
//using grpc::ClientAsyncResponseReader;
//using grpc::ClientContext;
//using grpc::CompletionQueue;
//using grpc::Status;

//#include "../cpp/generated/health.pb.h"
#include "../cpp/generated/mandelbrot.pb.h"

#include <X11/Xlib.h>

#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>

#include <stdio.h>
#include <stdlib.h>		/* getenv(), etc. */
#include <unistd.h>		/* sleep(), etc.  */

using namespace std; 

char grpcMode[64] ;

typedef struct {
  char colorName[128];
  XColor color;
} myColorElement;

typedef struct {
  double Real;
  double Imaginary;
}   complexPoint;

typedef struct {
  int x;
  int y;
}   windowPoint;

complexPoint complexAdd(complexPoint m1, complexPoint m2) {
  complexPoint result ;
  result.Real = m1.Real + m2.Real;
  result.Imaginary = m1.Imaginary + m2.Imaginary;
  return result;
}

//
// multiplying to imaginary number is nothing more than a matrix multiplication.
// The difference comes into play with the 'i' variables are multiplied together.
// In normal matrix math this would result in i^2. However since i represents the 
// square root of -1 you use this knowledge to reduce the problem.
//
//         5, 3i
//      X 10, 2i
//       =======
//        10i, 6i^2
//     50,30i
//     ==========
//       50,40i, -6 OR 44, 40i

complexPoint complexMultiply(complexPoint m1, complexPoint m2){
  complexPoint result ;

  result.Real = m1.Real * m2.Real + (-1 * m1.Imaginary * m2.Imaginary);
  result.Imaginary = m1.Real * m2.Imaginary + m1.Imaginary * m2.Real;
  return result;
}

int calcFractal(complexPoint c){
  int i;
  long result = -1l;
  double x1, y1;

  complexPoint z;

  z.Real = 0.0;
  z.Imaginary = 0.0;

  for (i=0; i <= 1000; i++){
    result = i;
    z = complexMultiply(z, z);
    z = complexAdd(z, c);
    //printf("%f;   %f\n", z.Real, z.Imaginary);

    if ((pow (z.Real, 2) + pow(z.Imaginary, 2)) > 4)
    {
      //printf("Stopping after: %ld\n", result);
      break;
    }

  }
  return result;
}

/*
 * function: create_simple_window. Creates a window with a white background
 *           in the given size.
 * input:    display, size of the window (in pixels), and location of the window
 *           (in pixels).
 * output:   the window's ID.
 * notes:    window is created with a black border, 2 pixels wide.
 *           the window is automatically mapped after its creation.
 */
Window
create_simple_window(Display* display, int width, int height, int x, int y)
{
  int screen_num = DefaultScreen(display);
  int win_border_width = 2;
  Window win;

  /* create a simple window, as a direct child of the screen's */
  /* root window. Use the screen's black and white colors as   */
  /* the foreground and background colors of the window,       */
  /* respectively. Place the new window's top-left corner at   */
  /* the given 'x,y' coordinates.                              */
  win = XCreateSimpleWindow(display, RootWindow(display, screen_num),
                            x, y, width, height, win_border_width,
                            BlackPixel(display, screen_num),
                            WhitePixel(display, screen_num));

  /* make the window actually appear on the screen. */
  XMapWindow(display, win);

  /* flush all pending requests to the X server. */
  XFlush(display);

  return win;
}

GC
create_gc(Display* display, Window win, int reverse_video)
{
  GC gc;				/* handle of newly created GC.  */
  unsigned long valuemask = 0;		/* which values in 'values' to  */
					/* check when creating the GC.  */
  XGCValues values;			/* initial values for the GC.   */
  unsigned int line_width = 2;		/* line width for the GC.       */
  int line_style = LineSolid;		/* style for lines drawing and  */
  int cap_style = CapButt;		/* style of the line's edje and */
  int join_style = JoinBevel;		/*  joined lines.		*/
  int screen_num = DefaultScreen(display);

  gc = XCreateGC(display, win, valuemask, &values);
  if (gc < 0) {
	  fprintf(stderr, "XCreateGC: \n");
  }

  /* allocate foreground and background colors for this GC. */
  if (reverse_video) {
    XSetForeground(display, gc, WhitePixel(display, screen_num));
    XSetBackground(display, gc, BlackPixel(display, screen_num));
  }
  else {
    XSetForeground(display, gc, BlackPixel(display, screen_num));
    XSetBackground(display, gc, WhitePixel(display, screen_num));
  }

  /* define the style of lines that will be drawn using this GC. */
  XSetLineAttributes(display, gc,
                     line_width, line_style, cap_style, join_style);

  /* define the fill style for the GC. to be 'solid filling'. */
  XSetFillStyle(display, gc, FillSolid);

  return gc;
}

int main(int argc, char* argv[])
{
  Display* display;		/* pointer to X Display structure.           */
  int screen_num;		/* number of screen to place the window on.  */
  Window win;			/* pointer to the newly created window.      */
  unsigned int display_width,
               display_height;	/* height and width of the X display.        */
  unsigned int width, height;	/* height and width for the new window.      */
  char *display_name = getenv("DISPLAY");  /* address of the X display.      */
  GC gc;			/* GC (graphics context) used for drawing    */
				/*  in our window.			     */
  Colormap screen_colormap;     /* color map to use for allocating colors.   */
  XColor red, brown, blue, yellow, green;
				/* used for allocation of the given color    */
				/* map entries.                              */

  int myMaxColor = 0;

  strcpy(grpcMode, "grpc-unary");

  Status rc;			/* return status of various Xlib functions.  */

  printf("Starting %s\n", display_name);
  /* open connection with the X server. */
  display = XOpenDisplay(display_name);
  if (display == NULL) {
    fprintf(stderr, "%s: cannot connect to X server '%s'\n",
            argv[0], display_name);
    exit(1);
  }

  printf("Opened Display\n");

  /* get the geometry of the default screen for our display. */
  screen_num = DefaultScreen(display);
  display_width = DisplayWidth(display, screen_num);
  display_height = DisplayHeight(display, screen_num);

  /* make the new window occupy 1/9 of the screen's size. */
  //width = (display_width / 2);
  //height = (display_height / 2);
  
  
  width = 1280;
  height = 960;
  
  printf("window width - '%d'; height - '%d'\n", width, height);

  /* create a simple window, as a direct child of the screen's   */
  /* root window. Use the screen's white color as the background */
  /* color of the window. Place the new window's top-left corner */
  /* at the given 'x,y' coordinates.                             */
  win = create_simple_window(display, width, height, 0, 0);

  /* allocate a new GC (graphics context) for drawing in the window. */
  gc = create_gc(display, win, 0);
  XSync(display, False);

  /* get access to the screen's color map. */
  screen_colormap = DefaultColormap(display, DefaultScreen(display));

  /* allocate the set of colors we will want to use for the drawing. */

  myColorElement myColorList[2048] ;
  
  std::ifstream rgbFP ("/etc/X11//rgb.txt");
  if (rgbFP.is_open())
  {
    string inLine;
    while( getline (rgbFP, inLine)){
        int r, g, b;
        char myLine[128];
        myColorElement theColor;
        int len = inLine.size();

        std::copy(inLine.begin(), inLine.end(), myLine); 
        myLine[inLine.length()] = 0;
        strcpy(theColor.colorName, &myLine[13]);
        //printf("%s:   %s\n", myLine, theColor.colorName);
        rc = XAllocNamedColor(display, screen_colormap, theColor.colorName, &theColor.color, &theColor.color);
        if (rc !=  0) {
          //printf("%d\n", myMaxColor);
          myColorList[myMaxColor] = theColor;
          myMaxColor++;
        }
      }
  }
  else
  {
    printf("Could not read the system rbg file\n");
    exit(-1);
  }
  rgbFP.close();

  printf("Found: %d\n", myMaxColor);
 
  {
    int x, y, z;
    unsigned long thisColor;

    struct {
      complexPoint upperLeft;
      complexPoint lowerRight;
    } fractalRegion;

    struct {
      windowPoint upperLeft;
      windowPoint lowerRight;
    } windowRegion ;

    complexPoint fractalPoint ;
    
    fractalRegion.upperLeft.Real = -2.0 ;
    fractalRegion.upperLeft.Imaginary = 1.25 ;
    fractalRegion.lowerRight.Real = 0.5 ;
    fractalRegion.lowerRight.Imaginary = -1.25 ;

    windowRegion.upperLeft.x = 0;
    windowRegion.upperLeft.y = 0;
    windowRegion.lowerRight.x = width;
    windowRegion.lowerRight.y = height;

    double complexHorizontalStride, complexVerticleStride;

    complexHorizontalStride = (fractalRegion.lowerRight.Real - fractalRegion.upperLeft.Real) / (double)(windowRegion.lowerRight.x - windowRegion.upperLeft.x);
    complexVerticleStride = (fractalRegion.lowerRight.Imaginary - fractalRegion.upperLeft.Imaginary) / (double)(windowRegion.lowerRight.y - windowRegion.upperLeft.y);

    printf("%f  -  %f\n", complexHorizontalStride, complexVerticleStride);

    for (y=0; y < height; y++) {
      fractalPoint.Real = fractalRegion.upperLeft.Real;
      fractalPoint.Imaginary = fractalRegion.upperLeft.Imaginary + (y * complexVerticleStride) ;

      if (strcmp("grpc-resultstream", grpcMode) == 1) {

      } else {
        for (x=0; x < width; x++) {
          fractalPoint.Real = fractalRegion.upperLeft.Real + (x * complexHorizontalStride);
          //printf("%f - %f\n", fractalPoint.Real, fractalPoint.Imaginary)
          if (strcmp("grpc-uanry", grpcMode) == 1) {
            
          } else {
            thisColor = calcFractal(fractalPoint);
          }
          //printf("Calculated: %f - %f : %ld\n", fractalPoint.Real, fractalPoint.Imaginary, thisColor);
          if (thisColor > myMaxColor) {
            thisColor = myMaxColor;
          }

          XSetForeground(display, gc, myColorList[thisColor].color.pixel);
          XDrawPoint(display, win, gc, x, y);
          //XFlush(display);
        }
      }
      XFlush(display);
    }
  }

  /* flush all pending requests to the X server. */
  XFlush(display);

  /* make a delay for a short period. */
  sleep(30);

  /* close the connection to the X server. */
  XCloseDisplay(display);
  return(0);
}
