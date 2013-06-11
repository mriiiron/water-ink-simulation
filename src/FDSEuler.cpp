#include <iostream>
#include <GL\glut.h>
#include "FDSFluid.h"
#include "FDSEuler.h"

using namespace std;

FDSEulerFluid::FDSEulerFluid(GLint _dimension, GLint _width, GLint _height, GLint _depth, GLint _locationX, GLint _locationY): FDSFluid(_dimension, _width, _height, _depth, _locationX, _locationY) {
	p_estimating = new GLfloat[size];  zeroField(p_estimating, size);
	p_correction = new GLfloat[size];  zeroField(p_correction, size);
	dp_forward = new GLfloat[size];  zeroField(dp_forward, size);
	dvx_forward = new GLfloat[size];  zeroField(dvx_forward, size);
	dvy_forward = new GLfloat[size];  zeroField(dvy_forward, size);
	vx_estimating = new GLfloat[size];  zeroField(vx_estimating, size);
	vx_correction = new GLfloat[size];  zeroField(vx_correction, size);
	vy_estimating = new GLfloat[size];  zeroField(vy_estimating, size);
	vy_correction = new GLfloat[size];  zeroField(vy_correction, size);
	viscousity = 0.5f;
}

FDSEulerFluid::~FDSEulerFluid() {
	delete p_estimating;
	delete p_correction;
	delete dp_forward;
	delete dvx_forward;
	delete dvy_forward;
	delete vx_estimating;
	delete vx_correction;
	delete vy_estimating;
	delete vy_correction;
}

GLvoid FDSEulerFluid::update() {

	GLint cell;

	// 先苟且处理边缘：保持网格边缘密度不变，碰撞速度反向
	for (GLint x = 1; x < width - 1; x++) {
		border_inverseY(getCell(x, 0));
		border_inverseY(getCell(x, height - 1));
	}
	for (GLint y = 1; y < height - 1; y++) {
		border_inverseX(getCell(0, y));
		border_inverseX(getCell(width - 1, y));
	}

	// 再苟且处理四个角：反转速度，密度不变
	border_inverseBoth(getCell(0, 0));
	border_inverseBoth(getCell(0, height - 1));
	border_inverseBoth(getCell(width - 1, 0));
	border_inverseBoth(getCell(width - 1, height - 1));

	GLfloat dp_xPart, dvx_xPart, dvy_xPart, dp_yPart, dvx_yPart, dvy_yPart;
	GLfloat dp_backward_estimating, dvx_backward_estimating, dvy_backward_estimating;

	// 更新密度场: MacCormack 法, 预估步
	for (GLint y = 1; y < height - 1; y++) {
		for (GLint x = 1; x < width - 1; x++) {
			cell = getCell(x, y);
			if (x == 2 && y == 42) {
				cout << "fuck" << endl;
			}
			dp_xPart = p[cell] * (vx[cell + 1] - vx[cell]) + vx[cell] * (p[cell + 1] - p[cell]);
			dp_yPart = p[cell] * (vy[cell + width] - vy[cell]) + vy[cell] * (p[cell + width] - p[cell]);
			dvx_xPart = vx[cell] * (vx[cell + 1] - vx[cell]) + (p[cell + 1] - p[cell]) / p[cell];
			dvx_yPart = vy[cell] * (vx[cell + width] - vx[cell]);
			dvy_xPart = vx[cell] * (vy[cell + 1] - vy[cell]);
			dvy_yPart = vy[cell] * (vy[cell + width] - vy[cell]) + (p[cell + width] - p[cell]) / p[cell];
			dp_forward[cell] = -(dp_xPart + dp_yPart);
			dvx_forward[cell] = -(dvx_xPart + dvx_yPart);
			dvy_forward[cell] = -(dvy_xPart + dvy_yPart);
			p_estimating[cell] = p[cell] + dp_forward[cell];
			vx_estimating[cell] = vx[cell] + dvx_forward[cell];
			vy_estimating[cell] = vy[cell] + dvy_forward[cell];
		}
	}

	// 校正步
	for (GLint y = 1; y < height - 1; y++) {
		for (GLint x = 1; x < width - 1; x++) {
			cell = getCell(x, y);
			if (x == 2 && y == 42) {
				cout << "fuck" << endl;
			}
			dp_xPart = p_estimating[cell] * (vx_estimating[cell] - vx_estimating[cell - 1]) + vx_estimating[cell] * (p_estimating[cell] - p_estimating[cell - 1]);
			dp_yPart = p_estimating[cell] * (vy_estimating[cell] - vy_estimating[cell - width]) + vy_estimating[cell] * (p_estimating[cell] - p_estimating[cell - width]);
			dvx_xPart = vx_estimating[cell] * (vx_estimating[cell] - vx_estimating[cell - 1]) + (p_estimating[cell] - p_estimating[cell - 1]) / p_estimating[cell];
			dvx_yPart = vy_estimating[cell] * (vx_estimating[cell] - vx_estimating[cell - width]);
			dvy_xPart = vx_estimating[cell] * (vy_estimating[cell] - vy_estimating[cell - 1]);
			dvy_yPart = vy_estimating[cell] * (vy_estimating[cell] - vy_estimating[cell - width]) + (p_estimating[cell] - p_estimating[cell - width]) / p_estimating[cell];
			dp_backward_estimating = -(dp_xPart + dp_yPart);
			dvx_backward_estimating = -(dvx_xPart + dvx_yPart);
			dvy_backward_estimating = -(dvy_xPart + dvy_yPart);
			p_correction[cell] = p[cell] + 0.5f * (dp_forward[cell] + dp_backward_estimating);
			vx_correction[cell] = vx[cell] + 0.5f * (dvx_forward[cell] + dvx_backward_estimating);
			vy_correction[cell] = vy[cell] + 0.5f * (dvy_forward[cell] + dvy_backward_estimating);
		}
	}

	GLint dvx, dvy;

	// 处理粘性
	for (GLint y = 1; y < height; y++) {
		for (GLint x = 0; x < width; x++) {
			cell = getCell(x, y);
			dvx = 0.5f * (vx[cell] - vx[cell - width]) * viscousity;
			vx_correction[cell] -= dvx;
			vx_correction[cell - width] += dvx;
		}
	}
	for (GLint x = 1; x < width; x++) {
		for (GLint y = 0; y < height; y++) {
			cell = getCell(x, y);
			dvy = 0.5f * (vy[cell] - vy[cell - 1]) * viscousity;
			vy_correction[cell] -= dvy;
			vy_correction[cell - 1] += dvy;
		}
	}
	
	// 将计算结果应用到目标场
	swapField(p_correction, p);
	swapField(vx_correction, vx);
	swapField(vy_correction, vy);

}