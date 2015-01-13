/*
 * rasterplot.h
 *
 *  Created on: 2 Dec 2014
 *      Author: alan
 */

#ifndef RASTERPLOT_H_
#define RASTERPLOT_H_
#include "../utilities/SocketQueuer.h"
#include "../glut_framework/GlutFramework.h"
#include <map>

using namespace glutFramework;


class RasterPlot : public GlutFramework {
public:
	RasterPlot(int , char **, SocketQueuer *, std::map<int, char*>, std::map<int, int>);
	void printgl(float, float, void *, char*, ...);
	void printglstroke(float, float, float, float, char*, ...);
	void display(float);
	void reshape(int, int);
	virtual ~RasterPlot();

private:
	void safelyshut(void);
};

#endif /* RASTERPLOT_H_ */
