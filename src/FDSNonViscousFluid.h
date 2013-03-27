#pragma once


class FDSNonViscousFluid: public FDSFluid {

public:

	FDSNonViscousFluid(GLint dimension, GLint width, GLint height, GLint depth);
	~FDSNonViscousFluid();

	GLvoid update();
	GLvoid render();

private:

	GLfloat* p_estimating;
	GLfloat* p_correction;
	GLfloat* vx_estimating;
	GLfloat* vx_correction;
	GLfloat* vy_estimating;
	GLfloat* vy_correction;

};