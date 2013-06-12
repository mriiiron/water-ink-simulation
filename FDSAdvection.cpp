#include <cmath>
#include <ctime>
#include <cstdlib>
#include <GL\glut.h>
#include "FDSFluid.h"
#include "FDSAdvection.h"

#define ROUND(x)	int(x + 0.5)

FDSAdvectionFluid::FDSAdvectionFluid(GLint _dimension, GLint _width, GLint _height, GLint _depth,  GLint _locationX, GLint _locationY): FDSFluid(_dimension, _width, _height, _depth, _locationX, _locationY) {
	p_buffer = new GLfloat[size];  zeroField(p_buffer, size);
	vx_buffer_0 = new GLfloat[size];  zeroField(vx_buffer_0, size);
	vx_buffer_1 = new GLfloat[size];  zeroField(vx_buffer_1, size);
	vy_buffer_0 = new GLfloat[size];  zeroField(vy_buffer_0, size);
	vy_buffer_1 = new GLfloat[size];  zeroField(vy_buffer_1, size);
	ink_buffer = new GLfloat[size];  zeroField(ink_buffer, size);
	curl_buffer = new GLfloat[size];  zeroField(curl_buffer, size);
	back_adv_source = new GLint[size];  zeroFieldInt(back_adv_source, size);
	back_adv_source_fracs = new GLfloat[4 * size];  zeroField(back_adv_source_fracs, 4 * size);
	back_adv_drain_accum = new GLfloat[size];  zeroField(back_adv_drain_accum, size);
	velocity_max = 3.0f;
	velocity_advection_intensity = 0.1f;
	pressure_advection_intensity = 0.1f;
	ink_advection_intensity = 0.05f;
	ink_pressure_intensity = 1.0f;
	ink_decay_intensity = 0.995f;
	pressure_force = 0.02f;
	srand((GLuint)time(NULL));
}

FDSAdvectionFluid::~FDSAdvectionFluid() {
	if ( p_buffer != NULL ) { delete p_buffer; }
	if ( vx_buffer_0 != NULL ) { delete vx_buffer_0; }
	if ( vx_buffer_1 != NULL ) { delete vx_buffer_1; }
	if ( vy_buffer_0 != NULL ) { delete vy_buffer_0; }
	if ( vy_buffer_1 != NULL ) { delete vy_buffer_1; }
	if ( ink_buffer != NULL ) { delete ink_buffer; }
	if ( curl_buffer != NULL ) { delete curl_buffer; }
	if ( back_adv_source != NULL ) { delete back_adv_source; }
	if ( back_adv_source_fracs != NULL ) { delete back_adv_source_fracs; }
	if ( back_adv_drain_accum != NULL ) { delete back_adv_drain_accum; }
}

GLvoid FDSAdvectionFluid::reset() {
	FDSFluid::reset();
	zeroField(p_buffer, size);
	zeroField(vx_buffer_0, size);
	zeroField(vx_buffer_1, size);
	zeroField(vy_buffer_0, size);
	zeroField(vy_buffer_1, size);
	zeroField(ink_buffer, size);
	zeroField(curl_buffer, size);
	zeroFieldInt(back_adv_source, size);
	zeroField(back_adv_source_fracs, 4 * size);
	zeroField(back_adv_drain_accum, size);
	srand((GLuint)time(NULL));
}

GLvoid FDSAdvectionFluid::fluidPressure(GLint iterate_times, GLfloat intensity) {
	for (GLint iterate = 0; iterate < iterate_times; iterate++) {
		for (GLint y = 0; y < height - 1; y++) {
			for (GLint x = 0; x < width - 1; x++) {
				GLint cell = getCell(x, y);
				GLfloat dp_x = (p[cell] + ink[cell]) - (p[cell + 1] + ink[cell]);
				GLfloat dp_y = (p[cell] + ink[cell]) - (p[cell + width] + ink[cell + width]);
				GLfloat multiplier = -0.0001f * y * y + 2.0f;
				if (dp_y < 0.0f) {
					dp_y /= multiplier;
				}
				else {
					dp_y *= multiplier;
				}
				if (dp_x != 0.0f || dp_y != 0.0f) {
					vx[cell] += dp_x * intensity;
					vx[cell + 1] += dp_x * intensity;
					vy[cell] += dp_y * intensity;
					vy[cell + width] += dp_y * intensity;
				}
			}
		}
	}
}

GLvoid FDSAdvectionFluid::inkPressure(GLfloat intensity) {
	for (GLint cell = 0; cell < size; cell++) {
		if (ink[cell] > 0.0f) {
			GLfloat basicPressure;
			GLfloat inkForce = ink[cell] * intensity;
			if (inkForce <= 1.0f) {
				basicPressure = 0.0f;
			}
			else if (inkForce >= 2.0f) {
				basicPressure = 2.0f;
			}
			else {
				basicPressure = 2.0f * (inkForce - 1.0f) * (inkForce - 1.0f);
			}
			GLfloat turb = (rand() % 11 - 5) / 10.0f;
			vx[cell] += turb;
			vy[cell] -= basicPressure;
			if (cell % width > 0) {
				vx[cell - 1] -= (basicPressure * 0.7f + turb * 0.5f);
				vy[cell - 1] -= basicPressure * 0.2f;
			}
			if (cell % width < width - 1) {
				vx[cell + 1] += (basicPressure * 0.7f + turb * 0.5f);
				vy[cell + 1] -= basicPressure * 0.2f;
			}
			if (cell >= width) {
				vx[cell - width] += turb * 0.4f;
				vy[cell - width] -= ink[cell] * intensity * 0.5f;
			}
		}
	}
}

GLvoid FDSAdvectionFluid::inkDecay(GLfloat intensity) {
	for (GLint cell = 0; cell < size; cell++) {
		if (ink[cell] < 0.01f) {
			ink[cell] = 0.0f;
			continue;
		}
		ink[cell] *= intensity;
	}
}

GLvoid FDSAdvectionFluid::diffuseField(GLfloat* src, GLfloat* dst, GLint iterate_times, GLfloat center_weight) {
	GLint cell;
	GLfloat neighbor_weight = (1.0f - center_weight) / 4.0f;
	GLfloat border_weight = center_weight + neighbor_weight;
	GLfloat corner_weight = center_weight + 2.0f * neighbor_weight;
	for (GLint iterate = 0; iterate < iterate_times; iterate++) {
		for (GLint x = 1; x < width - 1; x++) {
			cell = getCell(x, 0);
			dst[cell] = border_weight * src[cell] + neighbor_weight * (src[cell - 1] + src[cell + 1] + src[cell + width]);
			cell = getCell(x, height - 1);
			dst[cell] = border_weight * src[cell] + neighbor_weight * (src[cell - 1] + src[cell + 1] + src[cell - width]);
		}
		for (GLint y = 1; y < height - 1; y++) {
			cell = getCell(0, y);
			dst[cell] = border_weight * src[cell] + neighbor_weight * (src[cell + 1] + src[cell + width] + src[cell - width]);
			cell = getCell(width - 1 ,y);
			dst[cell] = border_weight * src[cell] + neighbor_weight * (src[cell - 1] + src[cell + width] + src[cell - width]);
		}
		cell = getCell(0, 0);
		dst[cell] = corner_weight * src[cell] + neighbor_weight * (src[cell + 1] + src[cell + width]);
		cell = getCell(width - 1, 0);
		dst[cell] = corner_weight * src[cell] + neighbor_weight * (src[cell - 1] + src[cell + width]);
		cell = getCell(0, height - 1);
		dst[cell] = corner_weight * src[cell] + neighbor_weight * (src[cell + 1] + src[cell - width]);
		cell = getCell(width - 1, height - 1);
		dst[cell] = corner_weight * src[cell] + neighbor_weight * (src[cell - 1] + src[cell - width]);
		for (GLint x = 1; x < width - 1; x++) {
			for (GLint y = 1; y < height - 1; y++) {
				cell = getCell(x,y);
				dst[cell] = center_weight * src[cell] + neighbor_weight * (src[cell + 1] + src[cell - 1] + src[cell + width] + src[cell - width]);
			}
		}
	}
}

GLvoid FDSAdvectionFluid::forwardAdvection(GLfloat* src, GLfloat* dst, GLfloat intensity) {
	copyField(src, dst, size);
	if (intensity == 0.0f) return;
	for (GLint y = 0; y < height; y++) {
		for (GLint x = 0; x < width; x++) {
			GLint cell = getCell(x, y);
			if (vx[cell] != 0.0f || vy[cell] != 0.0f) {
				GLfloat dst_x = x + vx[cell] * intensity, dst_y = y + vy[cell] * intensity;
				if (dst_x < 0.0f) dst_x = 0.0f;
				else if (dst_x > width - 1.0001f) dst_x = width - 1.0001f;
				if (dst_y < 0.0f) dst_y = 0.0f;
				else if (dst_y > height - 1.0001f) dst_y = height - 1.0001f;
				GLint dst_cell = getCell((GLint)dst_x, (GLint)dst_y);
				GLclampf fx = dst_x - (int)dst_x;
				GLclampf fy = dst_y - (int)dst_y;
				GLfloat flow = src[cell];
				GLclampf w00 = (1.0f - fy) * (1.0f - fx) * flow;
				GLclampf w01 = (1.0f - fy) * fx * flow;
				GLclampf w10 = fy * (1.0f - fx) * flow;
				GLclampf w11 = fy * fx * flow;
				dst[cell] -= (w00 + w01 + w10 + w11);
				dst[dst_cell] += w00;
				dst[dst_cell + 1] += w01;
				dst[dst_cell + width] += w10;
				dst[dst_cell + width + 1] += w11;
			}
		}
	}
}

GLvoid FDSAdvectionFluid::backwardAdvection(GLfloat* src, GLfloat* dst, GLfloat intensity) {
	copyField(src, dst, size);
	if (intensity == 0.0f) return;
	for (GLint y = 0; y < height; y++) {
		for (GLint x = 0; x < width; x++) {
			GLint cell = getCell(x, y);
			if (vx[cell] != 0.0f || vy[cell] != 0.0f) {
				GLfloat src_x = x - vx[cell] * intensity, src_y = y - vy[cell] * intensity;
				if (src_x < 0.0f) src_x = 0.0f;
				else if (src_x > width - 1.0001f) src_x = width - 1.0001f;
				if (src_y < 0.0f) src_y = 0.0f;
				else if (src_y > height - 1.0001f) src_y = height - 1.0001f;
				GLint src_cell = getCell((GLint)src_x, (GLint)src_y);
				GLclampf fx = src_x - (int)src_x;
				GLclampf fy = src_y - (int)src_y;
				GLclampf w00 = (1.0f - fy) * (1.0f - fx) * src[src_cell];
				GLclampf w01 = (1.0f - fy) * fx * src[src_cell + 1];
				GLclampf w10 = fy * (1.0f - fx) * src[src_cell + width];
				GLclampf w11 = fy * fx * src[src_cell + 1 + width];
				// add to (out) dest cell
				dst[cell] += w00 + w01 + w10 + w11;
				dst[src_cell] -= w00;
				dst[src_cell + 1] -= w01;
				dst[src_cell + width] -= w10;
				dst[src_cell + 1 + width] -= w11;
			}
		}
	}
}

GLvoid FDSAdvectionFluid::backwardAdvectionUnsigned(GLfloat* src, GLfloat* dst, GLfloat intensity) {
	copyField(src, dst, size);
	if (intensity == 0.0f) return;
	for (GLint y = 0; y < height; y++) {
		for (GLint x = 0; x < width; x++) {
			GLint cell = getCell(x, y);
			if (vx[cell] != 0.0f || vy[cell] != 0.0f) {
				GLfloat src_x = x - vx[cell] * intensity, src_y = y - vy[cell] * intensity;
				if (src_x < 0.0f) src_x = 0.0f;
				else if (src_x > width - 1.0001f) src_x = width - 1.0001f;
				if (src_y < 0.0f) src_y = 0.0f;
				else if (src_y > height - 1.0001f) src_y = height - 1.0001f;
				GLint src_cell = getCell((GLint)src_x, (GLint)src_y);
				GLclampf fx = src_x - (int)src_x;
				GLclampf fy = src_y - (int)src_y;
				GLclampf w00_frac = (1.0f - fy) * (1.0f - fx);
				GLclampf w01_frac = (1.0f - fy) * fx;
				GLclampf w10_frac = fy * (1.0f - fx);
				GLclampf w11_frac = fy * fx;
				back_adv_source[cell] = src_cell;
				back_adv_source_fracs[4 * cell] = w00_frac;
				back_adv_source_fracs[4 * cell + 1] = w01_frac;
				back_adv_source_fracs[4 * cell + 2] = w10_frac;
				back_adv_source_fracs[4 * cell + 3] = w11_frac;
				back_adv_drain_accum[src_cell] += w00_frac;
				back_adv_drain_accum[src_cell + 1] += w01_frac;
				back_adv_drain_accum[src_cell + width] += w10_frac;
				back_adv_drain_accum[src_cell + 1 + width] += w11_frac;
			}
			else {
				back_adv_source[cell] = BACK_ADVECTION_NO_SRC;
			}
		}
	}
	for (GLint cell = 0; cell < size; cell++) {
		GLint src_cell = back_adv_source[cell];
		if (src_cell == BACK_ADVECTION_NO_SRC)
			continue;
		GLclampf w00_frac = back_adv_source_fracs[4 * cell];
		GLclampf w01_frac = back_adv_source_fracs[4 * cell + 1];
		GLclampf w10_frac = back_adv_source_fracs[4 * cell + 2];
		GLclampf w11_frac = back_adv_source_fracs[4 * cell + 3];
		GLclampf accum_00 = back_adv_drain_accum[src_cell];
		GLclampf accum_01 = back_adv_drain_accum[src_cell + 1];
		GLclampf accum_10 = back_adv_drain_accum[src_cell + width];
		GLclampf accum_11 = back_adv_drain_accum[src_cell + 1 + width];
		if (accum_00 < 1.0f) accum_00 = 1.0f;
		if (accum_01 < 1.0f) accum_01 = 1.0f;
		if (accum_10 < 1.0f) accum_10 = 1.0f;
		if (accum_11 < 1.0f) accum_11 = 1.0f;
		w00_frac /= accum_00;
		w01_frac /= accum_01;
		w10_frac /= accum_10;
		w11_frac /= accum_11;
		dst[cell] += w00_frac * src[src_cell] + w01_frac * src[src_cell + 1] + w10_frac * src[src_cell + width] + w11_frac * src[src_cell + 1 + width];
		dst[src_cell] -= w00_frac * src[src_cell];
		dst[src_cell + 1] -= w01_frac * src[src_cell + 1];
		dst[src_cell + width] -= w10_frac * src[src_cell + width];
		dst[src_cell + width + 1] -= w11_frac * src[src_cell + width + 1];
	}
}

GLfloat FDSAdvectionFluid::calcCurl(GLint x, GLint y) {
	GLint cell = getCell(x, y);
	GLfloat x_curl = (vx[cell - width] - vx[cell + width]) * 0.5f;
    GLfloat y_curl = (vy[cell + 1] - vy[cell - 1]) * 0.5f;
    return (x_curl + y_curl);
}

GLvoid FDSAdvectionFluid::vorticityConfinement(GLfloat intensity) {
	zeroFieldFloat(curl_buffer, size);
	GLint cell;
	for (GLint x = 1; x < width - 1; x++) {
        for (GLint y = 1; y < height - 1; y++) {
			cell = getCell(x, y);
			curl_buffer[cell] = calcCurl(x, y);
		}
	}
	copyEdge(vx, vx_buffer_0, width, height);
	copyEdge(vy, vy_buffer_0, width, height);
	for (GLint x = 1; x < width - 1; x++) {
        for (GLint y = 1; y < height - 1; y++) {
			cell = getCell(x, y);
			GLfloat lr_curl_grad = (curl_buffer[cell - 1] - curl_buffer[cell + 1]) * 0.5f;
			GLfloat ud_curl_grad = (curl_buffer[cell + width] - curl_buffer[cell - width]) * 0.5f;
			GLfloat length = (GLfloat)sqrtf(lr_curl_grad * lr_curl_grad + ud_curl_grad * ud_curl_grad) + 0.000001f;
			lr_curl_grad /= length;
			ud_curl_grad /= length;
			GLfloat v = calcCurl(x, y);
			vx_buffer_0[cell] = vx[cell] + ud_curl_grad * v * intensity;   
            vy_buffer_0[cell] = vy[cell] + lr_curl_grad * v * intensity;
		}
    }
	swapField(vx, vx_buffer_0);
	swapField(vy, vy_buffer_0);
}

GLvoid FDSAdvectionFluid::velocityFriction(GLfloat paramA, GLfloat paramB, GLfloat paramC, GLint iterate_times, GLfloat intensity) {
	for (GLint iterate = 0; iterate < iterate_times; iterate++) {
		for (GLint cell = 0; cell < size; cell++) {
			GLfloat len2 = vx[cell] * vx[cell] + vy[cell] * vy[cell];
			if (len2 < 0.0001f) continue;
			GLfloat len = sqrt(len2);
			GLfloat new_len = len - intensity * (paramA * len2 + paramB * len + paramC);
			if (new_len < 0.0f) new_len = 0.0f;
			vx[cell] = vx[cell] / len * new_len;
			vy[cell] = vy[cell] / len * new_len;
		}
	}
}

GLvoid FDSAdvectionFluid::restrictBorderVelocity() {
	GLint cell;
	for (GLint x = 0; x < width; x++) {
		cell = getCell(x, 0);
		if (vy[cell] < 0.0f) vy[cell] = 0.0f;
		cell = getCell(x, height - 1);
		if (vy[cell] > 0.0f) vy[cell] = 0.0f;
	}
	for (GLint y = 0; y < height; y++) {
		cell = getCell(0, y);
		if (vx[cell] < 0.0f) vx[cell] = 0.0f;
		cell = getCell(width - 1, y);
		if (vx[cell] > 0.0f) vx[cell] = 0.0f;
	}
}

GLvoid FDSAdvectionFluid::update() {
	
	// 预处理
	// TODO: 场平滑
	diffuseField(p, p_buffer, 1, 0.9f);
	swapField(p, p_buffer);
	diffuseField(vx, vx_buffer_0, 1, 0.7f);
	swapField(vx, vx_buffer_0);
	diffuseField(vy, vy_buffer_0, 1, 0.7f);
	swapField(vy, vy_buffer_0);
	diffuseField(ink, ink_buffer, 1, 0.9f);
	swapField(ink, ink_buffer);

	// 墨更新
	forwardAdvection(ink, ink_buffer, ink_advection_intensity);
	swapField(ink, ink_buffer);
	backwardAdvectionUnsigned(ink, ink_buffer, ink_advection_intensity);
	swapField(ink, ink_buffer);

	// 墨影响压力
	inkPressure(ink_pressure_intensity);

	// 墨渐隐
	if (ink_decay_open) {
		inkDecay(ink_decay_intensity);
	}

	// TODO: 粘滞力
	// handleViscousity();

	// 涡旋
	vorticityConfinement(0.05f);

	// 压力
	fluidPressure(1, pressure_force);

	// 规定最大速度
	velocityClamp(velocity_max);

	// 速度场平流
	forwardAdvection(vx, vx_buffer_0, velocity_advection_intensity);
	forwardAdvection(vy, vy_buffer_0, velocity_advection_intensity);
	backwardAdvection(vx_buffer_0, vx_buffer_1, velocity_advection_intensity);
	backwardAdvection(vy_buffer_0, vy_buffer_1, velocity_advection_intensity);
	swapField(vx_buffer_1, vx);
	swapField(vy_buffer_1, vy);

	// 密度场平流
	forwardAdvection(p, p_buffer, pressure_advection_intensity);
	swapField(p, p_buffer);
	backwardAdvectionUnsigned(p, p_buffer, pressure_advection_intensity);
	swapField(p, p_buffer);

	// 速度场减淡
	velocityFriction(0.8f, 0.03f, 0.001f, 1, 0.01f);
	
	// 限制边界向外的速度
	restrictBorderVelocity();

}

GLvoid FDSAdvectionFluid::switchInkDecay() {
	ink_decay_open = !ink_decay_open;
}