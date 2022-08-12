#include "pch.h"
#include "GPUService.h"

#include "CoordHelper.h"
#include "Mandelbrot.hpp"

#include <vector>
#include <algorithm>
#include <iostream>

#include <boost/compute/types/struct.hpp>
#include <boost/compute/utility/source.hpp>
#include <boost/optional.hpp>

namespace
{
    constexpr auto calculate_palette_func_name = "calculate_palette";
    constexpr auto calculate_palette_source = BOOST_COMPUTE_STRINGIZE_SOURCE(
        kernel void calculate_palette(
            global float* range,
            global int* numOfRanges,
            global uchar4 * colours,
            global int* histogram,
            global int* ranges_size_and_max_iter,
            global uchar4 * palette)
    {
        size_t it = get_global_id(0);
        int ranges_size = ranges_size_and_max_iter[0];
        int max_iter = ranges_size_and_max_iter[1];
        uchar4 result;
        result.x = 37;
        result.y = 37;
        result.z = 37;

        if (it != max_iter)
        {
            int ran = 0;
            for (int i = 1; i < ranges_size; i++)
            {
                if (range[i] > it) { break; }
                ran = i;
            }
            double rangeTotal = numOfRanges[ran];
            int rangeStart = range[ran];

            uchar4 startColor = colours[ran];

            uchar4 endColor;
            if (ran + 1)
                endColor = colours[ran + 1];
            uchar4 colorDiff;
            colorDiff.x = endColor.x - startColor.x;
            colorDiff.y = endColor.y - startColor.y;
            colorDiff.z = endColor.z - startColor.z;

            int totalPixels = 0;
            for (int i = rangeStart; i <= it; i++) { totalPixels += histogram[i]; }

            result.x = startColor.x + ((colorDiff.x * (double)totalPixels) / rangeTotal);
            result.y = startColor.y + ((colorDiff.y * (double)totalPixels) / rangeTotal);
            result.z = startColor.z + ((colorDiff.z * (double)totalPixels) / rangeTotal);
        }
        palette[it] = result;
    });

    constexpr auto calculate_colours_func_name = "calculate_colours";

}

GPUService::GPUService() : m_cache(5)
{
    m_device = compute::system::default_device();
    m_context = compute::context(m_device);
    m_queue = compute::command_queue(m_context, m_device);
}

std::string GPUService::GetDevice()
{
    return m_device.name();
}

ColourImage GPUService::GenerateImage(const FractalParams& p)
{
    auto coords = CalculateCoordImage(p);

    auto result_data = CalculateDataImage(coords, p);

    std::vector<int> histogram(Mandelbrot::MAX_ITER + 1);
    DataImage img(p.width, p.heigth);
    for (size_t y = 0u; y < p.heigth; y++) {
        for (size_t x = 0u; x < p.width; x++) {
            *img.at(y, x) = result_data.first[y * p.width + x];
            histogram[result_data.first[y * p.width + x]]++;
        }
    }

    auto the_thing = CalculateColours(p, result_data.second, histogram);

    return the_thing;
}

compute::vector<DataCoord> GPUService::CalculateCoordImage(const FractalParams& p)
{
    using namespace std::chrono;
    auto s1 = high_resolution_clock::now();

    std::vector<ImageCoord> input;
    input.reserve(p.heigth * p.width);

    for (unsigned int y = 0u; y < p.heigth; y++) {
        for (unsigned int x = 0u; x < p.width; x++) {
            input.push_back({ x, y });
        }
    } 
    auto s2 = high_resolution_clock::now();

    compute::vector<ImageCoord> input_gpu(input.size(), m_context);
    compute::vector<DataCoord> res_gpu(input.size(), m_context);

    // transfer data from the host to the device
    compute::copy(
        input.begin(), input.end(), input_gpu.begin(), m_queue
    );
    auto s3 = high_resolution_clock::now();
    auto f = CoordHelper::GetImg2DataGPU(p);

    // calculate the square-root of each element in-place
    compute::transform(
        input_gpu.begin(),
        input_gpu.end(),
        res_gpu.begin(),
        f,
        m_queue
    );

    auto s4 = high_resolution_clock::now();
    std::cout << __func__ << " Setup took: " << duration_cast<milliseconds>(s2 - s1).count() << " ms\n";
    std::cout << __func__ << " Copy took: " << duration_cast<milliseconds>(s3 - s2).count() << " ms\n";
    std::cout << __func__ << " Transform took: " << duration_cast<milliseconds>(s4 - s3).count() << " ms\n";
    return res_gpu;
}

std::pair<std::vector<int>, compute::vector<int>> GPUService::CalculateDataImage(compute::vector<DataCoord>& input, const FractalParams& p)
{
    using namespace std::chrono;
    auto s1 = high_resolution_clock::now();
    compute::vector<int> res_data_gpu(input.size(), m_context);

    auto f = Mandelbrot::GetProcFuncGPU();

    // calculate the square-root of each element in-place
    compute::transform(
        input.begin(),
        input.end(),
        res_data_gpu.begin(),
        f,
        m_queue
    );

    std::vector<int> result_data(res_data_gpu.size());
    compute::copy(
        res_data_gpu.begin(), res_data_gpu.end(), result_data.begin(), m_queue
    );

    auto s2 = high_resolution_clock::now();
    std::cout << __func__ << " took: " << duration_cast<milliseconds>(s2 - s1).count() << " ms\n";
    return { std::move(result_data), std::move(res_data_gpu) };
}

ColourImage GPUService::CalculateColours(const FractalParams& p, const compute::vector<int>& data, const std::vector<int>& histogram)
{
    ColourImage image(p.width, p.heigth);

    std::vector<int> data_cpu(data.size());
    auto fut = compute::copy_async(data.begin(), data.end(), data_cpu.begin(), m_queue);
    auto palette_gpu = CalculatePalette(p, histogram);
    fut.wait();

    std::vector<boost::compute::uchar4_> palette_cpu(palette_gpu.size());
    compute::copy(palette_gpu.begin(), palette_gpu.end(), palette_cpu.begin(), m_queue);

    for (unsigned int y = 0u; y < p.heigth; y++)
    {
        for (unsigned int x = 0u; x < p.width; x++)
        {
            const auto& current = data_cpu[y * p.width + x];
            *image.at(y, x) = Colour(palette_cpu[current].x, palette_cpu[current].y, palette_cpu[current].z);
        }
    }
    return image;
}

compute::vector<boost::compute::uchar4_> GPUService::CalculatePalette(const FractalParams& p, const std::vector<int>& histogram_cpu)
{
    std::vector<float> range_cpu;
    std::vector<int> numOfRanges_cpu;
    std::vector<boost::compute::uchar4_> colours_cpu;
    range_cpu.reserve(p.colours.size());
    numOfRanges_cpu.reserve(p.colours.size());
    colours_cpu.reserve(p.colours.size());
    
    compute::vector<float> range_gpu(p.colours.size(), m_context);
    compute::vector<int> numOfRanges_gpu(p.colours.size(), m_context);
    compute::vector<boost::compute::uchar4_> colours_gpu(p.colours.size(), m_context);
    compute::vector<int> histogram_gpu(histogram_cpu.size(), m_context);

    auto fut_hist = compute::copy_async(histogram_cpu.begin(), histogram_cpu.end(), histogram_gpu.begin(), m_queue);

    for (auto& c : p.colours)
    {
        range_cpu.push_back(c.first * Mandelbrot::MAX_ITER);
        numOfRanges_cpu.push_back(0);
        colours_cpu.push_back({ c.second.r, c.second.g, c.second.b, 0 });
    }

    auto fut_range = compute::copy_async(range_cpu.begin(), range_cpu.end(), range_gpu.begin(), m_queue);
    auto fut_col = compute::copy_async(colours_cpu.begin(), colours_cpu.end(), colours_gpu.begin(), m_queue);

    int rangeIndex = 0;
    for (int i = 0; i < Mandelbrot::MAX_ITER; i++)
    {
        int pixels = histogram_cpu[i];

        if (i >= range_cpu[rangeIndex + 1]) { rangeIndex++; }

        numOfRanges_cpu[rangeIndex] = numOfRanges_cpu[rangeIndex] + pixels;
    }
    auto fut_num_rang = compute::copy_async(numOfRanges_cpu.begin(), numOfRanges_cpu.end(), numOfRanges_gpu.begin(), m_queue);

    boost::optional<compute::program> calculate_palette_program = m_cache.get(calculate_palette_func_name);
    if (!calculate_palette_program) {
        // create and build square program
        calculate_palette_program = compute::program::build_with_source(calculate_palette_source, m_context);
        m_cache.insert(calculate_palette_func_name, calculate_palette_program.get());
    }

    // create square kernel
    compute::kernel calculate_palette_kernel(calculate_palette_program.get(), calculate_palette_func_name);

    compute::vector<int> ranges_size_and_max_iter(m_context);
    ranges_size_and_max_iter.push_back(colours_gpu.size());
    ranges_size_and_max_iter.push_back(Mandelbrot::MAX_ITER);

    compute::vector<boost::compute::uchar4_> palette_gpu(Mandelbrot::MAX_ITER + 1, m_context);

    fut_col.wait();
    fut_range.wait();
    fut_num_rang.wait();
    fut_hist.wait();

    calculate_palette_kernel.set_arg(0, range_gpu.get_buffer());
    calculate_palette_kernel.set_arg(1, numOfRanges_gpu.get_buffer());
    calculate_palette_kernel.set_arg(2, colours_gpu.get_buffer());
    calculate_palette_kernel.set_arg(3, histogram_gpu.get_buffer());
    calculate_palette_kernel.set_arg(4, ranges_size_and_max_iter.get_buffer());
    calculate_palette_kernel.set_arg(5, palette_gpu.get_buffer());
    m_queue.enqueue_1d_range_kernel(calculate_palette_kernel, 0, Mandelbrot::MAX_ITER+1, 0);

    return palette_gpu;
}

std::vector<Colour> GPUService::CalculateImageColours(const FractalParams& p, const compute::vector<int>& data_gpu, const compute::vector<boost::compute::uchar4_>& palette_gpu)
{
    const auto image_size = p.width * p.heigth;
    compute::vector<boost::compute::uchar4_> image_gpu(image_size, m_context);

    boost::optional<compute::program> calculate_colours_program = m_cache.get(calculate_colours_func_name);
    if (!calculate_colours_program) {
        const char source[] = BOOST_COMPUTE_STRINGIZE_SOURCE(
            kernel void calculate_colours(
                global uchar4 * palette,
                global int* data,
                global uchar4* image)
        {
            size_t it = get_global_id(0);
            image[it] = palette[data[it]];
        });


        // create and build square program
        calculate_colours_program = compute::program::build_with_source(source, m_context);
        m_cache.insert(calculate_colours_func_name, calculate_colours_program.get());
    }

    // create square kernel
    compute::kernel calculate_colours_kernel(calculate_colours_program.get(), calculate_colours_func_name);
   
    calculate_colours_kernel.set_arg(0, palette_gpu.get_buffer());
    calculate_colours_kernel.set_arg(1, data_gpu.get_buffer());
    calculate_colours_kernel.set_arg(2, image_gpu.get_buffer());
    m_queue.enqueue_1d_range_kernel(calculate_colours_kernel, 0, image_size, 0);

    std::vector<boost::compute::uchar4_> image_cpu(image_size);
    std::vector<Colour> col_image_cpu;
    col_image_cpu.reserve(image_size);

    compute::copy(image_gpu.begin(), image_gpu.end(), image_cpu.begin(), m_queue);
    for (size_t i = 0; i < image_size; i++)
        col_image_cpu.emplace_back(image_cpu[i].x, image_cpu[i].y, image_cpu[i].z );

    return col_image_cpu;
}
