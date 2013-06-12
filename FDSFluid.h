#pragma once

#define NULL							0


class FDSFluid {

public:

	static const GLint GRID_SIZE			= 4;

	static const GLint FLUID_2D				= 0;
	static const GLint FLUID_3D				= 1;

	static const GLint RENDER_DENSITY		= 0;
	static const GLint RENDER_VELOCITY		= 1;
	static const GLint RENDER_INK			= 2;

	static const GLint MAX_CELL_INK			= 2;

	FDSFluid(GLint _dimension, GLint _width, GLint _height, GLint _depth, GLint _locationX, GLint _locationY);
	virtual ~FDSFluid();

	GLint getWidth() { return width; }
	GLint getHeight() { return height; }
	GLint getSize() { return size; }
	GLint getLocationX() { return locationX; }
	GLint getLocationY() { return locationY; }
	GLint getCell(GLint x, GLint y) { return x + y * width; }
	GLint getCell(GLint x, GLint y, GLint z); // unimplemented
	GLfloat getP(GLint x, GLint y) { return p[getCell(x, y)]; }
	GLboolean isInkDecay() { return ink_decay_open; }

	virtual GLvoid update() = 0;
	virtual GLvoid render(GLint field);
	virtual GLvoid reset();

	GLvoid inkDropAt(GLint x, GLint y, GLfloat amount);
	GLvoid inkLine(GLint x0, GLint y0, GLint x1, GLint y1, GLfloat amount);

protected:

	GLint dimension;
	GLint width, height, depth, size;
	GLint locationX, locationY;

	GLboolean ink_decay_open;

	GLfloat* p;
	GLfloat* vx;
	GLfloat* vy;
	GLfloat* vz;
	GLfloat* ink;

	GLfloat* fluidDensityVertices;
	GLfloat* fluidDensityColors;
	GLfloat* fluidVelocityVertices;
	GLfloat* fluidVelocityColors;
	GLfloat* inkVertices;
	GLfloat* inkColors;

	GLvoid velocityClamp(GLfloat maxVelocity);
	GLvoid inkPointAt(GLint x, GLint y, GLfloat amount);

	GLvoid copyField(GLfloat* src, GLfloat* dst, GLint size) {
		for (GLint i = 0; i < size; i++) {
			dst[i] = src[i];
		}
	}

	GLvoid copyEdge(GLfloat* src, GLfloat* dst, GLint width, GLint height) {
		for (GLint i = 0; i < width; i++) {
			dst[getCell(i, 0)] = src[getCell(i, 0)];
			dst[getCell(i, height - 1)] = src[getCell(i, height - 1)];
		}
		for (GLint i = 1; i < height - 1; i++) {
			dst[getCell(0, i)] = src[getCell(0, i)];
			dst[getCell(width - 1, i)] = src[getCell(width - 1, i)];
		}
	}
 
	GLvoid zeroField(GLfloat* src, GLint size) {
		for (GLint i = 0; i < size; i++) {
			src[i] = 0.0f;
		}
	}

	GLvoid swapField(GLfloat* &f0, GLfloat* &f1) {
		GLfloat* t = f0;
		f0 = f1;
		f1 = t;
	}

	GLvoid setFieldValue(GLfloat* src, GLfloat value, GLint size) {
		for (GLint i = 0; i < size; i++) {
			src[i] = value;
		}
	}

};