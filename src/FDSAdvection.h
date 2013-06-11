#pragma once

class FDSAdvectionFluid: public FDSFluid {

public:

	FDSAdvectionFluid(GLint _dimension, GLint _width, GLint _height, GLint _depth, GLint _locationX, GLint _locationY);
	~FDSAdvectionFluid();

	GLfloat velocity_max;
	GLfloat velocity_advection_intensity;
	GLfloat pressure_force;
	GLfloat pressure_advection_intensity;
	GLfloat ink_advection_intensity;
	GLfloat ink_pressure_intensity;
	GLfloat ink_decay_intensity;

	GLvoid update();
	GLvoid switchInkDecay();

private:

	static const GLint BACK_ADVECTION_NO_SRC			= -1;

	GLfloat* p_buffer;
	GLfloat* vx_buffer_0;
	GLfloat* vx_buffer_1;
	GLfloat* vy_buffer_0;
	GLfloat* vy_buffer_1;
	GLfloat* ink_buffer;
	GLfloat* curl_buffer;
	GLint* back_adv_source;
	GLfloat* back_adv_source_fracs;
	GLfloat* back_adv_drain_accum;

	GLvoid forwardAdvection(GLfloat* src, GLfloat* dst, GLfloat intensity);
	GLvoid backwardAdvection(GLfloat* src, GLfloat* dst, GLfloat intensity);
	GLvoid backwardAdvectionUnsigned(GLfloat* src, GLfloat* dst, GLfloat intensity);

	GLvoid fluidPressure(GLint iterate_times, GLfloat intensity);
	GLvoid inkPressure(GLfloat intensity);
	GLvoid inkDecay(GLfloat intensity);
	GLvoid diffuseField(GLfloat* src, GLfloat* dst, GLint iterate_times, GLfloat center_weight);

	GLfloat calcCurl(GLint x, GLint y);
	GLvoid vorticityConfinement(GLfloat intensity);

	GLvoid velocityFriction(GLfloat paramA, GLfloat paramB, GLfloat paramC, GLint iterate_times, GLfloat intensity);
	GLvoid restrictBorderVelocity();

	GLvoid border_inverseX(GLint cell) {
		vx[cell] = -vx[cell];
	}

	GLvoid border_inverseY(GLint cell) {
		vy[cell] = -vy[cell];
	}

	GLvoid border_inverseBoth(GLint cell) {
		vx[cell] = -vx[cell];
		vy[cell] = -vy[cell];
	}

	GLvoid zeroFieldInt(GLint* src, GLint size) {
		for (GLint i = 0; i < size; i++) {
			src[i] = 0;
		}
	}

	GLvoid zeroFieldFloat(GLfloat* src, GLint size) {
		for (GLint i = 0; i < size; i++) {
			src[i] = 0;
		}
	}

};