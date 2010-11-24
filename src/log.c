/* Copyright Swann Perarnau 2009
*
*   contact : Swann.Perarnau@imag.fr
*
* This software is a computer program whose purpose is to help the
* random generation of graph structures and adding various properties
* on those structures.
*
* This software is governed by the CeCILL  license under French law and
* abiding by the rules of distribution of free software.  You can  use,
* modify and/ or redistribute the software under the terms of the CeCILL
* license as circulated by CEA, CNRS and INRIA at the following URL
* "http://www.cecill.info".
*
* As a counterpart to the access to the source code and  rights to copy,
* modify and redistribute granted by the license, users are provided only
* with a limited warranty  and the software's author,  the holder of the
* economic rights,  and the successive licensors  have only  limited
* liability.
*
* In this respect, the user's attention is drawn to the risks associated
* with loading,  using,  modifying and/or developing or reproducing the
* software by the user in light of its specific status of free software,
* that may mean  that it is complicated to manipulate,  and  that  also
* therefore means  that it is reserved for developers  and  experienced
* professionals having in-depth computer knowledge. Users are therefore
* encouraged to load and test the software's suitability as regards their
* requirements in conditions enabling the security of their systems and/or
* data to be ensured and,  more generally, to use and operate it in the
* same conditions as regards security.
*
* The fact that you are presently reading this means that you have had
* knowledge of the CeCILL license and that you accept its terms.
*/
#include "log.h"
#include <stdarg.h>

static enum log_level log_filter;
static FILE *logfd = NULL;
static const char *namespace;

static const char *log_string[] = {
	"quiet",
	"error",
	"warning",
	"normal",
	"info",
	"debug",
};

int log_init(FILE *f,const char *nm)
{
	if(f == NULL)
		return 1;
	if(nm == NULL)
		return 1;
	logfd = f;
	log_filter = LOG_NORMAL;
	namespace = nm;
	return 0;
}

enum log_level log_filter_above(enum log_level l)
{
	enum log_level r = log_filter;
	log_filter = l;
	return r;
}

void log_msg(enum log_level l,const char *file, unsigned int line, const char *format, ...)
{
	va_list ap;
	if(l <= log_filter) {
		if(log_filter == LOG_DEBUG)
			fprintf(logfd,"%s:\t%s:\t%s:\t%u:\t",namespace,log_string[l],file,line);
		else
			fprintf(logfd,"%s:\t%s:\t",namespace,log_string[l]);
		va_start(ap,format);
		vfprintf(logfd,format,ap);
		va_end(ap);
	}
}
