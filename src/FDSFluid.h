#pragma once

#define NULL							0


class FDSFluid {

public:

	static const GLint FLUID_2D = 0;
	static const GLint FLUID_3D = 1;

	FDSFluid(GLint _dimension = FLUID_2D, GLint _width = 100, GLint _height = 100, GLint _depth = 1);
	virtual ~FDSFluid();

	GLint getWidth() { return width; }
	GLint getHeight() { return height; }
	GLint getSize() { return size; }
	GLint getLocationX() { return locationX; }
	GLint getLocationY() { return locationY; }
	GLint getCell(GLint x, GLint y) { return x + width * y; }
	GLint getCell(GLint x, GLint y, GLint z); // unimplemented
	GLfloat getP(GLint x, GLint y) { return p[getCell(x, y)]; }
	GLvoid setLocation(GLint _locationX, GLint _locationY) { locationX = _locationX;  locationY = _locationY; }

	virtual GLvoid update() = 0;
	virtual GLvoid render() = 0;

protected:

	GLint dimension;
	GLint width, height, depth, size;
	GLint locationX, locationY;

	GLfloat* p;
	GLfloat* vx;
	GLfloat* vy;
	GLfloat* vz;

	GLvoid copyField(GLfloat* src, GLfloat* dst, GLint size);
	GLvoid zeroField(GLfloat* src, GLint size);

};