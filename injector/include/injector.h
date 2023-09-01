#ifndef INJECTOR_H
#define INJECTOR_H

#include <windows.h>
typedef DWORD injector_pid_t;

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

#define INJERR_SUCCESS 0               /* linux, windows, macos */
#define INJERR_OTHER -1                /* linux, windows, macos */
#define INJERR_NO_MEMORY -2            /* linux, windows, macos */
#define INJERR_NO_PROCESS -3           /* linux, windows, macos */
#define INJERR_NO_LIBRARY -4           /* linux */
#define INJERR_NO_FUNCTION -4          /* linux */
#define INJERR_ERROR_IN_TARGET -5      /* linux, windows, macos */
#define INJERR_FILE_NOT_FOUND -6       /* linux, windows, macos */
#define INJERR_INVALID_MEMORY_AREA -7  /* linux, macos */
#define INJERR_PERMISSION -8           /* linux, windows, macos */
#define INJERR_UNSUPPORTED_TARGET -9   /* linux, windows, macos */
#define INJERR_INVALID_ELF_FORMAT -10  /* linux */
#define INJERR_WAIT_TRACEE -11         /* linux */
#define INJERR_FUNCTION_MISSING -12    /* linux, windows, macos */

typedef struct injector injector_t;

/*!
 * \brief Attach to the specified process.
 * \param[out]  injector the address where the newly created injector handle will be stored
 * \param[in]   pid      the process id to be attached
 * \return               zero on success. Otherwise, error code
 */
int injector_attach(injector_t **injector, injector_pid_t pid);

/*!
 * \brief Detach from the attached process and destroy the specified handle.
 * \param[in]   injector the injector handle to destroy
 * \return               zero on success. Otherwise, error code
 */
int injector_detach(injector_t *injector);

/*!
 * \brief Inject the specified shared library into the target process.
 * \param[in]   injector the injector handle specifying the target process
 * \param[in]   path     the path name of the shared library
 * \param[out]  handle   the address where the newly created module handle will be stored
 * \return               zero on success. Otherwise, error code
 *
 * Note on Linux:
 * This calls functions inside of the target process interrupted by \c ptrace().
 * If the target process is interrupted while holding a non-reentrant lock and
 * injector calls a function requiring the same lock, the process stops forever.
 * If the lock type is reentrant, the status guarded by the lock may become inconsistent.
 * As far as I checked, \c dlopen() internally calls \c malloc() requiring non-reentrant
 * locks. \c dlopen() also uses a reentrant lock to guard information about loaded files.
 */
int injector_inject(injector_t *injector, const char *path, void **handle);

/*!
 * \brief Uninject the shared library specified by \c handle.
 * \param[in]   injector the injector handle specifying the target process
 * \param[in]   handle   the module handle created by \c injector_inject
 * \return               zero on success. Otherwise, error code
 * \remarks This fearute isn't supported for musl-libc processes.
 *     See [Functional differences from glibc](https://wiki.musl-libc.org/functional-differences-from-glibc.html#Unloading_libraries).
 */
int injector_uninject(injector_t *injector, void *handle);

#if defined(INJECTOR_DOC) || defined(__linux__) || defined(__APPLE__)
/*!
 * \brief Call the specified function taking no arguments in the target process (Linux and macOS only)
 * \param[in]   injector the injector handle specifying the target process
 * \param[in]   handle   the module handle created by \c injector_inject or special-handles such as \c RTLD_DEFAULT
 * \param[in]   name     the function name
 *
 * The \c handle and \c name arguments are passed to \c dlsym ([Linux](https://man7.org/linux/man-pages/man3/dlvsym.3.html), [macOS](https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man3/dlsym.3.html)) and then the return value of \c dlsym is called without arguments in the target process.
 *
 * This is same with the combination of injector_remote_func_addr() and injector_remote_call() without extra arguments.
 *
 * \note
 *   (Linux only)
 *   If the function in the target process internally calls non-[async-signal-safe]((https://man7.org/linux/man-pages/man7/signal-safety.7.html))
 *   functions, it may stop the target process or cause unexpected behaviour.
 * \sa injector_remote_func_addr(), injector_remote_call(), injector_remote_vcall()
 */
int injector_call(injector_t *injector, void *handle, const char* name);
#endif

/*!
 * \brief Get the message of the last error.
 * \remarks The message is updated only when \c injector functions return non-zero.
 */
const char *injector_error(void);

#if defined(INJECTOR_DOC) || defined(__linux__) || defined(_WIN32)
#include <stdarg.h>
#include <stdint.h>

/*!
 * \brief Get the function address in the target process (Linux only)
 * \param[in]   injector      the injector handle specifying the target process
 * \param[in]   handle        the module handle created by \c injector_inject or special-handles such as \c RTLD_DEFAULT
 * \param[in]   name          the function name
 * \param[out]  func_addr_out the address where the function address in the target process will be stored
 * \return                    zero on success. Otherwise, error code
 *
 * \b Example
 * \code
 * // Inject libfoo.so and then call foo_func(1, 2, 3) in it.
 * void *handle;
 * if (injector_inject(injector, "libfoo.so", &handle) != 0) {
 *    return;
 * }
 * size_t func_addr;
 * if (injector_remote_func_addr(injector, handle, "foo_func", &func_addr) != 0) {
 *    return;
 * }
 * long retval;
 * if (injector_remote_call(injector, &retval, func_addr, 1, 2, 3) != 0) {
 *    return;
 * }
 * printf("The return value of foo_func(1, 2, 3) is %ld.\n", retval);
 * \endcode
 */
int injector_remote_func_addr(injector_t *injector, void *handle, const char* name, size_t *func_addr_out);

/*!
 * \brief Call the function in the target process (Linux only)
 * \param[in]   injector  the injector handle specifying the target process
 * \param[out]  retval    \c NULL or the address where the return value of the function call will be stored
 * \param[in]   func_addr the function address in the target process
 * \param[in]   ...       arguments passed to the function
 * \return                zero on success. Otherwise, error code
 * \remarks
 *   The types of the arguments must be integer or pointer.
 *   If it is a pointer, it must point to a valid address in the target process.
 *   The number of arguments must be less than or equal to six.
 * \note
 *   If the function in the target process internally calls non-[async-signal-safe]((https://man7.org/linux/man-pages/man7/signal-safety.7.html))
 *   functions, it may stop the target process or cause unexpected behaviour.
 * \sa injector_remote_func_addr(), injector_remote_vcall()
 */
int injector_remote_call(injector_t *injector, intptr_t *retval, size_t func_addr, ...);

/*!
 * \brief Call the function in the target process (Linux only)
 * \param[in]   injector  the injector handle specifying the target process
 * \param[out]  retval    \c NULL or the address where the return value of the function call will be stored
 * \param[in]   func_addr the function address in the target process
 * \param[in]   ap        arguments passed to the function
 * \return                zero on success. Otherwise, error code
 * \remarks
 *   The types of the arguments must be integer or pointer.
 *   If it is a pointer, it must point to a valid address in the target process.
 *   The number of arguments must be less than or equal to six.
 * \note
 *   If the function in the target process internally calls non-[async-signal-safe]((https://man7.org/linux/man-pages/man7/signal-safety.7.html))
 *   functions, it may stop the target process or cause unexpected behaviour.
 * \sa injector_remote_func_addr(), injector_remote_call()
 */
int injector_remote_vcall(injector_t *injector, intptr_t *retval, size_t func_addr, va_list ap);
#endif

#if defined(INJECTOR_DOC) || defined(_WIN32)
/*!
 * \brief Same with \c injector_inject except the type of the \c path argument. (Windows only)
 * \param[in]   injector the injector handle specifying the target process
 * \param[in]   path     the path name of the shared library
 * \param[out]  handle   the address where the newly created module handle will be stored
 * \return               zero on success. Otherwise, error code
 */
int injector_inject_w(injector_t *injector, const wchar_t *path, void **handle);
#endif

#if defined(INJECTOR_DOC) || (defined(__linux__) && defined(__x86_64__))
#define INJECTOR_HAS_INJECT_IN_CLONED_THREAD 1 // feature test macro
/*!
 * \brief Inject the specified shared library into the target process by the \c clone system call. (Linux x86_64 only)
 * \param[in]   injector the injector handle specifying the target process
 * \param[in]   path     the path name of the shared library
 * \param[out]  handle   the address where the newly created module handle will be stored
 * \return               zero on success. Otherwise, error code
 *
 * This calls `dlopen()` in a thread created by \c [clone()](https://man7.org/linux/man-pages/man2/clone.2.html). Note that no wonder there are unexpected
 * pitfalls because some resources allocated in \c [pthread_create()](https://man7.org/linux/man-pages/man3/pthread_create.3.html) lack in the \c clone()-ed thread.
 * Use it at your own risk.
 */
int injector_inject_in_cloned_thread(injector_t *injector, const char *path, void **handle);
#endif

#if 0
{
#endif
#ifdef __cplusplus
}; /* extern "C" */
#endif

#endif
