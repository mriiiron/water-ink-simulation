#pragma once

class FDSFluid {

public:

	const static GLint CELL_SIZE = 1;
	const static GLint GRID_SIZE = 100;

	GLfloat stamizing;
	GLint interactRange;
	GLfloat ratioP2V;

	GLint baseX, baseY;

	FDSFluid();
	GLfloat getDensity(GLint i, GLint j) { return density[i][j]; }
	GLvoid update();
	GLvoid paint();
	GLvoid stimulate();

private:

	GLfloat density[GRID_SIZE][GRID_SIZE];
	GLfloat vx[GRID_SIZE][GRID_SIZE], vy[GRID_SIZE][GRID_SIZE];

	GLint restricted(int n) { if (n >= GRID_SIZE) return (GRID_SIZE - 1);  if (n < 0) return 0;  return n; }
	GLvoid flowExec(GLint x, GLint y, GLfloat sx, GLfloat sy, GLfloat dx, GLfloat dy, GLfloat inflow, GLfloat outflow);

};