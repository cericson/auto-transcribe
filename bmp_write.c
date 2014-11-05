#include <stdio.h>
#include <math.h>
#include "bmp_write.h"
#include "file_rw.h"

#define DEBUG 0

// Writes out the bitmap header
void write_bmp_header(FILE* fp, int h, int w)
{
	// Calculate file size given 54 byte header and 3 bytes per pixel
	// plus row padding
	int size=54+3*h*w;
	size+=(w%4)*h;
	fputc(66,fp);			// Magic numbers for .bmp
	fputc(77,fp);
	write_int32(size,fp);	// Size of .bmp
	write_int16(0,fp);		// Application specific info
	write_int16(0,fp);
	write_int32(54,fp);		// Offset of pixel data
	write_int32(40,fp);		// Remaining header size
	write_int32(w,fp);		// Width info
	write_int32(h,fp);		// Height info
	write_int16(1,fp);		// Number of color planes
	write_int16(24,fp);		// Bits/pixel
	write_int32(0,fp);		// Bl_RBG:related to compression
	write_int32(size-54,fp);// Size of raw .bmp data
	write_int32(2835,fp);	// Horizontal resolution: default
	write_int32(2835,fp);	// Vertical resolution: default
	write_int32(0,fp);		// Number of palette colors
	write_int32(0,fp);		// Number of important colors

}

// Writes a 24 bit RGB color value to file
// Assumes RGB values are floats ranging from 0 to 1
void write_color(struct RGB* color, FILE* fp)
{
	fputc((int)(255*color->B+.5),fp);
	fputc((int)(255*color->G+.5),fp);
	fputc((int)(255*color->R+.5),fp);
}

// Converts HSL to RGB: algorithm from http://en.wikipedia.org/wiki/HSL_and_HSV
// Assumes both RGB and HSL values are floats ranging from 0 to 1
void toRGB(struct HSL* in, struct RGB* out)
{
	float c, h_p, x, m;

#if DEBUG
	printf("(h,s,l)=(%.3f,%.3f,%.3f)\n",in->H,in->S,in->L);
#endif

	c = (1-fabs(2*in->L - 1))*in->S;
	h_p = in->H*6;
	x = c*(1 - fabs(fmod(h_p,2) - 1));

#if DEBUG
	printf("c = %.3f, h_p = %.3f, x = %.3f\n",c,h_p,x);
#endif

	if ((0 <= h_p) && (h_p < 1))
	{
		out->R = c;
		out->G = x;
		out->B = 0;
	}
	else if ((1 <= h_p) && (h_p < 2))
	{
		out->R = x;
		out->G = c;
		out->B = 0;
	}
	else if ((2 <= h_p) && (h_p < 3))
	{
		out->R = 0;
		out->G = c;
		out->B = x;
	}
	else if ((3 <= h_p) && (h_p < 4))
	{
		out->R = 0;
		out->G = x;
		out->B = c;
	}
	else if ((4 <= h_p) && (h_p < 5))
	{
		out->R = x;
		out->G = 0;
		out->B = c;
	}
	else if ((5 <= h_p) && (h_p < 6))
	{
		out->R = c;
		out->G = 0;
		out->B = x;
	}
	else
	{
		out->R = 0;
		out->G = 0;
		out->B = 0;
	}
	
	m = in->L - c/2;

	out->R = out->R + m;
	out->G = out->G + m;
	out->B = out->B + m;

#if DEBUG
	printf("(r,g,b)=(%.3f,%.3f,%.3f)\n",out->R,out->G,out->B);
#endif

}

