#define CL_TARGET_OPENCL_VERSION 110

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>

#include <CL/cl.hpp>

#include "main.h"

#define ERROR(cond, err_str) if(cond) {std::cerr << err_str << std::endl; exit(1);};

const char base58[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

std::string base58entry(long entry) {
    if(entry == 0)
        return "a";

    std::string result;
    while(entry != 0) {
        result.insert(0, 1, base58[entry % sizeof(base58)]);
        entry /= sizeof(base58);
    }
    return result;
}

typedef struct {
    unsigned int len;
    char data[16] = {0};
} input_t;

cl::Device device;
cl::Context context;
cl::Program program;
cl::CommandQueue queue;

bool executing = false;

constexpr int ELEMENTS = 63161283;

void initCL() {
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    ERROR(platforms.size() == 0, "No platforms found");

    std::vector<cl::Device> devices;
    platforms.front().getDevices(CL_DEVICE_TYPE_GPU, &devices);
    ERROR(devices.size() == 0, "No devices found");

    device = devices.front();

    std::ifstream file("kernel.cl");
    std::string src(std::istreambuf_iterator<char>(file), (std::istreambuf_iterator<char>()));

    cl::Program::Sources sources(1, std::make_pair(src.c_str(), src.length() + 1));
    context = cl::Context(device);
    program = cl::Program(context, sources);

    cl_int err = program.build("-O3");
    ERROR(err != 0, "Build error\n" + program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(device));
}

std::string runCL() {
    cl_int err;
    int i = 0;
    // long combinations = std::pow(sizeof(base58), base58entry(block.difficulty).size());

    input_t *inputs = new input_t[ELEMENTS];
    
    char hashcopy[65];
    block.hash.copy(hashcopy, 65, 0);

    executing = true;
    do {
        std::string tempid = "_" + std::to_string(block.id); // used for optimization
        for(int j = 0; j < ELEMENTS; i++, j++) {
            std::string id = base58entry(i) + tempid;
            id.copy(inputs[j].data, id.size(), 0);
            inputs[j].len = id.size();
        }

        cl::Buffer inputBuf(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, ELEMENTS*sizeof(input_t), inputs, &err);
        ERROR(err != 0, "Input buffer creation error");

        cl::Buffer hashBuf(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(hashcopy), hashcopy, &err);
        ERROR(err != 0, "Hash buffer creation error");

        int index = -1;
        cl::Buffer indexBuf(context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, sizeof(int), &index, &err);
        ERROR(err != 0, "Index buffer creation error");

        cl::Kernel kernel(program, "miner", &err);
        ERROR(err != 0, "Kernel generation error");

        kernel.setArg(0, inputBuf);
        kernel.setArg(1, hashBuf);
        kernel.setArg(2, indexBuf);

        queue = cl::CommandQueue(context, device);
        queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(ELEMENTS));
        queue.enqueueReadBuffer(indexBuf, CL_TRUE, 0, sizeof(index), &index);

        if(index != -1) {
            std::string result(inputs[index].data);
            delete[] inputs;
            return result;
        }
    } while(executing);
    return "";
}

void stopCL() {
    executing = false;
    queue.finish();
}