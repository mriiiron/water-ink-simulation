#pragma once

class FDSAdvectionFluid: public FDSFluid {

public:

	static const GLint ADVECTION_COMMON			= 0;
	static const GLint ADVECTION_STAM			= 1;
	static const GLint ADVECTION_INTERGRATED	= 2;

	GLfloat stamizing;
	GLint interactRange;
	GLfloat ratioP2V;

	GLint baseX, baseY;
	GLint width, height;

	FDSAdvectionFluid(GLint _baseX, GLint _baseY, GLint _width, GLint _height);
	~FDSAdvectionFluid();
	GLint getCell(GLint x, GLint y) { return x + width * y; }
	GLfloat getDensity(GLint x, GLint y) { return density[getCell(x, y)]; }
	GLvoid update();
	GLvoid paint();
	GLvoid stimulate();

private:

	GLfloat* density;
	GLfloat* density_0;
	GLfloat* vx;
	GLfloat* vy;

	// GLvoid flowExec(GLint x, GLint y, GLfloat sx, GLfloat sy, GLfloat dx, GLfloat dy, GLfloat inflow, GLfloat outflow);

	GLvoid copyField(GLfloat* src, GLfloat* dst);

	GLvoid commonAdvection(GLfloat* src, GLfloat* dst, GLfloat intensity);
	GLvoid stamAdvection(GLfloat* src, GLfloat* dst, GLfloat intensity);
	GLvoid intergratedAdvection(GLfloat* src, GLfloat* dst, GLfloat stamizing);

};