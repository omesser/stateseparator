#ifndef N_LOGGER_H
#define N_LOGGER_H

#ifdef __NRUNONSERVER__

// On the server, use standard output
#include <iostream>
using std::cout;

#else // run native

// During development, output to log file
#include <fstream>
using std::ofstream;

#endif // __NRUNONSERVER__

#include <sstream>
#include <string>

using std::bad_alloc;
using std::stringstream;
using std::ostream;
using std::string;
using std::endl;

class logger;


/*****************************************************************************
 *                               OUTPUT MACROS                               *
 *****************************************************************************/
#define NDATA(msg)		logger::getInstance()->_data(msg);
#define NERROR(msg)		logger::getInstance()->_error(msg);
#define NWARNING(msg)	logger::getInstance()->_warning(msg);

#ifdef __NLOGALL__
# define NLOG(msg)			logger::getInstance()->_log(msg);
# define NLOGNUM(str,num)	logger::getInstance()->_log(str, num);
#else // __NLOGALL__ not defined
# define NLOG(msg)
# define NLOGNUM(str,num)
#endif // __NLOGALL__


/*****************************************************************************
 *                                Class logger                               *
 *****************************************************************************/
class logger {
public:
	inline static logger* getInstance();
	inline void _data(const stringstream& msg);
	inline void _data(const string& msg);
	inline void _error(const stringstream& msg);
	inline void _error(const string& msg);
	inline void _warning(const string& msg);

#ifdef __NLOGALL__
	inline void _log(const stringstream& msg);
	inline void _log(const string& msg);
	inline void _log(const string& str, int num);
	inline void _log(const string& str, unsigned int num);
	inline void _log(const string& str, double num);
#endif // __NLOGALL__

private:
	// The singleton instance
	static logger* _theInstance;

	// Data members
#ifndef __NRUNONSERVER__
	// Development - output to log file
	ofstream _fileOutput;
#endif // __NRUNONSERVER__
	ostream& _output;

	// Constrtuctor
	inline logger();

#ifndef __NRUNONSERVER__
	// Development - need to close the output file
	inline ~logger();
#endif // __NRUNONSERVER__
};


/*****************************************************************************
 *                          PUBLIC MEMBER FUNCTIONS                          *
 *****************************************************************************/
logger* logger::getInstance() {
	if (_theInstance == NULL) {
		_theInstance = new logger;
	}
	return _theInstance;
}

void logger::_data(const stringstream& msg) {
	_data(msg.str());
}

void logger::_data(const string& msg) {
	_output << msg << endl;
	_output.flush();
}

void logger::_error(const stringstream& msg) {
	_error(msg.str());
}

void logger::_error(const string& msg) {
	_output << "ERROR: " << msg << endl;
	_output.flush();
}

void logger::_warning(const string& msg) {
	_output << "WARNING: " << msg << endl;
	_output.flush();
}

#ifdef __NLOGALL__

void logger::_log(const stringstream& msg) {
	_log(msg.str());
}

void logger::_log(const string& msg) {
	_output << "INFO: " << msg << endl;
	_output.flush();
}

void logger::_log(const string& str, int num) {
	_output << "INFO: " << str << " " << num << endl;
	_output.flush();
}

void logger::_log(const string& str, unsigned int num) {
	_output << "INFO: " << str << " " << num << endl;
	_output.flush();
}

void logger::_log(const string& str, double num) {
	_output << "INFO: " << str << " " << num << endl;
	_output.flush();
}

#endif // __NLOGALL__


/*****************************************************************************
 *                          PRIVATE MEMBER FUNCTIONS                         *
 *****************************************************************************/
#ifdef __NRUNONSERVER__

logger::logger() : _output(cout) {}

#else // run native

logger::logger() : _fileOutput("NSeparator.log"), _output(_fileOutput) {}

logger::~logger() {
	_fileOutput.close();
}

#endif // __NRUNONSERVER__

#endif // N_LOGGER_H
