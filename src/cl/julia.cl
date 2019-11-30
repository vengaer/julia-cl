R"(
    unsigned char compute_color(unsigned i, unsigned iters, unsigned mult) {
        unsigned c = 255 * mult * i / iters;
        /* Wrap if too big */
        if(c > 255) {
            c = 255 - c;
        }
        return c;
    }

    float2 compute_components(unsigned idx, unsigned width, unsigned height) {
        float re, im;

        if(width > height) {
            float scale = 2.f / (float)height;
            /* Center horizontally */
            re = scale * (idx % width) - 1.f - scale * ((float)((width - height)/ 2.f));
            im = scale * (idx / width) - 1.f;
        }
        else {
            float scale = 2.f / (float)width;
            re = scale * (idx % width) - 1.f;
            /* Center vertically */
            im = scale * (idx / width) - 1.f  - scale * ((float)((height - width) / 2.f));
        }

        return (float2)( re, im );
    }

    __kernel void julia_solve(float c_re, float c_im, 
                              unsigned iters, unsigned width, unsigned height,
                              __global unsigned char *result) {
        unsigned idx = get_global_id(0);

        float2 coords = compute_components(idx, width, height);
        float re = coords.x;
        float im = coords.y;

        float re_tmp;
        unsigned i;
        for(i = 0; i < iters; i++) {
            re_tmp = re * re - im * im + c_re;
            im = 2 * re * im  + c_im;
            re = re_tmp;

            if(re * re + im * im > 4) {
                break;
            }
        }
        
        result[idx * 3]     = compute_color(i, iters, 1);
        result[idx * 3 + 1] = compute_color(i, iters, 4);
        result[idx * 3 + 2] = compute_color(i, iters, 16);
    }
)"
