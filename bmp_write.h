#ifndef BMP_WRITE
#define BMP_WRITE

// Red, green, blue color value (range: 0-1)
struct RGB
{
	float R;
	float G;
	float B;
};

// Hue, saturation, lightness color value (range: 0-1)
struct HSL
{
	float H;
	float S;
	float L;
};

void write_bmp_header(FILE* fp, int h, int w);
void write_color(struct RGB* color, FILE* fp);
void toRGB(struct HSL* in, struct RGB* out);

#endif