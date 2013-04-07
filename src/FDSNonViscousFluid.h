#pragma once


class FDSNonViscousFluid: public FDSFluid {

public:

	FDSNonViscousFluid(GLint dimension, GLint width, GLint height, GLint depth);
	~FDSNonViscousFluid();

	GLvoid stimulate() { p[getCell(40, 25)] *= 2.0f; }

	GLvoid init();
	GLvoid update();
	GLvoid render();

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

};