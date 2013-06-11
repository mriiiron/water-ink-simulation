#pragma once


class FDSEulerFluid: public FDSFluid {

public:

	FDSEulerFluid(GLint _dimension, GLint _width, GLint _height, GLint _depth, GLint _locationX, GLint _locationY);
	~FDSEulerFluid();

	GLvoid update();

private:

	GLfloat* p_estimating;
	GLfloat* p_correction;
	GLfloat* dp_forward;
	GLfloat* dvx_forward;
	GLfloat* dvy_forward;
	GLfloat* vx_estimating;
	GLfloat* vx_correction;
	GLfloat* vy_estimating;
	GLfloat* vy_correction;

	GLclampf viscousity;

	GLvoid border_inverseX(GLint cell) {
		p_correction[cell] = p_estimating[cell] = p[cell];
		vx_correction[cell] = vx_estimating[cell] = -vx[cell];
		vy_correction[cell] = vy_estimating[cell] = vy[cell];
	}

	GLvoid border_inverseY(GLint cell) {
		p_correction[cell] = p_estimating[cell] = p[cell];
		vx_correction[cell] = vx_estimating[cell] = vx[cell];
		vy_correction[cell] = vy_estimating[cell] = -vy[cell];
	}

	GLvoid border_inverseBoth(GLint cell) {
		p_correction[cell] = p_estimating[cell] = p[cell];
		vx_correction[cell] = vx_estimating[cell] = -vx[cell];
		vy_correction[cell] = vy_estimating[cell] = -vy[cell];
	}

};