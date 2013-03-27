#include <GL\glut.h>
#include "FDSFluid.h"
#include "FDSNonViscousFluid.h"

FDSNonViscousFluid::FDSNonViscousFluid(GLint dimension, GLint width, GLint height, GLint depth): FDSFluid(dimension, width, height, depth) {
	p_estimating = new GLfloat[size];
	p_correction = new GLfloat[size];
	vx_estimating = new GLfloat[size];
	vx_correction = new GLfloat[size];
	vy_estimating = new GLfloat[size];
	vy_correction = new GLfloat[size];
}

FDSNonViscousFluid::~FDSNonViscousFluid() {
	delete p_estimating;
	delete p_correction;
	delete vx_estimating;
	delete vx_correction;
	delete vy_estimating;
	delete vy_correction;
}

GLvoid FDSNonViscousFluid::update() {

	// 更新密度场: MacCormack 法, 预估步
	for (GLint x = 0; x < width; x++) {
		for (GLint y = 0; y < height; y++) {
			GLint cell = getCell(x, y);
			GLfloat p_xPart, p_yPart, vx_xPart, vx_yPart, vy_xPart, vy_yPart;
			if (x == width - 1) {
				p_xPart = p[cell] * (vx[cell] - vx[cell - 1]) + vx[cell] * (p[cell] - p[cell - 1]);
				vx_xPart = vx[cell] * (vx[cell] - vx[cell - 1]) + (p[cell] - p[cell - 1]) / p[cell];
				vy_xPart = vx[cell] * (vy[cell] - vy[cell - 1]);
			}
			else {
				p_xPart = p[cell] * (vx[cell + 1] - vx[cell]) + vx[cell] * (p[cell + 1] - p[cell]);
				vx_xPart = vx[cell] * (vx[cell + 1] - vx[cell]) + (p[cell + 1] - p[cell]) / p[cell];
				vy_xPart = vx[cell] * (vy[cell + 1] - vy[cell]);
			}
			if (y == height - 1) {
				p_yPart = p[cell] * (vy[cell] - vy[cell - width]) + vy[cell] * (p[cell] - p[cell - width]);
				vx_yPart = vy[cell] * (vx[cell] - vx[cell - width]);
				vy_yPart = vy[cell] * (vy[cell] - vy[cell - width]) + (p[cell] - p[cell - width]) / p[cell];
			}
			else {
				p_yPart = p[cell] * (vy[cell + width] - vy[cell]) + vy[cell] * (p[cell + width] - p[cell]);
				vx_yPart = vy[cell] * (vx[cell + width] - vx[cell]);
				vy_yPart = vy[cell] * (vy[cell + width] - vy[cell]) + (p[cell + width] - p[cell]) / p[cell];
			}
			p_estimating[cell] = p[cell] - (p_xPart + p_yPart);
			vx_estimating[cell] = vx[cell] - (vx_xPart + vx_yPart);
			vy_estimating[cell] = vy[cell] - (vy_xPart + vy_yPart);
		}
	}

	// 校正步
	for (GLint x = 0; x < width; x++) {
		for (GLint y = 0; y < height; y++) {
			GLint cell = getCell(x, y);

		}
	}

}

GLvoid FDSNonViscousFluid::render() {

}