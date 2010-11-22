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
#ifndef LOG_H
#define LOG_H 1
#include <stdio.h>

/* log functions for ggen:
 *	multiple log levels : ERROR,WARNING,NORMAL,INFO,DEBUG
 *	configurable output : stdout or file
 * Warning: this code does not call any abort() or anything when
 * logging, you are responsible for cleanup. And the library side
 * of ggen should never abort anyway.
 */

enum log_level {
	LOG_QUIET	= 0,
	LOG_ERROR	= 1,
	LOG_WARNING	= 2,
	LOG_NORMAL	= 3,
	LOG_INFO	= 4,
	LOG_DEBUG	= 5
};

/* initialize logging, must be called first,
 * use file descriptor as log output.
 * defaults log filter on LOG_NORMAL.
 * return 0 on success */
int log_init(FILE* f, const char *nm);

/* set log filter, only levels equals or lower than filter are logged
 * return previous filter level
 */
enum log_level log_filter_above(enum log_level l);

/* logs the msg with level l. Message follows printf syntax */
void log_msg(enum log_level l,const char *file, unsigned int line, const char *format, ...);

#define error(...)	log_msg(LOG_ERROR,__FILE__,__LINE__, __VA_ARGS__)
#define warning(...)	log_msg(LOG_WARNING,__FILE__,__LINE__, __VA_ARGS__)
#define normal(...)	log_msg(LOG_NORMAL,__FILE__,__LINE__, __VA_ARGS__)
#define info(...)	log_msg(LOG_INFO,__FILE__,__LINE__, __VA_ARGS__)
#define debug(...)	log_msg(LOG_DEBUG,__FILE__,__LINE__, __VA_ARGS__)

#endif // LOG_H
