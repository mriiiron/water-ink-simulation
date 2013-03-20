#include <cmath>
#include <GL\glut.h>
#include "FDSFluid.h"

#define ROUND(x)	int(x + 0.5)

FDSFluid::FDSFluid() {
	stamizing = 1.0f;  interactRange = 1;  ratioP2V = 0.5f;
	baseX = -50;  baseY = -50;
	for (GLint i = 0; i < GRID_SIZE; i++) {
		for (GLint j = 0; j < GRID_SIZE; j++) {
			density[i][j] = 1.0f;
			vx[i][j] = vy[i][j] = 0.0f;
		}
	}
}

GLvoid FDSFluid::flowExec(GLint x, GLint y, GLfloat sx, GLfloat sy, GLfloat dx, GLfloat dy, GLfloat inflow, GLfloat outflow) {
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

GLvoid FDSFluid::update() {
	
	// Step 1. 速度场更新密度场
	GLfloat sx, sy, dx, dy;
	GLfloat inflow, outflow;
	for (GLint i = 0; i < GRID_SIZE; i++) {
		for (GLint j = 0; j < GRID_SIZE; j++) {
			GLfloat flow = sqrt(vx[i][j] * vx[i][j] + vy[i][j] * vy[i][j]);		
			if (flow > 0.0f) {
				outflow = flow * stamizing;  inflow = flow - outflow;
				sx = i - vx[i][j];  sy = j - vy[i][j];  dx = i + vx[i][j];  dy = j + vy[i][j];
				flowExec(i, j, sx, sy, dx, dy, inflow, outflow);
			}
		}
	}

	// Step 2. 密度场产生压力，更新速度场
	GLfloat dp;
	if (interactRange == 1) {
		for (GLint i = 0; i < GRID_SIZE; i++) {
			for (GLint j = 0; j < GRID_SIZE - 1; j++) {
				dp = density[j][i] - density[j + 1][i];
				if (abs(dp) > 0.001f) {
					vx[j][i] += ratioP2V * dp;  vx[j + 1][i] += ratioP2V * dp;
				}
				dp = density[i][j] - density[i][j + 1];
				if (abs(dp) > 0.001f) {
					vy[i][j] += ratioP2V * dp;  vy[i][j + 1] += ratioP2V * dp;
				}
			}
		}
	}
	else if (interactRange == 2) {

	}
	// else if ..

}

GLvoid FDSFluid::stimulate() {
	density[25][46] *= 2;
}