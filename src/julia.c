#include "display.h"
#include "graphics.h"
#include "julia.h"

#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WG_SIZE 256

static char const *cl_source =
    #include "julia.cl"
        ;

static cl_device_id device;
static cl_context context;
static cl_program program;
static cl_kernel kernel;
static cl_command_queue queue;

static cl_mem result;

static bool create_device(void) {
    cl_platform_id platform;
    cl_int err = clGetPlatformIDs(1, &platform, NULL);

    if(err != CL_SUCCESS) {
        fputs("Failed to identify CL platform\n", stderr);
        return false;
    }

    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);

    if(err == CL_DEVICE_NOT_FOUND) {
        fputs("Failed to identify GPU, using CPU instead\n", stderr);
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL);
    }

    if(err != CL_SUCCESS) {
        fputs("Failed to identify device\n", stderr);
        return false;
    }

    return true;
}
static inline bool create_context(void) {
    cl_int err;

    context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    if(err != CL_SUCCESS) {
        fputs("Failed to create context\n", stderr);
        return false;
    }
    return true;
}

static bool build_program(void) {
    cl_int err;
    size_t src_size = strlen(cl_source);
    program = clCreateProgramWithSource(context, 1, &cl_source, &src_size, &err);
    if(err != CL_SUCCESS) {
        fputs("Failed to create program\n", stderr);
        return false;
    }
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);

    if(err != CL_SUCCESS) {
        size_t log_size;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

        char *buf = malloc(log_size + 1);
        buf[log_size] = 0;

        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size + 1, buf, NULL);
        printf("%s\n", buf);
        free(buf);

        return false;
    }

    return true;
}

static inline bool create_command_queue(void) {
    cl_int err;
    queue = clCreateCommandQueueWithProperties(context, device, NULL, &err);
    if(err != CL_SUCCESS) {
        fputs("Failed to create command queue\n", stderr);
        return false;
    }
    return true;
}

static bool setup_buffers(void) {
    cl_int err;
    uint32_t width, height;
    if(!display_get_resolution(&width, &height)) {
        fputs("Could not get screen resolution\n", stderr);
        return false;
    }

    result = clCreateBuffer(context, CL_MEM_WRITE_ONLY, width * height * 4, NULL, &err);
    if(err != CL_SUCCESS) {
        fputs("Failed to allocate im buffer\n", stderr);
        return false;
    }

    return true;
}

static bool create_kernel(void) {
    cl_int err;
    kernel = clCreateKernel(program, "julia_solve", &err);
    if(err != CL_SUCCESS) {
        fputs("Failed to create kernel\n", stderr);
        return false;
    }
    return true;
}

static bool set_initial_kernel_args(unsigned iters) {
    cl_float c = 0.f;
    cl_uint dim = 100;
    cl_uint is = iters;
    cl_int err = clSetKernelArg(kernel, 0, sizeof(cl_float), &c);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_float), &c);
    err |= clSetKernelArg(kernel, 2, sizeof(cl_uint), &is);
    err |= clSetKernelArg(kernel, 3, sizeof(cl_uint), &dim);
    err |= clSetKernelArg(kernel, 4, sizeof(cl_uint), &dim);
    err |= clSetKernelArg(kernel, 5, sizeof(cl_mem), &result);
    if(err != CL_SUCCESS) {
        fputs("Failed to set kernel args\n", stderr);
        return false;
    }
    return true;
}

bool julia_init(unsigned iters) {
    if(!create_device()) {
        return false;
    }
    if(!create_context()) {
        return false;
    }
    if(!build_program()) {
        clReleaseContext(context);
        return false;
    }
    if(!create_command_queue()) {
        clReleaseProgram(program);
        clReleaseContext(context);
        return false;
    }
    if(!setup_buffers()) {
        clReleaseCommandQueue(queue);
        clReleaseProgram(program);
        clReleaseContext(context);
        return false;
    }
    if(!create_kernel()) {
        clReleaseMemObject(result);
        clReleaseCommandQueue(queue);
        clReleaseProgram(program);
        clReleaseContext(context);
        return false;
    }
    if(!set_initial_kernel_args(iters)) {
        julia_cleanup();
        return false;
    }

    return true;
}

void julia_cleanup(void) {
    clReleaseKernel(kernel);
    clReleaseMemObject(result);
    clReleaseCommandQueue(queue);
    clReleaseProgram(program);
    clReleaseContext(context);
}
bool julia_update_dims(cl_uint width, cl_uint height) {
    cl_int err = clSetKernelArg(kernel, 3, sizeof(cl_uint), &width);
    err |= clSetKernelArg(kernel, 4, sizeof(cl_uint), &height);
    if(err != CL_SUCCESS) {
        fputs("Failed to update Julia dims\n", stderr);
        return false;
    }

    return true;
}

bool julia_update_constant(struct complexptf const *c) {
    cl_float f = c->re;
    cl_int err = clSetKernelArg(kernel, 0, sizeof(cl_float), &f);
    f = c->im;
    err |= clSetKernelArg(kernel, 1, sizeof(cl_float), &f);

    if(err != CL_SUCCESS) {
        fputs("Failed to update Julia constant\n", stderr);
        return false;
    }
    return true;
}

bool julia_run_kernel(unsigned char *out) {
    uint32_t width, height;
    gl_framebuffer_size(&width, &height);
    size_t local_size, global_size;

    local_size = WG_SIZE;
    global_size = ceil(width * height / (float)local_size) * local_size;
    cl_int err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);

    if(err != CL_SUCCESS) {
        fputs("Error while enqueueing kernel\n", stderr);
        return false;
    }

    clFinish(queue);
    
    clEnqueueReadBuffer(queue, result, CL_TRUE, 0, width * height * 4, out, 0, NULL, NULL);
    return true;
}
