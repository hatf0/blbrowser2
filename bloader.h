#ifndef BLOADER_H
#define BLOADER_H

/*
 * BLoader framework
 * Used to communicate with and through BLoader
 */

#define STR2(str) #str
#define STR(str) STR2(str)

#define BLOADER_VERSION 3
#define BLOADER_VERSION_MAJOR 0
#define BLOADER_VERSION_MINOR 0
#define BLOADER_VERSION_REVISION 3
#define BLOADER_VERSION_STRING ("v" STR(BLOADER_VERSION_MAJOR) "." STR(BLOADER_VERSION_MINOR) "." STR(BLOADER_VERSION_REVISION))

/*
 * Macros to make it easier to specify what level the text printed out to console should be.
 * Info would be a blue, warning would be a dark grey/light grey, and error would be red.
 */

#define BLOADER_CONSOLE_INFO "\x05"
#define BLOADER_CONSOLE_ERROR "\x03"
#define BLOADER_CONSOLE_WARN "\x04" 

/*
 * Handling the exporting of the library
 */
#ifdef BLOADER_DLL
#define BLOADER_EXPORT __declspec(dllexport)
#define BLOADER_IMPORT
#else
#define BLOADER_EXPORT __declspec(dllimport)
#define BLOADER_IMPORT __declspec(dllexport)
#endif // BLOADER_DLL

#ifdef __cplusplus
#define BL_EXTERN extern "C"
extern "C" {
#else
#define BL_EXTERN
#endif // __cplusplus

/*
 * Structure declarations
 */

// List of possible errors
enum blerror
{
	BLOADER_OK,
	BLOADER_NO_MODULE = -1,
	BLOADER_NO_INIT = -2,
	BLOADER_NO_DEINIT = -3,
	BLOADER_FAIL_INIT = -4,
	BLOADER_FAIL_DEINIT = -5,
	BL_OK = 0,
	BL_INVALID_POINTER,
	BL_INVALID_LIBRARY,
	BL_INVALID_VERSION,
	BL_MISSING_INITIALIZE,
	BL_MISSING_INFO,
	BL_LIBRARY_INIT_ERROR,
	BL_LIBRARY_NO_NAME
};

// A module informations
typedef struct BLmodule blmodule;
typedef struct BLinfo blinfo;

// Callback for console functions
typedef void (*bl_callback_void)(void * object, int argc, const char * argv[]);
typedef bool (*bl_callback_bool)(void * object, int argc, const char * argv[]);
typedef int (*bl_callback_int)(void * object, int argc, const char * argv[]);
typedef float (*bl_callback_float)(void * object, int argc, const char * argv[]);
typedef const char * (*bl_callback_string)(void * object, int argc, const char * argv[]);

/*
 * General functions
 */

// Get version for this library when compiled
BLOADER_EXPORT int bloader_version();
BLOADER_EXPORT const char* bloader_versionString();

BLOADER_EXPORT const char * bloader_getError(int errorcode);

BLOADER_EXPORT int bloader_load(const char * name);
BLOADER_EXPORT int bloader_unload(const char * name);

/*
 * Module handling
 */

BLOADER_EXPORT int bloader_module_count();
BLOADER_EXPORT blmodule * bloader_module_fromIndex(int i);
BLOADER_EXPORT blmodule * bloader_module_fromName(const char * name);
BLOADER_EXPORT int bloader_module_exist(const char * name);
BLOADER_EXPORT int bloader_module_loaded(const blmodule * module);
BLOADER_EXPORT const blinfo * const bloader_module_info(const blmodule * module);

/*
 * Module communication
 */

//int bloader_com_send(const blmodule * module, void * data, int size);
//int bloader_com_receive(const blmodule * module, void ** data, int * size);
//int bloader_com_poll(const blmodule * module, void ** data, int * size);

/*
 * BL console features
 */

BLOADER_EXPORT void bloader_consolefunction_void(blmodule * module, const char * nameSpace, const char * name,
	bl_callback_void, const char * usage, int minArgs, int maxArgs);
BLOADER_EXPORT void bloader_consolefunction_bool(blmodule * module, const char * nameSpace, const char * name,
	bl_callback_bool, const char * usage, int minArgs, int maxArgs);
BLOADER_EXPORT void bloader_consolefunction_int(blmodule * module, const char * nameSpace, const char * name,
	bl_callback_int, const char * usage, int minArgs, int maxArgs);
BLOADER_EXPORT void bloader_consolefunction_float(blmodule * module, const char * nameSpace, const char * name,
	bl_callback_float, const char * usage, int minArgs, int maxArgs);
BLOADER_EXPORT void bloader_consolefunction_string(blmodule * module, const char * nameSpace, const char * name,
	bl_callback_string, const char * usage, int minArgs, int maxArgs);

BLOADER_EXPORT void bloader_consolevariable_bool(blmodule * module, const char * name, int * var);
BLOADER_EXPORT void bloader_consolevariable_int(blmodule * module, const char * name, int * var);
BLOADER_EXPORT void bloader_consolevariable_float(blmodule * module, const char * name, float * var);
BLOADER_EXPORT void bloader_consolevariable_string(blmodule * module, const char * name, char * var);

BLOADER_EXPORT const char * bloader_getVariable(const char * name);
BLOADER_EXPORT void bloader_setVariable(const char * name, const char * value);

BLOADER_EXPORT int bloader_printf(const char * format, ...);
BLOADER_EXPORT int bloader_printf_info(const char * format, ...);
BLOADER_EXPORT int bloader_printf_warn(const char * format, ...);
BLOADER_EXPORT int bloader_printf_error(const char * format, ...);

/*
 * Interface for other libraries
 */

BLOADER_EXPORT void * bloader_symbol(const blmodule * module, const char * func);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // BLOADER_H
