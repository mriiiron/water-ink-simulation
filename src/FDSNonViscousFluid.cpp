#include <GL\glut.h>
#include "FDSFluid.h"
#include "FDSNonViscousFluid.h"

FDSNonViscousFluid::FDSNonViscousFluid(GLint dimension, GLint width, GLint height, GLint depth): FDSFluid(dimension, width, height, depth) {
	p_estimating = new GLfloat[size];  zeroField(p_estimating, size);
	p_correction = new GLfloat[size];  zeroField(p_correction, size);
	dp_forward = new GLfloat[size];  zeroField(dp_forward, size);
	dvx_forward = new GLfloat[size];  zeroField(dvx_forward, size);
	dvy_forward = new GLfloat[size];  zeroField(dvy_forward, size);
	vx_estimating = new GLfloat[size];  zeroField(vx_estimating, size);
	vx_correction = new GLfloat[size];  zeroField(vx_correction, size);
	vy_estimating = new GLfloat[size];  zeroField(vy_estimating, size);
	vy_correction = new GLfloat[size];  zeroField(vy_correction, size);
}

FDSNonViscousFluid::~FDSNonViscousFluid() {
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

GLvoid FDSNonViscousFluid::init() {
	for (GLint i = 0; i < size; i++) {
		p[i] = 1.0f;
	}
}

GLvoid FDSNonViscousFluid::update() {

	GLint cell;
	GLfloat dp_xPart, dvx_xPart, dvy_xPart, dp_yPart, dvx_yPart, dvy_yPart;
	GLfloat dp_backward_estimating, dvx_backward_estimating, dvy_backward_estimating;

	// 更新密度场: MacCormack 法, 预估步
	for (GLint x = 1; x < width - 1; x++) {
		for (GLint y = 1; y < height - 1; y++) {
			cell = getCell(x, y);
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
	for (GLint x = 1; x < width - 1; x++) {
		for (GLint y = 0; y < height - 1; y++) {
			cell = getCell(x, y);
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

	// 将计算结果应用到目标场
	copyField(p_correction, p, size);
	copyField(vx_correction, vx, size);
	copyField(vy_correction, vy, size);

}

GLvoid FDSNonViscousFluid::render() {

}