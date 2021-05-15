#ifndef _MULTITHREADING_H
#define _MULTITHREADING_H

/*___________________________________________________________________________*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include <stddef.h>

#include "multithreading_defines.h"

/*___________________________________________________________________________*/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief This this type represents a thread entry point function
 * 
 * The only parameter (void*) is used to pass the thread context when the entry function is called
 * 
 * This function should never end !
 * 
 * TODO handle case if the function ends
 */
typedef void (*thread_entry_t)(void*);

/**
 * @brief This structure represents a thread, it defines:
 * - the stack pointer when the thread is suspended
 * - the thread priority (cooperative < 0, preemptive > 0), priority = 0 means the thread is disabled
 * - the stack location (end) and size (size)
 * - the local storage pointer
 * 
 * This structure is 9B long
 * 
 * Warning : Every definition of a "struct thread_t" variable must be stack
 */
struct thread_t
{
    void *sp;
    int8_t priority;
    struct
    {
        void *end;
        size_t size;
    } stack;
    void *local_storage;
};

/**
 * @brief This structure gathers the current threads informations
 * - current : this pointer refers to the current thread at any time
 * - list : gather all the pointers to all the threads structures
 * - count : current count of threads
 * - current_idx : index of the current thread in the list
 * 
 * This structure is 3B + 2B*THREAD_MAX long
 */
struct k_thread_meta
{
    struct thread_t *current;                  // used in multithreading.asm (when saving current thread context)
    struct thread_t *list[K_THREAD_MAX_COUNT]; // the first thread defined in the structure is the "main" thread
    uint8_t count;
    uint8_t current_idx;
};

/**
 * @brief This is the thread structure representing the main thread
 * 
 * @see struct thread_t
 */
extern struct thread_t k_thread_main;

/**
 * @brief This global variable contains all meta informations about current threads
 * 
 * @see struct k_thread_meta
 */
extern struct k_thread_meta k_thread;

/*___________________________________________________________________________*/

/**
 * @brief Define a new thread at runtime and initialize its stack
 * 
 * @param th thread structure pointer
 * @param entry thread entry function
 * @param stack thread stack start location
 * @param stack_size thread stack size
 * @param priority thread priority
 * @param context_p thread context passed to entry function
 * @param local_storage thread local_storage
 */
void k_thread_create(struct thread_t *const th, thread_entry_t entry, void *const stack_end, const size_t stack_size, const int8_t priority, void *const context_p, void *const local_storage);

/**
 * @brief Initialize a thread stack 
 * 
 * @param th thread structure pointer
 * @param entry thread entry function
 * @param stack thread stack start location
 * @param context_p thread context
 */
void _k_thread_stack_create(struct thread_t *const th, thread_entry_t entry, void *const stack, void *const context_p);

/**
 * @brief Register a thread that has been defined at compilation time
 * - thread structure must be entirely defined
 * - stack must be entirely defined
 * 
 * This function must be called only if the thread have been defined at compilation time
 * 
 * @param th thread structure pointer
 * 
 * @return int : 0 if success else (<0) error code
 */
int k_thread_register(struct thread_t *const th);

/*___________________________________________________________________________*/

/**
 * @brief Switch from current thread to {to} thread
 * 
 * {from} must necessary be the current thread
 * 
 * @param from : current thread
 * @param to : to thread
 */
void _k_thread_switch(struct thread_t *from, struct thread_t *to);

/**
 * @brief Choice the next thread to be executed
 * 
 * This function is called in k_yield function
 * 
 * @return struct thread_t* : next thread to be executed
 */
struct thread_t *_k_scheduler(void);

/**
 * @brief Release the CPU
 * 
 * This function must be called from a cooperative thread which is the current thread to tell the kernel that it is releasing the CPU
 * this function will call the scheduler (_k_scheduler) which will determine the next thread to be executed.
 * 
 * This function will return in the new main thread
 */
void k_yield(void);

// TODO
void cpu_idle(void);

/*___________________________________________________________________________*/

/**
 * @brief Get current thread local storage pointer
 * 
 * @return void* local storage pointer
 */
void * k_thread_local_storage(void);

/*___________________________________________________________________________*/

#ifdef __cplusplus
}
#endif

#endif