#pragma once

#define NULL							0


class FDSFluid {

public:

	static const GLint FLUID_2D = 0;
	static const GLint FLUID_3D = 1;

	FDSFluid(GLint dimension = FLUID_2D, GLint width = 100, GLint height = 100, GLint depth = 1);
	virtual ~FDSFluid();

	GLint getCell(GLint x, GLint y) { return x + width * y; }
	GLint getCell(GLint x, GLint y, GLint z);
	virtual GLvoid update() = 0;
	virtual GLvoid render() = 0;

protected:

	GLint width, height, depth, size;
	GLint locationX, locationY;

	GLfloat* p;
	GLfloat* vx;
	GLfloat* vy;
	GLfloat* vz;

	GLvoid copyField(GLfloat* src, GLfloat* dst);

};