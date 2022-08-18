#include "pch.h"
#include "GPUService.h"

#include <vector>
#include <algorithm>
#include <iostream>
#include <thread>
#include <future>

#include <boost/compute/types/struct.hpp>
#include <boost/compute/utility/source.hpp>
#include <boost/optional.hpp>

BOOST_COMPUTE_ADAPT_STRUCT(Colour, Colour, (r, g, b))

namespace
{
    constexpr auto calculate_palette_func_name = "calculate_palette";
    const std::string calculate_palette_source =
compute::type_definition<Colour>() +
R"( kernel void calculate_palette(
            global float* range,
            global size_t* numOfRanges,
            global Colour* colours,
            global size_t* histogram,
            global size_t* ranges_size_and_max_iter,
            global Colour* palette)
    {
        size_t it = get_global_id(0);
        size_t ranges_size = ranges_size_and_max_iter[0];
        size_t max_iter = ranges_size_and_max_iter[1];
        Colour result;
        result.r = 37;
        result.g = 37;
        result.b = 37;

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

            Colour startColor = colours[ran];

            Colour endColor;
            if (ran + 1)
                endColor = colours[ran + 1];
            Colour colorDiff;
            colorDiff.r = endColor.r - startColor.r;
            colorDiff.g = endColor.g - startColor.g;
            colorDiff.b = endColor.b - startColor.b;

            size_t totalPixels = 0;
            for (int i = rangeStart; i <= it; i++) { totalPixels += histogram[i]; }

            result.r = startColor.r + ((colorDiff.r * (double)totalPixels) / rangeTotal);
            result.g = startColor.g + ((colorDiff.g * (double)totalPixels) / rangeTotal);
            result.b = startColor.b + ((colorDiff.b * (double)totalPixels) / rangeTotal);
        }
        palette[it] = result;
    };
)";
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

ColourImage GPUService::GenerateImage(const FractalParams& p, FractalAlgo* alg)
{
    m_alg = alg;
    auto coords = CalculateCoordImage(p);
    auto result_data = CalculateDataImage(coords, p);

    std::vector<size_t> histogram(m_alg->GetMaxIterations() + 1);
    for (size_t y = 0u; y < p.heigth; y++) {
        for (size_t x = 0u; x < p.width; x++) {
            histogram[result_data.first[y * p.width + x]]++;
        }
    }

    auto the_thing = CalculateColours(p, result_data.second, histogram);

    m_alg = nullptr;
    return the_thing;
}

compute::vector<DataCoord> GPUService::CalculateCoordImage(const FractalParams& p)
{
    using namespace std::chrono;
    const auto s1 = high_resolution_clock::now();

    std::vector<ImageCoord> input;
    input.reserve(p.heigth * p.width);

    for (size_t y = 0u; y < p.heigth; y++) {
        for (size_t x = 0u; x < p.width; x++) {
            input.push_back({ x, y });
        }
    } 
    const auto s2 = high_resolution_clock::now();

    compute::vector<ImageCoord> input_gpu(input.size(), m_context);
    compute::vector<DataCoord> res_gpu(input.size(), m_context);

    // transfer data from the host to the device
    compute::copy(
        input.begin(), input.end(), input_gpu.begin(), m_queue
    );
    const auto s3 = high_resolution_clock::now();
    const auto f = CoordHelper::GetImg2DataGPU(p);

    // calculate the square-root of each element in-place
    compute::transform(
        input_gpu.begin(),
        input_gpu.end(),
        res_gpu.begin(),
        f,
        m_queue
    );

    const auto s4 = high_resolution_clock::now();
    std::cout << __func__ << " Setup took: " << duration_cast<milliseconds>(s2 - s1).count() << " ms\n";
    std::cout << __func__ << " Copy took: " << duration_cast<milliseconds>(s3 - s2).count() << " ms\n";
    std::cout << __func__ << " Transform took: " << duration_cast<milliseconds>(s4 - s3).count() << " ms\n";
    return res_gpu;
}

std::pair<std::vector<unsigned int>, compute::vector<unsigned int>> GPUService::CalculateDataImage(compute::vector<DataCoord>& input, const FractalParams& p)
{
    using namespace std::chrono;
    auto s1 = high_resolution_clock::now();
    compute::vector<unsigned int> res_data_gpu(input.size(), m_context);

    auto f = m_alg->GetProcessCoordGPU();

    compute::transform(
        input.begin(),
        input.end(),
        res_data_gpu.begin(),
        f,
        m_queue
    );

    std::vector<unsigned int> result_data(res_data_gpu.size());
    compute::copy(
        res_data_gpu.begin(), res_data_gpu.end(), result_data.begin(), m_queue
    );

    auto s2 = high_resolution_clock::now();
    std::cout << __func__ << " took: " << duration_cast<milliseconds>(s2 - s1).count() << " ms\n";
    return { std::move(result_data), std::move(res_data_gpu) };
}

ColourImage GPUService::CalculateColours(const FractalParams& p, const compute::vector<unsigned int>& data, const std::vector<size_t>& histogram)
{
    ColourImage image(p.width, p.heigth);

    std::vector<unsigned int> data_cpu(data.size());
    auto fut = compute::copy_async(data.begin(), data.end(), data_cpu.begin(), m_queue);
    auto palette_gpu = CalculatePalette(p, histogram);
    fut.wait();

    std::vector<Colour> palette_cpu(palette_gpu.size());
    compute::copy(palette_gpu.begin(), palette_gpu.end(), palette_cpu.begin(), m_queue);
    std::vector<std::future<void>> futures;
    futures.reserve(p.heigth);

    for (size_t y = 0u; y < p.heigth; y++)
    {
        futures.push_back(
        std::async([&image, &data_cpu, &palette_cpu, y, width = p.width]() {
                for (size_t x = 0u; x < width; x++)
                {
                    const auto& current = data_cpu[y * width + x];
                    *image.at(y, x) = palette_cpu[current];
                }
            })
        );
    }
    for (const auto& f : futures)
        f.wait();

    return image;
}

compute::vector<Colour> GPUService::CalculatePalette(const FractalParams& p, const std::vector<size_t>& histogram_cpu)
{
    std::vector<float> range_cpu;
    std::vector<size_t> numOfRanges_cpu;
    std::vector<Colour> colours_cpu;
    range_cpu.reserve(p.colours.size());
    numOfRanges_cpu.reserve(p.colours.size());
    colours_cpu.reserve(p.colours.size());
    
    compute::vector<float> range_gpu(p.colours.size(), m_context);
    compute::vector<size_t> numOfRanges_gpu(p.colours.size(), m_context);
    compute::vector<Colour> colours_gpu(p.colours.size(), m_context);
    compute::vector<size_t> histogram_gpu(histogram_cpu.size(), m_context);

    auto fut_hist = compute::copy_async(histogram_cpu.begin(), histogram_cpu.end(), histogram_gpu.begin(), m_queue);

    for (auto& c : p.colours)
    {
        range_cpu.push_back(c.first * m_alg->GetMaxIterations());
        numOfRanges_cpu.push_back(0);
        colours_cpu.push_back(c.second);
    }

    auto fut_range = compute::copy_async(range_cpu.begin(), range_cpu.end(), range_gpu.begin(), m_queue);
    auto fut_col = compute::copy_async(colours_cpu.begin(), colours_cpu.end(), colours_gpu.begin(), m_queue);

    int rangeIndex = 0;
    for (unsigned int i = 0u; i < m_alg->GetMaxIterations(); i++)
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

    compute::vector<size_t> ranges_size_and_max_iter(m_context);
    ranges_size_and_max_iter.push_back(colours_gpu.size());
    ranges_size_and_max_iter.push_back(m_alg->GetMaxIterations());

    compute::vector<Colour> palette_gpu(m_alg->GetMaxIterations() + 1, m_context);

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
    m_queue.enqueue_1d_range_kernel(calculate_palette_kernel, 0, m_alg->GetMaxIterations() +1, 0);

    return palette_gpu;
}

std::vector<Colour> GPUService::CalculateImageColours(const FractalParams& p, const compute::vector<unsigned int>& data_gpu, const compute::vector<Colour>& palette_gpu)
{
    const auto image_size = p.width * p.heigth;
    compute::vector<Colour> image_gpu(image_size, m_context);
    compute::kernel k;

    boost::optional<compute::program> calculate_colours_program = m_cache.get(calculate_colours_func_name);
    if (!calculate_colours_program) {
        const char source[] = BOOST_COMPUTE_STRINGIZE_SOURCE(
            kernel void calculate_colours(
                global Colour* palette,
                global unsigned int* data,
                global Colour* image)
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

    std::vector<Colour> image_cpu(image_size);
    compute::copy(image_gpu.begin(), image_gpu.end(), image_cpu.begin(), m_queue);

    return image_cpu;
}
