#include <Cg/cgGL.h>
#include "cg-lib.h"

void mygl_CPUprocessing_init(void);
void mygl_GPUprocessing_init(int ImageWidth,int ImageHeight);
void display(void);
void reshape(int w, int h);
void motion(int x, int y);
void keyboard(unsigned char key, int x, int y);
void readframe();
void get_pixel(GLubyte *val, GLubyte *buff, int width, int i, int j );
void set_pixel(GLubyte *val, GLubyte *buff, int width, int i, int j );
