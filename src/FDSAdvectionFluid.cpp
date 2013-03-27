#include <cmath>
#include <GL\glut.h>
#include "FDSFluid.h"
#include "FDSAdvectionFluid.h"

#define ROUND(x)	int(x + 0.5)

FDSAdvectionFluid::FDSAdvectionFluid(GLint dimension, GLint width, GLint height, GLint depth): FDSFluid(dimension, width, height, depth) {
	stamizing = 0.5f; 
	interactRange = 1; 
	ratioP2V = 0.5f;
}

GLvoid FDSAdvectionFluid::copyField(GLfloat* src, GLfloat* dst) {
	for (GLint cell = 0; cell < width * height; cell++)	{
		dst[cell] = src[cell];
	}
}

//GLvoid FDSAdvectionFluid::flowExec(GLint x, GLint y, GLfloat sx, GLfloat sy, GLfloat dx, GLfloat dy, GLfloat inflow, GLfloat outflow) {
//	int sxInt = ROUND(sx), syInt = ROUND(sy), dxInt = ROUND(dx), dyInt = ROUND(dy);
//	if (abs(sxInt - sx) < 0.001) {
//		if (abs(syInt - sy) < 0.001f) {
//			if (inflow > 0.0f) {
//				p[sxInt][syInt] -= inflow;
//			}
//			if (outflow > 0.0f) {
//				p[dxInt][dyInt] += outflow;
//			}	
//		}
//		else {
//			GLint syFloor = floor(sy), dyFloor = floor(dy);
//			GLfloat wBottom = sy - syFloor, wTop = syFloor + 1.0f - sy;
//			if (inflow > 0.0f) {
//				p[sxInt][syFloor] -= (inflow * wTop);  p[sxInt][syFloor + 1] -= (inflow * wBottom);
//			}
//			if (outflow > 0.0f) {
//				p[dxInt][dyFloor] += (outflow * wBottom);  p[dxInt][dyFloor + 1] += (outflow * wTop);
//			}
//		}
//	}
//	else if (abs(syInt - sy) < 0.001f) {
//		GLint sxFloor = floor(sx), dxFloor = floor(dx);
//		GLfloat wLeft = sx - sxFloor, wRight = sxFloor + 1.0f - sx;
//		if (inflow > 0.0f) {
//			p[sxFloor][syInt] -= (inflow * wRight);  p[sxFloor + 1][syInt] -= (inflow * wLeft);
//		}
//		if (outflow > 0.0f) {
//			p[dxFloor][dyInt] += (outflow * wLeft);  p[dxFloor + 1][dyInt] += (outflow * wRight);
//		}
//	}
//	else {
//		GLint sxFloor = floor(sx), syFloor = floor(sy), dxFloor = floor(dx), dyFloor = floor(dy);
//		GLfloat wRightTop = (sxFloor + 1.0f - sx) * (syFloor + 1.0f - sy), wLeftTop = (sx - sxFloor) * (syFloor + 1.0f - sy);
//		GLfloat wRightBottom = (sxFloor + 1.0f - sx) * (sy - syFloor), wLeftBottom = (sx - sxFloor) * (sy - syFloor);
//		if (inflow > 0.0f) {
//			p[sxFloor][syFloor] -= (inflow * wRightTop);  p[sxFloor + 1][syFloor] -= (inflow * wLeftTop);
//			p[sxFloor][syFloor + 1] -= (inflow * wRightBottom);  p[sxFloor + 1][syFloor + 1] -= (inflow * wLeftBottom);
//		}
//		if (outflow > 0.0f) {
//			p[dxFloor][dyFloor] += (outflow * wLeftBottom);  p[dxFloor + 1][dyFloor] += (outflow * wRightBottom);
//			p[dxFloor][dyFloor + 1] += (outflow * wLeftTop);  p[dxFloor + 1][dyFloor + 1] += (outflow * wRightTop);
//		}
//	}
//	p[x][y] += (inflow - outflow);
//}

GLvoid FDSAdvectionFluid::commonAdvection(GLfloat* src, GLfloat* dst, GLfloat intensity) {
	GLfloat dstx, dsty;
	GLclampf fx, fy, w00, w10, w01, w11;
	for (GLint x = 0; x < width; x++) {
		for (GLint y = 0; y < height; y++) {
			GLint cell = getCell(x, y);
			GLfloat flow = intensity * sqrtf(vx[cell] * vx[cell] + vy[cell] * vy[cell]);
			dstx = x + vx[cell];  dsty = y + vy[cell];
			// Handle border situation here
			fx = dstx - int(dstx);  fy = dsty - int(dsty);
			w00 = (1.0f - fx) * (1.0f - fy);  w10 = fx * (1.0f - fy);
			w01 = (1.0f - fx) * fy;  w11 = fx * fy;


		}

	}

}

GLvoid FDSAdvectionFluid::stamAdvection(GLfloat* src, GLfloat* dst, GLfloat intensity) {

}

GLvoid FDSAdvectionFluid::intergratedAdvection(GLfloat* src, GLfloat* dst, GLfloat stamizing) {

}

GLvoid FDSAdvectionFluid::update() {
	
	// Step 1. 速度场更新密度场
	GLfloat* p_0 = new GLfloat[size];
	copyField(p, p_0);
	commonAdvection(p, p_0, 1.0f);
	//stamAdvection();
	//intergratedAdvection();

	//GLfloat sx, sy;
	//for (GLint x = 0; x < width; x++) {
	//	for (GLint y = 0; y < height; y++) {
	//		GLint cell = getCell(x, y);
	//		if (vx[cell] != 0.0f || vy[cell] != 0.0f) {
	//			GLfloat flow = sqrtf(vx[cell] * vx[cell] + vy[cell] * vy[cell]);		
	//			sx = x - vx[cell];  sy = y - vy[cell];
	//		}
	//	}
	//}

	// Step 2. 密度场产生压力，更新速度场
	//GLfloat dp;
	//if (interactRange == 1) {
	//	for (GLint x = 0; x < width; x++) {
	//		for (GLint y = 0; y < height - 1; y++) {
	//			dp = p[y][x] - p[j + 1][x];
	//			if (abs(dp) > 0.001f) {
	//				vx[y][x] += ratioP2V * dp;  vx[j + 1][x] += ratioP2V * dp;
	//			}
	//			dp = p[x][y] - p[x][j + 1];
	//			if (abs(dp) > 0.001f) {
	//				vy[x][y] += ratioP2V * dp;  vy[x][j + 1] += ratioP2V * dp;
	//			}
	//		}
	//	}
	//}
	//else if (interactRange == 2) {

	//}
	// else if ..

}

GLvoid FDSAdvectionFluid::render() {

}

GLvoid FDSAdvectionFluid::stimulate() {
	p[getCell(25, 46)] *= 2;
}