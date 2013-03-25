#include <cmath>
#include <GL\glut.h>
#include "FDSFluid.h"
#include "FDSAdvectionFluid.h"

#define ROUND(x)	int(x + 0.5)

FDSAdvectionFluid::FDSAdvectionFluid(GLint _baseX, GLint _baseY, GLint _width, GLint _height) {
	baseX = _baseX;  baseY = _baseY;
	width = _width;  height = _height;
	stamizing = 0.5f;  interactRange = 1;  ratioP2V = 0.5f;
	GLint size = width * height;
	density = new GLfloat[size];
	vx = new GLfloat[size];
	vy = new GLfloat[size];
	for (GLint cell = 0; cell < size; cell++) {
		density[cell] = density_0[cell] = 1.0f;
		vx[cell] = vy[cell] = 0.0f;
	}
}

GLvoid FDSAdvectionFluid::copyField(GLfloat* src, GLfloat* dst) {
	for (GLint cell = 0; cell < width * height; cell++)	{
		dst[cell] = src[cell];
	}
}

GLvoid FDSAdvectionFluid::flowExec(GLint x, GLint y, GLfloat sx, GLfloat sy, GLfloat dx, GLfloat dy, GLfloat inflow, GLfloat outflow) {
	int sxInt = ROUND(sx), syInt = ROUND(sy), dxInt = ROUND(dx), dyInt = ROUND(dy);
	if (abs(sxInt - sx) < 0.001) {
		if (abs(syInt - sy) < 0.001f) {
			if (inflow > 0.0f) {
				density[sxInt][syInt] -= inflow;
			}
			if (outflow > 0.0f) {
				density[dxInt][dyInt] += outflow;
			}	
		}
		else {
			GLint syFloor = floor(sy), dyFloor = floor(dy);
			GLfloat wBottom = sy - syFloor, wTop = syFloor + 1.0f - sy;
			if (inflow > 0.0f) {
				density[sxInt][syFloor] -= (inflow * wTop);  density[sxInt][syFloor + 1] -= (inflow * wBottom);
			}
			if (outflow > 0.0f) {
				density[dxInt][dyFloor] += (outflow * wBottom);  density[dxInt][dyFloor + 1] += (outflow * wTop);
			}
		}
	}
	else if (abs(syInt - sy) < 0.001f) {
		GLint sxFloor = floor(sx), dxFloor = floor(dx);
		GLfloat wLeft = sx - sxFloor, wRight = sxFloor + 1.0f - sx;
		if (inflow > 0.0f) {
			density[sxFloor][syInt] -= (inflow * wRight);  density[sxFloor + 1][syInt] -= (inflow * wLeft);
		}
		if (outflow > 0.0f) {
			density[dxFloor][dyInt] += (outflow * wLeft);  density[dxFloor + 1][dyInt] += (outflow * wRight);
		}
	}
	else {
		GLint sxFloor = floor(sx), syFloor = floor(sy), dxFloor = floor(dx), dyFloor = floor(dy);
		GLfloat wRightTop = (sxFloor + 1.0f - sx) * (syFloor + 1.0f - sy), wLeftTop = (sx - sxFloor) * (syFloor + 1.0f - sy);
		GLfloat wRightBottom = (sxFloor + 1.0f - sx) * (sy - syFloor), wLeftBottom = (sx - sxFloor) * (sy - syFloor);
		if (inflow > 0.0f) {
			density[sxFloor][syFloor] -= (inflow * wRightTop);  density[sxFloor + 1][syFloor] -= (inflow * wLeftTop);
			density[sxFloor][syFloor + 1] -= (inflow * wRightBottom);  density[sxFloor + 1][syFloor + 1] -= (inflow * wLeftBottom);
		}
		if (outflow > 0.0f) {
			density[dxFloor][dyFloor] += (outflow * wLeftBottom);  density[dxFloor + 1][dyFloor] += (outflow * wRightBottom);
			density[dxFloor][dyFloor + 1] += (outflow * wLeftTop);  density[dxFloor + 1][dyFloor + 1] += (outflow * wRightTop);
		}
	}
	density[x][y] += (inflow - outflow);
}


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
	commonAdvection(density, density_0, 1.0f);
	//stamAdvection();
	//intergratedAdvection();

	GLfloat sx, sy;
	for (GLint x = 0; x < width; x++) {
		for (GLint y = 0; y < height; y++) {
			GLint cell = getCell(x, y);
			if (vx[cell] != 0.0f || vy[cell] != 0.0f) {
				GLfloat flow = sqrtf(vx[cell] * vx[cell] + vy[cell] * vy[cell]);		
				sx = x - vx[cell];  sy = y - vy[cell];
			}
		}
	}

	// Step 2. 密度场产生压力，更新速度场
	GLfloat dp;
	if (interactRange == 1) {
		for (GLint x = 0; x < width; x++) {
			for (GLint y = 0; y < height - 1; y++) {
				dp = density[y][x] - density[j + 1][x];
				if (abs(dp) > 0.001f) {
					vx[y][x] += ratioP2V * dp;  vx[j + 1][x] += ratioP2V * dp;
				}
				dp = density[x][y] - density[x][j + 1];
				if (abs(dp) > 0.001f) {
					vy[x][y] += ratioP2V * dp;  vy[x][j + 1] += ratioP2V * dp;
				}
			}
		}
	}
	else if (interactRange == 2) {

	}
	// else if ..

}

GLvoid FDSAdvectionFluid::stimulate() {
	density[25][46] *= 2;
}