# Julia-CL

GPU-accelerated visualization of the quadratic [Julia set](https://en.wikipedia.org/wiki/Julia_set). Pixel values are computed using OpenCL and the result visualized using OpenGL. The input is gotten from a particle moving inside the circle $`Re(z)^2 + Im(z)^2 = 4`$ in the complex plane.

[![Build Status](https://gitlab.com/vilhelmengstrom/julia-cl/badges/master/build.svg)](https://gitlab.com/vilhelmengstrom/julia-cl/commits/master)

## Dependencies
- GNU C (for raw string support)
- GPU and driver supporting OpenCL 2.0 (proprietary drivers may be required)
- [OpenCL](https://www.khronos.org/opencl) along with an ICD loader implementation and headers (see [this](https://wiki.archlinux.org/index.php/GPGPU#OpenCL_Development) for more info)
- [glfw](https://www.glfw.org)
- [glew](http://glew.sourceforge.net)
- libx11

## Function and Input
The value of each pixel is computed using the recursive function
```math
z_{i+1} = z_i^2 + c(t), i = 0,1,2,...,N-1
```
for some arbitrary $`N`$ where $`c(t)`$ is a complex-valued, non-deterministic function of time. $`z_0`$ is simply the position of the pixel when mapping screen space to the complex plane (with the origin of the complex plane being the center of the window).

A pixel at position $`(Re(z), Im(z))`$ in the complex plane belongs to the set iff its distance to the origin after $`N-1`$ iterations is at most 2 units of length (i.e. in the disk $`Re(z)^2 + Im(z)^2 \leq 4`$). If at any iteration the distance is larger the 2, the pixel does not belong to the set.

The value of $`c(t)`$ is chosen as the position of a particle moving with constant velocity inside the set's boundary. All collisions with the edge of the circle are elastic. The direction of the particle after a collision is computed from its incident direction and the normal of the circle.
